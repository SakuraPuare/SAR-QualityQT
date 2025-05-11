#include "include/filter_settings_dialog.h"
#include <QCheckBox>
#include <QMessageBox>
#include <QApplication>

FilterSettingsDialog::FilterSettingsDialog(QWidget *parent, 
                                           SAR::Core::FilterType filterType,
                                           const SAR::Core::FilterParameters& params)
    : QDialog(parent), currentParams(params)
{
    // 设置参数默认值（如果未提供）
    if (filterType != SAR::Core::FilterType::Custom) {
        currentParams.type = filterType;
    }
    
    // 设置对话框标题
    setWindowTitle(tr("滤波器设置"));
    
    // 设置对话框UI
    setupUi();
    
    // 更新参数显示
    updateParameterVisibility();
    
    // 连接信号与槽
    connect(filterTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FilterSettingsDialog::onFilterTypeChanged);
    connect(kernelSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &FilterSettingsDialog::onParameterChanged);
    connect(sigmaSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &FilterSettingsDialog::onParameterChanged);
    connect(param1SpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &FilterSettingsDialog::onParameterChanged);
    connect(param2SpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &FilterSettingsDialog::onParameterChanged);
    connect(dampingSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &FilterSettingsDialog::onParameterChanged);
    connect(previewCheckBox, &QCheckBox::toggled,
            this, &FilterSettingsDialog::onPreviewToggled);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    // 根据当前滤波器类型设置下拉框
    int index = static_cast<int>(currentParams.type);
    if (index >= 0 && index < filterTypeComboBox->count()) {
        filterTypeComboBox->setCurrentIndex(index);
    }
    
    // 更新SpinBox的值
    kernelSizeSpinBox->setValue(currentParams.kernelSize);
    sigmaSpinBox->setValue(currentParams.sigma);
    param1SpinBox->setValue(currentParams.param1);
    param2SpinBox->setValue(currentParams.param2);
    dampingSpinBox->setValue(currentParams.damping);
    
    // 调整对话框大小
    resize(400, 300);
}

FilterSettingsDialog::~FilterSettingsDialog()
{
}

SAR::Core::FilterParameters FilterSettingsDialog::getFilterParameters() const
{
    return currentParams;
}

void FilterSettingsDialog::setupUi()
{
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 创建表单布局
    QFormLayout *formLayout = new QFormLayout();
    
    // 滤波器类型下拉框
    filterTypeComboBox = new QComboBox(this);
    filterTypeComboBox->addItem(tr("低通滤波"), static_cast<int>(SAR::Core::FilterType::LowPass));
    filterTypeComboBox->addItem(tr("高通滤波"), static_cast<int>(SAR::Core::FilterType::HighPass));
    filterTypeComboBox->addItem(tr("带通滤波"), static_cast<int>(SAR::Core::FilterType::BandPass));
    filterTypeComboBox->addItem(tr("中值滤波"), static_cast<int>(SAR::Core::FilterType::Median));
    filterTypeComboBox->addItem(tr("高斯滤波"), static_cast<int>(SAR::Core::FilterType::Gaussian));
    filterTypeComboBox->addItem(tr("双边滤波"), static_cast<int>(SAR::Core::FilterType::Bilateral));
    filterTypeComboBox->addItem(tr("Lee滤波"), static_cast<int>(SAR::Core::FilterType::Lee));
    filterTypeComboBox->addItem(tr("Frost滤波"), static_cast<int>(SAR::Core::FilterType::Frost));
    filterTypeComboBox->addItem(tr("Kuan滤波"), static_cast<int>(SAR::Core::FilterType::Kuan));
    formLayout->addRow(tr("滤波器类型:"), filterTypeComboBox);
    
    // 内核大小
    kernelSizeSpinBox = new QSpinBox(this);
    kernelSizeSpinBox->setRange(1, 31);
    kernelSizeSpinBox->setSingleStep(2);  // 步长为2，只允许奇数
    kernelSizeSpinBox->setValue(3);
    formLayout->addRow(tr("内核大小:"), kernelSizeSpinBox);
    
    // Sigma值
    sigmaSpinBox = new QDoubleSpinBox(this);
    sigmaSpinBox->setRange(0.1, 10.0);
    sigmaSpinBox->setSingleStep(0.1);
    sigmaSpinBox->setValue(1.0);
    formLayout->addRow(tr("Sigma值:"), sigmaSpinBox);
    
    // 参数1（通用）
    param1SpinBox = new QDoubleSpinBox(this);
    param1SpinBox->setRange(0.0, 100.0);
    param1SpinBox->setSingleStep(1.0);
    param1SpinBox->setValue(0.0);
    param1Label = new QLabel(tr("半径:"), this);
    formLayout->addRow(param1Label, param1SpinBox);
    
    // 参数2（通用）
    param2SpinBox = new QDoubleSpinBox(this);
    param2SpinBox->setRange(0.0, 100.0);
    param2SpinBox->setSingleStep(1.0);
    param2SpinBox->setValue(0.0);
    param2Label = new QLabel(tr("外半径:"), this);
    formLayout->addRow(param2Label, param2SpinBox);
    
    // 阻尼系数（Lee/Frost）
    dampingSpinBox = new QDoubleSpinBox(this);
    dampingSpinBox->setRange(0.1, 10.0);
    dampingSpinBox->setSingleStep(0.1);
    dampingSpinBox->setValue(1.0);
    dampingLabel = new QLabel(tr("阻尼系数:"), this);
    formLayout->addRow(dampingLabel, dampingSpinBox);
    
    // 预览复选框
    previewCheckBox = new QCheckBox(tr("实时预览"), this);
    formLayout->addRow("", previewCheckBox);
    
    // 添加表单布局
    mainLayout->addLayout(formLayout);
    
    // 添加按钮
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(buttonBox);
    
    // 设置布局
    setLayout(mainLayout);
}

void FilterSettingsDialog::updateParameterVisibility()
{
    // 根据滤波器类型显示/隐藏相关参数
    SAR::Core::FilterType type = static_cast<SAR::Core::FilterType>(filterTypeComboBox->currentData().toInt());
    
    // 默认隐藏所有特殊参数
    param1Label->hide();
    param1SpinBox->hide();
    param2Label->hide();
    param2SpinBox->hide();
    dampingLabel->hide();
    dampingSpinBox->hide();
    
    // 根据滤波器类型显示相关参数
    switch (type) {
        case SAR::Core::FilterType::LowPass:
        case SAR::Core::FilterType::HighPass:
            param1Label->setText(tr("半径:"));
            param1Label->show();
            param1SpinBox->show();
            break;
            
        case SAR::Core::FilterType::BandPass:
            param1Label->setText(tr("内半径:"));
            param1Label->show();
            param1SpinBox->show();
            param2Label->setText(tr("外半径:"));
            param2Label->show();
            param2SpinBox->show();
            break;
            
        case SAR::Core::FilterType::Bilateral:
            sigmaSpinBox->show();
            param1Label->setText(tr("空间Sigma:"));
            param1Label->show();
            param1SpinBox->show();
            break;
            
        case SAR::Core::FilterType::Lee:
        case SAR::Core::FilterType::Frost:
        case SAR::Core::FilterType::Kuan:
            dampingLabel->show();
            dampingSpinBox->show();
            break;
            
        case SAR::Core::FilterType::Median:
        case SAR::Core::FilterType::Gaussian:
        default:
            // 只显示基本参数
            break;
    }
    
    // 对于某些滤波器，内核大小不适用
    kernelSizeSpinBox->setEnabled(type != SAR::Core::FilterType::LowPass &&
                                 type != SAR::Core::FilterType::HighPass &&
                                 type != SAR::Core::FilterType::BandPass);
    
    // 对于某些滤波器，Sigma不适用
    sigmaSpinBox->setEnabled(type == SAR::Core::FilterType::Gaussian ||
                            type == SAR::Core::FilterType::Bilateral);
}

void FilterSettingsDialog::onFilterTypeChanged(int index)
{
    if (index >= 0) {
        currentParams.type = static_cast<SAR::Core::FilterType>(filterTypeComboBox->currentData().toInt());
        updateParameterVisibility();
        onParameterChanged();
    }
}

void FilterSettingsDialog::onPreviewToggled(bool checked)
{
    if (checked) {
        updatePreview();
    }
}

void FilterSettingsDialog::onParameterChanged()
{
    // 更新参数
    currentParams.kernelSize = kernelSizeSpinBox->value();
    currentParams.sigma = sigmaSpinBox->value();
    currentParams.param1 = param1SpinBox->value();
    currentParams.param2 = param2SpinBox->value();
    currentParams.damping = dampingSpinBox->value();
    
    // 如果启用了预览，更新预览
    if (previewCheckBox->isChecked()) {
        updatePreview();
    }
}

void FilterSettingsDialog::updatePreview()
{
    // 这里实现预览功能
    // 由于需要访问主窗口的图像，这个功能可能需要进一步实现
    // 或者从主窗口传入一个回调函数来处理预览
    
    // 这里仅仅是一个占位符
    QApplication::processEvents();
} 