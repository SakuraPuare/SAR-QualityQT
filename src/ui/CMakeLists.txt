# src/ui/CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# UI模块源文件
set(UI_SOURCES
    mainwindow.cpp
    include/mainwindow.h
    mainwindow.ui
)

# 创建UI库
add_library(SARUI STATIC
    ${UI_SOURCES}
)

# 设置自动UI处理
set_target_properties(SARUI PROPERTIES
    AUTOUIC ON
    AUTOMOC ON
    AUTORCC ON
)

# 设置包含目录
target_include_directories(SARUI PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs
)

# 链接依赖库
target_link_libraries(SARUI PUBLIC
    SARCore
    Qt6::Widgets
    Qt6::PrintSupport
)

# 添加子目录（如果有）
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/widgets/CMakeLists.txt")
    add_subdirectory(widgets)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/dialogs/CMakeLists.txt")
    add_subdirectory(dialogs)
endif()

# 设置编译选项
target_compile_features(SARUI PUBLIC cxx_std_17) 