cmake_minimum_required(VERSION 3.16)

project(SAR-QualityQT VERSION 0.1 LANGUAGES CXX)

# 设置 CMake 策略
if (POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
endif ()

# 全局设置
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置模块化功能编译选项
option(CONFIG_ENABLE_ISLR "启用ISLR分析" ON)
option(CONFIG_ENABLE_PSLR "启用PSLR分析" ON)
option(CONFIG_ENABLE_RANGE_RES "启用距离分辨率分析" ON)
option(CONFIG_ENABLE_AZIMUTH_RES "启用方位分辨率分析" ON)
option(CONFIG_ENABLE_RASR "启用RASR分析" ON)
option(CONFIG_ENABLE_AASR "启用AASR分析" ON)
option(CONFIG_ENABLE_SNR "启用SNR分析" ON)
option(CONFIG_ENABLE_NESZ "启用NESZ分析" ON)
option(CONFIG_ENABLE_RADIOMETRIC_ACC "启用辐射精度分析" ON)
option(CONFIG_ENABLE_RADIOMETRIC_RES "启用辐射分辨率分析" ON)
option(CONFIG_ENABLE_ENL "启用ENL分析" ON)

# 为所有目标添加编译定义
add_compile_definitions(
        $<$<BOOL:${CONFIG_ENABLE_ISLR}>:CONFIG_ENABLE_ISLR>
        $<$<BOOL:${CONFIG_ENABLE_PSLR}>:CONFIG_ENABLE_PSLR>
        $<$<BOOL:${CONFIG_ENABLE_RANGE_RES}>:CONFIG_ENABLE_RANGE_RES>
        $<$<BOOL:${CONFIG_ENABLE_AZIMUTH_RES}>:CONFIG_ENABLE_AZIMUTH_RES>
        $<$<BOOL:${CONFIG_ENABLE_RASR}>:CONFIG_ENABLE_RASR>
        $<$<BOOL:${CONFIG_ENABLE_AASR}>:CONFIG_ENABLE_AASR>
        $<$<BOOL:${CONFIG_ENABLE_SNR}>:CONFIG_ENABLE_SNR>
        $<$<BOOL:${CONFIG_ENABLE_NESZ}>:CONFIG_ENABLE_NESZ>
        $<$<BOOL:${CONFIG_ENABLE_RADIOMETRIC_ACC}>:CONFIG_ENABLE_RADIOMETRIC_ACC>
        $<$<BOOL:${CONFIG_ENABLE_RADIOMETRIC_RES}>:CONFIG_ENABLE_RADIOMETRIC_RES>
        $<$<BOOL:${CONFIG_ENABLE_ENL}>:CONFIG_ENABLE_ENL>
)

# 条件添加 Qt 路径
if (WIN32)
    set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake")
    if (MSVC)
        # Windows + MSVC
        list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.9.0/msvc2022_64")
        add_compile_options(/Zc:__cplusplus)
    else ()
        # Windows + MinGW
        list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.9.0/mingw_64")
    endif ()
endif ()

# 查找 Qt 组件
find_package(Qt6 REQUIRED COMPONENTS Widgets LinguistTools Core PrintSupport)

# 查找 OpenCV
find_package(OpenCV REQUIRED)
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")

# 查找 GDAL
find_package(GDAL CONFIG REQUIRED)
message(STATUS "GDAL version: ${GDAL_VERSION}")

# 查找 GTest
find_package(GTest REQUIRED)

# 创建资源文件目录
if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/resources")
endif ()

# 添加子目录
add_subdirectory(src)

# 启用测试
enable_testing()
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    add_subdirectory(tests)
endif ()

# 安装配置
include(GNUInstallDirs)
install(TARGETS SAR-QualityQT
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
