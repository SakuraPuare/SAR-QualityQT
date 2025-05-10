#ifndef ANALYSIS_H
#define ANALYSIS_H

// 包含所有分析相关的头文件
#include "analysis_utils.h"
#include "analysis_config.h"

// 信噪比分析
#include "snr.h"

// 清晰度分析
#include "clarity.h"

// 信息内容分析
#include "infocontent.h"

// 辐射度分析
#include "radiometric.h"

// GLCM 纹理分析
#include "glcm.h"

// 积分旁瓣比分析
#include "islr.h"

// 峰值旁瓣比分析
#include "pslr.h"

// 距离分辨率分析
#include "range_resolution.h"

// 方位分辨率分析
#include "azimuth_resolution.h"

// 噪声等效后向散射系数分析
#include "nesz.h"

// 全局和局部分析类
#include "global.h"
#include "local.h"

// 命名空间简写（可选）
namespace SAR {
    // 使分析模块更容易访问
    using namespace Analysis;
}

#endif // ANALYSIS_H 