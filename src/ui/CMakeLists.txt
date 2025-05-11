# src/ui/CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../core/include
)

# UI 模块源文件
set(UI_SOURCES
        # 主窗口
        mainwindow.cpp
        include/mainwindow.h
        mainwindow.ui

        # UI设置模块
        ui_setup.cpp
        ui_setup.h

        # 图像处理模块
        image_handler.cpp
        image_handler.h

        # 拖放处理模块
        drag_drop_handler.cpp
        drag_drop_handler.h

        # 分析控制器模块
        analysis_controller.h
        analysis_controller.cpp

        # 报告生成器模块
        report_generator.h
        report_generator.cpp

        # 自定义图形视图
        include/drag_drop_graphics_view.h
)

# 创建 UI 库
add_library(SARUI STATIC
        ${UI_SOURCES}
)

# 设置自动 UI 处理
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
        ${CMAKE_CURRENT_SOURCE_DIR}/../core/include
        ${OpenCV_INCLUDE_DIRS}
)

# 链接依赖库
target_link_libraries(SARUI PUBLIC
        SARCore
        SARAnalysis
        Qt6::Widgets
        Qt6::PrintSupport
        ${OpenCV_LIBS}
)

# 添加子目录（如果有）
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/widgets/CMakeLists.txt")
    add_subdirectory(widgets)
endif ()

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/dialogs/CMakeLists.txt")
    add_subdirectory(dialogs)
endif ()

# 设置编译选项
target_compile_features(SARUI PUBLIC cxx_std_17) 