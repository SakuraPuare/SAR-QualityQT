# SAR-QualityQT

<div align="center">
    <p>
        <b>合成孔径雷达 (SAR) 图像质量评估工具</b>
    </p>
    <p>
        <a href="#特性">特性</a> •
        <a href="#安装说明">安装说明</a> •
        <a href="#使用方法">使用方法</a> •
        <a href="#分析方法">分析方法</a> •
        <a href="#贡献">贡献</a> •
        <a href="#许可证">许可证</a>
    </p>
</div>

## 项目概述

SAR-QualityQT 是一款基于 Qt6 开发的跨平台合成孔径雷达 (SAR) 图像质量评估工具。该应用程序提供了多种质量评估指标和分析方法，用于评估
SAR 图像的质量、清晰度和信息内容。

本工具集成了先进的图像处理算法，支持多种 SAR 数据格式，并提供友好的用户界面，方便用户进行图像质量分析和对比。该软件支持多语言（中文和英文），可在
Windows、macOS 和 Linux 平台上运行。

## 特性

- **多种分析方法**：
    - 辐射度分析 (Radiometric Analysis)
    - 信噪比分析 (SNR Analysis)
    - 信息内容分析 (Information Content)
    - 清晰度分析 (Clarity Analysis)
    - 灰度共生矩阵分析 (GLCM Analysis)

- **图像处理功能**：
    - 支持多种 SAR 图像格式（通过 GDAL 库）
    - 图像预处理和增强
    - 区域选择与裁剪
    - 数据可视化

- **用户友好界面**：
    - 直观的 Qt6 界面设计
    - 交互式图像查看
    - 结果导出与报告生成

- **跨平台支持**：
    - Windows
    - macOS
    - Linux

- **国际化支持**：
    - 中文
    - 英文

## 安装说明

### 系统要求

- C++17 兼容的编译器（MSVC, GCC, Clang）
- Qt 6.9.0 或更高版本
- OpenCV 4.x
- GDAL 3.x
- CMake 3.16 或更高版本

### 从源码构建

1. 克隆仓库：

```bash
git clone https://github.com/yourusername/SAR-QualityQT.git
cd SAR-QualityQT
```

2. 使用 CMake 配置项目：

```bash
mkdir build && cd build
cmake ..
```

3. 编译项目：

```bash
cmake --build . --config Release
```

4. 运行应用程序：

```bash
./SAR-QualityQT
```

### Windows 平台特别说明

Windows 用户可以选择使用 MSVC 或 MinGW 编译器：

#### 使用 MSVC

```bash
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config Release
```

#### 使用 MinGW

```bash
cmake -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=../mingw-w64-toolchain.cmake ..
cmake --build .
```

## 使用方法

1. 启动应用程序后，可以通过"文件"菜单打开 SAR 图像文件
2. 选择适当的分析方法（辐射度、SNR、信息内容等）
3. 设置分析参数
4. 运行分析并查看结果
5. 导出分析结果或生成报告

## 分析方法

### 辐射度分析

评估图像的辐射特性，包括亮度、对比度和动态范围。

### 信噪比分析

计算图像的信噪比，评估信号强度相对于噪声的质量。

### 信息内容分析

通过熵分析和其他统计方法评估图像中包含的信息量。

### 清晰度分析

评估图像的清晰度和分辨率。

### 灰度共生矩阵分析

使用 GLCM 计算纹理特征，包括对比度、同质性、能量和相关性等。

## 贡献

欢迎贡献代码、报告问题或提出功能建议！请参阅[贡献指南](docs/CONTRIBUTING.md)了解更多信息。

## 许可证

本项目采用 [MIT 许可证](LICENSE) 发布。
