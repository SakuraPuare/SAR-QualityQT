#include "include/threshold_settings_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QGroupBox>
#include <QFormLayout>
#include <QDoubleSpinBox>

namespace SAR {
namespace UI {

ThresholdSettingsDialog::ThresholdSettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("分析算法阈值设置"));
    loadDefaultThresholds();
    setupUI();
    loadThresholds();
}

ThresholdSettingsDialog::~ThresholdSettingsDialog() = default;

void ThresholdSettingsDialog::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    auto* formLayout = new QFormLayout;

    // 为每个阈值创建输入控件
    for (auto it = defaultThresholds.constBegin(); it != defaultThresholds.constEnd(); ++it) {
        auto* spinBox = new QDoubleSpinBox(this);
        spinBox->setRange(-1000, 1000);
        spinBox->setDecimals(2);
        spinBox->setSuffix(" " + it.value().second); // 添加单位
        spinBox->setValue(it.value().first);
        
        thresholdSpinBoxes[it.key()] = spinBox;
        formLayout->addRow(it.key() + ":", spinBox);
    }

    auto* groupBox = new QGroupBox(tr("阈值设置"), this);
    groupBox->setLayout(formLayout);
    mainLayout->addWidget(groupBox);

    // 按钮布局
    auto* buttonLayout = new QHBoxLayout;
    auto* resetButton = new QPushButton(tr("重置为默认值"), this);
    auto* okButton = new QPushButton(tr("确定"), this);
    auto* cancelButton = new QPushButton(tr("取消"), this);

    buttonLayout->addWidget(resetButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);

    // 连接信号和槽
    connect(resetButton, &QPushButton::clicked, this, &ThresholdSettingsDialog::loadDefaultThresholds);
    connect(okButton, &QPushButton::clicked, this, [this]() {
        saveThresholds();
        accept();
    });
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void ThresholdSettingsDialog::loadDefaultThresholds()
{
    defaultThresholds.clear();
    // 设置默认阈值
    defaultThresholds["峰值旁瓣比"] = qMakePair(-20.0, "dB");
    defaultThresholds["积分旁瓣比"] = qMakePair(-13.0, "dB");
    defaultThresholds["方位向模糊度"] = qMakePair(-20.0, "dB");
    defaultThresholds["距离向模糊度"] = qMakePair(-20.0, "dB");
    defaultThresholds["信噪比"] = qMakePair(8.0, "dB");
    defaultThresholds["噪声等效后向散射系数"] = qMakePair(-19.0, "dB");
    defaultThresholds["绝对辐射精度"] = qMakePair(1.5, "dB");
    defaultThresholds["相对辐射精度"] = qMakePair(1.0, "dB");
    defaultThresholds["等效视数"] = qMakePair(3.0, "");

    // 如果已经创建了 spinBox，更新它们的值
    for (auto it = defaultThresholds.constBegin(); it != defaultThresholds.constEnd(); ++it) {
        if (thresholdSpinBoxes.contains(it.key())) {
            thresholdSpinBoxes[it.key()]->setValue(it.value().first);
        }
    }
}

void ThresholdSettingsDialog::saveThresholds()
{
    QSettings settings;
    settings.beginGroup("AnalysisThresholds");
    
    for (auto it = thresholdSpinBoxes.constBegin(); it != thresholdSpinBoxes.constEnd(); ++it) {
        settings.setValue(it.key(), it.value()->value());
    }
    
    settings.endGroup();
}

void ThresholdSettingsDialog::loadThresholds()
{
    QSettings settings;
    settings.beginGroup("AnalysisThresholds");
    
    for (auto it = thresholdSpinBoxes.constBegin(); it != thresholdSpinBoxes.constEnd(); ++it) {
        double value = settings.value(it.key(), defaultThresholds[it.key()].first).toDouble();
        it.value()->setValue(value);
    }
    
    settings.endGroup();
}

QMap<QString, QPair<double, QString>> ThresholdSettingsDialog::getThresholds() const
{
    QMap<QString, QPair<double, QString>> thresholds;
    
    for (auto it = thresholdSpinBoxes.constBegin(); it != thresholdSpinBoxes.constEnd(); ++it) {
        thresholds[it.key()] = qMakePair(it.value()->value(), 
                                        defaultThresholds[it.key()].second);
    }
    
    return thresholds;
}

void ThresholdSettingsDialog::setThresholds(const QMap<QString, QPair<double, QString>>& thresholds)
{
    for (auto it = thresholds.constBegin(); it != thresholds.constEnd(); ++it) {
        if (thresholdSpinBoxes.contains(it.key())) {
            thresholdSpinBoxes[it.key()]->setValue(it.value().first);
        }
    }
}

} // namespace UI
} // namespace SAR 