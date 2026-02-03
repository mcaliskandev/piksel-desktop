#ifndef SETTINGSWINDOW_HPP
#define SETTINGSWINDOW_HPP

#include "shell/PikselSystemClient.hpp"
#include <QString>
#include <QWidget>
#include <memory>

class QColor;
namespace Ui {
class SettingsWindow;
class Form;
}

class SettingsWindow: public QWidget
{
    Q_OBJECT
public:
    explicit SettingsWindow(QWidget *parent = nullptr);
    ~SettingsWindow() override;

signals:
    void wallpaperBackgroundColorChanged(const QString &hexColor);
    
private:
    void loadFromCore();
    void applyToCore();
    void updatePreview(const QColor &color);

private slots:
    void onChooseColor();
    void onApplyColor();

private:
    PikselSystemClient m_core;
    QString m_currentHex;
    Ui::SettingsWindow *m_ui = nullptr;
    std::unique_ptr<Ui::Form> m_wallpaperUi;
};

#endif // SETTINGSWINDOW_HPP
