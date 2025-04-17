#include "gtest/gtest.h"
#include "imagehandler.h"       // 用于加载图像
#include "analysis_utils.h"     // 假设包含所有 perform*Analysis 函数的声明
#include <gdal.h>
#include <opencv2/core.hpp>     // cv::Mat
#include <QCoreApplication>     // 需要 QApplication 用于 translate
#include <QString>
#include <iostream>             // 用于 std::cerr
#include <filesystem>           // 用于检查文件是否存在
#include <qapplication.h>

// 定义测试图像的相对路径
const std::string testImagePath = "data/LC08_L1TP_127035_20200425_20200509_01_T1_B2.TIF";

// Helper function for ImageHandler logging (can be simple for tests)
void testLogger(const QString& msg) {
    std::cerr << "[Test Logger] " << msg.toStdString() << std::endl;
}

// Test fixture for setting up QApplication (needed for QCoreApplication::translate)
class AnalysisTest : public ::testing::Test {
protected:
    static QApplication* app; // Static to initialize once

    static void SetUpTestSuite() {
        // QApplication requires argc and argv, even if dummy ones
        int argc = 1;
        char* argv[] = {(char*)"test_executable", nullptr};
        if (!QCoreApplication::instance()) { // Create only if it doesn't exist
             app = new QApplication(argc, argv);
             std::cerr << "QApplication instance created for tests." << std::endl;
        } else {
             std::cerr << "QApplication instance already exists." << std::endl;
             app = static_cast<QApplication*>(QCoreApplication::instance()); // Use existing one
        }
         // Initialize GDAL drivers (required by ImageHandler)
        GDALAllRegister();
    }

    static void TearDownTestSuite() {
       // Don't delete app here if it might be used by other test suites
       // Or manage its lifecycle carefully if multiple fixtures need it.
       // For simplicity here, we assume it's okay to leave it running
       // or handle cleanup outside if necessary.
       // delete app;
       // app = nullptr;
       // GDALCleanupAll(); // Optional: Clean up GDAL resources
       std::cerr << "QApplication TearDownTestSuite finished." << std::endl;
    }


    void SetUp() override {
        // Check if the test image exists before running each test
        ASSERT_TRUE(std::filesystem::exists(testImagePath))
            << "Test image file not found at: " << testImagePath;
    }
};

QApplication* AnalysisTest::app = nullptr; // Initialize static member

// Test case for Clarity Analysis
TEST_F(AnalysisTest, PerformClarityAnalysis) {
    ImageHandler handler(testLogger);
    ASSERT_TRUE(handler.loadImage(QString::fromStdString(testImagePath)))
        << "Failed to load image: " << testImagePath;
    ASSERT_TRUE(handler.isValid());

    const cv::Mat& imgData = handler.getImageData();
    ASSERT_FALSE(imgData.empty());

    AnalysisResult result = performClarityAnalysis(imgData);

    EXPECT_TRUE(result.success) << "Clarity analysis failed. Log:\n" << result.detailedLog.toStdString();
    EXPECT_FALSE(result.overviewSummary.isEmpty()) << "Overview summary should not be empty on success.";
}

// Test case for GLCM Analysis
TEST_F(AnalysisTest, PerformGLCMAnalysis) {
    ImageHandler handler(testLogger);
    ASSERT_TRUE(handler.loadImage(QString::fromStdString(testImagePath)))
        << "Failed to load image: " << testImagePath;
    ASSERT_TRUE(handler.isValid());

    const cv::Mat& imgData = handler.getImageData();
    ASSERT_FALSE(imgData.empty());

    AnalysisResult result = performGLCMAnalysis(imgData);

    EXPECT_TRUE(result.success) << "GLCM analysis failed. Log:\n" << result.detailedLog.toStdString();
    EXPECT_FALSE(result.overviewSummary.isEmpty()) << "Overview summary should not be empty on success.";
}

// Test case for Information Content Analysis
TEST_F(AnalysisTest, PerformInfoContentAnalysis) {
    ImageHandler handler(testLogger);
    ASSERT_TRUE(handler.loadImage(QString::fromStdString(testImagePath)))
        << "Failed to load image: " << testImagePath;
    ASSERT_TRUE(handler.isValid());

    const cv::Mat& imgData = handler.getImageData();
    ASSERT_FALSE(imgData.empty());

    AnalysisResult result = performInfoContentAnalysis(imgData);

    EXPECT_TRUE(result.success) << "Info Content analysis failed. Log:\n" << result.detailedLog.toStdString();
    EXPECT_FALSE(result.overviewSummary.isEmpty()) << "Overview summary should not be empty on success.";
}

// Test case for Radiometric Analysis
TEST_F(AnalysisTest, PerformRadiometricAnalysis) {
    ImageHandler handler(testLogger);
    ASSERT_TRUE(handler.loadImage(QString::fromStdString(testImagePath)))
        << "Failed to load image: " << testImagePath;
    ASSERT_TRUE(handler.isValid());

    const cv::Mat& imgData = handler.getImageData();
    ASSERT_FALSE(imgData.empty());

    AnalysisResult result = performRadiometricAnalysis(imgData);

    EXPECT_TRUE(result.success) << "Radiometric analysis failed. Log:\n" << result.detailedLog.toStdString();
    EXPECT_FALSE(result.overviewSummary.isEmpty()) << "Overview summary should not be empty on success.";
}

// Test case for SNR Analysis
TEST_F(AnalysisTest, PerformSNRAnalysis) {
    ImageHandler handler(testLogger);
    ASSERT_TRUE(handler.loadImage(QString::fromStdString(testImagePath)))
        << "Failed to load image: " << testImagePath;
    ASSERT_TRUE(handler.isValid());

    const cv::Mat& imgData = handler.getImageData();
    ASSERT_FALSE(imgData.empty());

    AnalysisResult result = performSNRAnalysis(imgData);

    EXPECT_TRUE(result.success) << "SNR analysis failed. Log:\n" << result.detailedLog.toStdString();
    EXPECT_FALSE(result.overviewSummary.isEmpty()) << "Overview summary should not be empty on success.";
}

// Optional: Add a main function if you are not using a test runner that provides one
// int main(int argc, char **argv) {
//   ::testing::InitGoogleTest(&argc, argv);
//   // QApplication needs to be created before running tests that use Qt features
//   QApplication app(argc, argv); // Create QApplication
//   return RUN_ALL_TESTS();
// }
