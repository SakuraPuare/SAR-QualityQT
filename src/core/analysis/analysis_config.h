#ifndef ANALYSIS_CONFIG_H
#define ANALYSIS_CONFIG_H

#include <QFlags>

namespace SAR {
namespace Analysis {

/**
 * @brief 分析方法枚举
 * 定义所有可用的分析方法
 */
enum class AnalysisMethod {
    ISLR              = 1 << 0,  // 积分旁瓣比
    PSLR              = 1 << 1,  // 峰值旁瓣比
    RangeResolution   = 1 << 2,  // 距离分辨率
    AzimuthResolution = 1 << 3,  // 方位分辨率
    RASR              = 1 << 4,  // 距离模糊度
    AASR              = 1 << 5,  // 方位模糊度
    SNR               = 1 << 6,  // 信噪比分析
    NESZ              = 1 << 7,  // 噪声等效后向散射系数
    RadiometricAcc    = 1 << 8,  // 辐射精度
    RadiometricRes    = 1 << 9,  // 辐射分辨率
    ENL               = 1 << 10, // 等效视数
    Clarity           = 1 << 11, // 清晰度分析
    GLCM              = 1 << 12, // GLCM 纹理分析
    InfoContent       = 1 << 13  // 信息内容分析
};
Q_DECLARE_FLAGS(AnalysisMethods, AnalysisMethod)
Q_DECLARE_OPERATORS_FOR_FLAGS(AnalysisMethods)

/**
 * @brief 分析模块配置类
 * 用于控制分析模块的全局配置
 */
class AnalysisConfig {
public:
    /**
     * @brief 获取默认配置实例
     * @return 配置实例引用
     */
    static AnalysisConfig& getInstance();

    /**
     * @brief 检查分析方法是否启用
     * @param method 要检查的分析方法
     * @return 是否启用
     */
    bool isMethodEnabled(AnalysisMethod method) const;

    /**
     * @brief 启用分析方法
     * @param method 要启用的分析方法
     */
    void enableMethod(AnalysisMethod method);

    /**
     * @brief 禁用分析方法
     * @param method 要禁用的分析方法
     */
    void disableMethod(AnalysisMethod method);

    /**
     * @brief 设置启用的分析方法
     * @param methods 要启用的分析方法
     */
    void setEnabledMethods(AnalysisMethods methods);

    /**
     * @brief 获取所有启用的分析方法
     * @return 启用的分析方法集合
     */
    AnalysisMethods getEnabledMethods() const;

private:
    // 私有构造函数，用于单例模式
    AnalysisConfig();
    
    // 禁止拷贝和赋值
    AnalysisConfig(const AnalysisConfig&) = delete;
    AnalysisConfig& operator=(const AnalysisConfig&) = delete;
    
    // 启用的方法集合
    AnalysisMethods enabledMethods;
};

} // namespace Analysis
} // namespace SAR

#endif // ANALYSIS_CONFIG_H 