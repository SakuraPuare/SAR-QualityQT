#ifndef GLCM_ANALYSIS_H
#define GLCM_ANALYSIS_H

#include <opencv2/opencv.hpp>
#include <QString>
#include <vector>
#include <map>

namespace SAR {
namespace Analysis {

/**
 * @brief GLCM (灰度共生矩阵) 分析功能
 */
class GLCMAnalysis {
public:
    struct GLCMFeatures {
        double contrast;        // 对比度
        double dissimilarity;   // 不相似性 
        double homogeneity;     // 同质性
        double angular_second_moment; // 角二阶矩 (能量)
        double energy;          // 能量
        double correlation;     // 相关性
        double entropy;         // 熵
    };
    
    /**
     * @brief 计算图像的GLCM特征
     * @param image 输入图像
     * @param distance GLCM距离
     * @param angles GLCM角度，默认为{0, 45, 90, 135}度
     * @return GLCM特征结构体
     */
    static GLCMFeatures calculateGLCMFeatures(const cv::Mat& image, int distance = 1, 
        const std::vector<double>& angles = {0, 45, 90, 135});
    
    /**
     * @brief 计算图像的GLCM矩阵
     * @param image 输入图像
     * @param distance 像素间距
     * @param angle 角度（度）
     * @return GLCM矩阵
     */
    static cv::Mat calculateGLCM(const cv::Mat& image, int distance, double angle);
    
    /**
     * @brief 获取GLCM特征的文本描述
     * @param features GLCM特征结构
     * @return 特征描述文本
     */
    static QString getGLCMFeaturesDescription(const GLCMFeatures& features);
};

} // namespace Analysis
} // namespace SAR

#endif // GLCM_ANALYSIS_H 