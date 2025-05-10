# SAR-QualityQT 项目结构说明

## 项目目录结构

项目采用模块化结构，各模块功能明确分离：

```
SAR-QualityQT
├── CMakeLists.txt              # 主CMake构建文件
├── PROJECT_STRUCTURE.md        # 本文件
├── README.md                   # 项目说明文件
├── data/                       # 数据文件目录
├── docs/                       # 文档目录
├── i18n/                       # 国际化翻译文件
├── resources/                  # 资源文件
│   ├── icons/                  # 图标资源
│   ├── images/                 # 图像资源
│   └── styles/                 # 样式表
├── src/                        # 源代码
│   ├── CMakeLists.txt          # 源代码构建配置
│   ├── main.cpp                # 程序入口
│   ├── core/                   # 核心功能模块
│   │   ├── CMakeLists.txt      # 核心模块构建配置
│   │   ├── imagehandler.cpp    # 图像处理实现
│   │   ├── include/            # 核心模块头文件
│   │   │   └── imagehandler.h  # 图像处理头文件
│   │   └── analysis/           # 分析算法模块
│   │       ├── CMakeLists.txt  # 分析模块构建配置
│   │       ├── clarity.cpp     # 清晰度分析实现
│   │       ├── clarity.h       # 清晰度分析头文件
│   │       ├── glcm.cpp        # GLCM分析实现
│   │       ├── glcm.h          # GLCM分析头文件
│   │       ├── global.cpp      # 全局分析实现
│   │       ├── global.h        # 全局分析头文件
│   │       ├── infocontent.cpp # 信息内容分析实现
│   │       ├── infocontent.h   # 信息内容分析头文件
│   │       ├── local.cpp       # 局部分析实现
│   │       ├── local.h         # 局部分析头文件
│   │       ├── radiometric.cpp # 辐射度分析实现 
│   │       ├── radiometric.h   # 辐射度分析头文件
│   │       ├── snr.cpp         # 信噪比分析实现
│   │       ├── snr.h           # 信噪比分析头文件
│   │       └── analysis_utils.h # 分析工具类头文件
│   └── ui/                     # 用户界面模块
│       ├── CMakeLists.txt      # UI模块构建配置
│       ├── mainwindow.cpp      # 主窗口实现
│       ├── mainwindow.ui       # 主窗口UI定义
│       ├── include/            # UI模块头文件
│       │   └── mainwindow.h    # 主窗口头文件
│       ├── widgets/            # 自定义控件
│       └── dialogs/            # 对话框
└── tests/                      # 测试目录
    ├── CMakeLists.txt          # 测试构建配置
    └── analysis_tests.cpp      # 分析模块测试
```

## 命名空间结构

项目使用命名空间组织代码：

```cpp
namespace SAR {
    namespace Core {
        // 图像处理等核心功能
        class ImageHandler;
    }
    
    namespace Analysis {
        // 各种分析算法
        class Clarity;
        class GLCM;
        class Global;
        class InfoContent;
        class Local;
        class Radiometric;
        class SNR;
    }
    
    namespace UI {
        // UI相关组件
        class MainWindow;
        
        namespace Widgets {
            // 自定义控件
        }
        
        namespace Dialogs {
            // 对话框
        }
    }
}
```

## 构建系统

项目使用CMake构建系统，采用模块化结构：

- 主CMakeLists.txt：定义项目、查找依赖、包含子目录
- src/CMakeLists.txt：组织主程序构建
- src/core/CMakeLists.txt：构建核心库(SARCore)
- src/core/analysis/CMakeLists.txt：构建分析库(SARAnalysis)
- src/ui/CMakeLists.txt：构建UI库(SARUI)
- tests/CMakeLists.txt：构建测试可执行文件

## 依赖关系

- SARAnalysis：依赖OpenCV、Qt6::Core
- SARCore：依赖SARAnalysis、OpenCV、GDAL、Qt6::Core、Qt6::Widgets
- SARUI：依赖SARCore、Qt6::Widgets、Qt6::PrintSupport
- SAR-QualityQT (主程序)：依赖SARCore、SARUI

## 后续开发建议

1. 使用资源文件(.qrc)管理图标和样式资源
2. 完善国际化支持，加载翻译文件
3. 逐步实现各个功能模块
4. 为每个模块编写单元测试
5. 添加新功能时，优先考虑是否应该放入现有模块或创建新模块 