#include "SystemService.hpp"
#include "config/Config.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QProcess>
#include <QStandardPaths>
#include <QStringList>
#include <algorithm>

static QString scanWifiNetworksJson()
{
    // Best-effort scan using NetworkManager (nmcli). If unavailable, return an empty list.
    const QString nmcli = QStandardPaths::findExecutable(QStringLiteral("nmcli"));
    if (nmcli.isEmpty())
        return QStringLiteral("[]");

    QProcess proc;
    proc.start(nmcli,
               {QStringLiteral("-t"),
                QStringLiteral("-f"),
                QStringLiteral("SSID,SIGNAL"),
                QStringLiteral("dev"),
                QStringLiteral("wifi"),
                QStringLiteral("list"),
                QStringLiteral("--rescan"),
                QStringLiteral("yes")});

    if (!proc.waitForStarted(250))
        return QStringLiteral("[]");
    if (!proc.waitForFinished(2500))
        return QStringLiteral("[]");

    if (proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0)
        return QStringLiteral("[]");

    const QString out = QString::fromUtf8(proc.readAllStandardOutput());
    if (out.trimmed().isEmpty())
        return QStringLiteral("[]");

    // nmcli -t output is "SSID:SIGNAL" per line, but SSID may contain ":".
    // We parse by reading a trailing ":<int>" strength, and treat the rest as SSID.
    QMap<QString, int> bestStrengthBySsid;
    const QStringList lines = out.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        const int lastColon = line.lastIndexOf(QLatin1Char(':'));
        if (lastColon <= 0 || lastColon >= line.size() - 1)
            continue;

        bool ok = false;
        const int strength = line.mid(lastColon + 1).toInt(&ok);
        if (!ok)
            continue;

        const QString ssid = line.left(lastColon).trimmed();
        if (ssid.isEmpty())
            continue;

        const int clamped = std::max(0, std::min(100, strength));
        auto it = bestStrengthBySsid.find(ssid);
        if (it == bestStrengthBySsid.end() || clamped > it.value())
            bestStrengthBySsid[ssid] = clamped;
    }

    QJsonArray arr;
    for (auto it = bestStrengthBySsid.cbegin(); it != bestStrengthBySsid.cend(); ++it) {
        QJsonObject o;
        o.insert(QStringLiteral("name"), it.key());
        o.insert(QStringLiteral("strength"), it.value());
        arr.push_back(o);
    }

    const QJsonDocument doc(arr);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

static bool bluetoothPowered(const QString &bluetoothctl)
{
    QProcess showProc;
    showProc.start(bluetoothctl, {QStringLiteral("show")});
    if (!showProc.waitForStarted(250) || !showProc.waitForFinished(1500))
        return false;
    if (showProc.exitStatus() != QProcess::NormalExit || showProc.exitCode() != 0)
        return false;

    const QString out = QString::fromUtf8(showProc.readAllStandardOutput());
    const QStringList lines = out.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        const QString trimmed = line.trimmed();
        if (!trimmed.startsWith(QStringLiteral("Powered:"), Qt::CaseInsensitive))
            continue;
        return trimmed.contains(QStringLiteral("yes"), Qt::CaseInsensitive);
    }

    return false;
}

static bool bluetoothConnected(const QString &bluetoothctl, const QString &address)
{
    if (address.isEmpty())
        return false;

    QProcess infoProc;
    infoProc.start(bluetoothctl, {QStringLiteral("info"), address});
    if (!infoProc.waitForStarted(250) || !infoProc.waitForFinished(1500))
        return false;
    if (infoProc.exitStatus() != QProcess::NormalExit || infoProc.exitCode() != 0)
        return false;

    const QString out = QString::fromUtf8(infoProc.readAllStandardOutput());
    const QStringList lines = out.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        const QString trimmed = line.trimmed();
        if (!trimmed.startsWith(QStringLiteral("Connected:"), Qt::CaseInsensitive))
            continue;
        return trimmed.contains(QStringLiteral("yes"), Qt::CaseInsensitive);
    }

    return false;
}

static QString scanBluetoothDevicesJson()
{
    const QString bluetoothctl = QStandardPaths::findExecutable(QStringLiteral("bluetoothctl"));

    QJsonObject root;
    QJsonArray devices;
    root.insert(QStringLiteral("powered"), false);
    root.insert(QStringLiteral("devices"), devices);

    if (bluetoothctl.isEmpty())
        return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Compact));

    const bool powered = bluetoothPowered(bluetoothctl);
    root.insert(QStringLiteral("powered"), powered);

    QProcess devicesProc;
    devicesProc.start(bluetoothctl, {QStringLiteral("devices")});
    if (!devicesProc.waitForStarted(250) || !devicesProc.waitForFinished(2000))
        return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Compact));
    if (devicesProc.exitStatus() != QProcess::NormalExit || devicesProc.exitCode() != 0)
        return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Compact));

    const QString out = QString::fromUtf8(devicesProc.readAllStandardOutput());
    const QStringList lines = out.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        // Example: "Device 11:22:33:44:55:66 My Headphones"
        if (!line.startsWith(QStringLiteral("Device ")))
            continue;

        const QStringList parts = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        if (parts.size() < 2)
            continue;

        const QString address = parts.value(1).trimmed();
        if (address.isEmpty())
            continue;

        QString name;
        if (parts.size() > 2)
            name = parts.mid(2).join(QStringLiteral(" ")).trimmed();

        QJsonObject device;
        device.insert(QStringLiteral("address"), address);
        device.insert(QStringLiteral("name"), name);
        device.insert(QStringLiteral("connected"), bluetoothConnected(bluetoothctl, address));
        devices.push_back(device);
    }

    root.insert(QStringLiteral("devices"), devices);
    return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Compact));
}

SystemService::SystemService(Config *config, QObject *parent)
    : QObject(parent),
      m_config(config)
{
}

QString SystemService::GetSetting(const QString &key) {
    if (key == QStringLiteral("network/wifiNetworks"))
        return scanWifiNetworksJson();
    if (key == QStringLiteral("bluetooth/devices"))
        return scanBluetoothDevicesJson();
    return m_config->get(key);
}

void SystemService::SetSetting(const QString &key, const QString &value) {
    const auto oldValue = m_config->get(key);
    m_config->set(key, value);
    if (oldValue != value)
        emit SettingChanged(key, value);
}
