#include "glcm.h"
#include <cmath>
#include <opencv2/imgproc.hpp>

namespace SAR {
namespace Analysis {

GLCMAnalysis::GLCMFeatures GLCMAnalysis::calculateGLCMFeatures(
    const cv::Mat& image, int distance, const std::vector<double>& angles) {
    
    // 初始化特征结构
    GLCMFeatures features = {0};
    
    // 对每个角度计算GLCM特征并累加
    for (double angle : angles) {
        cv::Mat glcm = calculateGLCM(image, distance, angle);
        
        // 计算特征
        double contrast = 0.0;
        double dissimilarity = 0.0;
        double homogeneity = 0.0;
        double angular_second_moment = 0.0;
        double energy = 0.0;
        double correlation = 0.0;
        double entropy = 0.0;
        
        // 计算均值和标准差
        double meanI = 0.0, meanJ = 0.0;
        double sigmaI = 0.0, sigmaJ = 0.0;
        
        // 首先计算均值
        for (int i = 0; i < glcm.rows; i++) {
            for (int j = 0; j < glcm.cols; j++) {
                double prob = glcm.at<double>(i, j);
                meanI += i * prob;
                meanJ += j * prob;
            }
        }
        
        // 计算标准差
        for (int i = 0; i < glcm.rows; i++) {
            for (int j = 0; j < glcm.cols; j++) {
                double prob = glcm.at<double>(i, j);
                sigmaI += prob * std::pow(i - meanI, 2);
                sigmaJ += prob * std::pow(j - meanJ, 2);
            }
        }
        sigmaI = std::sqrt(sigmaI);
        sigmaJ = std::sqrt(sigmaJ);
        
        // 计算各个特征
        for (int i = 0; i < glcm.rows; i++) {
            for (int j = 0; j < glcm.cols; j++) {
                double prob = glcm.at<double>(i, j);
                
                // 对比度 (Contrast)
                contrast += prob * std::pow(i - j, 2);
                
                // 不相似性 (Dissimilarity)
                dissimilarity += prob * std::abs(i - j);
                
                // 同质性 (Homogeneity)
                homogeneity += prob / (1 + std::pow(i - j, 2));
                
                // 角二阶矩 (ASM - Angular Second Moment)
                angular_second_moment += std::pow(prob, 2);
                
                // 能量 (Energy) - ASM的平方根
                
                // 相关性 (Correlation)
                if (sigmaI > 0 && sigmaJ > 0) {
                    correlation += ((i - meanI) * (j - meanJ) * prob) / (sigmaI * sigmaJ);
                }
                
                // 熵 (Entropy)
                if (prob > 0) {
                    entropy -= prob * std::log2(prob);
                }
            }
        }
        
        // 计算能量
        energy = std::sqrt(angular_second_moment);
        
        // 累加特征
        features.contrast += contrast;
        features.dissimilarity += dissimilarity;
        features.homogeneity += homogeneity;
        features.angular_second_moment += angular_second_moment;
        features.energy += energy;
        features.correlation += correlation;
        features.entropy += entropy;
    }
    
    // 计算平均值
    int numAngles = angles.size();
    if (numAngles > 0) {
        features.contrast /= numAngles;
        features.dissimilarity /= numAngles;
        features.homogeneity /= numAngles;
        features.angular_second_moment /= numAngles;
        features.energy /= numAngles;
        features.correlation /= numAngles;
        features.entropy /= numAngles;
    }
    
    return features;
}

cv::Mat GLCMAnalysis::calculateGLCM(const cv::Mat& image, int distance, double angle) {
    // 转换为灰度图像
    cv::Mat gray;
    if (image.channels() > 1) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    // 转换为8位图像（如果需要）
    if (gray.depth() != CV_8U) {
        gray.convertTo(gray, CV_8U);
    }
    
    // 计算角度对应的偏移
    int offsetX = 0, offsetY = 0;
    if (angle == 0) {        // 0度
        offsetX = distance;
        offsetY = 0;
    } else if (angle == 45) { // 45度
        offsetX = distance;
        offsetY = -distance;
    } else if (angle == 90) { // 90度
        offsetX = 0;
        offsetY = -distance;
    } else if (angle == 135) { // 135度
        offsetX = -distance;
        offsetY = -distance;
    } else {
        // 其他角度，使用余弦和正弦计算
        offsetX = static_cast<int>(std::round(distance * std::cos(angle * CV_PI / 180.0)));
        offsetY = static_cast<int>(std::round(distance * std::sin(angle * CV_PI / 180.0)));
    }
    
    // 创建GLCM矩阵
    int levels = 256; // 灰度级别数
    cv::Mat glcm = cv::Mat::zeros(levels, levels, CV_64F);
    
    // 构造GLCM
    int count = 0;
    for (int i = 0; i < gray.rows; i++) {
        for (int j = 0; j < gray.cols; j++) {
            int ni = i + offsetY;
            int nj = j + offsetX;
            
            if (ni >= 0 && ni < gray.rows && nj >= 0 && nj < gray.cols) {
                int value1 = gray.at<uchar>(i, j);
                int value2 = gray.at<uchar>(ni, nj);
                glcm.at<double>(value1, value2) += 1.0;
                count++;
            }
        }
    }
    
    // 归一化GLCM
    if (count > 0) {
        glcm /= count;
    }
    
    return glcm;
}

QString GLCMAnalysis::getGLCMFeaturesDescription(const GLCMFeatures& features) {
    QString description;
    
    description = QString("GLCM特征分析结果：\n\n");
    description += QString("对比度: %1\n").arg(features.contrast, 0, 'f', 4);
    description += QString("不相似性: %1\n").arg(features.dissimilarity, 0, 'f', 4);
    description += QString("同质性: %1\n").arg(features.homogeneity, 0, 'f', 4);
    description += QString("角二阶矩: %1\n").arg(features.angular_second_moment, 0, 'f', 4);
    description += QString("能量: %1\n").arg(features.energy, 0, 'f', 4);
    description += QString("相关性: %1\n").arg(features.correlation, 0, 'f', 4);
    description += QString("熵: %1\n").arg(features.entropy, 0, 'f', 4);
    
    return description;
}

} // namespace Analysis
} // namespace SAR 