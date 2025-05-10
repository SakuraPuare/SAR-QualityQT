#include <gtest/gtest.h>
#include <QString>
#include <QPixmap>
#include <QSize>
#include <functional>
#include <string>
#include "../src/core/include/imagehandler.h"
#include "test_utils.h"

namespace {

// 图像处理测试套件
class ImageHandlerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 设置日志记录函数
        logger = [this](const QString& message) {
            logMessages.push_back(message);
        };
        
        imageHandler.setLogger(logger);
        
        // 创建测试目录
        testDir = SAR::Tests::TestUtils::createTempTestDir("image_tests");
    }
    
    SAR::Core::ImageHandler imageHandler;
    std::function<void(const QString&)> logger;
    std::vector<QString> logMessages;
    std::string testDir;
};

// 测试图像加载与验证
TEST_F(ImageHandlerTest, TestImageLoading) {
    // 创建测试图像文件
    std::string testFilePath = testDir + "/test_image.tif";
    bool created = SAR::Tests::TestUtils::saveTestImage(testFilePath);
    ASSERT_TRUE(created) << "无法创建测试图像文件";
    
    // 加载图像
    QString qtPath = QString::fromStdString(testFilePath);
    bool loadResult = imageHandler.loadImage(qtPath);
    EXPECT_TRUE(loadResult) << "图像加载应该成功";
    EXPECT_TRUE(imageHandler.isValid()) << "加载后图像应该有效";
    
    // 验证图像属性
    EXPECT_EQ(imageHandler.getFilename(), qtPath) << "加载后文件名应一致";
    EXPECT_EQ(imageHandler.getDimensionsString(), "256 x 256") << "图像尺寸应为 256x256";
    
    // 获取图像数据
    const cv::Mat& imageData = imageHandler.getImageData();
    EXPECT_FALSE(imageData.empty()) << "图像数据不应为空";
    EXPECT_EQ(imageData.rows, 256) << "图像行数应为 256";
    EXPECT_EQ(imageData.cols, 256) << "图像列数应为 256";
}

// 测试图像显示
TEST_F(ImageHandlerTest, TestImageDisplay) {
    // 创建测试图像文件
    std::string testFilePath = testDir + "/test_display.tif";
    SAR::Tests::TestUtils::saveTestImage(testFilePath);
    
    // 加载图像
    imageHandler.loadImage(QString::fromStdString(testFilePath));
    
    // 获取显示用的 QPixmap
    QPixmap pixmap = imageHandler.getDisplayPixmap(QSize(128, 128));
    EXPECT_FALSE(pixmap.isNull()) << "生成的 QPixmap 不应为空";
    EXPECT_EQ(pixmap.width(), 128) << "缩放后宽度应为 128";
    EXPECT_EQ(pixmap.height(), 128) << "缩放后高度应为 128";
}

// 测试错误处理
TEST_F(ImageHandlerTest, TestErrorHandling) {
    // 尝试加载不存在的文件
    bool loadResult = imageHandler.loadImage("nonexistent_file.tif");
    EXPECT_FALSE(loadResult) << "不存在的文件加载应该失败";
    EXPECT_FALSE(imageHandler.isValid()) << "加载失败后图像应该无效";
    
    // 检查是否有错误日志
    bool hasErrorLog = false;
    for (const QString& log : logMessages) {
        if (log.contains("Error", Qt::CaseInsensitive) || 
            log.contains("Failed", Qt::CaseInsensitive)) {
            hasErrorLog = true;
            break;
        }
    }
    EXPECT_TRUE(hasErrorLog) << "加载失败应记录错误日志";
}

// 测试关闭图像
TEST_F(ImageHandlerTest, TestImageClosing) {
    // 创建测试图像文件
    std::string testFilePath = testDir + "/test_close.tif";
    SAR::Tests::TestUtils::saveTestImage(testFilePath);
    
    // 加载图像
    imageHandler.loadImage(QString::fromStdString(testFilePath));
    EXPECT_TRUE(imageHandler.isValid()) << "加载后图像应该有效";
    
    // 关闭图像
    imageHandler.closeImage();
    EXPECT_FALSE(imageHandler.isValid()) << "关闭后图像应该无效";
    EXPECT_TRUE(imageHandler.getFilename().isEmpty()) << "关闭后文件名应为空";
}

} // namespace 