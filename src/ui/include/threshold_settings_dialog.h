#ifndef SAR_THRESHOLD_SETTINGS_DIALOG_H
#define SAR_THRESHOLD_SETTINGS_DIALOG_H

#include <QDialog>
#include <QMap>
#include <QString>
#include <QDoubleSpinBox>

namespace SAR {
namespace UI {

class ThresholdSettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit ThresholdSettingsDialog(QWidget* parent = nullptr);
    ~ThresholdSettingsDialog() override;

    // 获取当前设置的阈值
    QMap<QString, QPair<double, QString>> getThresholds() const;
    // 设置当前阈值
    void setThresholds(const QMap<QString, QPair<double, QString>>& thresholds);

private:
    void setupUI();
    void loadDefaultThresholds();
    void saveThresholds();
    void loadThresholds();

    QMap<QString, QDoubleSpinBox*> thresholdSpinBoxes;
    QMap<QString, QPair<double, QString>> defaultThresholds;
};

} // namespace UI
} // namespace SAR

#endif // SAR_THRESHOLD_SETTINGS_DIALOG_H 