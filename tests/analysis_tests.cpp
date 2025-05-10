#include <gtest/gtest.h>
#include <QApplication>
#include <QString>
#include <QDebug>

#include "core/analysis/clarity.h"
#include "core/analysis/glcm.h"
#include "core/imagehandler.h"

// 全局测试环境
class SARTestEnvironment : public ::testing::Environment {
public:
    SARTestEnvironment(int argc, char** argv) {
        m_app = new QApplication(argc, argv);
    }
    
    ~SARTestEnvironment() override {
        delete m_app;
    }
    
    void SetUp() override {
        qDebug() << "设置全局测试环境";
    }
    
    void TearDown() override {
        qDebug() << "清理全局测试环境";
    }
    
private:
    QApplication* m_app;
};

// 清晰度分析测试
class ClarityAnalysisTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试图像
        testImage = cv::Mat::zeros(100, 100, CV_8UC1);
        
        // 添加一些渐变
        for (int i = 0; i < testImage.rows; i++) {
            for (int j = 0; j < testImage.cols; j++) {
                testImage.at<uchar>(i, j) = static_cast<uchar>((i + j) % 256);
            }
        }
    }
    
    cv::Mat testImage;
};

TEST_F(ClarityAnalysisTest, CalculateClarity) {
    double clarity = SAR::Analysis::ClarityAnalysis::calculateClarity(testImage);
    EXPECT_GT(clarity, 0.0);
}

TEST_F(ClarityAnalysisTest, CalculateGradientEnergy) {
    double energy = SAR::Analysis::ClarityAnalysis::calculateGradientEnergy(testImage);
    EXPECT_GT(energy, 0.0);
}

TEST_F(ClarityAnalysisTest, CalculateTenengradVariance) {
    double variance = SAR::Analysis::ClarityAnalysis::calculateTenengradVariance(testImage);
    EXPECT_GT(variance, 0.0);
}

TEST_F(ClarityAnalysisTest, CalculateEntropy) {
    double entropy = SAR::Analysis::ClarityAnalysis::calculateEntropy(testImage);
    EXPECT_GT(entropy, 0.0);
}

// GLCM分析测试
class GLCMAnalysisTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试图像
        testImage = cv::Mat::zeros(50, 50, CV_8UC1);
        
        // 添加一些纹理
        for (int i = 0; i < testImage.rows; i++) {
            for (int j = 0; j < testImage.cols; j++) {
                if ((i / 10 + j / 10) % 2 == 0) {
                    testImage.at<uchar>(i, j) = 255;
                } else {
                    testImage.at<uchar>(i, j) = 0;
                }
            }
        }
    }
    
    cv::Mat testImage;
};

TEST_F(GLCMAnalysisTest, CalculateGLCMFeatures) {
    SAR::Analysis::GLCMAnalysis::GLCMFeatures features = 
        SAR::Analysis::GLCMAnalysis::calculateGLCMFeatures(testImage);
    
    EXPECT_GT(features.contrast, 0.0);
    EXPECT_GE(features.homogeneity, 0.0);
    EXPECT_LE(features.homogeneity, 1.0);
    EXPECT_GE(features.energy, 0.0);
    EXPECT_LE(features.energy, 1.0);
}

// 主函数
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new SARTestEnvironment(argc, argv));
    return RUN_ALL_TESTS();
}
