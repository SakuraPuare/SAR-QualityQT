# SAR图像分析方法配置指南

本文档提供了如何配置SAR-QualityQT中启用的分析方法的说明。通过这些配置选项，您可以选择性地启用或禁用特定的分析方法，以满足您的特定需求。

## 保留的分析方法

以下是当前支持的主要分析方法：

1. 积分旁瓣比 (ISLR)
2. 峰值旁瓣比 (PSLR)
3. 距离模糊度
4. 方位模糊度
5. 信噪比分析 (SNR)
6. 噪声等效后向散射系数 (NESZ)
7. 辐射精度
8. 辐射分辨率
9. 等效视数 (ENL)

## 配置方法

有两种方式可以配置启用的分析方法：

### 1. 通过CMake编译选项

最简单的方法是在编译时通过CMake选项来控制启用的分析方法。

#### 命令行方式

```bash
# 例如，只启用积分旁瓣比、峰值旁瓣比和信噪比分析
cmake -DENABLE_ISLR=ON -DENABLE_PSLR=ON -DENABLE_RANGE_RES=OFF -DENABLE_AZIMUTH_RES=OFF -DENABLE_SNR=ON -DENABLE_NESZ=OFF -DENABLE_RADIOMETRIC_ACC=OFF -DENABLE_RADIOMETRIC_RES=OFF -DENABLE_ENL=OFF ..
```

#### 在Qt Creator中设置

1. 打开项目设置
2. 转到"构建设置"
3. 在CMAKE_ARGS中添加所需的选项：
   ```
   -DENABLE_ISLR=ON
   -DENABLE_PSLR=ON
   -DENABLE_SNR=ON
   -DENABLE_RANGE_RES=OFF
   # 其他选项...
   ```

### 2. 直接修改源代码

也可以通过修改源代码来永久性地配置启用的分析方法：

1. 打开 `src/core/analysis/analysis_config.h`
2. 修改各个分析方法的宏定义值（1为启用，0为禁用）：

```cpp
#ifndef CONFIG_ENABLE_ISLR
#define CONFIG_ENABLE_ISLR 1           // 积分旁瓣比
#endif

#ifndef CONFIG_ENABLE_PSLR
#define CONFIG_ENABLE_PSLR 0           // 峰值旁瓣比，设为0表示禁用
#endif

// 其他分析方法的配置...
```

## 可用的配置选项

以下是所有可用的配置选项及其含义：

| CMake选项                | 配置宏                         | 描述                      |
|-------------------------|--------------------------------|---------------------------|
| ENABLE_ISLR             | CONFIG_ENABLE_ISLR             | 积分旁瓣比分析              |
| ENABLE_PSLR             | CONFIG_ENABLE_PSLR             | 峰值旁瓣比分析              |
| ENABLE_RANGE_RES        | CONFIG_ENABLE_RANGE_RES        | 距离模糊度分析              |
| ENABLE_AZIMUTH_RES      | CONFIG_ENABLE_AZIMUTH_RES      | 方位模糊度分析              |
| ENABLE_SNR              | CONFIG_ENABLE_SNR              | 信噪比分析                 |
| ENABLE_NESZ             | CONFIG_ENABLE_NESZ             | 噪声等效后向散射系数分析     |
| ENABLE_RADIOMETRIC_ACC  | CONFIG_ENABLE_RADIOMETRIC_ACC  | 辐射精度分析               |
| ENABLE_RADIOMETRIC_RES  | CONFIG_ENABLE_RADIOMETRIC_RES  | 辐射分辨率分析              |
| ENABLE_ENL              | CONFIG_ENABLE_ENL              | 等效视数分析               |
| ENABLE_CLARITY          | CONFIG_ENABLE_CLARITY          | 清晰度分析（默认禁用）       |
| ENABLE_GLCM             | CONFIG_ENABLE_GLCM             | GLCM纹理分析（默认禁用）     |
| ENABLE_INFO_CONTENT     | CONFIG_ENABLE_INFO_CONTENT     | 信息内容分析（默认禁用）     |

## 效果

配置后的效果：

1. 界面上只会显示已启用的分析方法选项
2. 代码中只会编译和链接已启用的分析方法相关代码
3. 未启用的分析方法在运行时不可用

## 注意事项

- 修改配置后需要重新编译整个项目才能生效
- 至少需要保留一种分析方法，否则程序将无法正常工作
- 如果禁用了所有分析方法，界面上的分析功能将无法使用

## 示例：创建精简版本

如果您只需要基本的SAR图像质量分析功能，可以使用以下配置创建一个精简版本：

```bash
cmake -DENABLE_ISLR=ON -DENABLE_PSLR=ON -DENABLE_SNR=ON -DENABLE_RANGE_RES=OFF -DENABLE_AZIMUTH_RES=OFF -DENABLE_NESZ=OFF -DENABLE_RADIOMETRIC_ACC=OFF -DENABLE_RADIOMETRIC_RES=OFF -DENABLE_ENL=OFF -DENABLE_CLARITY=OFF -DENABLE_GLCM=OFF -DENABLE_INFO_CONTENT=OFF ..
```

这将创建一个只包含积分旁瓣比、峰值旁瓣比和信噪比分析的精简版本。 