#include "PanelBattery.hpp"

#include <QDir>
#include <QFile>
#include <optional>

static std::optional<int> readBatteryCapacityPercent()
{
    const QDir powerSupplyDir(QStringLiteral("/sys/class/power_supply"));
    if (!powerSupplyDir.exists())
        return std::nullopt;

    const QStringList batteryDirs =
        powerSupplyDir.entryList({QStringLiteral("BAT*")}, QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

    for (const QString &batteryDirName : batteryDirs) {
        QFile capacityFile(powerSupplyDir.filePath(batteryDirName + QStringLiteral("/capacity")));
        if (!capacityFile.open(QIODevice::ReadOnly | QIODevice::Text)) continue;

        const QString raw = QString::fromUtf8(capacityFile.readAll()).trimmed();
        bool ok = false;
        const int value = raw.toInt(&ok);
        if (ok && value >= 0 && value <= 100)
            return value;
    }

    return std::nullopt;
}

PanelBatteryStatus::PanelBatteryStatus(QObject* parent)
    : QObject(parent)
{
    connect(&m_timer, &QTimer::timeout, this, &PanelBatteryStatus::updateNow);
    m_timer.start(30000);
    updateNow();
}

void PanelBatteryStatus::refresh()
{
    updateNow();
}

void PanelBatteryStatus::updateNow()
{
    const std::optional<int> percentage = readBatteryCapacityPercent();
    const bool nextAvailable = percentage.has_value();
    const int nextPercentage = nextAvailable ? *percentage : -1;

    if (m_available != nextAvailable) {
        m_available = nextAvailable;
        emit availableChanged();
    }
    if (m_percentage != nextPercentage) {
        m_percentage = nextPercentage;
        emit percentageChanged();
    }
}
