#ifndef ANALYSIS_CONFIG_H
#define ANALYSIS_CONFIG_H

namespace SAR {
namespace Analysis {

/**
 * @brief 分析方法配置
 * 控制启用/禁用各种分析方法
 * 可以通过修改此处的定义或通过CMake参数控制
 */

// 启用所有分析方法
#ifndef CONFIG_ENABLE_ISLR
#define CONFIG_ENABLE_ISLR 1           // 积分旁瓣比
#endif

#ifndef CONFIG_ENABLE_PSLR
#define CONFIG_ENABLE_PSLR 1           // 峰值旁瓣比
#endif

#ifndef CONFIG_ENABLE_RANGE_RES
#define CONFIG_ENABLE_RANGE_RES 1      // 距离分辨率
#endif

#ifndef CONFIG_ENABLE_AZIMUTH_RES
#define CONFIG_ENABLE_AZIMUTH_RES 1    // 方位分辨率
#endif

#ifndef CONFIG_ENABLE_RASR
#define CONFIG_ENABLE_RASR 1           // 距离模糊度
#endif

#ifndef CONFIG_ENABLE_AASR
#define CONFIG_ENABLE_AASR 1           // 方位模糊度
#endif

#ifndef CONFIG_ENABLE_SNR
#define CONFIG_ENABLE_SNR 1            // 信噪比分析
#endif

#ifndef CONFIG_ENABLE_NESZ
#define CONFIG_ENABLE_NESZ 1           // 噪声等效后向散射系数
#endif

#ifndef CONFIG_ENABLE_RADIOMETRIC_ACC
#define CONFIG_ENABLE_RADIOMETRIC_ACC 1 // 辐射精度
#endif

#ifndef CONFIG_ENABLE_RADIOMETRIC_RES
#define CONFIG_ENABLE_RADIOMETRIC_RES 1 // 辐射分辨率
#endif

#ifndef CONFIG_ENABLE_ENL
#define CONFIG_ENABLE_ENL 1            // 等效视数
#endif

// 启用其他分析方法
#ifndef CONFIG_ENABLE_CLARITY
#define CONFIG_ENABLE_CLARITY 1        // 清晰度分析
#endif

#ifndef CONFIG_ENABLE_GLCM
#define CONFIG_ENABLE_GLCM 1           // GLCM 纹理分析
#endif

#ifndef CONFIG_ENABLE_INFO_CONTENT
#define CONFIG_ENABLE_INFO_CONTENT 1   // 信息内容分析
#endif

} // namespace Analysis
} // namespace SAR

#endif // ANALYSIS_CONFIG_H 