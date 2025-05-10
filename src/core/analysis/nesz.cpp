#include "nesz.h"
#include <opencv2/imgproc.hpp>
#include <cmath>

namespace SAR {
namespace Analysis {

// 光速常量（米/秒）
constexpr double SPEED_OF_LIGHT = 299792458.0;
// 玻尔兹曼常数（J/K）
constexpr double BOLTZMANN_CONSTANT = 1.380649e-23;

NESZ::NESZ() 
    : lastNESZ(0.0), transmitPower(1000.0), antennaGain(30.0), 
      systemLoss(3.0), hasParameters(false) {
}

double NESZ::calculateNESZ(const cv::Mat& image) {
    if (image.empty()) {
        return 0.0;
    }

    // 将图像转换为32位浮点类型进行处理
    cv::Mat floatImage;
    if (image.type() != CV_32F) {
        image.convertTo(floatImage, CV_32F);
    } else {
        floatImage = image.clone();
    }

    if (hasParameters) {
        // 如果已经设置了系统参数，使用理论公式计算
        lastNESZ = calculateTheoreticalNESZ();
    } else {
        // 否则使用图像估计噪声水平
        double noiseLevel = estimateNoiseLevel(floatImage);
        
        // 将噪声水平转换为NESZ（假设校准因子为1.0）
        // 在实际应用中，此处需要考虑SAR系统的校准因子和处理增益
        lastNESZ = 10.0 * std::log10(noiseLevel);
    }

    return lastNESZ;
}

double NESZ::calculateNESZWithROI(const cv::Mat& image, const cv::Rect& noiseROI) {
    if (image.empty() || 
        noiseROI.x < 0 || noiseROI.y < 0 || 
        noiseROI.x + noiseROI.width > image.cols || 
        noiseROI.y + noiseROI.height > image.rows) {
        return 0.0;
    }

    // 将噪声区域提取出来
    cv::Mat noiseRegion = image(noiseROI).clone();
    
    // 使用均方差估计噪声水平
    cv::Scalar mean, stdDev;
    cv::meanStdDev(noiseRegion, mean, stdDev);
    double noiseLevel = stdDev[0] * stdDev[0];
    
    // 将噪声水平转换为NESZ（假设校准因子为1.0）
    // 在实际应用中，此处需要考虑SAR系统的校准因子和处理增益
    lastNESZ = 10.0 * std::log10(noiseLevel);
    
    return lastNESZ;
}

void NESZ::setSystemParameters(double transmitPower, double antennaGain, double systemLoss) {
    // 输入参数验证
    if (transmitPower > 0 && antennaGain > 0 && systemLoss >= 0) {
        this->transmitPower = transmitPower;
        this->antennaGain = antennaGain;
        this->systemLoss = systemLoss;
        hasParameters = true;
    }
}

QString NESZ::getResultDescription() const {
    return QString("噪声等效后向散射系数(NESZ): %1 dB")
        .arg(lastNESZ, 0, 'f', 2);
}

double NESZ::estimateNoiseLevel(const cv::Mat& image) {
    // 使用高斯滤波得到平滑图像
    cv::Mat smoothed;
    cv::GaussianBlur(image, smoothed, cv::Size(5, 5), 1.5);

    // 噪声是原图与平滑图像的差异
    cv::Mat noise = image - smoothed;

    // 计算噪声的功率
    cv::Scalar mean, stdDev;
    cv::meanStdDev(noise, mean, stdDev);
    double noisePower = stdDev[0] * stdDev[0];
    
    return noisePower;
}

double NESZ::calculateTheoreticalNESZ() {
    // 简化的NESZ理论计算公式
    // NESZ = (systemLoss * BOLTZMANN_CONSTANT * T * B * F * R^3 * v) / (Pt * G^2 * λ^2 * c * τ)
    // 其中:
    // T是系统噪声温度（K）
    // B是带宽（Hz）
    // F是噪声系数
    // R是距离（m）
    // v是平台速度（m/s）
    // Pt是发射功率（W）
    // G是天线增益
    // λ是波长（m）
    // c是光速（m/s）
    // τ是脉冲宽度（s）
    
    // 这里使用一个简化版本进行示例
    // 在实际应用中，需要根据具体的SAR系统参数进行更精确的计算
    
    // 假设的参数值
    double T = 290.0;        // 噪声温度(K)
    double B = 100e6;        // 带宽(Hz)
    double F = 3.0;          // 噪声系数
    double R = 600e3;        // 距离(m)
    double v = 7500.0;       // 平台速度(m/s)
    double lambda = 0.03;    // 波长(m)(X波段)
    double tau = 10e-6;      // 脉冲宽度(s)
    
    // 将dB转换为线性单位
    double G = std::pow(10.0, antennaGain / 10.0);
    double L = std::pow(10.0, systemLoss / 10.0);
    
    // 计算NESZ
    double numerator = L * BOLTZMANN_CONSTANT * T * B * F * R * R * R * v;
    double denominator = transmitPower * G * G * lambda * lambda * SPEED_OF_LIGHT * tau;
    
    double nesz = numerator / denominator;
    
    // 转换为dB
    return 10.0 * std::log10(nesz);
}

} // namespace Analysis
} // namespace SAR 