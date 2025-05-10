cmake_minimum_required(VERSION 3.16)

project(SAR-QualityQT VERSION 0.1 LANGUAGES CXX)

# 设置 CMake 策略
if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
endif()

# 全局设置
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 条件添加 Qt 路径
if(WIN32)
    if(MSVC)
        # Windows + MSVC
        list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.9.0/msvc2022_64")
        add_compile_options(/Zc:__cplusplus)
    else()
        # Windows + MinGW
        list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.9.0/mingw_64")
    endif()
endif()

# 查找 Qt 组件
find_package(Qt6 REQUIRED COMPONENTS Widgets LinguistTools Core PrintSupport)

# 查找 OpenCV
find_package(OpenCV REQUIRED)

# 查找 GDAL
find_package(GDAL CONFIG REQUIRED)

# 查找 GTest
find_package(GTest REQUIRED)

# 创建资源文件目录
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/resources")
endif()

# 添加子目录
add_subdirectory(src)

# 启用测试
enable_testing()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
  add_subdirectory(tests)
endif()

# 安装配置
include(GNUInstallDirs)
install(TARGETS SAR-QualityQT
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)