#include "PanelNetwork.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

static QVariantList parseNetworksJson(const QString &json)
{
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (!doc.isArray())
        return {};

    QVariantList result;
    const QJsonArray arr = doc.array();
    result.reserve(arr.size());
    for (const QJsonValue &v : arr) {
        if (!v.isObject())
            continue;
        const QJsonObject o = v.toObject();

        const QString name = o.value(QStringLiteral("name")).toString(o.value(QStringLiteral("ssid")).toString());
        const int strength = o.value(QStringLiteral("strength")).toInt(o.value(QStringLiteral("signal")).toInt(-1));

        QVariantMap row;
        row.insert(QStringLiteral("name"), name);
        row.insert(QStringLiteral("strength"), strength);
        result.push_back(row);
    }
    return result;
}

PanelNetworkStatus::PanelNetworkStatus(QObject *parent)
    : QObject(parent)
    , m_core(this)
{
    connect(&m_core, &PikselSystemClient::settingFetched, this, [this](const QString &key, const QString &value) {
        if (key != QStringLiteral("network/wifiNetworks"))
            return;
        const QVariantList next = parseNetworksJson(value);
        if (next != m_networks) {
            m_networks = next;
            emit networksChanged();
        }
    });

    updateNow();
}

void PanelNetworkStatus::refresh()
{
    updateNow();
}

void PanelNetworkStatus::updateNow()
{
    // Async DBus call to avoid UI freeze while PikselSystem runs `nmcli`.
    m_core.getSettingAsyncDeferred(QStringLiteral("network/wifiNetworks"), QStringLiteral("[]"));
}
