# 测试目录CMakeLists.txt

# 设置测试源文件
set(TEST_SOURCES
    analysis_tests.cpp
)

# 包含GTest模块
include(GoogleTest)

# 创建测试可执行文件
add_executable(analysis_tests ${TEST_SOURCES})

# 设置包含目录
target_include_directories(analysis_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# 链接测试库
target_link_libraries(analysis_tests PRIVATE
    SARQualityLib        # 核心库
    Qt6::Widgets         # Qt组件
    GTest::gtest         # Google Test
    GTest::gtest_main    # Google Test main函数
)

# 添加测试
add_test(NAME AnalysisTests COMMAND analysis_tests)

# 自动发现测试
gtest_discover_tests(analysis_tests) 