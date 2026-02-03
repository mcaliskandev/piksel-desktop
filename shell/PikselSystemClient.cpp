#include "PikselSystemClient.hpp"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusReply>
#include <QTimer>

PikselSystemClient::PikselSystemClient(QObject *parent)
    : QObject(parent),
      m_service(QStringLiteral("org.piksel.System")),
      m_path(QStringLiteral("/org/piksel/System")),
      m_interface(QStringLiteral("org.piksel.System"))
{
    QDBusConnection::sessionBus().connect(
        m_service,
        m_path,
        m_interface,
        QStringLiteral("SettingChanged"),
        this,
        SLOT(onSettingChanged(QString,QString)));
}

bool PikselSystemClient::isAvailable() const
{
    auto iface = QDBusConnection::sessionBus().interface();
    return iface && iface->isServiceRegistered(m_service);
}

void PikselSystemClient::getSettingAsync(const QString &key, const QString &fallback)
{
    QDBusInterface iface(m_service, m_path, m_interface, QDBusConnection::sessionBus());
    if (!iface.isValid()) {
        qWarning().noquote() << "PikselSystemClient: DBus iface invalid for async GetSetting(" << key << ")";
        emit settingFetched(key, fallback);
        return;
    }

    auto *watcher = new QDBusPendingCallWatcher(iface.asyncCall(QStringLiteral("GetSetting"), key), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher, key, fallback]() {
        QDBusPendingReply<QString> reply(*watcher);
        watcher->deleteLater();
        emit settingFetched(key, reply.isValid() ? reply.value() : fallback);
    });
}

void PikselSystemClient::getSettingAsyncDeferred(const QString &key, const QString &fallback)
{
    QTimer::singleShot(0, this, [this, key, fallback]() {
        getSettingAsync(key, fallback);
    });
}

QString PikselSystemClient::getSetting(const QString &key, const QString &fallback) const
{
    QDBusInterface iface(m_service, m_path, m_interface, QDBusConnection::sessionBus());
    if (!iface.isValid())
    {
        qWarning().noquote() << "PikselSystemClient: DBus iface invalid for GetSetting(" << key << ")";
        return fallback;
    }

    QDBusReply<QString> reply = iface.call(QStringLiteral("GetSetting"), key);
    const QString value = reply.isValid() ? reply.value() : fallback;
    if (key == QStringLiteral("network/wifiNetworks") && value.isEmpty())
        qWarning() << "PikselSystemClient: network/wifiNetworks returned empty string (likely old PikselSystem); try stopping old PikselSystem";
    return value;
}

bool PikselSystemClient::setSetting(const QString &key, const QString &value) const
{
    QDBusInterface iface(m_service, m_path, m_interface, QDBusConnection::sessionBus());
    if (!iface.isValid())
    {
        qWarning().noquote() << "PikselSystemClient: DBus iface invalid for SetSetting(" << key << ")";
        return false;
    }

    QDBusReply<void> reply = iface.call(QStringLiteral("SetSetting"), key, value);
    return reply.isValid();
}

void PikselSystemClient::onSettingChanged(const QString &key, const QString &value)
{
    emit settingChanged(key, value);
}
