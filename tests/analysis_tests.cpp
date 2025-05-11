#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>
#include "../src/core/analysis/snr.h"
#include "../src/core/analysis/clarity.h"
#include "../src/core/analysis/glcm.h"
#include "../src/core/analysis/radiometric.h"
#include "test_utils.h"

namespace {

// 创建SNR的测试版本，实现抽象基类的纯虚函数
class TestSNR : public SAR::Analysis::SNR {
public:
    TestSNR() : SAR::Analysis::SNR() {}
};

// SNR 测试套件
class SNRTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试图像
        cleanImage = SAR::Tests::TestUtils::createTestImage(256, 256, false);
        noisyImage = SAR::Tests::TestUtils::createTestImage(256, 256, true);
        
        // 定义信号和噪声区域
        signalROI = cv::Rect(64, 64, 128, 128);  // 中间的矩形区域
        noiseROI = cv::Rect(10, 10, 40, 40);     // 左上角区域
    }
    
    TestSNR snrAnalyzer;
    cv::Mat cleanImage;
    cv::Mat noisyImage;
    cv::Rect signalROI;
    cv::Rect noiseROI;
};

// 测试基本 SNR 计算
TEST_F(SNRTest, TestBasicSNRCalculation) {
    double snr = snrAnalyzer.calculateSNR(cleanImage);
    EXPECT_GT(snr, 0.0) << "SNR 应该为正值";
    
    double noisySnr = snrAnalyzer.calculateSNR(noisyImage);
    EXPECT_GT(snr, noisySnr) << "干净图像的 SNR 应高于有噪声图像";
}

// 测试使用 ROI 的 SNR 计算
TEST_F(SNRTest, TestSNRWithROI) {
    double snr = snrAnalyzer.calculateSNRWithROI(cleanImage, signalROI, noiseROI);
    EXPECT_GT(snr, 0.0) << "使用 ROI 的 SNR 应该为正值";
}

// 测试噪声水平估计
TEST_F(SNRTest, TestNoiseEstimation) {
    double noiseLevel = snrAnalyzer.estimateNoiseLevel(noisyImage);
    EXPECT_GT(noiseLevel, 0.0) << "噪声水平应该为正值";
    
    double cleanNoiseLevel = snrAnalyzer.estimateNoiseLevel(cleanImage);
    EXPECT_GT(noiseLevel, cleanNoiseLevel) << "有噪声图像的噪声水平应高于干净图像";
}

// 清晰度测试套件
class ClarityTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试图像
        clearImage = SAR::Tests::TestUtils::createTestImage(256, 256, false);
        blurImage = clearImage.clone();
        
        // 对图像进行模糊处理
        cv::GaussianBlur(blurImage, blurImage, cv::Size(15, 15), 5.0);
    }
    
    SAR::Analysis::Clarity clarityAnalyzer;
    cv::Mat clearImage;
    cv::Mat blurImage;
};

// 测试基本清晰度计算
TEST_F(ClarityTest, TestClarityScore) {
    double clearScore = clarityAnalyzer.calculateClarityScore(clearImage);
    double blurScore = clarityAnalyzer.calculateClarityScore(blurImage);
    
    EXPECT_GT(clearScore, 0.0) << "清晰度得分应该为正值";
    EXPECT_GT(clearScore, blurScore) << "清晰图像的清晰度得分应高于模糊图像";
}

// 测试边缘强度计算
TEST_F(ClarityTest, TestEdgeStrength) {
    double clearEdge = clarityAnalyzer.calculateEdgeStrength(clearImage);
    double blurEdge = clarityAnalyzer.calculateEdgeStrength(blurImage);
    
    EXPECT_GT(clearEdge, 0.0) << "边缘强度应该为正值";
    EXPECT_GT(clearEdge, blurEdge) << "清晰图像的边缘强度应高于模糊图像";
}

// 测试边缘情况：空图像
TEST_F(ClarityTest, TestEmptyImage) {
    cv::Mat emptyImage;
    
    EXPECT_THROW({
        clarityAnalyzer.calculateClarityScore(emptyImage);
    }, std::exception) << "空图像应抛出异常";
}

} // namespace 