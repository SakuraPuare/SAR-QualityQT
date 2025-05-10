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
    - 积分旁瓣比 (ISLR) 分析
    - 峰值旁瓣比 (PSLR) 分析
    - 距离模糊度分析
    - 方位模糊度分析
    - 信噪比 (SNR) 分析
    - 噪声等效后向散射系数 (NESZ) 分析
    - 辐射精度分析
    - 辐射分辨率分析
    - 等效视数 (ENL) 分析
    - 灰度共生矩阵 (GLCM) 分析（可选）
    - 信息内容分析（可选）
    - 清晰度分析（可选）

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

- **可定制分析方法**：
    - 通过编译选项启用/禁用分析方法
    - 按需裁剪功能，优化性能和内存占用

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

3. 自定义分析方法（可选）：

```bash
# 例如，只启用积分旁瓣比、峰值旁瓣比和信噪比分析
cmake -DENABLE_ISLR=ON -DENABLE_PSLR=ON -DENABLE_SNR=ON -DENABLE_RANGE_RES=OFF -DENABLE_AZIMUTH_RES=OFF -DENABLE_NESZ=OFF -DENABLE_RADIOMETRIC_ACC=OFF -DENABLE_RADIOMETRIC_RES=OFF -DENABLE_ENL=OFF ..
```
更多配置选项可参考[分析方法配置指南](docs/ANALYSIS_CONFIG.md)。

4. 编译项目：

```bash
cmake --build . --config Release
```

5. 运行应用程序：

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
2. 选择适当的分析方法（积分旁瓣比、峰值旁瓣比、信噪比等）
3. 设置分析参数
4. 运行分析并查看结果
5. 导出分析结果或生成报告

## 分析方法

### SAR 图像质量评估指标

本工具支持以下主要SAR图像质量评估指标：

#### 积分旁瓣比 (ISLR)
评估SAR系统脉冲压缩性能，计算旁瓣能量与主瓣能量之比。

#### 峰值旁瓣比 (PSLR)
评估SAR系统的目标识别能力，计算最大旁瓣峰值与主瓣峰值之比。

#### 距离模糊度
评估SAR图像在距离方向上的分辨能力。

#### 方位模糊度
评估SAR图像在方位方向上的分辨能力。

#### 信噪比 (SNR) 分析
计算图像的信噪比，评估信号强度相对于噪声的质量。

#### 噪声等效后向散射系数 (NESZ)
测量SAR系统噪声水平，评估系统检测弱雷达回波的能力。

#### 辐射精度
评估SAR系统测量后向散射系数的精确度。

#### 辐射分辨率
描述系统区分不同后向散射强度的能力。

#### 等效视数 (ENL)
评估多视处理后的图像质量和噪声抑制效果。

### 可选分析指标

以下指标可通过配置选项启用：

#### 灰度共生矩阵 (GLCM) 分析
使用 GLCM 计算纹理特征，包括对比度、同质性、能量和相关性等。

#### 信息内容分析
通过熵分析和其他统计方法评估图像中包含的信息量。

#### 清晰度分析
评估图像的清晰度和分辨率。

## 配置分析方法

您可以通过编译时选项自定义启用的分析方法，详细信息请参考[分析方法配置指南](docs/ANALYSIS_CONFIG.md)。

## 贡献

欢迎贡献代码、报告问题或提出功能建议！请参阅[贡献指南](docs/CONTRIBUTING.md)了解更多信息。

## 许可证

本项目采用 [MIT 许可证](LICENSE) 发布。
