# 源代码主要构建配置

# 设置核心库的源文件
set(CORE_SOURCES
    core/imagehandler.cpp
    core/imagehandler.h
    core/analysis/clarity.cpp
    core/analysis/clarity.h
    core/analysis/glcm.cpp
    core/analysis/glcm.h
    # 添加其他核心文件
)

# 设置UI的源文件
set(UI_SOURCES
    ui/mainwindow.cpp
    ui/mainwindow.h
    ui/mainwindow.ui
    # 添加其他UI文件
)

# 设置主程序源文件
set(MAIN_SOURCES
    main.cpp
)

# 设置所有源文件
set(PROJECT_SOURCES
    ${CORE_SOURCES}
    ${UI_SOURCES}
    ${MAIN_SOURCES}
)

# 设置包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 创建核心库
add_library(SARQualityLib STATIC
    ${CORE_SOURCES}
)

# 为核心库设置包含目录
target_include_directories(SARQualityLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接核心库的依赖
target_link_libraries(SARQualityLib PUBLIC
    Qt6::Core
    Qt6::Widgets
    GDAL::GDAL
    ${OpenCV_LIBS}
)

# 创建可执行文件
qt_add_executable(SAR-QualityQT
    MANUAL_FINALIZATION
    ${UI_SOURCES}
    ${MAIN_SOURCES}
)

# 自动处理UI文件
set_target_properties(SAR-QualityQT PROPERTIES
    AUTOUIC ON
    AUTOMOC ON
    AUTORCC ON
)

# 链接可执行文件的依赖
target_link_libraries(SAR-QualityQT PRIVATE
    SARQualityLib
    Qt6::Widgets
    Qt6::Core
    Qt6::PrintSupport
)

# 翻译文件处理
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../i18n")
    # 设置翻译文件
    set(TS_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../i18n/SAR-QualityQT_zh_CN.ts
        ${CMAKE_CURRENT_SOURCE_DIR}/../i18n/SAR-QualityQT_en.ts
    )
    
    # 添加翻译
    qt_add_translations(SAR-QualityQT
        TS_FILES ${TS_FILES}
    )
endif() 