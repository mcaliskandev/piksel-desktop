#ifndef WALLPAPER_HPP
#define WALLPAPER_HPP

#include <QColor>
#include <QQuickWidget>
#include <QString>
#include <string>
#include "shell/PikselSystemClient.hpp"
#include "shell/ShellComponent.hpp"

class QQuickItem;

class PikselWallpaper : public QQuickWidget, public ShellComponent {
    Q_OBJECT
public:
    PikselWallpaper(QWidget* parent = nullptr);
    virtual ComponentType id() const override { return ComponentType::WALLPAPER; }
    virtual QWidget* widget() { return this; }

public slots:
    void applyColor(const QString &hexColor);

private slots:
    void onCoreSettingChanged(const QString &key, const QString &value);

private:
    void syncColorToQml();

    QQuickItem* m_rootItem = nullptr;
    QColor m_backgroundColor;
    PikselSystemClient m_core;
};

#endif // WALLPAPER_HPP
