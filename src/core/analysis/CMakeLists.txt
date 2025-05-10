# src/core/analysis/CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

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