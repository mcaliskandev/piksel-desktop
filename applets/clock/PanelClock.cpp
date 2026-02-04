#include "PanelClock.hpp"

#include <QDateTime>
#include <QLocale>
#include <QTimer>

PanelClockStatus::PanelClockStatus(QObject* parent)
    : QObject(parent)
{
    connect(&m_timer, &QTimer::timeout, this, &PanelClockStatus::updateNow);
    m_timer.start(1000);
    updateNow();
}

QString PanelClockStatus::text() const
{
    return QStringLiteral("%1-%2 %3").arg(m_month, m_day, m_hourMin);
}

QString PanelClockStatus::month() const
{
    return m_month;
}

QString PanelClockStatus::day() const
{
    return m_day;
}

QString PanelClockStatus::hourMin() const
{
    return m_hourMin;
}

void PanelClockStatus::updateNow()
{
    const QDateTime now = QDateTime::currentDateTime();
    const QLocale english(QLocale::English);

    const QString nextMonth = english.toString(now.date(), QStringLiteral("MMM")).toUpper();
    const QString nextDay = now.toString(QStringLiteral("dd"));
    const QString nextHourMin = now.toString(QStringLiteral("HH:mm"));

    if (nextMonth == m_month && nextDay == m_day && nextHourMin == m_hourMin)
        return;

    m_month = nextMonth;
    m_day = nextDay;
    m_hourMin = nextHourMin;
    emit textChanged();
}
