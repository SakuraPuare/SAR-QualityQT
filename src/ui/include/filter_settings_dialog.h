#ifndef FILTER_SETTINGS_DIALOG_H
#define FILTER_SETTINGS_DIALOG_H

#include <QDialog>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QSlider>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QCheckBox>

#include "../../core/imagefilters.h"

namespace Ui {
class FilterSettingsDialog;
}

/**
 * @brief 滤波器设置对话框类
 */
class FilterSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口
     * @param filterType 滤波器类型
     * @param params 滤波器参数
     */
    explicit FilterSettingsDialog(QWidget *parent = nullptr, 
                                  SAR::Core::FilterType filterType = SAR::Core::FilterType::Gaussian,
                                  const SAR::Core::FilterParameters& params = SAR::Core::FilterParameters());
    
    /**
     * @brief 析构函数
     */
    ~FilterSettingsDialog();
    
    /**
     * @brief 获取设置的滤波器参数
     * @return 滤波器参数
     */
    SAR::Core::FilterParameters getFilterParameters() const;

private slots:
    /**
     * @brief 滤波器类型改变时的处理
     * @param index 当前索引
     */
    void onFilterTypeChanged(int index);
    
    /**
     * @brief 实时预览切换时的处理
     * @param checked 是否勾选
     */
    void onPreviewToggled(bool checked);
    
    /**
     * @brief 参数值改变时的处理
     */
    void onParameterChanged();

private:
    void setupUi();
    void updateParameterVisibility();
    void updatePreview();
    
    QComboBox *filterTypeComboBox;
    QSpinBox *kernelSizeSpinBox;
    QDoubleSpinBox *sigmaSpinBox;
    QDoubleSpinBox *param1SpinBox;
    QDoubleSpinBox *param2SpinBox;
    QDoubleSpinBox *dampingSpinBox;
    QLabel *param1Label;
    QLabel *param2Label;
    QLabel *dampingLabel;
    QCheckBox *previewCheckBox;
    QDialogButtonBox *buttonBox;
    
    SAR::Core::FilterParameters currentParams;
};

#endif // FILTER_SETTINGS_DIALOG_H