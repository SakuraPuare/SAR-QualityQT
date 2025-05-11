#ifndef ANALYSIS_FACTORY_H
#define ANALYSIS_FACTORY_H

#include <QString>
#include <QRect>
#include <functional>
#include <map>
#include "analysis_base.h"
#include "../include/analysis_result.h"

namespace SAR {
namespace Analysis {

/**
 * @brief 分析方法工厂类
 * 用于创建和管理各种分析方法
 */
class AnalysisFactory {
public:
    /**
     * @brief 获取单例实例
     * @return 工厂类的引用
     */
    static AnalysisFactory& getInstance();

    /**
     * @brief 分析方法创建函数类型
     */
    using AnalysisCreator = std::function<AnalysisBase*()>;
    
    /**
     * @brief 分析执行函数类型
     */
    using AnalysisExecutor = std::function<SAR::Core::AnalysisResultItem(const QString&, const QRect&)>;

    /**
     * @brief 注册分析方法创建函数
     * @param methodName 方法名称
     * @param creator 创建函数
     */
    void registerAnalysis(const QString& methodName, AnalysisCreator creator);

    /**
     * @brief 注册分析方法执行函数
     * @param methodName 方法名称
     * @param executor 执行函数
     */
    void registerExecutor(const QString& methodName, AnalysisExecutor executor);

    /**
     * @brief 创建指定分析方法的实例
     * @param methodName 方法名称
     * @return 分析方法实例（如果不存在返回nullptr）
     */
    AnalysisBase* createAnalysis(const QString& methodName);

    /**
     * @brief 执行指定的分析方法
     * @param methodName 方法名称
     * @param imagePath 图像路径
     * @param roi 感兴趣区域
     * @return 分析结果项
     */
    SAR::Core::AnalysisResultItem executeAnalysis(const QString& methodName, const QString& imagePath, const QRect& roi);

    /**
     * @brief 检查分析方法是否存在
     * @param methodName 方法名称
     * @return 是否存在
     */
    bool hasAnalysisMethod(const QString& methodName) const;

    /**
     * @brief 获取所有注册的分析方法名称
     * @return 方法名称列表
     */
    QStringList getAvailableMethods() const;

private:
    // 私有构造函数（单例模式）
    AnalysisFactory();
    
    // 删除复制构造和赋值操作
    AnalysisFactory(const AnalysisFactory&) = delete;
    AnalysisFactory& operator=(const AnalysisFactory&) = delete;
    
    // 分析方法创建函数映射
    std::map<QString, AnalysisCreator> creators;
    
    // 分析方法执行函数映射
    std::map<QString, AnalysisExecutor> executors;
};

} // namespace Analysis
} // namespace SAR

#endif // ANALYSIS_FACTORY_H 