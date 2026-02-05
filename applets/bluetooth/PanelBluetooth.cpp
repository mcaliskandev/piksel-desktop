#include "PanelBluetooth.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace {
struct BluetoothPayload {
    bool powered = false;
    QVariantList devices;
};

BluetoothPayload parseBluetoothJson(const QString &json)
{
    BluetoothPayload payload;
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (!doc.isObject())
        return payload;

    const QJsonObject root = doc.object();
    payload.powered = root.value(QStringLiteral("powered")).toBool(false);

    const QJsonArray arr = root.value(QStringLiteral("devices")).toArray();
    payload.devices.reserve(arr.size());
    for (const QJsonValue &v : arr) {
        if (!v.isObject())
            continue;
        const QJsonObject o = v.toObject();

        QVariantMap row;
        row.insert(QStringLiteral("address"), o.value(QStringLiteral("address")).toString());
        row.insert(QStringLiteral("name"), o.value(QStringLiteral("name")).toString());
        row.insert(QStringLiteral("connected"), o.value(QStringLiteral("connected")).toBool(false));
        payload.devices.push_back(row);
    }

    return payload;
}
} // namespace

PanelBluetoothStatus::PanelBluetoothStatus(QObject *parent)
    : QObject(parent)
    , m_core(this)
{
    connect(&m_core, &PikselSystemClient::settingFetched, this, [this](const QString &key, const QString &value) {
        if (key != QStringLiteral("bluetooth/devices"))
            return;

        const BluetoothPayload next = parseBluetoothJson(value);

        if (next.powered != m_powered) {
            m_powered = next.powered;
            emit poweredChanged();
        }
        if (next.devices != m_devices) {
            m_devices = next.devices;
            emit devicesChanged();
        }
    });

    updateNow();
}

void PanelBluetoothStatus::refresh()
{
    updateNow();
}

void PanelBluetoothStatus::updateNow()
{
    m_core.getSettingAsyncDeferred(
        QStringLiteral("bluetooth/devices"),
        QStringLiteral("{\"powered\":false,\"devices\":[]}"));
}

