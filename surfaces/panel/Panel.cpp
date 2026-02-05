#include "Panel.hpp"

#include "applets/battery/PanelBattery.hpp"
#include "applets/bluetooth/PanelBluetooth.hpp"
#include "applets/clock/PanelClock.hpp"
#include "applets/network/PanelNetwork.hpp"
#include "applets/runningapps/PanelRunningApps.hpp"

#include <QDebug>
#include <QGuiApplication>
#include <QQmlContext>
#include <QPoint>
#include <QQuickItem>
#include <QScreen>
#include <algorithm>
#include <iostream>

#include "shell/AppDockModel.hpp"

PikselPanel::PikselPanel(QWidget* parent)
    : QQuickWidget(parent)
{
    if (!parent)
        setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::BypassWindowManagerHint);

    setResizeMode(QQuickWidget::SizeRootObjectToView);

    connect(&settingsWindow, &SettingsWindow::wallpaperBackgroundColorChanged, this, &PikselPanel::wallpaperBackgroundColorChanged);

    rootContext()->setContextProperty("panel", this);
    m_battery = std::make_unique<PanelBatteryStatus>(this);
    m_bluetooth = std::make_unique<PanelBluetoothStatus>(this);
    m_clock = std::make_unique<PanelClockStatus>(this);
    m_network = std::make_unique<PanelNetworkStatus>(this);
    m_runningApps = std::make_unique<PanelRunningApps>(this);
    rootContext()->setContextProperty("panelBattery", m_battery.get());
    rootContext()->setContextProperty("panelBluetooth", m_bluetooth.get());
    rootContext()->setContextProperty("panelClock", m_clock.get());
    rootContext()->setContextProperty("panelNetwork", m_network.get());
    rootContext()->setContextProperty("panelRunningApps", m_runningApps.get());
    rootContext()->setContextProperty("dockApps", m_dockModel);

    setSource(QUrl(QStringLiteral("qrc:/surfaces/panel/PikselPanel.qml")));
    if (status() != QQuickWidget::Ready) {
        qCritical() << "Failed to load QML panel:" << errors();
    }

    m_calendarWidget = new QQuickWidget(parent);
    m_calendarWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_calendarWidget->setClearColor(Qt::transparent);
    m_calendarWidget->setSource(QUrl(QStringLiteral("qrc:/surfaces/panel/CalendarOverlay.qml")));
    if (m_calendarWidget->status() != QQuickWidget::Ready) {
        qCritical() << "Failed to load QML calendar:" << m_calendarWidget->errors();
    }
    m_calendarRoot = m_calendarWidget->rootObject();
    const QSize size = calendarSize();
    if (size.isValid())
        m_calendarWidget->setFixedSize(size);
    m_calendarWidget->hide();
    if (m_calendarRoot) {
        connect(m_calendarRoot, SIGNAL(datePicked(QVariant)), this, SLOT(handleCalendarDatePicked(QVariant)));
    }

    m_volumeWidget = new QQuickWidget(parent);
    m_volumeWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_volumeWidget->setClearColor(Qt::transparent);
    m_volumeWidget->setSource(QUrl(QStringLiteral("qrc:/surfaces/panel/VolumeOverlay.qml")));
    if (m_volumeWidget->status() != QQuickWidget::Ready) {
        qCritical() << "Failed to load QML volume:" << m_volumeWidget->errors();
    }
    m_volumeRoot = m_volumeWidget->rootObject();
    const QSize overlaySize = volumeSize();
    if (overlaySize.isValid())
        m_volumeWidget->setFixedSize(overlaySize);
    m_volumeWidget->hide();

    m_networkWidget = new QQuickWidget(parent);
    m_networkWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_networkWidget->setClearColor(Qt::transparent);
    m_networkWidget->rootContext()->setContextProperty("panelNetwork", m_network.get());
    m_networkWidget->setSource(QUrl(QStringLiteral("qrc:/surfaces/panel/NetworkOverlay.qml")));
    if (m_networkWidget->status() != QQuickWidget::Ready) {
        qCritical() << "Failed to load QML network:" << m_networkWidget->errors();
    }
    m_networkRoot = m_networkWidget->rootObject();
    const QSize overlayNetworkSize = networkSize();
    if (overlayNetworkSize.isValid())
        m_networkWidget->setFixedSize(overlayNetworkSize);
    m_networkWidget->hide();

    m_bluetoothWidget = new QQuickWidget(parent);
    m_bluetoothWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_bluetoothWidget->setClearColor(Qt::transparent);
    m_bluetoothWidget->rootContext()->setContextProperty("panelBluetooth", m_bluetooth.get());
    m_bluetoothWidget->setSource(QUrl(QStringLiteral("qrc:/surfaces/panel/BluetoothOverlay.qml")));
    if (m_bluetoothWidget->status() != QQuickWidget::Ready) {
        qCritical() << "Failed to load QML bluetooth:" << m_bluetoothWidget->errors();
    }
    m_bluetoothRoot = m_bluetoothWidget->rootObject();
    const QSize overlayBluetoothSize = bluetoothSize();
    if (overlayBluetoothSize.isValid())
        m_bluetoothWidget->setFixedSize(overlayBluetoothSize);
    m_bluetoothWidget->hide();

    m_pinnedAppsWidget = new QQuickWidget(parent);
    m_pinnedAppsWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_pinnedAppsWidget->setClearColor(Qt::transparent);
    m_pinnedAppsWidget->rootContext()->setContextProperty("dockApps", m_dockModel);
    m_pinnedAppsWidget->setSource(QUrl(QStringLiteral("qrc:/surfaces/panel/PinnedAppsOverlay.qml")));
    if (m_pinnedAppsWidget->status() != QQuickWidget::Ready) {
        qCritical() << "Failed to load QML pinned apps:" << m_pinnedAppsWidget->errors();
    }
    m_pinnedAppsRoot = m_pinnedAppsWidget->rootObject();
    const QSize overlayPinnedSize = pinnedAppsSize();
    if (overlayPinnedSize.isValid())
        m_pinnedAppsWidget->setFixedSize(overlayPinnedSize);
    m_pinnedAppsWidget->hide();

    m_dockContextWidget = new QQuickWidget(nullptr);
    m_dockContextWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_dockContextWidget->setClearColor(Qt::transparent);
    m_dockContextWidget->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    m_dockContextWidget->setAttribute(Qt::WA_TranslucentBackground);
    m_dockContextWidget->rootContext()->setContextProperty("panel", this);
    m_dockContextWidget->rootContext()->setContextProperty("dockApps", m_dockModel);
    m_dockContextWidget->setSource(QUrl(QStringLiteral("qrc:/surfaces/panel/DockContextMenuOverlay.qml")));
    if (m_dockContextWidget->status() != QQuickWidget::Ready) {
        qCritical() << "Failed to load QML dock context menu:" << m_dockContextWidget->errors();
    }
    m_dockContextRoot = m_dockContextWidget->rootObject();
    if (m_dockContextRoot) {
        const int w = qRound(m_dockContextRoot->property("implicitWidth").toReal());
        const int h = qRound(m_dockContextRoot->property("implicitHeight").toReal());
        if (w > 0 && h > 0)
            m_dockContextWidget->setFixedSize(w, h);
    }
    m_dockContextWidget->hide();
}

PikselPanel::~PikselPanel() = default;

void PikselPanel::setDockModel(AppDockModel* dockModel)
{
    m_dockModel = dockModel;
    rootContext()->setContextProperty("dockApps", m_dockModel);
    if (m_pinnedAppsWidget)
        m_pinnedAppsWidget->rootContext()->setContextProperty("dockApps", m_dockModel);
    if (m_dockContextWidget)
        m_dockContextWidget->rootContext()->setContextProperty("dockApps", m_dockModel);
}

QPointF PikselPanel::mapToGlobalPoint(const QPointF& local) const {
    const QPoint global = QWidget::mapToGlobal(local.toPoint());
    return QPointF(global);
}

QPointF PikselPanel::mapToGlobalPoint(const qreal x, const qreal y) const {
    return mapToGlobalPoint(QPointF(x, y));
}

QRect PikselPanel::availableScreenGeometry() const {
    if (const QScreen* currentScreen = screen())
        return currentScreen->availableGeometry();
    if (const QScreen* primary = QGuiApplication::primaryScreen())
        return primary->availableGeometry();
    return QRect(0, 0, width(), height());
}

void PikselPanel::onTriggerDockContextMenu(const qreal anchorLeftX, const qreal panelTopY, const QString& appId)
{
    if (!m_dockContextWidget || !m_dockContextRoot)
        return;

    m_dockContextRoot->setProperty("appId", appId);

    const int implicitW = qRound(m_dockContextRoot->property("implicitWidth").toReal());
    const int implicitH = qRound(m_dockContextRoot->property("implicitHeight").toReal());
    if (implicitW > 0 && implicitH > 0)
        m_dockContextWidget->setFixedSize(implicitW, implicitH);

    const int margin = 8;
    const int anchorSpacing = 6;
    const QPoint anchorPoint(qRound(anchorLeftX), qRound(panelTopY));

    QScreen* screen = QGuiApplication::screenAt(anchorPoint);
    if (!screen)
        screen = QGuiApplication::primaryScreen();

    const QRect screenGeo = screen ? screen->availableGeometry() : QRect(0, 0, 1920, 1080);

    int x = qRound(anchorLeftX);
    int y = qRound(panelTopY) - m_dockContextWidget->height() - anchorSpacing;
    if (y < screenGeo.y() + margin)
        y = qRound(panelTopY) + anchorSpacing;

    x = std::clamp(x, screenGeo.x() + margin, screenGeo.x() + screenGeo.width() - m_dockContextWidget->width() - margin);
    y = std::clamp(y, screenGeo.y() + margin, screenGeo.y() + screenGeo.height() - m_dockContextWidget->height() - margin);

    m_dockContextWidget->move(x, y);
    m_dockContextWidget->show();
    m_dockContextWidget->raise();
    m_dockContextWidget->setFocus();
}

void PikselPanel::hideDockContextMenu()
{
    if (m_dockContextWidget)
        m_dockContextWidget->hide();
}

void PikselPanel::onTriggerLauncher() {
    std::clog << "Panel requested show launcher" << std::endl;
    emit requestShow(ComponentType::LAUNCHER);
}

void PikselPanel::onTriggerSettings() {
    std::clog << "Show Settings " << std::endl;
    settingsWindow.show();
    settingsWindow.raise();
    settingsWindow.activateWindow();
}

void PikselPanel::onTriggerCalendar(const qreal anchorRightX, const qreal panelTopY) {
    if (!m_calendarWidget)
        return;
    if (m_calendarWidget->isVisible()) {
        hideCalendar();
        return;
    }

    const QSize size = calendarSize();
    if (size.isValid())
        m_calendarWidget->setFixedSize(size);

    const int margin = 8;
    QScreen *screen = nullptr;
    if (m_calendarWidget->parentWidget())
        screen = m_calendarWidget->parentWidget()->screen();
    if (!screen)
        screen = QGuiApplication::primaryScreen();

    QRect screenGeo(0, 0, 0, 0);
    if (screen)
        screenGeo = screen->availableGeometry();

    int x = qRound(anchorRightX) - m_calendarWidget->width();
    int y = qRound(panelTopY) - m_calendarWidget->height();
    if (screenGeo.isValid()) {
        const int minX = screenGeo.x() + margin;
        const int maxX = screenGeo.x() + screenGeo.width() - m_calendarWidget->width() - margin;
        const int minY = screenGeo.y() + margin;
        const int maxY = screenGeo.y() + screenGeo.height() - m_calendarWidget->height() - margin;
        x = std::max(minX, std::min(x, maxX));
        y = std::max(minY, std::min(y, maxY));
    }

    if (QWidget *parent = m_calendarWidget->parentWidget()) {
        const QPoint local = parent->mapFromGlobal(QPoint(x, y));
        m_calendarWidget->move(local);
    } else {
        m_calendarWidget->move(x, y);
    }

    m_calendarWidget->show();
    m_calendarWidget->raise();
}

void PikselPanel::onTriggerVolume(const qreal anchorRightX, const qreal panelTopY) {
    if (!m_volumeWidget)
        return;
    if (m_volumeWidget->isVisible()) {
        hideVolume();
        return;
    }

    const QSize size = volumeSize();
    if (size.isValid())
        m_volumeWidget->setFixedSize(size);

    const int margin = 8;
    const int anchorSpacing = 6;
    QScreen *screen = nullptr;
    if (m_volumeWidget->parentWidget())
        screen = m_volumeWidget->parentWidget()->screen();
    if (!screen)
        screen = QGuiApplication::primaryScreen();

    QRect screenGeo(0, 0, 0, 0);
    if (screen)
        screenGeo = screen->availableGeometry();

    const int halfWidth = m_volumeWidget->width() / 2;
    int x = qRound(anchorRightX) - halfWidth;
    int y = qRound(panelTopY) - m_volumeWidget->height() - anchorSpacing;
    if (screenGeo.isValid()) {
        const int minX = screenGeo.x() + margin;
        const int maxX = screenGeo.x() + screenGeo.width() - m_volumeWidget->width() - margin;
        const int minY = screenGeo.y() + margin;
        const int maxY = screenGeo.y() + screenGeo.height() - m_volumeWidget->height() - margin;
        x = std::max(minX, std::min(x, maxX));
        y = std::max(minY, std::min(y, maxY));
    }

    if (QWidget *parent = m_volumeWidget->parentWidget()) {
        const QPoint local = parent->mapFromGlobal(QPoint(x, y));
        m_volumeWidget->move(local);
    } else {
        m_volumeWidget->move(x, y);
    }

    m_volumeWidget->show();
    m_volumeWidget->raise();
}

void PikselPanel::onTriggerNetwork(const qreal anchorCenterX, const qreal panelTopY) {
    if (!m_networkWidget)
        return;
    if (m_networkWidget->isVisible()) {
        hideNetwork();
        return;
    }

    if (m_network)
        m_network->refresh();

    const QSize size = networkSize();
    if (size.isValid())
        m_networkWidget->setFixedSize(size);

    const int margin = 8;
    const int anchorSpacing = 6;
    QScreen *screen = nullptr;
    if (m_networkWidget->parentWidget())
        screen = m_networkWidget->parentWidget()->screen();
    if (!screen)
        screen = QGuiApplication::primaryScreen();

    QRect screenGeo(0, 0, 0, 0);
    if (screen)
        screenGeo = screen->availableGeometry();

    const int halfWidth = m_networkWidget->width() / 2;
    int x = qRound(anchorCenterX) - halfWidth;
    int y = qRound(panelTopY) - m_networkWidget->height() - anchorSpacing;
    if (screenGeo.isValid()) {
        const int minX = screenGeo.x() + margin;
        const int maxX = screenGeo.x() + screenGeo.width() - m_networkWidget->width() - margin;
        const int minY = screenGeo.y() + margin;
        const int maxY = screenGeo.y() + screenGeo.height() - m_networkWidget->height() - margin;
        x = std::max(minX, std::min(x, maxX));
        y = std::max(minY, std::min(y, maxY));
    }

    if (QWidget *parent = m_networkWidget->parentWidget()) {
        const QPoint local = parent->mapFromGlobal(QPoint(x, y));
        m_networkWidget->move(local);
    } else {
        m_networkWidget->move(x, y);
    }

    m_networkWidget->show();
    m_networkWidget->raise();
}

void PikselPanel::onTriggerBluetooth(const qreal anchorCenterX, const qreal panelTopY)
{
    if (!m_bluetoothWidget)
        return;
    if (m_bluetoothWidget->isVisible()) {
        hideBluetooth();
        return;
    }

    if (m_bluetooth)
        m_bluetooth->refresh();

    const QSize size = bluetoothSize();
    if (size.isValid())
        m_bluetoothWidget->setFixedSize(size);

    const int margin = 8;
    const int anchorSpacing = 6;
    QScreen *screen = nullptr;
    if (m_bluetoothWidget->parentWidget())
        screen = m_bluetoothWidget->parentWidget()->screen();
    if (!screen)
        screen = QGuiApplication::primaryScreen();

    QRect screenGeo(0, 0, 0, 0);
    if (screen)
        screenGeo = screen->availableGeometry();

    const int halfWidth = m_bluetoothWidget->width() / 2;
    int x = qRound(anchorCenterX) - halfWidth;
    int y = qRound(panelTopY) - m_bluetoothWidget->height() - anchorSpacing;
    if (screenGeo.isValid()) {
        const int minX = screenGeo.x() + margin;
        const int maxX = screenGeo.x() + screenGeo.width() - m_bluetoothWidget->width() - margin;
        const int minY = screenGeo.y() + margin;
        const int maxY = screenGeo.y() + screenGeo.height() - m_bluetoothWidget->height() - margin;
        x = std::max(minX, std::min(x, maxX));
        y = std::max(minY, std::min(y, maxY));
    }

    if (QWidget *parent = m_bluetoothWidget->parentWidget()) {
        const QPoint local = parent->mapFromGlobal(QPoint(x, y));
        m_bluetoothWidget->move(local);
    } else {
        m_bluetoothWidget->move(x, y);
    }

    m_bluetoothWidget->show();
    m_bluetoothWidget->raise();
}

void PikselPanel::onTriggerPinnedApps(const qreal anchorLeftX, const qreal panelTopY)
{
    if (!m_pinnedAppsWidget)
        return;
    if (m_pinnedAppsWidget->isVisible()) {
        hidePinnedApps();
        return;
    }

    const QSize size = pinnedAppsSize();
    if (size.isValid())
        m_pinnedAppsWidget->setFixedSize(size);

    const int margin = 8;
    const int anchorSpacing = 6;
    QScreen *screen = nullptr;
    if (m_pinnedAppsWidget->parentWidget())
        screen = m_pinnedAppsWidget->parentWidget()->screen();
    if (!screen)
        screen = QGuiApplication::primaryScreen();

    QRect screenGeo(0, 0, 0, 0);
    if (screen)
        screenGeo = screen->availableGeometry();

    int x = qRound(anchorLeftX);
    int y = qRound(panelTopY) - m_pinnedAppsWidget->height() - anchorSpacing;
    if (screenGeo.isValid()) {
        const int minX = screenGeo.x() + margin;
        const int maxX = screenGeo.x() + screenGeo.width() - m_pinnedAppsWidget->width() - margin;
        const int minY = screenGeo.y() + margin;
        const int maxY = screenGeo.y() + screenGeo.height() - m_pinnedAppsWidget->height() - margin;
        x = std::max(minX, std::min(x, maxX));
        y = std::max(minY, std::min(y, maxY));
    }

    if (QWidget *parent = m_pinnedAppsWidget->parentWidget()) {
        const QPoint local = parent->mapFromGlobal(QPoint(x, y));
        m_pinnedAppsWidget->move(local);
    } else {
        m_pinnedAppsWidget->move(x, y);
    }

    m_pinnedAppsWidget->show();
    m_pinnedAppsWidget->raise();
}

void PikselPanel::hideCalendar() {
    if (m_calendarWidget)
        m_calendarWidget->hide();
}

void PikselPanel::hideVolume() {
    if (m_volumeWidget)
        m_volumeWidget->hide();
}

void PikselPanel::hideNetwork() {
    if (m_networkWidget)
        m_networkWidget->hide();
}

void PikselPanel::hideBluetooth()
{
    if (m_bluetoothWidget)
        m_bluetoothWidget->hide();
}

void PikselPanel::hidePinnedApps()
{
    if (m_pinnedAppsWidget)
        m_pinnedAppsWidget->hide();
}

void PikselPanel::handleCalendarDatePicked(const QVariant &picked) {
    Q_UNUSED(picked);
    hideCalendar();
}

QSize PikselPanel::calendarSize() const {
    if (!m_calendarRoot)
        return QSize();
    const QVariant w = m_calendarRoot->property("implicitWidth");
    const QVariant h = m_calendarRoot->property("implicitHeight");
    if (w.isValid() && h.isValid())
        return QSize(w.toInt(), h.toInt());
    return QSize();
}

QSize PikselPanel::volumeSize() const {
    if (!m_volumeRoot)
        return QSize();
    const QVariant w = m_volumeRoot->property("implicitWidth");
    const QVariant h = m_volumeRoot->property("implicitHeight");
    if (w.isValid() && h.isValid())
        return QSize(w.toInt(), h.toInt());
    return QSize();
}

QSize PikselPanel::networkSize() const {
    if (!m_networkRoot)
        return QSize();
    const QVariant w = m_networkRoot->property("implicitWidth");
    const QVariant h = m_networkRoot->property("implicitHeight");
    if (w.isValid() && h.isValid())
        return QSize(w.toInt(), h.toInt());
    return QSize();
}

QSize PikselPanel::bluetoothSize() const
{
    if (!m_bluetoothRoot)
        return QSize();
    const QVariant w = m_bluetoothRoot->property("implicitWidth");
    const QVariant h = m_bluetoothRoot->property("implicitHeight");
    if (w.isValid() && h.isValid())
        return QSize(w.toInt(), h.toInt());
    return QSize();
}

QSize PikselPanel::pinnedAppsSize() const
{
    if (!m_pinnedAppsRoot)
        return QSize();
    const QVariant w = m_pinnedAppsRoot->property("implicitWidth");
    const QVariant h = m_pinnedAppsRoot->property("implicitHeight");
    if (w.isValid() && h.isValid())
        return QSize(w.toInt(), h.toInt());
    return QSize();
}

#ifdef TEST_PANEL
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    PikselPanel panel(500, 10);
    panel.move(0, 0);
    panel.show();
    return app.exec();
}
#endif
