cmake_minimum_required(VERSION 3.16)

# 包含 GTest
find_package(GTest REQUIRED)
include(GoogleTest)

# SNR 分析测试
add_executable(analysis_tests
        analysis_tests.cpp
)

target_link_libraries(analysis_tests
        PRIVATE
        SARAnalysis
        SARCore
        GTest::GTest
        GTest::Main
        ${OpenCV_LIBS}
)

# 图像处理测试
add_executable(core_tests
        core_tests.cpp
)

target_link_libraries(core_tests
        PRIVATE
        SARCore
        GTest::GTest
        GTest::Main
        ${OpenCV_LIBS}
        GDAL::GDAL
)

# 添加测试
gtest_discover_tests(analysis_tests)
gtest_discover_tests(core_tests)