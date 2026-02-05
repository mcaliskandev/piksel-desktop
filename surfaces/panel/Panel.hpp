#ifndef PANEL_HPP
#define PANEL_HPP

#include <QQuickWidget>
#include <QPointF>
#include <QRect>
#include <QSize>
#include <QString>
#include <QVariant>
#include <memory>
#include <string>

#include "settings/SettingsWindow.hpp"
#include "shell/ShellComponent.hpp"

class PanelBatteryStatus;
class PanelBluetoothStatus;
class PanelClockStatus;
class PanelNetworkStatus;
class PanelRunningApps;
class AppDockModel;

class PikselPanel : public QQuickWidget, public ShellComponent {
    Q_OBJECT
public:
    PikselPanel(QWidget* parent = nullptr);
    ~PikselPanel();
    void setDockModel(AppDockModel* dockModel);
    virtual ComponentType id() const override { return ComponentType::PANEL; }
    virtual QWidget* widget() { return this; }

    Q_INVOKABLE QPointF mapToGlobalPoint(const QPointF& local) const;
    Q_INVOKABLE QPointF mapToGlobalPoint(const qreal x, const qreal y) const;
    Q_INVOKABLE QRect availableScreenGeometry() const;
    Q_INVOKABLE void onTriggerCalendar(const qreal anchorRightX, const qreal panelTopY);
    Q_INVOKABLE void onTriggerVolume(const qreal anchorRightX, const qreal panelTopY);
    Q_INVOKABLE void onTriggerNetwork(const qreal anchorCenterX, const qreal panelTopY);
    Q_INVOKABLE void onTriggerBluetooth(const qreal anchorCenterX, const qreal panelTopY);
    Q_INVOKABLE void onTriggerPinnedApps(const qreal anchorLeftX, const qreal panelTopY);
    Q_INVOKABLE void onTriggerDockContextMenu(const qreal anchorLeftX, const qreal panelTopY, const QString& appId);
    Q_INVOKABLE void hideDockContextMenu();

    SettingsWindow settingsWindow;

signals:
    void requestShow(const ComponentType &componentId);
    void wallpaperBackgroundColorChanged(const QString &hexColor);

public slots:
    void onTriggerLauncher();
    void onTriggerSettings();

private slots:
    void handleCalendarDatePicked(const QVariant &picked);

private:
    void hideCalendar();
    void hideVolume();
    void hideNetwork();
    void hideBluetooth();
    void hidePinnedApps();
    QSize calendarSize() const;
    QSize volumeSize() const;
    QSize networkSize() const;
    QSize bluetoothSize() const;
    QSize pinnedAppsSize() const;

    std::unique_ptr<PanelBatteryStatus> m_battery;
    std::unique_ptr<PanelBluetoothStatus> m_bluetooth;
    std::unique_ptr<PanelClockStatus> m_clock;
    std::unique_ptr<PanelNetworkStatus> m_network;
    std::unique_ptr<PanelRunningApps> m_runningApps;
    AppDockModel* m_dockModel = nullptr;
    QQuickWidget *m_calendarWidget = nullptr;
    QObject *m_calendarRoot = nullptr;
    QQuickWidget *m_volumeWidget = nullptr;
    QObject *m_volumeRoot = nullptr;
    QQuickWidget *m_networkWidget = nullptr;
    QObject *m_networkRoot = nullptr;
    QQuickWidget *m_bluetoothWidget = nullptr;
    QObject *m_bluetoothRoot = nullptr;
    QQuickWidget *m_pinnedAppsWidget = nullptr;
    QObject *m_pinnedAppsRoot = nullptr;
    QQuickWidget *m_dockContextWidget = nullptr;
    QObject *m_dockContextRoot = nullptr;
};

#endif // PANEL_HPP
