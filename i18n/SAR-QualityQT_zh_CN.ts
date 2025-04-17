<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_CN">
<context>
    <name>Analysis</name>
    <message>
        <location filename="../src/analysis_clarity.cpp" line="12"/>
        <source>Clarity (Gradient Magnitude)</source>
        <translation>清晰度 (梯度幅值)</translation>
    </message>
    <message>
        <location filename="../src/analysis_clarity.cpp" line="13"/>
        <source>Clarity Analysis (Average Gradient Magnitude):
</source>
        <translation>清晰度分析 (平均梯度幅值):
</translation>
    </message>
    <message>
        <location filename="../src/analysis_clarity.cpp" line="16"/>
        <source>Clarity (GradMag): </source>
        <translation>清晰度 (梯度幅值): </translation>
    </message>
    <message>
        <location filename="../src/analysis_clarity.cpp" line="19"/>
        <location filename="../src/analysis_glcm.cpp" line="19"/>
        <location filename="../src/analysis_radiometric.cpp" line="19"/>
        <location filename="../src/analysis_snr.cpp" line="22"/>
        <source>
Error: No valid image data provided.</source>
        <translation>
错误：未提供有效的图像数据。</translation>
    </message>
    <message>
        <location filename="../src/analysis_clarity.cpp" line="23"/>
        <location filename="../src/analysis_glcm.cpp" line="23"/>
        <location filename="../src/analysis_infocontent.cpp" line="24"/>
        <location filename="../src/analysis_radiometric.cpp" line="23"/>
        <location filename="../src/analysis_snr.cpp" line="26"/>
        <source>Error - No Data</source>
        <translation>错误 - 无数据</translation>
    </message>
    <message>
        <location filename="../src/analysis_clarity.cpp" line="31"/>
        <source>Preparing 8-bit grayscale image...
</source>
        <translation>正在准备 8 位灰度图像...
</translation>
    </message>
    <message>
        <location filename="../src/analysis_clarity.cpp" line="37"/>
        <source>
Error: Failed to prepare 8-bit single channel image for gradient calculation.</source>
        <translation>
错误：无法为梯度计算准备 8 位单通道图像。</translation>
    </message>
    <message>
        <location filename="../src/analysis_clarity.cpp" line="42"/>
        <location filename="../src/analysis_glcm.cpp" line="42"/>
        <location filename="../src/analysis_infocontent.cpp" line="121"/>
        <location filename="../src/analysis_radiometric.cpp" line="77"/>
        <location filename="../src/analysis_snr.cpp" line="90"/>
        <source>Error - Prep Failed</source>
        <translation>错误 - 准备失败</translation>
    </message>
    <message>
        <location filename="../src/analysis_clarity.cpp" line="48"/>
        <source>Calculating gradients using Sobel operator...
</source>
        <translation>正在使用 Sobel 算子计算梯度...
</translation>
    </message>
    <message>
        <location filename="../src/analysis_clarity.cpp" line="62"/>
        <source>Calculating average gradient magnitude...
</source>
        <translation>正在计算平均梯度幅值...
</translation>
    </message>
    <message>
        <location filename="../src/analysis_clarity.cpp" line="67"/>
        <source>
--- Gradient Analysis Results ---
</source>
        <translation>
--- 梯度分析结果 ---
</translation>
    </message>
    <message>
        <location filename="../src/analysis_clarity.cpp" line="69"/>
        <source>Average Gradient Magnitude: %1
</source>
        <translation>平均梯度幅值：%1
</translation>
    </message>
    <message>
        <location filename="../src/analysis_clarity.cpp" line="72"/>
        <source>
Interpretation: Higher values generally indicate sharper details or more texture/noise.</source>
        <translation>
解释：值越高通常表示细节更清晰或纹理/噪声更多。</translation>
    </message>
    <message>
        <location filename="../src/analysis_clarity.cpp" line="78"/>
        <source>%1</source>
        <translation>%1</translation>
    </message>
    <message>
        <location filename="../src/analysis_clarity.cpp" line="80"/>
        <source>Internal Log: Clarity analysis completed: %1
</source>
        <translation>内部日志：清晰度分析完成：%1
</translation>
    </message>
    <message>
        <location filename="../src/analysis_clarity.cpp" line="87"/>
        <source>
Error during gradient calculation: %1</source>
        <translation>
梯度计算过程中出错：%1</translation>
    </message>
    <message>
        <location filename="../src/analysis_clarity.cpp" line="92"/>
        <location filename="../src/analysis_glcm.cpp" line="119"/>
        <location filename="../src/analysis_infocontent.cpp" line="225"/>
        <location filename="../src/analysis_radiometric.cpp" line="149"/>
        <location filename="../src/analysis_snr.cpp" line="169"/>
        <source>Error - Calculation Failed</source>
        <translation>错误 - 计算失败</translation>
    </message>
    <message>
        <location filename="../src/analysis_clarity.cpp" line="94"/>
        <source>Internal Log: OpenCV Error during Clarity (Gradient) calculation: %1
</source>
        <translation>内部日志：清晰度 (梯度) 计算过程中发生 OpenCV 错误：%1
</translation>
    </message>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="13"/>
        <source>GLCM Texture</source>
        <translation>GLCM 纹理</translation>
    </message>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="15"/>
        <source>GLCM Analysis Results:
</source>
        <translation>GLCM 分析结果：
</translation>
    </message>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="16"/>
        <source>GLCM: </source>
        <translation>GLCM: </translation>
    </message>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="30"/>
        <source>Preparing 8-bit grayscale image for GLCM...
</source>
        <translation>正在为 GLCM 准备 8 位灰度图像...
</translation>
    </message>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="37"/>
        <source>
Error: Failed to prepare 8-bit single channel image required for GLCM.</source>
        <translation>
错误：无法准备 GLCM 所需的 8 位单通道图像。</translation>
    </message>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="52"/>
        <source>
Calculating GLCM with offset (dx=%1, dy=%2), levels=%3, symmetric=true, normalized=true
</source>
        <translation>
正在计算 GLCM (偏移量 dx=%1, dy=%2), 灰度级=%3, 对称=true, 归一化=true
</translation>
    </message>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="75"/>
        <source>
--- Texture Features (Offset dx=%1, dy=%2) ---
</source>
        <translation>
--- 纹理特征 (偏移量 dx=%1, dy=%2) ---
</translation>
    </message>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="80"/>
        <source>Contrast: %1
</source>
        <translation>对比度：%1
</translation>
    </message>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="82"/>
        <source>Correlation: %1
</source>
        <translation>相关性：%1
</translation>
    </message>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="85"/>
        <source>Energy (ASM): %1
</source>
        <translation>能量 (角二阶矩): %1
</translation>
    </message>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="88"/>
        <source>Homogeneity (IDM): %1
</source>
        <translation>同质性 (逆差距): %1
</translation>
    </message>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="94"/>
        <source>Contr=%1, Corr=%2, Ener=%3, Homo=%4</source>
        <translation>对比=%1, 相关=%2, 能量=%3, 同质=%4</translation>
    </message>
    <message>
        <source>Contr=%.3f, Corr=%.3f, Ener=%.3f, Homo=%.3f</source>
        <translation type="vanished">对比=%.3f, 相关=%.3f, 能量=%.3f, 同质=%.3f</translation>
    </message>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="102"/>
        <source>Internal Log: GLCM features calculated: Contrast=%1, Correlation=%2, Energy=%3, Homogeneity=%4
</source>
        <translation>内部日志：GLCM 特征已计算：对比度=%1, 相关性=%2, 能量=%3, 同质性=%4
</translation>
    </message>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="113"/>
        <source>
Error during GLCM calculation or feature extraction: %1</source>
        <translation>
GLCM 计算或特征提取过程中出错：%1</translation>
    </message>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="121"/>
        <source>Internal Log: OpenCV Error during GLCM analysis: %1
</source>
        <translation>内部日志：GLCM 分析过程中发生 OpenCV 错误：%1
</translation>
    </message>
    <message>
        <location filename="../src/analysis_infocontent.cpp" line="14"/>
        <source>Information Content (Entropy)</source>
        <translation>信息量 (熵)</translation>
    </message>
    <message>
        <location filename="../src/analysis_infocontent.cpp" line="16"/>
        <source>Info Content (Entropy): </source>
        <translation>信息量 (熵): </translation>
    </message>
    <message>
        <location filename="../src/analysis_infocontent.cpp" line="19"/>
        <source>Error: No valid image data provided for entropy analysis.</source>
        <translation>错误：未为熵分析提供有效的图像数据。</translation>
    </message>
    <message>
        <location filename="../src/analysis_infocontent.cpp" line="33"/>
        <source>Preparing single-channel image for entropy...
</source>
        <translation>正在为熵计算准备单通道图像...
</translation>
    </message>
    <message>
        <location filename="../src/analysis_infocontent.cpp" line="36"/>
        <location filename="../src/analysis_snr.cpp" line="38"/>
        <source>Input is complex (2-channel), calculating magnitude.
</source>
        <translation>输入为复数数据 (2 通道)，正在计算幅度。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_infocontent.cpp" line="41"/>
        <location filename="../src/analysis_radiometric.cpp" line="44"/>
        <source>Converting complex channels to CV_32F for magnitude.
</source>
        <translation>正在将复数通道转换为 CV_32F 以计算幅度。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_infocontent.cpp" line="47"/>
        <source>Calculated magnitude from complex data.
</source>
        <translation>已从复数数据计算幅度。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_infocontent.cpp" line="50"/>
        <source>Input is multi-channel (%1), converting to grayscale.
</source>
        <translation>输入为多通道 (%1)，正在转换为灰度图。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_infocontent.cpp" line="57"/>
        <source>Input depth not directly supported by cvtColor, converting to CV_8U first.
</source>
        <translation>cvtColor 不直接支持输入深度，先转换为 CV_8U。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_infocontent.cpp" line="75"/>
        <source>Converted multi-channel to grayscale using standard conversion.
</source>
        <translation>已使用标准转换将多通道转换为灰度图。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_infocontent.cpp" line="79"/>
        <source>Standard grayscale conversion failed (%1), falling back to first channel.
</source>
        <translation>标准灰度转换失败 (%1)，回退到使用第一个通道。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_infocontent.cpp" line="86"/>
        <location filename="../src/analysis_infocontent.cpp" line="98"/>
        <source>Used the first channel.
</source>
        <translation>已使用第一个通道。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_infocontent.cpp" line="90"/>
        <source>Input has %1 channels, not standard BGR/BGRA. Using first channel.
</source>
        <translation>输入包含 %1 个通道，不是标准的 BGR/BGRA 格式。使用第一个通道。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_infocontent.cpp" line="101"/>
        <source>Input is already single-channel.
</source>
        <translation>输入已是单通道。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_infocontent.cpp" line="106"/>
        <source>Error: Input image has 0 channels.</source>
        <translation>错误：输入图像通道数为 0。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_infocontent.cpp" line="110"/>
        <source>Error - Invalid Channels</source>
        <translation>错误 - 无效通道</translation>
    </message>
    <message>
        <location filename="../src/analysis_infocontent.cpp" line="117"/>
        <source>
Error: Could not obtain a valid single-channel image.</source>
        <translation>
错误：无法获取有效的单通道图像。</translation>
    </message>
    <message>
        <location filename="../src/analysis_infocontent.cpp" line="128"/>
        <source>Converting single-channel image to 8-bit (CV_8U) for histogram...
</source>
        <translation>正在将单通道图像转换为 8 位 (CV_8U) 以计算直方图...
</translation>
    </message>
    <message>
        <location filename="../src/analysis_infocontent.cpp" line="132"/>
        <source>Normalizing data to 8-bit range (0-255).
</source>
        <translation>正在将数据归一化到 8 位范围 (0-255)。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_infocontent.cpp" line="141"/>
        <source>Image has constant value; entropy is expected to be 0.
</source>
        <translation>图像值为常量；预计熵为 0。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_infocontent.cpp" line="147"/>
        <source>Image is already 8-bit (CV_8U).
</source>
        <translation>图像已是 8 位 (CV_8U)。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_infocontent.cpp" line="152"/>
        <source>
Error: Failed to produce a valid CV_8UC1 image for histogram.</source>
        <translation>
错误：无法生成有效的 CV_8UC1 图像以计算直方图。</translation>
    </message>
    <message>
        <location filename="../src/analysis_infocontent.cpp" line="157"/>
        <source>Error - Conversion Failed</source>
        <translation>错误 - 转换失败</translation>
    </message>
    <message>
        <location filename="../src/analysis_infocontent.cpp" line="172"/>
        <source>Calculating histogram...
</source>
        <translation>正在计算直方图...
</translation>
    </message>
    <message>
        <location filename="../src/analysis_infocontent.cpp" line="180"/>
        <source>
Warning: Image contains no pixels. Cannot calculate entropy.</source>
        <translation>
警告：图像不含像素，无法计算熵。</translation>
    </message>
    <message>
        <location filename="../src/analysis_infocontent.cpp" line="185"/>
        <source>Warning - Empty Image</source>
        <translation>警告 - 空图像</translation>
    </message>
    <message>
        <location filename="../src/analysis_infocontent.cpp" line="190"/>
        <source>Calculating entropy from normalized histogram...
</source>
        <translation>正在从归一化直方图计算熵...
</translation>
    </message>
    <message>
        <location filename="../src/analysis_infocontent.cpp" line="199"/>
        <source>
--- Entropy Calculation Result ---
</source>
        <translation>
--- 熵计算结果 ---
</translation>
    </message>
    <message>
        <location filename="../src/analysis_infocontent.cpp" line="201"/>
        <source>Shannon Entropy: %1 bits/pixel
</source>
        <translation>香农熵：%1 比特/像素
</translation>
    </message>
    <message>
        <location filename="../src/analysis_infocontent.cpp" line="204"/>
        <source>
Interpretation: Higher entropy generally indicates more complexity.</source>
        <translation>
解释：熵越高通常表示信息复杂度越高。</translation>
    </message>
    <message>
        <location filename="../src/analysis_infocontent.cpp" line="210"/>
        <source>%1 bits/pixel</source>
        <translation>%1 比特/像素</translation>
    </message>
    <message>
        <source>%.4f bits/pixel</source>
        <translation type="vanished">%.4f 比特/像素</translation>
    </message>
    <message>
        <location filename="../src/analysis_infocontent.cpp" line="212"/>
        <source>Internal Log: Entropy calculation successful: %1 bits/pixel
</source>
        <translation>内部日志：熵计算成功：%1 比特/像素
</translation>
    </message>
    <message>
        <location filename="../src/analysis_infocontent.cpp" line="220"/>
        <source>
Error during histogram or entropy calculation: %1</source>
        <translation>
直方图或熵计算过程中出错：%1</translation>
    </message>
    <message>
        <location filename="../src/analysis_infocontent.cpp" line="227"/>
        <source>Internal Log: OpenCV Error during Entropy calculation: %1
</source>
        <translation>内部日志：熵计算过程中发生 OpenCV 错误：%1
</translation>
    </message>
    <message>
        <location filename="../src/analysis_radiometric.cpp" line="12"/>
        <source>Radiometric Statistics</source>
        <translation>辐射统计</translation>
    </message>
    <message>
        <location filename="../src/analysis_radiometric.cpp" line="13"/>
        <source>Radiometric Analysis (Basic Statistics):
</source>
        <translation>辐射分析 (基本统计):
</translation>
    </message>
    <message>
        <location filename="../src/analysis_radiometric.cpp" line="16"/>
        <source>Radiometric: </source>
        <translation>辐射统计：</translation>
    </message>
    <message>
        <location filename="../src/analysis_radiometric.cpp" line="32"/>
        <source>Preparing single-channel image for analysis...
</source>
        <translation>正在为分析准备单通道图像...
</translation>
    </message>
    <message>
        <location filename="../src/analysis_radiometric.cpp" line="35"/>
        <source>Input is complex, calculating magnitude.
</source>
        <translation>输入为复数数据，正在计算幅度。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_radiometric.cpp" line="37"/>
        <source>Using magnitude image calculated from complex data.
</source>
        <translation>使用从复数数据计算的幅度图像。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_radiometric.cpp" line="51"/>
        <source>Using single-channel input directly.
</source>
        <translation>直接使用单通道输入。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_radiometric.cpp" line="53"/>
        <source>Using single-channel input data.
</source>
        <translation>使用单通道输入数据。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_radiometric.cpp" line="58"/>
        <source>Input is multi-channel (%1), using first channel for basic stats.
</source>
        <translation>输入为多通道 (%1)，使用第一个通道进行基本统计。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_radiometric.cpp" line="62"/>
        <source>Using the first channel of the multi-channel input.
</source>
        <translation>使用多通道输入的第一个通道。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_radiometric.cpp" line="73"/>
        <source>
Error: Failed to prepare single-channel data.</source>
        <translation>
错误：无法准备单通道数据。</translation>
    </message>
    <message>
        <location filename="../src/analysis_radiometric.cpp" line="84"/>
        <source>Converting image data to 32-bit float (CV_32F) for precise statistics.
</source>
        <translation>正在将图像数据转换为 32 位浮点数 (CV_32F) 以进行精确统计。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_radiometric.cpp" line="92"/>
        <source>Calculating min, max, mean, and standard deviation...
</source>
        <translation>正在计算最小值、最大值、平均值和标准差...
</translation>
    </message>
    <message>
        <location filename="../src/analysis_radiometric.cpp" line="102"/>
        <source>
--- Basic Radiometric Statistics ---
</source>
        <translation>
--- 基本辐射统计 ---
</translation>
    </message>
    <message>
        <location filename="../src/analysis_radiometric.cpp" line="105"/>
        <source>Minimum Value: %1
</source>
        <translation>最小值：%1
</translation>
    </message>
    <message>
        <location filename="../src/analysis_radiometric.cpp" line="108"/>
        <source>Maximum Value: %1
</source>
        <translation>最大值：%1
</translation>
    </message>
    <message>
        <location filename="../src/analysis_radiometric.cpp" line="110"/>
        <source>Dynamic Range (Max - Min): %1
</source>
        <translation>动态范围 (最大值 - 最小值): %1
</translation>
    </message>
    <message>
        <location filename="../src/analysis_radiometric.cpp" line="114"/>
        <source>Mean Value (μ): %1
</source>
        <translation>平均值 (μ): %1
</translation>
    </message>
    <message>
        <location filename="../src/analysis_radiometric.cpp" line="117"/>
        <location filename="../src/analysis_snr.cpp" line="110"/>
        <source>Standard Deviation (σ): %1
</source>
        <translation>标准差 (σ): %1
</translation>
    </message>
    <message>
        <location filename="../src/analysis_radiometric.cpp" line="120"/>
        <source>
Interpretation:
- Dynamic Range indicates the spread of intensity values.
- Mean represents the average brightness.
- Standard Deviation estimates noise or texture variability.</source>
        <translation>
解释：
- 动态范围表示强度值的分布范围。
- 平均值代表平均亮度。
- 标准差估计噪声或纹理变化性。</translation>
    </message>
    <message>
        <location filename="../src/analysis_radiometric.cpp" line="129"/>
        <source>Range=%1, StdDev=%2</source>
        <translation>范围=%1, 标准差=%2</translation>
    </message>
    <message>
        <location filename="../src/analysis_radiometric.cpp" line="133"/>
        <source>Internal Log: Radiometric stats calculated: Min=%1, Max=%2, Mean=%3, StdDev=%4
</source>
        <translation>内部日志：辐射统计已计算：最小值=%1, 最大值=%2, 平均值=%3, 标准差=%4
</translation>
    </message>
    <message>
        <location filename="../src/analysis_radiometric.cpp" line="144"/>
        <source>
Error during radiometric statistics calculation: %1</source>
        <translation>
辐射统计计算过程中出错：%1</translation>
    </message>
    <message>
        <location filename="../src/analysis_radiometric.cpp" line="151"/>
        <source>Internal Log: OpenCV Error during Radiometric calculation: %1
</source>
        <translation>内部日志：辐射统计计算过程中发生 OpenCV 错误：%1
</translation>
    </message>
    <message>
        <location filename="../src/analysis_snr.cpp" line="15"/>
        <source>SNR/ENL</source>
        <translation>信噪比/等效视数</translation>
    </message>
    <message>
        <location filename="../src/analysis_snr.cpp" line="16"/>
        <source>SNR/ENL Analysis Results (Global):
</source>
        <translation>信噪比/等效视数分析结果 (全局):
</translation>
    </message>
    <message>
        <location filename="../src/analysis_snr.cpp" line="19"/>
        <source>SNR/ENL (Global): </source>
        <translation>信噪比/等效视数 (全局): </translation>
    </message>
    <message>
        <location filename="../src/analysis_snr.cpp" line="35"/>
        <source>Preparing single-channel float image...
</source>
        <translation>正在准备单通道浮点图像...
</translation>
    </message>
    <message>
        <location filename="../src/analysis_snr.cpp" line="43"/>
        <source>Converting complex channels to CV_32F for magnitude calculation.
</source>
        <translation>正在将复数通道转换为 CV_32F 以计算幅度。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_snr.cpp" line="51"/>
        <source>Using magnitude image.
</source>
        <translation>使用幅度图像。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_snr.cpp" line="54"/>
        <source>Input is single-channel.
</source>
        <translation>输入为单通道。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_snr.cpp" line="57"/>
        <source>Converting single-channel image (type: %1) to CV_32F.
</source>
        <translation>正在将单通道图像 (类型：%1) 转换为 CV_32F。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_snr.cpp" line="62"/>
        <source>Converted input to floating-point type (CV_32F).
</source>
        <translation>已将输入转换为浮点类型 (CV_32F)。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_snr.cpp" line="65"/>
        <source>Using existing single-channel floating-point data.
</source>
        <translation>使用现有的单通道浮点数据。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_snr.cpp" line="70"/>
        <source>
Error: Unsupported channel count (%1). Expected 1 or 2.</source>
        <translation>
错误：不支持的通道数 (%1)。预期为 1 或 2。</translation>
    </message>
    <message>
        <location filename="../src/analysis_snr.cpp" line="76"/>
        <source>Error - Unsupported Channels</source>
        <translation>错误 - 不支持的通道</translation>
    </message>
    <message>
        <location filename="../src/analysis_snr.cpp" line="85"/>
        <source>
Error: Failed to prepare a valid single-channel float image.</source>
        <translation>
错误：无法准备有效的单通道浮点图像。</translation>
    </message>
    <message>
        <location filename="../src/analysis_snr.cpp" line="98"/>
        <source>Calculating global mean and standard deviation...
</source>
        <translation>正在计算全局平均值和标准差...
</translation>
    </message>
    <message>
        <location filename="../src/analysis_snr.cpp" line="105"/>
        <source>
--- Global Statistics ---
</source>
        <translation>
--- 全局统计 ---
</translation>
    </message>
    <message>
        <location filename="../src/analysis_snr.cpp" line="108"/>
        <source>Mean (μ): %1
</source>
        <translation>平均值 (μ): %1
</translation>
    </message>
    <message>
        <location filename="../src/analysis_snr.cpp" line="118"/>
        <source>
--- Quality Metrics (Global) ---
</source>
        <translation>
--- 质量指标 (全局) ---
</translation>
    </message>
    <message>
        <location filename="../src/analysis_snr.cpp" line="121"/>
        <source>Signal-to-Noise Ratio (SNR = μ/σ): %1
</source>
        <translation>信噪比 (SNR = μ/σ): %1
</translation>
    </message>
    <message>
        <location filename="../src/analysis_snr.cpp" line="125"/>
        <source>Equivalent Number of Looks (ENL = (μ/σ)²): %1
</source>
        <translation>等效视数 (ENL = (μ/σ)²): %1
</translation>
    </message>
    <message>
        <location filename="../src/analysis_snr.cpp" line="131"/>
        <source>SNR=%1, ENL=%2</source>
        <translation>信噪比=%1, 等效视数=%2</translation>
    </message>
    <message>
        <source>SNR=%.2f, ENL=%.2f</source>
        <translation type="vanished">信噪比=%.2f, 等效视数=%.2f</translation>
    </message>
    <message>
        <location filename="../src/analysis_snr.cpp" line="135"/>
        <source>Internal Log: SNR/ENL calculated: Mean=%1, StdDev=%2, SNR=%3, ENL=%4
</source>
        <translation>内部日志：信噪比/等效视数已计算：平均值=%1, 标准差=%2, 信噪比=%3, 等效视数=%4
</translation>
    </message>
    <message>
        <location filename="../src/analysis_snr.cpp" line="144"/>
        <source>
Warning: Standard deviation is close to zero (σ = %1). Cannot calculate SNR/ENL reliably.
</source>
        <translation>
警告：标准差接近于零 (σ = %1)。无法可靠地计算信噪比/等效视数。</translation>
    </message>
    <message>
        <location filename="../src/analysis_snr.cpp" line="148"/>
        <source>N/A (σ ≈ 0)</source>
        <translation>N/A (σ ≈ 0)</translation>
    </message>
    <message>
        <location filename="../src/analysis_snr.cpp" line="150"/>
        <source>Internal Log: SNR/ENL calculation skipped: Standard deviation is near zero.
</source>
        <translation>内部日志：跳过信噪比/等效视数计算：标准差接近于零。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_snr.cpp" line="155"/>
        <source>

Note: These metrics are calculated globally. For more meaningful assessment, calculate over a statistically homogeneous region.</source>
        <translation>

注意：这些指标是全局计算的。为了进行更有意义的评估，请在统计均匀区域上进行计算。</translation>
    </message>
    <message>
        <location filename="../src/analysis_snr.cpp" line="163"/>
        <source>
Error during mean/standard deviation calculation: %1</source>
        <translation>
平均值/标准差计算过程中出错：%1</translation>
    </message>
    <message>
        <location filename="../src/analysis_snr.cpp" line="171"/>
        <source>Internal Log: OpenCV Error during meanStdDev: %1
</source>
        <translation>内部日志：meanStdDev 计算过程中发生 OpenCV 错误：%1
</translation>
    </message>
</context>
<context>
    <name>AnalysisHelper</name>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="139"/>
        <source>Input is complex (2-channel), calculating magnitude.
</source>
        <translation>输入为复数数据 (2 通道)，正在计算幅度。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="145"/>
        <source>Converting complex channels to CV_32F for magnitude calculation.
</source>
        <translation>正在将复数通道转换为 CV_32F 以计算幅度。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="153"/>
        <source>Input is multi-channel (%1), converting to grayscale.
</source>
        <translation>输入为多通道 (%1)，正在转换为灰度图。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="160"/>
        <source>Input depth (%1) not directly supported by cvtColor, converting to CV_8U first.
</source>
        <translation>cvtColor 不直接支持输入深度 (%1)，先转换为 CV_8U。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="180"/>
        <source>Standard grayscale conversion failed (%1), likely not BGR format. Falling back to first channel.
</source>
        <translation>标准灰度转换失败 (%1)，可能不是 BGR 格式。回退到使用第一个通道。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="190"/>
        <source>Input has %1 channels, not standard BGR/BGRA. Using first channel directly.
</source>
        <translation>输入包含 %1 个通道，不是标准的 BGR/BGRA 格式。直接使用第一个通道。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="199"/>
        <source>Input is already single-channel.
</source>
        <translation>输入已是单通道。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="203"/>
        <source>Error: Input image has 0 channels.
</source>
        <translation>错误：输入图像通道数为 0。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="209"/>
        <source>Error: Failed to obtain a single channel image from the input.
</source>
        <translation>错误：无法从输入获取单通道图像。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="218"/>
        <source>Normalizing single-channel image (type: %1) to 8-bit (0-255) for GLCM.
</source>
        <translation>正在将单通道图像 (类型：%1) 归一化到 8 位 (0-255) 以用于 GLCM。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="229"/>
        <source>Image has constant value after conversion to single channel.
</source>
        <translation>转换为单通道后，图像值为常量。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="234"/>
        <source>Single-channel image is already 8-bit (CV_8U).
</source>
        <translation>单通道图像已是 8 位 (CV_8U)。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="240"/>
        <source>Error: Failed to produce a valid CV_8UC1 image for GLCM.
</source>
        <translation>错误：无法生成有效的 CV_8UC1 图像以用于 GLCM。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="246"/>
        <source>Successfully prepared CV_8UC1 image for GLCM analysis.
</source>
        <translation>成功准备用于 GLCM 分析的 CV_8UC1 图像。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="289"/>
        <source>GLCM calculation complete. Normalized by %1 pairs.
</source>
        <translation>GLCM 计算完成。已通过 %1 对像元对进行归一化。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="295"/>
        <source>Warning: No valid pixel pairs found for GLCM calculation. GLCM is zero.
</source>
        <translation>警告：未找到用于 GLCM 计算的有效像元对。GLCM 为零。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="300"/>
        <source>GLCM calculation complete. Contains raw counts (%1 total pairs).
</source>
        <translation>GLCM 计算完成。包含原始计数 (共 %1 对像元对)。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="348"/>
        <source>Warning: GLCM sum (%1) is not close to 1. Adjusting means/stddev based on sum.
</source>
        <translation>警告：GLCM 总和 (%1) 不接近 1。根据总和调整均值/标准差。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="385"/>
        <source>Warning: GLCM Correlation is undefined because stddev_i or stddev_j is near zero.
</source>
        <translation>警告：GLCM 相关性未定义，因为 stddev_i 或 stddev_j 接近于零。
</translation>
    </message>
    <message>
        <location filename="../src/analysis_glcm.cpp" line="389"/>
        <source>GLCM feature calculation complete.
</source>
        <translation>GLCM 特征计算完成。
</translation>
    </message>
</context>
<context>
    <name>ImageHandler</name>
    <message>
        <location filename="../src/imagehandler.cpp" line="44"/>
        <source>Closed GDAL dataset: %1</source>
        <translation>已关闭 GDAL 数据集：%1</translation>
    </message>
    <message>
        <location filename="../src/imagehandler.cpp" line="49"/>
        <source>Released cv::Mat memory.</source>
        <translation>已释放 cv::Mat 内存。</translation>
    </message>
    <message>
        <location filename="../src/imagehandler.cpp" line="59"/>
        <source>Error: Invalid (empty) file path provided to loadImage.</source>
        <translation>错误：提供给 loadImage 的文件路径无效 (为空)。</translation>
    </message>
    <message>
        <location filename="../src/imagehandler.cpp" line="64"/>
        <source>ImageHandler: Attempting to load image: %1</source>
        <translation>ImageHandler: 正在尝试加载图像：%1</translation>
    </message>
    <message>
        <location filename="../src/imagehandler.cpp" line="73"/>
        <source>Error: Could not open image file &apos;%1&apos; with GDAL. Error: %2</source>
        <translation>错误：无法使用 GDAL 打开图像文件 &apos;%1&apos;。错误：%2</translation>
    </message>
    <message>
        <location filename="../src/imagehandler.cpp" line="81"/>
        <source>ImageHandler: Successfully opened dataset: %1</source>
        <translation>ImageHandler: 成功打开数据集：%1</translation>
    </message>
    <message>
        <location filename="../src/imagehandler.cpp" line="89"/>
        <source>Error: Image has invalid dimensions (%1x%2) or zero bands (%3).</source>
        <translation>错误：图像尺寸无效 (%1x%2) 或波段数为零 (%3)。</translation>
    </message>
    <message>
        <location filename="../src/imagehandler.cpp" line="101"/>
        <source>Error: Could not access raster band 1.</source>
        <translation>错误：无法访问栅格波段 1。</translation>
    </message>
    <message>
        <location filename="../src/imagehandler.cpp" line="108"/>
        <source>ImageHandler: Properties: %1x%2 pixels, %3 bands, Data type: %4</source>
        <translation>ImageHandler: 属性：%1x%2 像素，%3 波段，数据类型：%4</translation>
    </message>
    <message>
        <location filename="../src/imagehandler.cpp" line="132"/>
        <source>Error: Unsupported GDAL data type for reading: %1</source>
        <translation>错误：不支持的 GDAL 读取数据类型：%1</translation>
    </message>
    <message>
        <location filename="../src/imagehandler.cpp" line="153"/>
        <source>Error reading raster data using RasterIO. GDAL Error: %1</source>
        <translation>使用 RasterIO 读取栅格数据时出错。GDAL 错误：%1</translation>
    </message>
    <message>
        <location filename="../src/imagehandler.cpp" line="163"/>
        <source>Exception getting type string for cv::Mat</source>
        <translation>获取 cv::Mat 类型字符串时发生异常</translation>
    </message>
    <message>
        <location filename="../src/imagehandler.cpp" line="167"/>
        <source>ImageHandler: Image data successfully read into cv::Mat. Type: %1, Channels: %2</source>
        <translation>ImageHandler: 图像数据已成功读入 cv::Mat。类型：%1, 通道数：%2</translation>
    </message>
    <message>
        <location filename="../src/imagehandler.cpp" line="191"/>
        <location filename="../src/imagehandler.cpp" line="204"/>
        <source>N/A</source>
        <translation>N/A</translation>
    </message>
    <message>
        <location filename="../src/imagehandler.cpp" line="200"/>
        <source>Warning: Could not get band 1 to retrieve data type string.</source>
        <translation>警告：无法获取波段 1 以检索数据类型字符串。</translation>
    </message>
    <message>
        <location filename="../src/imagehandler.cpp" line="201"/>
        <source>Error</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="../src/imagehandler.cpp" line="216"/>
        <source>Error: prepareDisplayMat called when image is not valid.</source>
        <translation>错误：在图像无效时调用了 prepareDisplayMat。</translation>
    </message>
    <message>
        <location filename="../src/imagehandler.cpp" line="223"/>
        <source>ImageHandler: Calculating magnitude from complex data for display.</source>
        <translation>ImageHandler: 正在从复数数据计算幅度以供显示。</translation>
    </message>
    <message>
        <location filename="../src/imagehandler.cpp" line="228"/>
        <source>ImageHandler: Converting complex channels to CV_32F for magnitude calculation.</source>
        <translation>ImageHandler: 正在将复数通道转换为 CV_32F 以计算幅度。</translation>
    </message>
    <message>
        <location filename="../src/imagehandler.cpp" line="242"/>
        <source>ImageHandler: Display source is already CV_8U.</source>
        <translation>ImageHandler: 显示源已经是 CV_8U。</translation>
    </message>
    <message>
        <location filename="../src/imagehandler.cpp" line="246"/>
        <source>ImageHandler: Normalizing display source. Original range: [%1, %2]</source>
        <translation>ImageHandler: 正在归一化显示源。原始范围：[%1, %2]</translation>
    </message>
    <message>
        <location filename="../src/imagehandler.cpp" line="252"/>
        <source>ImageHandler: Display source has constant value. Converted to uniform gray.</source>
        <translation>ImageHandler: 显示源具有恒定值。已转换为均匀灰色。</translation>
    </message>
    <message>
        <location filename="../src/imagehandler.cpp" line="258"/>
        <source>Error: prepareDisplayMat resulted in unexpected type: %1</source>
        <translation>错误：prepareDisplayMat 生成了意外类型：%1</translation>
    </message>
    <message>
        <location filename="../src/imagehandler.cpp" line="274"/>
        <source>Error: getDisplayPixmap called when image is not valid.</source>
        <translation>错误：在图像无效时调用了 getDisplayPixmap。</translation>
    </message>
    <message>
        <location filename="../src/imagehandler.cpp" line="282"/>
        <source>Error: Failed to prepare CV_8UC1 Mat for display.</source>
        <translation>错误：无法准备用于显示的 CV_8UC1 Mat。</translation>
    </message>
    <message>
        <location filename="../src/imagehandler.cpp" line="295"/>
        <source>Error: QImage conversion resulted in a null image.</source>
        <translation>错误：QImage 转换导致空图像。</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="../src/mainwindow.cpp" line="48"/>
        <location filename="../src/mainwindow.cpp" line="97"/>
        <source>Image Display Area</source>
        <translation>图像显示区</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="54"/>
        <source>Summary of all selected analysis results will appear here...</source>
        <translation>所有选定分析结果的摘要将显示在此处...</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="56"/>
        <source>Detailed results for SNR/ENL Analysis...</source>
        <translation>信噪比/等效视数分析的详细结果...</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="58"/>
        <source>Detailed results for Information Content (Entropy)...</source>
        <translation>信息量 (熵) 分析的详细结果...</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="60"/>
        <source>Detailed results for Clarity (Gradient Magnitude)...</source>
        <translation>清晰度 (梯度幅值) 分析的详细结果...</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="62"/>
        <source>Detailed results for Radiometric Stats (Min, Max, Mean, StdDev)...</source>
        <translation>辐射统计 (最小值，最大值，平均值，标准差) 分析的详细结果...</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="64"/>
        <source>Detailed results for GLCM Texture Features...</source>
        <translation>GLCM 纹理特征分析的详细结果...</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="66"/>
        <source>Log messages (loading, analysis steps, errors) will appear here...</source>
        <translation>日志消息 (加载、分析步骤、错误) 将显示在此处...</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="87"/>
        <source>Closing current image and resetting UI.</source>
        <translation>正在关闭当前图像并重置界面。</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="131"/>
        <source>Drag entered with unsupported file type: .%1</source>
        <translation>拖入不支持的文件类型：.%1</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="157"/>
        <source>Image opening cancelled or invalid path provided.</source>
        <translation>图像打开已取消或提供了无效路径。</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="161"/>
        <source>Attempting to open image via MainWindow: %1</source>
        <translation>正在通过主窗口尝试打开图像：%1</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="168"/>
        <source>Image loaded successfully by ImageHandler: %1</source>
        <translation>ImageHandler 成功加载图像：%1</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="183"/>
        <source>Image successfully displayed.</source>
        <translation>图像显示成功。</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="186"/>
        <source>Error: ImageHandler provided a null QPixmap.</source>
        <translation>错误：ImageHandler 提供了空的 QPixmap。</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="188"/>
        <source>Error: Display failed</source>
        <translation>错误：显示失败</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="193"/>
        <source>Display Error</source>
        <translation>显示错误</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="194"/>
        <source>Failed to prepare image for display after loading.</source>
        <translation>加载后准备显示图像失败。</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="198"/>
        <source>ImageHandler failed to load the image.</source>
        <translation>ImageHandler 加载图像失败。</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="200"/>
        <source>Image Load Error</source>
        <translation>图像加载错误</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="201"/>
        <source>Could not open or read the selected image file.
Path: %1
Please check file integrity, permissions, and see logs for details.</source>
        <translation>无法打开或读取所选的图像文件。
路径：%1
请检查文件完整性、权限，并查看日志了解详细信息。</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="215"/>
        <source>Analysis Not Started</source>
        <translation>分析未开始</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="216"/>
        <source>Please open a valid image file before starting the analysis.</source>
        <translation>请先打开有效的图像文件，然后再开始分析。</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="217"/>
        <source>Analysis button clicked, but no valid image is loaded.</source>
        <translation>点击了分析按钮，但未加载有效图像。</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="221"/>
        <source>Analysis process started by user.</source>
        <translation>用户已启动分析过程。</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="244"/>
        <source>Analysis &apos;%1&apos; finished. Success: %2</source>
        <translation>分析 &apos;%1&apos; 完成。成功：%2</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="252"/>
        <source>Details for %1:
%2</source>
        <translation>%1 的详细信息：
%2</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="254"/>
        <source>Detailed log for %1 already contains internal messages.</source>
        <translation>%1 的详细日志已包含内部消息。</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="314"/>
        <source>No Analysis Selected</source>
        <translation>未选择分析方法</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="316"/>
        <source>Analysis stopped: No methods were selected.</source>
        <translation>分析已停止：未选择任何方法。</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="321"/>
        <source>Starting selected analyses...
</source>
        <translation>正在开始选定的分析...
</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="325"/>
        <source>Performing analysis step %1 of %2...</source>
        <translation>正在执行分析步骤 %1 / %2...</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="342"/>
        <source>All selected analyses finished.</source>
        <translation>所有选定的分析均已完成。</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="344"/>
        <source>
Analysis complete. Check individual tabs for detailed results.</source>
        <translation>
分析完成。请检查各个选项卡以获取详细结果。</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="348"/>
        <source>Selected image analyses have finished.</source>
        <translation>选定的图像分析已完成。</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="358"/>
        <source>Selected all analysis methods.</source>
        <translation>已选择所有分析方法。</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="359"/>
        <source>Deselected all analysis methods.</source>
        <translation>已取消选择所有分析方法。</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="365"/>
        <source>Open SAR Image File</source>
        <translation>打开 SAR 图像文件</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="366"/>
        <source>Supported Image Formats (*.tif *.tiff *.img *.hdr *.dat);;All Files (*.*)</source>
        <translation>支持的图像格式 (*.tif *.tiff *.img *.hdr *.dat);;所有文件 (*.*)</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="370"/>
        <source>Opening image via File menu: %1</source>
        <translation>正在通过文件菜单打开图像：%1</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="373"/>
        <source>Image opening via File menu cancelled by user.</source>
        <translation>用户已取消通过文件菜单打开图像。</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="45"/>
        <location filename="../src/mainwindow.cpp" line="46"/>
        <location filename="../src/mainwindow.cpp" line="47"/>
        <location filename="../src/mainwindow.cpp" line="91"/>
        <location filename="../src/mainwindow.cpp" line="92"/>
        <location filename="../src/mainwindow.cpp" line="93"/>
        <source>N/A</source>
        <translation>N/A</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="40"/>
        <source>Application started. GDAL initialized. Drag &amp; Drop enabled.</source>
        <translation>应用程序已启动。GDAL 已初始化。拖放功能已启用。</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="126"/>
        <source>Drag entered with supported file: %1</source>
        <translation>拖入支持的文件：%1</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="145"/>
        <source>File dropped: %1</source>
        <translation>文件已拖放：%1</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="315"/>
        <source>Please select at least one analysis method.</source>
        <translation>请至少选择一种分析方法。</translation>
    </message>
    <message>
        <location filename="../src/mainwindow.cpp" line="347"/>
        <source>Analysis Complete</source>
        <translation>分析完成</translation>
    </message>
</context>
</TS>
