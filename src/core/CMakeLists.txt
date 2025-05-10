# src/core/CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# 包含分析模块
add_subdirectory(analysis)

# 核心库源文件
set(CORE_SOURCES
    imagehandler.cpp
    include/imagehandler.h
)

# 创建核心库
add_library(SARCore STATIC
    ${CORE_SOURCES}
)

# 设置包含目录
target_include_directories(SARCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 链接依赖库
target_link_libraries(SARCore PUBLIC
    SARAnalysis
    Qt6::Core
    Qt6::Widgets
    GDAL::GDAL
    ${OpenCV_LIBS}
)

# 设置编译选项
target_compile_features(SARCore PUBLIC cxx_std_17) 