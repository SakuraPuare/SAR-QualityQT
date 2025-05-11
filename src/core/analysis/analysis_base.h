#ifndef ANALYSIS_BASE_H
#define ANALYSIS_BASE_H

#include <opencv2/core.hpp>
#include <QString>
#include <map>
#include <QVariant>
#include "analysis_utils.h"

namespace SAR {
namespace Analysis {

/**
 * @brief 分析基类
 * 所有分析类都应继承此基类，以提供统一的接口
 */
class AnalysisBase {
public:
    AnalysisBase();
    virtual ~AnalysisBase();

    /**
     * @brief 分析结果结构
     */
    struct Result {
        double numericValue;                       // 主要数值结果
        QString unit;                              // 单位
        std::map<QString, double> additionalValues; // 额外数值结果
        std::map<QString, QString> additionalInfo;  // 额外信息
        bool isSuccess;                            // 是否成功
        QString errorMessage;                      // 错误信息
        
        // 构造函数
        Result() : numericValue(0.0), isSuccess(true) {}
    };

    /**
     * @brief 设置分析参数
     * @param key 参数名
     * @param value 参数值
     */
    void setParameter(const QString& key, const QVariant& value);
    
    /**
     * @brief 获取分析参数
     * @param key 参数名
     * @param defaultValue 默认值
     * @return 参数值
     */
    QVariant getParameter(const QString& key, const QVariant& defaultValue = QVariant()) const;
    
    /**
     * @brief 执行分析
     * @param image 输入图像
     * @return 分析结果
     */
    virtual Result analyze(const cv::Mat& image) = 0;
    
    /**
     * @brief 使用ROI执行分析
     * @param image 输入图像
     * @param roi ROI区域
     * @return 分析结果
     */
    virtual Result analyzeWithROI(const cv::Mat& image, const cv::Rect& roi) = 0;
    
    /**
     * @brief 获取分析方法名称
     * @return 方法名称
     */
    virtual QString getMethodName() const = 0;
    
    /**
     * @brief 获取分析方法描述
     * @return 方法描述
     */
    virtual QString getDescription() const = 0;

protected:
    /**
     * @brief 转换图像为32位浮点格式
     * @param image 输入图像
     * @return 转换后的图像
     */
    cv::Mat convertToFloat(const cv::Mat& image) const;

    /**
     * @brief 验证输入图像
     * @param image 输入图像
     * @return 图像是否有效
     */
    bool validateImage(const cv::Mat& image) const;

    /**
     * @brief 验证ROI区域
     * @param image 输入图像
     * @param roi ROI区域
     * @return ROI是否有效
     */
    bool validateROI(const cv::Mat& image, const cv::Rect& roi) const;

    /**
     * @brief 记录日志
     * @param message 日志信息
     * @param level 日志级别
     */
    void log(const QString& message, int level = 0) const;
    
    /**
     * @brief 记录警告日志
     * @param message 警告信息
     */
    static void warn(const QString& message);
    
    /**
     * @brief 记录错误日志
     * @param message 错误信息
     */
    static void error(const QString& message);
    
    /**
     * @brief 创建失败结果
     * @param errorMessage 错误信息
     * @return 失败的结果对象
     */
    Result createFailureResult(const QString& errorMessage) const;
    
    /**
     * @brief 创建成功结果
     * @param value 结果值
     * @param unit 单位
     * @return 成功的结果对象
     */
    Result createSuccessResult(double value, const QString& unit) const;

private:
    // 参数存储
    std::map<QString, QVariant> parameters;
};

} // namespace Analysis
} // namespace SAR

#endif // ANALYSIS_BASE_H 