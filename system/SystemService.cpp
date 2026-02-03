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

SystemService::SystemService(Config *config, QObject *parent)
    : QObject(parent),
      m_config(config)
{
}

QString SystemService::GetSetting(const QString &key) {
    if (key == QStringLiteral("network/wifiNetworks"))
        return scanWifiNetworksJson();
    return m_config->get(key);
}

void SystemService::SetSetting(const QString &key, const QString &value) {
    const auto oldValue = m_config->get(key);
    m_config->set(key, value);
    if (oldValue != value)
        emit SettingChanged(key, value);
}
