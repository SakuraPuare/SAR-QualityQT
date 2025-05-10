# src/CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# 添加核心库
add_subdirectory(core)

# 添加界面
add_subdirectory(ui)

# 主应用程序
qt_add_executable(SAR-QualityQT
    main.cpp
)

# 链接核心库和 UI 库
target_link_libraries(SAR-QualityQT PRIVATE 
    SARCore
    SARUI
)

# 添加包含目录
target_include_directories(SAR-QualityQT PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/include
)

# 设置属性
set_target_properties(SAR-QualityQT PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.SAR-QualityQT
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
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