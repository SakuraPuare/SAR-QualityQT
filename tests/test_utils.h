#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <opencv2/opencv.hpp>
#include <string>
#include <random>
#include <ctime>
#include <filesystem>

namespace SAR {
namespace Tests {

/**
 * @brief 测试工具类
 * 提供创建测试图像和其他辅助功能
 */
class TestUtils {
public:
    /**
     * @brief 创建简单测试图像
     * @param width 图像宽度
     * @param height 图像高度
     * @param addNoise 是否添加噪声
     * @return OpenCV格式的测试图像
     */
    static cv::Mat createTestImage(int width, int height, bool addNoise = false) {
        cv::Mat image(height, width, CV_32F, cv::Scalar(128.0f));
        
        // 添加一个矩形信号区域
        cv::rectangle(image, cv::Point(width/4, height/4), 
                     cv::Point(3*width/4, 3*height/4), cv::Scalar(200.0f), -1);
        
        if (addNoise) {
            cv::Mat noise(height, width, CV_32F);
            cv::randn(noise, 0, 20.0);
            image += noise;
        }
        
        return image;
    }
    
    /**
     * @brief 在临时目录创建测试图像文件
     * @param filename 文件名（完整路径）
     * @param width 图像宽度
     * @param height 图像高度
     * @param addNoise 是否添加噪声
     * @return 是否成功创建文件
     */
    static bool saveTestImage(const std::string& filename, int width = 256, int height = 256, bool addNoise = false) {
        cv::Mat image = createTestImage(width, height, addNoise);
        return cv::imwrite(filename, image);
    }
    
    /**
     * @brief 生成随机字符串
     * @param length 字符串长度
     * @return 随机字符串
     */
    static std::string randomString(size_t length) {
        static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        
        std::string result;
        result.reserve(length);
        
        std::mt19937 gen(static_cast<unsigned int>(std::time(nullptr)));
        std::uniform_int_distribution<> dis(0, sizeof(alphanum) - 2);
        
        for (size_t i = 0; i < length; ++i) {
            result += alphanum[dis(gen)];
        }
        
        return result;
    }
    
    /**
     * @brief 创建临时测试目录
     * @param dirName 目录名称（可选）
     * @return 创建的目录路径
     */
    static std::string createTempTestDir(const std::string& dirName = "") {
        std::string tmpDir = "tests/test_data";
        if (!dirName.empty()) {
            tmpDir += "/" + dirName;
        } else {
            tmpDir += "/tmp_" + randomString(8);
        }
        
        // 使用 C++ 标准库 filesystem 创建目录
        std::filesystem::create_directories(tmpDir);
        return tmpDir;
    }
};

} // namespace Tests
} // namespace SAR

#endif // TEST_UTILS_H 