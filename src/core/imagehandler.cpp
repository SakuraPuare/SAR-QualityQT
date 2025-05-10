#include "imagehandler.h"

#include <QFileInfo>
#include <QImage> // 需要用于 QPixmap 转换
#include <QDebug> // 用于调试日志（如果需要）
#include <QCoreApplication> // <--- 包含 QCoreApplication 用于 translate
#include <vector>
#include <cmath>

#include <opencv2/imgproc.hpp> // 包含 cv::split, cv::magnitude, cv::normalize, cv::minMaxLoc, cv::convertTo
#include <opencv2/core/types_c.h> // For cv::typeToString

#include <gdal_priv.h>   // 包含 GDAL 完整定义
#include <cpl_error.h>   // 包含 CPLGetLastErrorMsg
#include <gdalwarper.h> // 如果未来需要重投影或重采样

namespace SAR {
namespace Core {

ImageHandler::ImageHandler()
    : m_dataset(nullptr)
{
    initGDAL();
}

ImageHandler::~ImageHandler()
{
    if (m_dataset) {
        GDALClose(m_dataset);
        m_dataset = nullptr;
    }
}

void ImageHandler::initGDAL()
{
    // 初始化GDAL
    static bool initialized = false;
    if (!initialized) {
        GDALAllRegister();
        initialized = true;
    }
}

bool ImageHandler::loadImage(const QString& filePath)
{
    // 关闭之前打开的图像
    if (m_dataset) {
        GDALClose(m_dataset);
        m_dataset = nullptr;
    }
    
    m_image = cv::Mat();
    m_filePath.clear();
    
    // 尝试使用GDAL加载
    if (loadWithGDAL(filePath)) {
        m_filePath = filePath;
        return true;
    }
    
    // 尝试使用OpenCV加载
    if (loadWithOpenCV(filePath)) {
        m_filePath = filePath;
        return true;
    }
    
    return false;
}

bool ImageHandler::loadWithGDAL(const QString& filePath)
{
    m_dataset = (GDALDataset*)GDALOpen(filePath.toUtf8().constData(), GA_ReadOnly);
    if (!m_dataset) {
        return false;
    }
    
    // 获取图像基本信息
    int width = m_dataset->GetRasterXSize();
    int height = m_dataset->GetRasterYSize();
    int bands = m_dataset->GetRasterCount();
    
    if (width <= 0 || height <= 0 || bands <= 0) {
        GDALClose(m_dataset);
        m_dataset = nullptr;
        return false;
    }
    
    // 确定OpenCV数据类型
    GDALDataType dataType = m_dataset->GetRasterBand(1)->GetRasterDataType();
    int cvType = CV_8U;
    
    switch (dataType) {
        case GDT_Byte:
            cvType = CV_8U;
            break;
        case GDT_UInt16:
        case GDT_Int16:
            cvType = CV_16U;
            break;
        case GDT_UInt32:
        case GDT_Int32:
            cvType = CV_32S;
            break;
        case GDT_Float32:
            cvType = CV_32F;
            break;
        case GDT_Float64:
            cvType = CV_64F;
            break;
        default:
            // 不支持的类型
            GDALClose(m_dataset);
            m_dataset = nullptr;
            return false;
    }
    
    // 创建OpenCV矩阵
    m_image.create(height, width, CV_MAKETYPE(cvType, bands));
    
    // 读取数据
    for (int b = 0; b < bands; b++) {
        GDALRasterBand* band = m_dataset->GetRasterBand(b + 1);
        if (band) {
            // 计算读取的数据大小
            size_t pixelSize = GDALGetDataTypeSize(dataType) / 8;
            size_t rowSize = width * pixelSize;
            
            // 读取数据
            for (int row = 0; row < height; row++) {
                void* rowBuffer = m_image.ptr(row) + b * pixelSize;
                CPLErr error = band->RasterIO(GF_Read, 0, row, width, 1, 
                                              rowBuffer, width, 1, 
                                              dataType, 0, 0);
                if (error != CE_None) {
                    // 读取失败
                    m_image = cv::Mat();
                    GDALClose(m_dataset);
                    m_dataset = nullptr;
                    return false;
                }
            }
        }
    }
    
    return true;
}

bool ImageHandler::loadWithOpenCV(const QString& filePath)
{
    m_image = cv::imread(filePath.toStdString(), cv::IMREAD_UNCHANGED);
    return !m_image.empty();
}

cv::Mat ImageHandler::getImage() const
{
    return m_image;
}

QImage ImageHandler::cvMatToQImage(const cv::Mat& cvImage)
{
    if (cvImage.empty()) {
        return QImage();
    }
    
    QImage qImage;
    
    switch (cvImage.type()) {
        case CV_8UC1:
            // 单通道8位图像
            qImage = QImage(cvImage.data, cvImage.cols, cvImage.rows, 
                           cvImage.step, QImage::Format_Grayscale8);
            break;
            
        case CV_8UC3:
            // BGR格式的3通道8位图像
            qImage = QImage(cvImage.data, cvImage.cols, cvImage.rows, 
                           cvImage.step, QImage::Format_RGB888).rgbSwapped();
            break;
            
        case CV_8UC4:
            // BGRA格式的4通道8位图像
            qImage = QImage(cvImage.data, cvImage.cols, cvImage.rows, 
                           cvImage.step, QImage::Format_ARGB32).rgbSwapped();
            break;
            
        default:
            // 其他格式：转换为8位三通道
            cv::Mat temp;
            if (cvImage.channels() == 1) {
                cv::cvtColor(cvImage, temp, cv::COLOR_GRAY2BGR);
            } else {
                cvImage.convertTo(temp, CV_8U);
            }
            
            if (temp.channels() == 3) {
                qImage = QImage(temp.data, temp.cols, temp.rows, 
                               temp.step, QImage::Format_RGB888).rgbSwapped();
            } else if (temp.channels() == 4) {
                qImage = QImage(temp.data, temp.cols, temp.rows, 
                               temp.step, QImage::Format_ARGB32).rgbSwapped();
            }
            
            break;
    }
    
    return qImage.copy(); // 创建深拷贝，避免原始数据释放导致问题
}

cv::Mat ImageHandler::extractROI(const cv::Rect& rect) const
{
    if (m_image.empty() || 
        rect.x < 0 || rect.y < 0 || 
        rect.width <= 0 || rect.height <= 0 ||
        rect.x + rect.width > m_image.cols || 
        rect.y + rect.height > m_image.rows) {
        return cv::Mat();
    }
    
    return m_image(rect).clone();
}

QString ImageHandler::getMetadata() const
{
    if (!m_dataset) {
        return QString();
    }
    
    QString metadata;
    metadata += QString("文件名: %1\n").arg(QFileInfo(m_filePath).fileName());
    metadata += QString("维度: %1 x %2\n").arg(m_image.cols).arg(m_image.rows);
    metadata += QString("通道数: %1\n").arg(m_image.channels());
    
    // 获取GDAL元数据
    char** mdList = m_dataset->GetMetadata();
    if (mdList != nullptr) {
        metadata += "\nGDAL元数据:\n";
        for (int i = 0; mdList[i] != nullptr; i++) {
            metadata += QString("%1\n").arg(mdList[i]);
        }
    }
    
    return metadata;
}

bool ImageHandler::saveImage(const QString& filePath, const cv::Mat& image)
{
    cv::Mat saveImage = image.empty() ? m_image : image;
    
    if (saveImage.empty()) {
        return false;
    }
    
    return cv::imwrite(filePath.toStdString(), saveImage);
}

} // namespace Core
} // namespace SAR 