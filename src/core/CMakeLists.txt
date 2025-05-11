# src/core/CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# 查找 glog 库
find_package(glog REQUIRED)

# 包含分析模块
add_subdirectory(analysis)

# 核心库源文件
set(CORE_SOURCES
    analysis/analysis_result.cpp
    logger.cpp
    imagehandler.cpp
    imagefilters.cpp
)

set(CORE_HEADERS
    include/analysis_result.h
    include/analysis_result_item.h
    include/logger.h
    include/imagehandler.h
    imagefilters.h
)

# 创建核心库
add_library(SARCore STATIC ${CORE_SOURCES} ${CORE_HEADERS})

# 设置包含目录
target_include_directories(SARCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 链接依赖库
target_link_libraries(SARCore PUBLIC
    Qt6::Core
    Qt6::Gui
    SARAnalysis
    GDAL::GDAL
    ${OpenCV_LIBS}
    glog::glog
)

# 设置编译选项
target_compile_features(SARCore PUBLIC cxx_std_17) 