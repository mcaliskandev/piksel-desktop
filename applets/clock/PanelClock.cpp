#include "PanelClock.hpp"

#include <QDateTime>
#include <QLocale>
#include <QTimer>

PanelClockStatus::PanelClockStatus(QObject* parent)
    : QObject(parent)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &PanelClockStatus::updateNow);
    m_timer->start(1500);
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
    m_month = english.toString(now.date(), QStringLiteral("MMM")).toUpper();
    m_day = now.toString(QStringLiteral("dd"));
    m_hourMin = now.toString(QStringLiteral("HH:mm"));

    emit textChanged();
}
