# src/core/analysis/CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# 查找glog库
find_package(glog REQUIRED)

# 分析模块源文件
set(ANALYSIS_SOURCES
        # 核心文件
        analysis_result.cpp
        ../include/analysis_result.h
        analysis_controller.cpp
        ../include/analysis_controller.h
        analysis_base.cpp
        analysis_base.h
        analysis_config.cpp
        analysis_config.h
        analysis_utils.cpp
        analysis_utils.h
        analysis_factory.cpp
        analysis_factory.h
        
        # 分析方法实现
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
        islr.cpp
        islr.h
        pslr.cpp
        pslr.h
        range_resolution.cpp
        range_resolution.h
        azimuth_resolution.cpp
        azimuth_resolution.h
        nesz.cpp
        nesz.h
        rasr.cpp
        rasr.h
        aasr.cpp
        aasr.h
)

# 创建分析库
add_library(SARAnalysis STATIC
        ${ANALYSIS_SOURCES}
)

# 设置包含目录
target_include_directories(SARAnalysis PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# 链接依赖库
target_link_libraries(SARAnalysis PUBLIC
        Qt6::Core
        Qt6::Widgets
        ${OpenCV_LIBS}
        glog::glog
)

# 设置编译选项
target_compile_features(SARAnalysis PUBLIC cxx_std_17)

# 显示分析模块状态
message(STATUS "SAR 分析模块已配置...")