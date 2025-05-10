# src/core/analysis/CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# 添加分析方法配置选项
option(ENABLE_ISLR "启用积分旁瓣比分析" ON)
option(ENABLE_PSLR "启用峰值旁瓣比分析" ON)
option(ENABLE_RANGE_RES "启用距离模糊度分析" ON)
option(ENABLE_AZIMUTH_RES "启用方位模糊度分析" ON)
option(ENABLE_SNR "启用信噪比分析" ON)
option(ENABLE_NESZ "启用噪声等效后向散射系数分析" ON)
option(ENABLE_RADIOMETRIC_ACC "启用辐射精度分析" ON)
option(ENABLE_RADIOMETRIC_RES "启用辐射分辨率分析" ON)
option(ENABLE_ENL "启用等效视数分析" ON)
option(ENABLE_CLARITY "启用清晰度分析" OFF)
option(ENABLE_GLCM "启用 GLCM 纹理分析" OFF)
option(ENABLE_INFO_CONTENT "启用信息内容分析" OFF)

# 根据选项设置编译定义
if(ENABLE_ISLR)
    add_compile_definitions(CONFIG_ENABLE_ISLR=1)
else()
    add_compile_definitions(CONFIG_ENABLE_ISLR=0)
endif()

if(ENABLE_PSLR)
    add_compile_definitions(CONFIG_ENABLE_PSLR=1)
else()
    add_compile_definitions(CONFIG_ENABLE_PSLR=0)
endif()

if(ENABLE_RANGE_RES)
    add_compile_definitions(CONFIG_ENABLE_RANGE_RES=1)
else()
    add_compile_definitions(CONFIG_ENABLE_RANGE_RES=0)
endif()

if(ENABLE_AZIMUTH_RES)
    add_compile_definitions(CONFIG_ENABLE_AZIMUTH_RES=1)
else()
    add_compile_definitions(CONFIG_ENABLE_AZIMUTH_RES=0)
endif()

if(ENABLE_SNR)
    add_compile_definitions(CONFIG_ENABLE_SNR=1)
else()
    add_compile_definitions(CONFIG_ENABLE_SNR=0)
endif()

if(ENABLE_NESZ)
    add_compile_definitions(CONFIG_ENABLE_NESZ=1)
else()
    add_compile_definitions(CONFIG_ENABLE_NESZ=0)
endif()

if(ENABLE_RADIOMETRIC_ACC)
    add_compile_definitions(CONFIG_ENABLE_RADIOMETRIC_ACC=1)
else()
    add_compile_definitions(CONFIG_ENABLE_RADIOMETRIC_ACC=0)
endif()

if(ENABLE_RADIOMETRIC_RES)
    add_compile_definitions(CONFIG_ENABLE_RADIOMETRIC_RES=1)
else()
    add_compile_definitions(CONFIG_ENABLE_RADIOMETRIC_RES=0)
endif()

if(ENABLE_ENL)
    add_compile_definitions(CONFIG_ENABLE_ENL=1)
else()
    add_compile_definitions(CONFIG_ENABLE_ENL=0)
endif()

if(ENABLE_CLARITY)
    add_compile_definitions(CONFIG_ENABLE_CLARITY=1)
else()
    add_compile_definitions(CONFIG_ENABLE_CLARITY=0)
endif()

if(ENABLE_GLCM)
    add_compile_definitions(CONFIG_ENABLE_GLCM=1)
else()
    add_compile_definitions(CONFIG_ENABLE_GLCM=0)
endif()

if(ENABLE_INFO_CONTENT)
    add_compile_definitions(CONFIG_ENABLE_INFO_CONTENT=1)
else()
    add_compile_definitions(CONFIG_ENABLE_INFO_CONTENT=0)
endif()

# 分析模块源文件
set(ANALYSIS_SOURCES
        clarity.cpp
        clarity.h
        glcm.cpp
        glcm.h
        global.cpp
        global.h
        infocontent.cpp
        infocontent.h
        local.cpp
        local.h
        radiometric.cpp
        radiometric.h
        snr.cpp
        snr.h
        analysis_utils.h
        analysis_config.h
)

# 创建分析库
add_library(SARAnalysis STATIC
        ${ANALYSIS_SOURCES}
)

# 设置包含目录
target_include_directories(SARAnalysis PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接依赖库
target_link_libraries(SARAnalysis PUBLIC
        Qt6::Core
        ${OpenCV_LIBS}
)

# 设置编译选项
target_compile_features(SARAnalysis PUBLIC cxx_std_17) 