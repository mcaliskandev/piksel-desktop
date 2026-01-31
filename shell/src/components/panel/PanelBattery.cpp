#include "PanelBattery.hpp"

#include <QDir>
#include <QFile>
#include <QTimer>

static QString readBatteryCapacityPercent()
{
    const QDir powerSupplyDir(QStringLiteral("/sys/class/power_supply"));
    if (!powerSupplyDir.exists()) return {};

    const QStringList batteryDirs =
        powerSupplyDir.entryList({QStringLiteral("BAT*")}, QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

    for (const QString &batteryDirName : batteryDirs) {
        QFile capacityFile(powerSupplyDir.filePath(batteryDirName + QStringLiteral("/capacity")));
        if (!capacityFile.open(QIODevice::ReadOnly | QIODevice::Text)) continue;

        const QString raw = QString::fromUtf8(capacityFile.readAll()).trimmed();
        bool ok = false;
        const int value = raw.toInt(&ok);
        if (ok && value >= 0 && value <= 100) return QString::number(value);
    }

    return {};
}

PanelBatteryStatus::PanelBatteryStatus(QObject* parent)
    : QObject(parent)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &PanelBatteryStatus::updateNow);
    m_timer->start(30000);
    updateNow();
}

void PanelBatteryStatus::refresh()
{
    updateNow();
}

void PanelBatteryStatus::updateNow()
{
    const QString raw = readBatteryCapacityPercent();
    const bool nextAvailable = !raw.isEmpty();
    const int nextPercentage = nextAvailable ? raw.toInt() : -1;

    if (m_available != nextAvailable) {
        m_available = nextAvailable;
        emit availableChanged();
    }
    if (m_percentage != nextPercentage) {
        m_percentage = nextPercentage;
        emit percentageChanged();
    }
}
