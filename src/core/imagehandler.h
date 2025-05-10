#ifndef IMAGEHANDLER_H
#define IMAGEHANDLER_H

#include <QString>
#include <QImage>
#include <opencv2/opencv.hpp>
#include <gdal_priv.h>

namespace SAR {
namespace Core {

/**
 * @brief 图像处理类，处理SAR图像的加载、转换和基本处理
 */
class ImageHandler {
public:
    /**
     * @brief 构造函数
     */
    ImageHandler();
    
    /**
     * @brief 析构函数
     */
    ~ImageHandler();
    
    /**
     * @brief 加载图像文件
     * @param filePath 图像文件路径
     * @return 是否成功加载
     */
    bool loadImage(const QString& filePath);
    
    /**
     * @brief 获取当前加载的图像
     * @return OpenCV图像
     */
    cv::Mat getImage() const;
    
    /**
     * @brief 将OpenCV图像转换为QImage以便显示
     * @param cvImage OpenCV图像
     * @return QImage图像
     */
    static QImage cvMatToQImage(const cv::Mat& cvImage);
    
    /**
     * @brief 提取图像的区域
     * @param rect 区域矩形
     * @return 裁剪后的图像
     */
    cv::Mat extractROI(const cv::Rect& rect) const;
    
    /**
     * @brief 获取文件元数据
     * @return 元数据字符串
     */
    QString getMetadata() const;
    
    /**
     * @brief 保存处理后的图像
     * @param filePath 保存路径
     * @param image 要保存的图像，如果为空则保存当前图像
     * @return 是否成功保存
     */
    bool saveImage(const QString& filePath, const cv::Mat& image = cv::Mat());
    
private:
    cv::Mat m_image;             // 当前加载的图像
    QString m_filePath;          // 图像文件路径
    GDALDataset* m_dataset;      // GDAL数据集
    
    /**
     * @brief 初始化GDAL库
     */
    void initGDAL();
    
    /**
     * @brief 使用GDAL加载地理图像
     * @param filePath 图像文件路径
     * @return 是否成功加载
     */
    bool loadWithGDAL(const QString& filePath);
    
    /**
     * @brief 使用OpenCV加载普通图像
     * @param filePath 图像文件路径
     * @return 是否成功加载
     */
    bool loadWithOpenCV(const QString& filePath);
};

} // namespace Core
} // namespace SAR

#endif // IMAGEHANDLER_H 