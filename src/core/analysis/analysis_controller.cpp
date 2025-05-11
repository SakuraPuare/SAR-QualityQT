#include "../include/analysis_controller.h"
#include "../include/imagehandler.h"
#include <QDateTime>
#include <cstdlib> // 添加标准库随机数函数头文件
#include <ctime> // 添加time函数头文件

namespace SAR {
namespace Core {

AnalysisController::AnalysisController(QObject *parent, 
                                       ImageHandler *imageHandler,
                                       std::function<void(int, const QString&)> progressCallback)
    : QObject(parent), imageHandler(imageHandler), progressCallback(progressCallback)
{
    // 初始化随机数种子
    std::srand(static_cast<unsigned int>(time(nullptr)));
}

AnalysisController::~AnalysisController()
{
}

AnalysisResult AnalysisController::performAnalysis(const QStringList &selectedMethods, 
                                                 const QString &imagePath,
                                                 const QRect &roi)
{
    AnalysisResult result;
    result.setImagePath(imagePath);
    result.setAnalysisTime(QDateTime::currentDateTime());
    
    int totalMethods = selectedMethods.size();
    int completedMethods = 0;
    
    // 执行所有选中的分析方法
    for (const QString &methodName : selectedMethods) {
        // 更新进度
        int progress = completedMethods * 100 / totalMethods;
        QString progressMessage = tr("正在分析：%1").arg(methodName);
        
        if (progressCallback) {
            progressCallback(progress, progressMessage);
        }
        emit analysisProgress(progress, progressMessage);
        
        // 执行单个分析方法
        AnalysisResultItem methodResult = analyzeMethod(methodName, imagePath, roi);
        
        // 添加到结果集合
        result.addResult(methodName, methodResult);
        
        // 更新完成的方法数量
        completedMethods++;
    }
    
    // 所有方法分析完成
    if (progressCallback) {
        progressCallback(100, tr("分析完成"));
    }
    emit analysisProgress(100, tr("分析完成"));
    emit analysisComplete(result);
    
    return result;
}

AnalysisResultItem AnalysisController::analyzeMethod(const QString &methodName, 
                                                   const QString &imagePath,
                                                   const QRect &roi)
{
    // 根据方法名称调用相应的分析函数
    if (methodName == "ISLR") {
        return analyzeISLR(imagePath, roi);
    } else if (methodName == "PSLR") {
        return analyzePSLR(imagePath, roi);
    } else if (methodName == "RASR") {
        return analyzeRASR(imagePath, roi);
    } else if (methodName == "AASR") {
        return analyzeAASR(imagePath, roi);
    } else if (methodName == "SNR") {
        return analyzeSNR(imagePath, roi);
    } else if (methodName == "NESZ") {
        return analyzeNESZ(imagePath, roi);
    } else if (methodName == "RadiometricAccuracy") {
        return analyzeRadiometricAccuracy(imagePath, roi);
    } else if (methodName == "RadiometricResolution") {
        return analyzeRadiometricResolution(imagePath, roi);
    } else if (methodName == "ENL") {
        return analyzeENL(imagePath, roi);
    } else {
        // 不支持的方法
        AnalysisResultItem errorResult;
        errorResult.methodName = methodName;
        errorResult.isSuccess = false;
        errorResult.errorMessage = tr("不支持的分析方法：%1").arg(methodName);
        return errorResult;
    }
}

// 下面是各种分析方法的实现
// 注意：这里只是示例实现，实际的分析算法需要根据具体需求实现

AnalysisResultItem AnalysisController::analyzeISLR(const QString &imagePath, const QRect &roi)
{
    AnalysisResultItem result;
    result.methodName = "ISLR";
    result.description = "积分旁瓣比（Integrated Sidelobe Ratio）";
    
    // 模拟实现：生成一个随机值
    result.numericValue = -25.0 + (rand() % 10);
    result.unit = "dB";
    
    // 添加详细数据
    result.additionalValues["峰值"] = result.numericValue - 2.0;
    result.additionalValues["均值"] = result.numericValue;
    result.additionalValues["标准差"] = 1.5;
    
    result.additionalInfo["分析时间"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    result.additionalInfo["分析区域"] = roi.isValid() ? QString("(%1,%2,%3,%4)").arg(roi.x()).arg(roi.y()).arg(roi.width()).arg(roi.height()) : "全图";
    
    return result;
}

AnalysisResultItem AnalysisController::analyzePSLR(const QString &imagePath, const QRect &roi)
{
    AnalysisResultItem result;
    result.methodName = "PSLR";
    result.description = "峰值旁瓣比（Peak Sidelobe Ratio）";
    
    // 模拟实现
    result.numericValue = -13.0 + (rand() % 5);
    result.unit = "dB";
    
    // 添加详细数据
    result.additionalValues["峰值"] = result.numericValue - 1.0;
    result.additionalValues["均值"] = result.numericValue;
    result.additionalValues["标准差"] = 0.8;
    
    result.additionalInfo["分析时间"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    result.additionalInfo["分析区域"] = roi.isValid() ? QString("(%1,%2,%3,%4)").arg(roi.x()).arg(roi.y()).arg(roi.width()).arg(roi.height()) : "全图";
    
    return result;
}

AnalysisResultItem AnalysisController::analyzeRASR(const QString &imagePath, const QRect &roi)
{
    AnalysisResultItem result;
    result.methodName = "RASR";
    result.description = "距离模糊度比（Range Ambiguity to Signal Ratio）";
    
    // 模拟实现
    result.numericValue = -20.0 + (rand() % 8);
    result.unit = "dB";
    
    // 添加详细数据
    result.additionalValues["峰值"] = result.numericValue - 1.5;
    result.additionalValues["均值"] = result.numericValue;
    result.additionalValues["标准差"] = 1.2;
    
    result.additionalInfo["分析时间"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    result.additionalInfo["分析区域"] = roi.isValid() ? QString("(%1,%2,%3,%4)").arg(roi.x()).arg(roi.y()).arg(roi.width()).arg(roi.height()) : "全图";
    
    return result;
}

AnalysisResultItem AnalysisController::analyzeAASR(const QString &imagePath, const QRect &roi)
{
    AnalysisResultItem result;
    result.methodName = "AASR";
    result.description = "方位模糊度比（Azimuth Ambiguity to Signal Ratio）";
    
    // 模拟实现
    result.numericValue = -18.0 + (rand() % 7);
    result.unit = "dB";
    
    // 添加详细数据
    result.additionalValues["峰值"] = result.numericValue - 1.3;
    result.additionalValues["均值"] = result.numericValue;
    result.additionalValues["标准差"] = 1.0;
    
    result.additionalInfo["分析时间"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    result.additionalInfo["分析区域"] = roi.isValid() ? QString("(%1,%2,%3,%4)").arg(roi.x()).arg(roi.y()).arg(roi.width()).arg(roi.height()) : "全图";
    
    return result;
}

AnalysisResultItem AnalysisController::analyzeSNR(const QString &imagePath, const QRect &roi)
{
    AnalysisResultItem result;
    result.methodName = "SNR";
    result.description = "信噪比（Signal-to-Noise Ratio）";
    
    // 模拟实现
    result.numericValue = 15.0 + (rand() % 10);
    result.unit = "dB";
    
    // 添加详细数据
    result.additionalValues["峰值"] = result.numericValue + 2.0;
    result.additionalValues["均值"] = result.numericValue;
    result.additionalValues["标准差"] = 1.8;
    
    result.additionalInfo["分析时间"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    result.additionalInfo["分析区域"] = roi.isValid() ? QString("(%1,%2,%3,%4)").arg(roi.x()).arg(roi.y()).arg(roi.width()).arg(roi.height()) : "全图";
    
    return result;
}

AnalysisResultItem AnalysisController::analyzeNESZ(const QString &imagePath, const QRect &roi)
{
    AnalysisResultItem result;
    result.methodName = "NESZ";
    result.description = "噪声等效辐射截面（Noise Equivalent Sigma Zero）";
    
    // 模拟实现
    result.numericValue = -22.0 + (rand() % 8);
    result.unit = "dB";
    
    // 添加详细数据
    result.additionalValues["峰值"] = result.numericValue - 1.5;
    result.additionalValues["均值"] = result.numericValue;
    result.additionalValues["标准差"] = 1.2;
    
    result.additionalInfo["分析时间"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    result.additionalInfo["分析区域"] = roi.isValid() ? QString("(%1,%2,%3,%4)").arg(roi.x()).arg(roi.y()).arg(roi.width()).arg(roi.height()) : "全图";
    
    return result;
}

AnalysisResultItem AnalysisController::analyzeRadiometricAccuracy(const QString &imagePath, const QRect &roi)
{
    AnalysisResultItem result;
    result.methodName = "RadiometricAccuracy";
    result.description = "辐射精度（Radiometric Accuracy）";
    
    // 模拟实现
    result.numericValue = 0.8 + (rand() % 5) / 10.0;
    result.unit = "dB";
    
    // 添加详细数据
    result.additionalValues["峰值"] = result.numericValue + 0.2;
    result.additionalValues["均值"] = result.numericValue;
    result.additionalValues["标准差"] = 0.15;
    
    result.additionalInfo["分析时间"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    result.additionalInfo["分析区域"] = roi.isValid() ? QString("(%1,%2,%3,%4)").arg(roi.x()).arg(roi.y()).arg(roi.width()).arg(roi.height()) : "全图";
    
    return result;
}

AnalysisResultItem AnalysisController::analyzeRadiometricResolution(const QString &imagePath, const QRect &roi)
{
    AnalysisResultItem result;
    result.methodName = "RadiometricResolution";
    result.description = "辐射分辨率（Radiometric Resolution）";
    
    // 模拟实现
    result.numericValue = 1.2 + (rand() % 8) / 10.0;
    result.unit = "dB";
    
    // 添加详细数据
    result.additionalValues["峰值"] = result.numericValue + 0.3;
    result.additionalValues["均值"] = result.numericValue;
    result.additionalValues["标准差"] = 0.2;
    
    result.additionalInfo["分析时间"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    result.additionalInfo["分析区域"] = roi.isValid() ? QString("(%1,%2,%3,%4)").arg(roi.x()).arg(roi.y()).arg(roi.width()).arg(roi.height()) : "全图";
    
    return result;
}

AnalysisResultItem AnalysisController::analyzeENL(const QString &imagePath, const QRect &roi)
{
    AnalysisResultItem result;
    result.methodName = "ENL";
    result.description = "等效视数（Equivalent Number of Looks）";
    
    // 模拟实现
    result.numericValue = 3.0 + (rand() % 7);
    result.unit = "";
    
    // 添加详细数据
    result.additionalValues["峰值"] = result.numericValue + 1.0;
    result.additionalValues["均值"] = result.numericValue;
    result.additionalValues["标准差"] = 0.9;
    
    result.additionalInfo["分析时间"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    result.additionalInfo["分析区域"] = roi.isValid() ? QString("(%1,%2,%3,%4)").arg(roi.x()).arg(roi.y()).arg(roi.width()).arg(roi.height()) : "全图";
    
    return result;
}

} // namespace Core
} // namespace SAR 