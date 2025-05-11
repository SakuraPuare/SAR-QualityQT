#include "rasr.h"
#include <opencv2/imgproc.hpp>
#include <cmath>
#include <numeric>

namespace SAR {
namespace Analysis {

RASR::RASR() : lastRASR(0.0) {
}

double RASR::calculateRASR(const cv::Mat& image, double PRF, double R0, double incidenceAngle,
                         const std::vector<double>& antennaGain) {
    if (image.empty()) {
        return 0.0;
    }

    // 将图像转换为 32 位浮点类型进行处理
    cv::Mat floatImage;
    if (image.type() != CV_32F) {
        image.convertTo(floatImage, CV_32F);
    } else {
        floatImage = image.clone();
    }

    // 计算RASR
    lastRASR = calculateRASRValue(floatImage, PRF, R0, incidenceAngle, antennaGain);

    return lastRASR;
}

double RASR::calculateRASRInROI(const cv::Mat& image, const cv::Rect& roi, 
                              double PRF, double R0, double incidenceAngle,
                              const std::vector<double>& antennaGain) {
    if (image.empty() || 
        roi.x < 0 || roi.y < 0 || 
        roi.x + roi.width > image.cols || 
        roi.y + roi.height > image.rows) {
        return 0.0;
    }

    // 将 ROI 区域提取出来
    cv::Mat roiImage = image(roi).clone();
    
    // 计算 ROI 区域的 RASR
    return calculateRASR(roiImage, PRF, R0, incidenceAngle, antennaGain);
}

QString RASR::getResultDescription() const {
    return QString("距离模糊度 (RASR): %1")
        .arg(lastRASR, 0, 'f', 4);
}

double RASR::calculateRASRValue(const cv::Mat& image, double PRF, double R0, 
                               double incidenceAngle, const std::vector<double>& antennaGain) {
    // 实现公式RASR计算
    // RASR = (∑ σ0(θn)Gr^2(θn)*R0^3*sin(θn))/( ∑ σ0(θ0)Gr^2(θ0)*R0^3*sin(θ0))
    // 其中n≠0表示非主瓣目标，n=0表示主瓣目标
    
    // 由于真实天线增益模式和后向散射系数通常无法直接从图像获取，
    // 这里提供一个简化的实现，可根据实际情况调整
    
    const int numAmbiguities = 5; // 考虑5个模糊区
    double numerator = 0.0; // 分子
    double denominator = 0.0; // 分母
    
    // 根据图像估计后向散射系数
    double sigma0 = cv::mean(image)[0] / 255.0; // 归一化
    
    // 主瓣计算
    double theta0 = incidenceAngle;
    double G0 = simulateAntennaGainPattern(theta0, antennaGain);
    denominator = sigma0 * G0 * G0 * std::pow(R0, 3) * std::sin(theta0);
    
    // 副瓣计算
    for (int n = -numAmbiguities; n <= numAmbiguities; n++) {
        if (n == 0) continue; // 跳过主瓣
        
        // 计算第n个模糊区的入射角
        double theta_n = theta0 + n * (PRF / (2.0 * R0));
        
        // 计算天线增益 
        double G_n = simulateAntennaGainPattern(theta_n, antennaGain);
        
        // 累加
        numerator += sigma0 * G_n * G_n * std::pow(R0, 3) * std::sin(theta_n);
    }
    
    // 防止除零
    if (std::abs(denominator) < 1e-10) {
        return 0.0;
    }
    
    return numerator / denominator;
}

double RASR::simulateAntennaGainPattern(double angle, const std::vector<double>& antennaGain) {
    // 如果提供了天线增益数据，则使用插值
    if (!antennaGain.empty()) {
        // 简单的线性插值
        int n = antennaGain.size();
        double angleStep = M_PI / (n - 1);
        int index = static_cast<int>(angle / angleStep);
        
        if (index < 0) return antennaGain.front();
        if (index >= n - 1) return antennaGain.back();
        
        double t = (angle - index * angleStep) / angleStep;
        return antennaGain[index] * (1.0 - t) + antennaGain[index + 1] * t;
    }
    
    // 否则使用简化的高斯模型模拟天线增益模式
    // 假设主瓣在0，旁瓣随角度减小
    const double beamwidth = 0.1; // 弧度
    return std::exp(-0.5 * std::pow(angle, 2) / std::pow(beamwidth, 2));
}

double RASR::calculateBackscatterCoefficient(const cv::Mat& image, int x, int y) {
    // 简化的后向散射系数估计
    // 实际应用中应使用适当的校准和模型
    if (x < 0 || x >= image.cols || y < 0 || y >= image.rows) {
        return 0.0;
    }
    
    return image.at<float>(y, x) / 255.0; // 归一化
}

} // namespace Analysis
} // namespace SAR 