#include "imagehandler.h"

#include <QFileInfo>
#include <QImage> // 需要用于 QPixmap 转换
#include <QDebug> // 用于调试日志（如果需要）
#include <QCoreApplication> // <--- 包含 QCoreApplication 用于 translate
#include <vector>
#include <cmath>
#include <algorithm> // 用于std::sort, std::nth_element

#include <opencv2/imgproc.hpp> // 包含 cv::split, cv::magnitude, cv::normalize, cv::minMaxLoc, cv::convertTo
#include <opencv2/core/types_c.h> // For cv::typeToString

#include <gdal_priv.h>   // 包含 GDAL 完整定义
#include <cpl_error.h>   // 包含 CPLGetLastErrorMsg
#include <gdalwarper.h> // 如果未来需要重投影或重采样

#include "imagefilters.h" // 包含滤波器实现

namespace SAR {
namespace Core {

ImageHandler::ImageHandler(std::function<void(const QString &)> logger)
    : m_logger(logger) {}

ImageHandler::~ImageHandler() {
    // 确保在析构时关闭图像，防止资源泄漏
    closeImage();
}

void ImageHandler::setLogger(std::function<void(const QString &)> logger) {
    m_logger = logger;
}

void ImageHandler::log(const QString &message) const {
    if (m_logger) {
        m_logger(message);
    } else {
        // 如果没有设置日志记录器，可以考虑输出到 qDebug 或不记录
        // qDebug() << message;
    }
}

void ImageHandler::closeImage() {
    if (poDataset != nullptr) {
        QString filename_copy = currentFilename;
        GDALClose(poDataset);
        poDataset = nullptr;
        // 使用 QCoreApplication::translate
        log(QCoreApplication::translate("ImageHandler", "Closed GDAL dataset: %1").arg(filename_copy));
    }
    if (!currentImage.empty()) {
        currentImage.release();
        // 使用 QCoreApplication::translate
        log(QCoreApplication::translate("ImageHandler", "Released cv::Mat memory."));
    }
    currentFilename.clear();
    isComplex = false;
    // 注意：这里不重置 UI 元素，因为 ImageHandler 不应该知道 UI
}

bool ImageHandler::loadImage(const QString &filePath) {
    if (filePath.isEmpty()) {
        // 使用 QCoreApplication::translate
        log(QCoreApplication::translate("ImageHandler", "Error: Invalid (empty) file path provided to loadImage."));
        return false;
    }

    // 使用 QCoreApplication::translate
    log(QCoreApplication::translate("ImageHandler", "ImageHandler: Attempting to load image: %1").arg(filePath));

    closeImage(); // Close previous before opening new

    poDataset = static_cast<GDALDataset *>(GDALOpen(filePath.toUtf8().constData(), GA_ReadOnly));

    if (poDataset == nullptr) {
        QString gdalErrorMsg = CPLGetLastErrorMsg();
        // 使用 QCoreApplication::translate
        log(QCoreApplication::translate("ImageHandler", "Error: Could not open image file '%1' with GDAL. Error: %2")
                .arg(filePath, gdalErrorMsg));
        // 注意：这里不显示 QMessageBox，只记录日志
        return false;
    }

    currentFilename = QFileInfo(filePath).fileName();
    // 使用 QCoreApplication::translate
    log(QCoreApplication::translate("ImageHandler", "ImageHandler: Successfully opened dataset: %1").arg(currentFilename));

    int width = poDataset->GetRasterXSize();
    int height = poDataset->GetRasterYSize();
    int numBands = poDataset->GetRasterCount();

    if (width <= 0 || height <= 0 || numBands < 1) {
        // 使用 QCoreApplication::translate
        log(QCoreApplication::translate("ImageHandler", "Error: Image has invalid dimensions (%1x%2) or zero bands (%3).")
                .arg(width)
                .arg(height)
                .arg(numBands));
        closeImage(); // 清理已打开的 GDAL 数据集
        return false;
    }

    // 仅处理第一个波段
    GDALRasterBand *poBand = poDataset->GetRasterBand(1);
    if (poBand == nullptr) {
        // 使用 QCoreApplication::translate
        log(QCoreApplication::translate("ImageHandler", "Error: Could not access raster band 1."));
        closeImage();
        return false;
    }

    GDALDataType dataType = poBand->GetRasterDataType();
    // 使用 QCoreApplication::translate
    log(QCoreApplication::translate("ImageHandler", "ImageHandler: Properties: %1x%2 pixels, %3 bands, Data type: %4")
            .arg(width)
            .arg(height)
            .arg(numBands)
            .arg(GDALGetDataTypeName(dataType)));

    int cvType = -1;
    isComplex = GDALDataTypeIsComplex(dataType); // 更新成员变量

    // GDAL 类型到 OpenCV 类型的映射 (与 MainWindow 中类似)
    switch (dataType) {
        case GDT_Byte:    cvType = CV_8U; break;
        case GDT_UInt16:  cvType = CV_16U; break;
        case GDT_Int16:   cvType = CV_16S; break;
        case GDT_UInt32:  cvType = CV_32S; break; // Map to CV_32S
        case GDT_Int32:   cvType = CV_32S; break;
        case GDT_Float32: cvType = CV_32F; break;
        case GDT_Float64: cvType = CV_64F; break;
        case GDT_CInt16:   cvType = CV_16S; break;
        case GDT_CInt32:   cvType = CV_32S; break;
        case GDT_CFloat32: cvType = CV_32F; break;
        case GDT_CFloat64: cvType = CV_64F; break;
        default:
            // 使用 QCoreApplication::translate
            log(QCoreApplication::translate("ImageHandler", "Error: Unsupported GDAL data type for reading: %1")
                    .arg(GDALGetDataTypeName(dataType)));
            closeImage();
            return false;
    }

    // 分配 OpenCV Mat 内存
    int cvChannels = isComplex ? 2 : 1;
    currentImage.create(height, width, CV_MAKETYPE(cvType, cvChannels));

    // 使用 RasterIO 读取数据
    CPLErr err = poBand->RasterIO(
        GF_Read, 0, 0, width, height,
        currentImage.ptr(), width, height,
        dataType,
        isComplex ? GDALGetDataTypeSizeBytes(dataType) : 0, // 像素间距
        isComplex ? GDALGetDataTypeSizeBytes(dataType) * width : 0 // 行间距
    );

    if (err != CE_None) {
        // 使用 QCoreApplication::translate
        log(QCoreApplication::translate("ImageHandler", "Error reading raster data using RasterIO. GDAL Error: %1")
                .arg(CPLGetLastErrorMsg()));
        closeImage();
        return false;
    }

    std::string typeStr = "Unknown";
    try {
        typeStr = cv::typeToString(currentImage.type());
    } catch (...) {
        log(QCoreApplication::translate("ImageHandler", "Exception getting type string for cv::Mat"));
    }

    // 使用 QCoreApplication::translate
    log(QCoreApplication::translate("ImageHandler", "ImageHandler: Image data successfully read into cv::Mat. Type: %1, Channels: %2")
            .arg(QString::fromStdString(typeStr))
            .arg(currentImage.channels()));

    // 加载成功
    return true;
}

bool ImageHandler::isValid() const {
    // 有效条件：GDAL 数据集已打开且 OpenCV Mat 不为空
    return poDataset != nullptr && !currentImage.empty();
}

QString ImageHandler::getFilename() const {
    return currentFilename;
}

QString ImageHandler::getDimensionsString() const {
    if (poDataset) {
        return QString("%1 x %2")
            .arg(poDataset->GetRasterXSize())
            .arg(poDataset->GetRasterYSize());
    }
    // 返回需要翻译的字符串
    return QCoreApplication::translate("ImageHandler", "N/A");
}

QString ImageHandler::getDataTypeString() const {
    if (poDataset) {
        GDALRasterBand *poBand = poDataset->GetRasterBand(1);
        if (poBand) {
            return GDALGetDataTypeName(poBand->GetRasterDataType());
        } else {
             log(QCoreApplication::translate("ImageHandler", "Warning: Could not get band 1 to retrieve data type string."));
             return QCoreApplication::translate("ImageHandler", "Error");
        }
    }
    return QCoreApplication::translate("ImageHandler", "N/A");
}

const cv::Mat &ImageHandler::getImageData() const {
    // 返回对内部 Mat 的常量引用
    // 调用者不应修改返回的 Mat
    return currentImage;
}

// --- SAR图像高动态范围处理方法实现 ---

void ImageHandler::setDisplayMode(ImageDisplayMode mode) {
    m_displayMode = mode;
    log(QCoreApplication::translate("ImageHandler", "图像显示模式已设置为: %1")
        .arg([this]() {
            switch (m_displayMode) {
                case ImageDisplayMode::Linear: return QCoreApplication::translate("ImageHandler", "线性缩放");
                case ImageDisplayMode::Logarithmic: return QCoreApplication::translate("ImageHandler", "对数缩放");
                case ImageDisplayMode::Sqrt: return QCoreApplication::translate("ImageHandler", "平方根缩放");
                case ImageDisplayMode::ClipPercent: return QCoreApplication::translate("ImageHandler", "百分比裁剪(%1%, %2%)")
                                            .arg(m_lowerClipPercent).arg(m_upperClipPercent);
                default: return QCoreApplication::translate("ImageHandler", "未知");
            }
        }()));
}

ImageDisplayMode ImageHandler::getDisplayMode() const {
    return m_displayMode;
}

void ImageHandler::setClipPercentile(double lower, double upper) {
    // 验证参数
    if (lower < 0) lower = 0;
    if (upper > 100) upper = 100;
    if (lower >= upper) {
        log(QCoreApplication::translate("ImageHandler", "警告: 裁剪百分比参数无效 (lower >= upper)，使用默认值"));
        lower = 1.0;
        upper = 99.0;
    }
    
    m_lowerClipPercent = lower;
    m_upperClipPercent = upper;
    
    log(QCoreApplication::translate("ImageHandler", "裁剪百分比已设置为: 下限=%1%, 上限=%2%")
        .arg(m_lowerClipPercent).arg(m_upperClipPercent));
}

QPair<double, double> ImageHandler::getClipPercentile() const {
    return qMakePair(m_lowerClipPercent, m_upperClipPercent);
}

bool ImageHandler::autoEnhance() {
    if (!isValid()) {
        log(QCoreApplication::translate("ImageHandler", "错误: 图像无效，无法执行自动增强"));
        return false;
    }
    
    // 检测图像特性，选择最佳显示模式
    cv::Scalar mean, stddev;
    cv::meanStdDev(currentImage, mean, stddev);
    double minVal, maxVal;
    cv::minMaxLoc(currentImage, &minVal, &maxVal);
    
    // 判断图像动态范围
    double dynamicRange = maxVal - minVal;
    double meanValue = mean[0];
    double stdValue = stddev[0];
    double cvValue = stdValue / meanValue; // 变异系数
    
    log(QCoreApplication::translate("ImageHandler", "图像统计: 最小值=%1, 最大值=%2, 均值=%3, 标准差=%4")
        .arg(minVal).arg(maxVal).arg(meanValue).arg(stdValue));
    
    // 基于图像特性自动选择显示模式
    if (maxVal > 1000 || dynamicRange > 1000) {
        // 高动态范围图像，使用对数缩放
        setDisplayMode(ImageDisplayMode::Logarithmic);
        log(QCoreApplication::translate("ImageHandler", "检测到高动态范围SAR图像，使用对数缩放"));
    } else if (cvValue > 2.0) {
        // 高变异系数，使用平方根缩放
        setDisplayMode(ImageDisplayMode::Sqrt);
        log(QCoreApplication::translate("ImageHandler", "检测到高变异系数图像，使用平方根缩放"));
    } else if (maxVal - minVal > 255) {
        // 中等动态范围，使用百分比裁剪
        setDisplayMode(ImageDisplayMode::ClipPercent);
        setClipPercentile(1.0, 99.0);
        log(QCoreApplication::translate("ImageHandler", "检测到中等动态范围图像，使用百分比裁剪"));
    } else {
        // 标准图像，使用线性缩放
        setDisplayMode(ImageDisplayMode::Linear);
        log(QCoreApplication::translate("ImageHandler", "检测到标准范围图像，使用线性缩放"));
    }
    
    return true;
}

// 修改prepareDisplayMat方法以使用新的显示模式

cv::Mat ImageHandler::prepareDisplayMat() const {
    if (!isValid()) {
        log(QCoreApplication::translate("ImageHandler", "Error: prepareDisplayMat called when image is not valid."));
        return cv::Mat(); // 返回空 Mat
    }

    cv::Mat sourceMat;
    if (isComplex) {
        // 计算幅度图
        log(QCoreApplication::translate("ImageHandler", "ImageHandler: Calculating magnitude from complex data for display."));
        std::vector<cv::Mat> channels;
        cv::split(currentImage, channels);
        // 转换为浮点数进行幅度计算，如果需要
        if (channels[0].depth() != CV_32F && channels[0].depth() != CV_64F) {
            log(QCoreApplication::translate("ImageHandler", "ImageHandler: Converting complex channels to CV_32F for magnitude calculation."));
            channels[0].convertTo(channels[0], CV_32F);
            channels[1].convertTo(channels[1], CV_32F);
        }
        cv::magnitude(channels[0], channels[1], sourceMat);
    } else {
        // 直接使用单通道实数图像
        sourceMat = currentImage;
    }

    // 将图像转换为浮点类型进行处理
    cv::Mat floatImage;
    if (sourceMat.depth() != CV_32F) {
        sourceMat.convertTo(floatImage, CV_32F);
    } else {
        floatImage = sourceMat.clone();
    }
    
    // 应用选定的显示模式
    cv::Mat enhancedImage;
    switch (m_displayMode) {
        case ImageDisplayMode::Logarithmic:
            enhancedImage = applyLogScaling(floatImage);
            break;
        case ImageDisplayMode::Sqrt:
            enhancedImage = applySqrtScaling(floatImage);
            break;
        case ImageDisplayMode::ClipPercent:
            enhancedImage = applyClipPercentScaling(floatImage);
            break;
        case ImageDisplayMode::Linear:
        default:
            enhancedImage = applyLinearScaling(floatImage);
            break;
    }
    
    // 归一化到 8 位灰度图 (0-255)
    cv::Mat displayMat;
    cv::normalize(enhancedImage, displayMat, 0, 255, cv::NORM_MINMAX, CV_8U);

    // 确保结果是 CV_8UC1
    if (displayMat.type() != CV_8UC1) {
        log(QCoreApplication::translate("ImageHandler", "Error: prepareDisplayMat resulted in unexpected type: %1").arg(cv::typeToString(displayMat.type())));
        // 尝试转换或返回空 Mat
        if (displayMat.channels() > 1) {
            cv::cvtColor(displayMat, displayMat, cv::COLOR_BGR2GRAY); // 假设 BGR，如果不是会出错
        }
        if (displayMat.type() != CV_8UC1) {
             return cv::Mat(); // 转换失败，返回空 Mat
        }
    }

    return displayMat;
}

// 实现不同的图像缩放方法

cv::Mat ImageHandler::applyLinearScaling(const cv::Mat& image) const {
    // 线性缩放就是直接使用原图像
    return image.clone();
}

cv::Mat ImageHandler::applyLogScaling(const cv::Mat& image) const {
    cv::Mat result;
    double minVal, maxVal;
    cv::minMaxLoc(image, &minVal, &maxVal);
    
    // 避免对小于或等于零的值取对数
    double offset = 0;
    if (minVal <= 0) {
        offset = 1.0 - minVal; // 确保最小值至少为1.0
    }
    
    // 应用对数变换: log(1 + (image - minVal))
    cv::log(image + offset, result);
    return result;
}

cv::Mat ImageHandler::applySqrtScaling(const cv::Mat& image) const {
    cv::Mat result;
    double minVal, maxVal;
    cv::minMaxLoc(image, &minVal, &maxVal);
    
    // 避免对负值取平方根
    double offset = 0;
    if (minVal < 0) {
        offset = -minVal; // 确保最小值至少为0
    }
    
    // 应用平方根变换: sqrt(image - minVal)
    cv::sqrt(image + offset, result);
    return result;
}

cv::Mat ImageHandler::applyClipPercentScaling(const cv::Mat& image) const {
    // 获取图像尺寸
    int width = image.cols;
    int height = image.rows;
    int totalPixels = width * height;
    
    // 将图像数据提取到向量中以计算百分位数
    std::vector<float> pixelValues(totalPixels);
    if (image.isContinuous()) {
        // 如果图像数据是连续的，可以直接复制
        std::memcpy(pixelValues.data(), image.ptr<float>(), totalPixels * sizeof(float));
    } else {
        // 如果不连续，需要逐行复制
        int idx = 0;
        for (int i = 0; i < height; i++) {
            const float* row = image.ptr<float>(i);
            for (int j = 0; j < width; j++) {
                pixelValues[idx++] = row[j];
            }
        }
    }
    
    // 确定下限和上限索引
    int lowerIdx = std::max(0, static_cast<int>(totalPixels * m_lowerClipPercent / 100.0));
    int upperIdx = std::min(totalPixels - 1, static_cast<int>(totalPixels * m_upperClipPercent / 100.0));
    
    if (lowerIdx >= upperIdx) {
        // 安全检查
        lowerIdx = 0;
        upperIdx = totalPixels - 1;
    }
    
    // 使用部分排序找到百分位数值
    std::nth_element(pixelValues.begin(), pixelValues.begin() + lowerIdx, pixelValues.end());
    float lowerVal = pixelValues[lowerIdx];
    
    std::nth_element(pixelValues.begin() + lowerIdx, pixelValues.begin() + upperIdx, pixelValues.end());
    float upperVal = pixelValues[upperIdx];
    
    // 创建结果图像
    cv::Mat result;
    
    // 对原图像进行裁剪和缩放
    result = image.clone();
    for (int i = 0; i < height; i++) {
        float* row = result.ptr<float>(i);
        for (int j = 0; j < width; j++) {
            // 裁剪值到指定范围
            row[j] = std::max(lowerVal, std::min(upperVal, row[j]));
            // 缩放到 [0, 1] 范围
            row[j] = (row[j] - lowerVal) / (upperVal - lowerVal);
        }
    }
    
    return result;
}

QPixmap ImageHandler::getDisplayPixmap(const QSize &targetSize) const {
    if (!isValid()) {
        // 使用 QCoreApplication::translate
        log(QCoreApplication::translate("ImageHandler", "Error: getDisplayPixmap called when image is not valid."));
        return QPixmap(); // 返回空 QPixmap
    }
    
    // 准备用于显示的 Mat
    cv::Mat displayMat = prepareDisplayMat();
    if (displayMat.empty()) {
        log(QCoreApplication::translate("ImageHandler", "Error: Failed to prepare display Mat."));
        return QPixmap();
    }
    
    // 将 OpenCV 的 Mat 转换为 QImage
    QImage image;
    if (displayMat.channels() == 1) {
        // 单通道（灰度图）
        image = QImage(displayMat.data, displayMat.cols, displayMat.rows, 
                       static_cast<int>(displayMat.step), QImage::Format_Grayscale8);
    } else if (displayMat.channels() == 3) {
        // 三通道（彩色图）- 需要将 BGR 转为 RGB
        cv::Mat rgbMat;
        cv::cvtColor(displayMat, rgbMat, cv::COLOR_BGR2RGB);
        image = QImage(rgbMat.data, rgbMat.cols, rgbMat.rows, 
                      static_cast<int>(rgbMat.step), QImage::Format_RGB888);
    } else {
        log(QCoreApplication::translate("ImageHandler", "Error: Unsupported number of channels for conversion to QImage."));
        return QPixmap();
    }
    
    // 创建一个副本，避免在 displayMat 释放后出现问题
    QImage imageCopy = image.copy();
    
    // 缩放图像（如果需要）
    if (!targetSize.isEmpty() && targetSize.isValid() && 
        (imageCopy.width() > targetSize.width() || imageCopy.height() > targetSize.height())) {
        imageCopy = imageCopy.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    
    // 转换为 QPixmap
    return QPixmap::fromImage(imageCopy);
}

cv::Mat ImageHandler::applyFilter(const FilterParameters& params, QString* logStr) const {
    if (!isValid()) {
        if (logStr) {
            *logStr += QCoreApplication::translate("ImageHandler", "错误：图像未加载或无效，无法应用滤波器");
        }
        if (m_logger) {
            m_logger(QCoreApplication::translate("ImageHandler", "错误：图像未加载或无效，无法应用滤波器"));
        }
        return cv::Mat();
    }

    // 调用滤波器实现
    cv::Mat filteredImage = ImageFilters::applyFilter(currentImage, params, logStr);
    
    if (filteredImage.empty()) {
        if (m_logger) {
            m_logger(QCoreApplication::translate("ImageHandler", "错误：滤波处理失败"));
        }
    } else {
        if (m_logger) {
            m_logger(QCoreApplication::translate("ImageHandler", "成功应用 %1 滤波器").arg(ImageFilters::getFilterTypeDescription(params.type)));
        }
    }
    
    return filteredImage;
}

bool ImageHandler::applyFilterInplace(const FilterParameters& params, QString* logStr) {
    if (!isValid()) {
        if (logStr) {
            *logStr += QCoreApplication::translate("ImageHandler", "错误：图像未加载或无效，无法应用滤波器");
        }
        if (m_logger) {
            m_logger(QCoreApplication::translate("ImageHandler", "错误：图像未加载或无效，无法应用滤波器"));
        }
        return false;
    }
    
    // 调用滤波器实现
    cv::Mat filteredImage = ImageFilters::applyFilter(currentImage, params, logStr);
    
    if (filteredImage.empty()) {
        if (m_logger) {
            m_logger(QCoreApplication::translate("ImageHandler", "错误：滤波处理失败"));
        }
        return false;
    }
    
    // 更新当前图像
    currentImage = filteredImage;
    
    if (m_logger) {
        m_logger(QCoreApplication::translate("ImageHandler", "成功应用 %1 滤波器并更新当前图像").arg(ImageFilters::getFilterTypeDescription(params.type)));
    }
    return true;
}

QStringList ImageHandler::getAvailableFilterTypes() {
    QStringList filterTypes;
    
    // 添加所有可用的滤波器类型
    filterTypes << ImageFilters::getFilterTypeDescription(FilterType::LowPass);
    filterTypes << ImageFilters::getFilterTypeDescription(FilterType::HighPass);
    filterTypes << ImageFilters::getFilterTypeDescription(FilterType::BandPass);
    filterTypes << ImageFilters::getFilterTypeDescription(FilterType::Median);
    filterTypes << ImageFilters::getFilterTypeDescription(FilterType::Gaussian);
    filterTypes << ImageFilters::getFilterTypeDescription(FilterType::Bilateral);
    filterTypes << ImageFilters::getFilterTypeDescription(FilterType::Lee);
    filterTypes << ImageFilters::getFilterTypeDescription(FilterType::Frost);
    filterTypes << ImageFilters::getFilterTypeDescription(FilterType::Kuan);
    
    return filterTypes;
}

} // namespace Core
} // namespace SAR 