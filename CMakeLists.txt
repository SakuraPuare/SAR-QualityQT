cmake_minimum_required(VERSION 3.16)

project(SAR-QualityQT VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 Qt6, OpenCV, GDAL
find_package(Qt6 REQUIRED COMPONENTS Widgets LinguistTools Core)
find_package(OpenCV REQUIRED)
find_package(GDAL CONFIG REQUIRED)

# --- 使用 GLOB 查找源文件和头文件 ---
# 查找当前目录下的 .cpp 文件
file(GLOB PROJECT_CPP_SOURCES "src/*.cpp")
# 查找当前目录下的 .h 文件
file(GLOB PROJECT_H_SOURCES "src/include/*.h")
# 查找当前目录下的 .ui 文件
file(GLOB PROJECT_UI_SOURCES "resources/*.ui")

# 查找翻译文件
file(GLOB TS_FILES "i18n/*.ts")

include_directories(src/include)

# 组合所有源文件类型 (CMake 能自动处理头文件和 UI 文件与 AUTOMOC/AUTOUIC 的关系)
set(PROJECT_SOURCES
    ${PROJECT_CPP_SOURCES}
    ${PROJECT_H_SOURCES}
    ${PROJECT_UI_SOURCES}
    )
# ---------------------------------------

# 使用 Qt6 的方式添加可执行文件和翻译
qt_add_executable(SAR-QualityQT
    MANUAL_FINALIZATION
    # 直接使用组合后的变量
    ${PROJECT_SOURCES}
)
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
