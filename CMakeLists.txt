cmake_minimum_required(VERSION 3.16)

project(SAR-QualityQT VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Conditionally add Qt path on Windows ---
if(WIN32)
  # 如果是 Windows 系统，将手动安装的 Qt 路径添加到查找路径
  # 请确保这个路径是正确的 Qt MinGW 64 位安装目录
  list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.9.0/mingw_64")
endif()
# -----------------------------------------

# 查找 Qt6, OpenCV, GDAL, GTest
find_package(Qt6 REQUIRED COMPONENTS Widgets LinguistTools Core)
find_package(OpenCV REQUIRED)
find_package(GDAL CONFIG REQUIRED)
find_package(GTest REQUIRED)

# --- 使用 GLOB 查找源文件和头文件 ---
# 查找当前目录下的 .cpp 文件
file(GLOB PROJECT_CPP_SOURCES "src/*.cpp")
# 查找当前目录下的 .h 文件
file(GLOB PROJECT_H_SOURCES "src/include/*.h")

# 查找翻译文件
# file(GLOB TS_FILES "i18n/*.ts") # 不再使用 GLOB，改为明确指定
set(TS_FILES
    i18n/SAR-QualityQT_zh_CN.ts # 目标语言：中文
    i18n/SAR-QualityQT_en.ts    # 源语言/基础语言：英文
)

include_directories(src/include)

# 组合所有源文件类型 (CMake 能自动处理头文件和 UI 文件与 AUTOMOC/AUTOUIC 的关系)
set(PROJECT_SOURCES
    ${PROJECT_CPP_SOURCES}
    ${PROJECT_H_SOURCES}
    )
# ---------------------------------------

# 使用 Qt6 的方式添加可执行文件和翻译
qt_add_executable(SAR-QualityQT
    MANUAL_FINALIZATION
    # 直接使用组合后的变量
    ${PROJECT_SOURCES}
)
# 确保 TS_FILES 变量包含所有需要的 .ts 文件路径
qt_add_translations(SAR-QualityQT TS_FILES ${TS_FILES})

# Define target properties for Android with Qt 6 as:
#    set_property(TARGET SAR-QualityQT APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

target_link_libraries(SAR-QualityQT PRIVATE Qt6::Widgets Qt6::Core GDAL::GDAL ${OpenCV_LIBS})

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# 如果你的目标 Qt 版本保证是 6.1 或更高，可以移除这个 if 判断
# set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.SAR-QualityQT)
# if(${QT_VERSION} VERSION_LESS 6.1.0) # 移除此条件判断
#   set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.SAR-QualityQT)
# endif()
set_target_properties(SAR-QualityQT PROPERTIES
#    ${BUNDLE_ID_OPTION} # 可以直接设置，或者如果不需要兼容老版本 macOS/iOS，可以移除
    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.SAR-QualityQT # 直接设置 Bundle ID
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS SAR-QualityQT
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# 始终为 Qt6 调用 finalize
qt_finalize_executable(SAR-QualityQT)

# 添加一个自定义目标来运行 lupdate
add_custom_target(update_translations ALL
    # 确保 -ts 参数后面跟着所有需要更新的 .ts 文件
    COMMAND lupdate -extensions cpp,h,ui ${PROJECT_SOURCES} -ts ${TS_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Updating translation source files (.ts)..."
    VERBATIM
)

# --- 配置测试 ---
enable_testing() # <--- 启用 CTest

# 添加测试可执行文件
add_executable(analysis_tests ${TEST_SOURCES})

# 链接测试可执行文件
target_link_libraries(analysis_tests PRIVATE
    SARQualityLib        # 链接核心库以测试其功能
    Qt6::Widgets         # 测试代码中的 Test Fixture 使用了 QApplication
    GTest::gtest         # Google Test 库
    GTest::gtest_main    # Google Test main 函数
    # GDAL 和 OpenCV 由 SARQualityLib 间接链接
)

include(GoogleTest) # <--- 包含 GoogleTest 模块

# 1. 明确将测试可执行文件添加为 CTest 测试
#    CTest 将在构建目录下查找并运行 'analysis_tests'
add_test(NAME AnalysisTestsCommand COMMAND analysis_tests)

# 2. 告诉 gtest_discover_tests 去检查哪个 CTest 测试 (通过 COMMAND 匹配)
#    它将运行 'analysis_tests --gtest_list_tests' 来发现具体的测试用例
#    并将它们添加到 CTest 中。
gtest_discover_tests(analysis_tests)

# --- 创建核心静态库 ---
add_library(SARQualityLib STATIC
    ${CORE_LIB_CPP_SOURCES}
    ${CORE_LIB_H_SOURCES}
)

# 为核心库设置包含目录 (PUBLIC 以便链接它的目标也能访问)
target_include_directories(SARQualityLib PUBLIC src/include)

# 链接核心库的依赖 (使用 PUBLIC)
target_link_libraries(SARQualityLib PUBLIC # <--- Changed from PRIVATE to PUBLIC
    Qt6::Core       # ImageHandler 和 analysis* 使用 QString, QCoreApplication
    GDAL::GDAL      # ImageHandler 使用
    ${OpenCV_LIBS}  # ImageHandler 和 analysis* 使用
)
# -------------------------
