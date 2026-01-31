#ifndef PANEL_CLOCK_STATUS_HPP
#define PANEL_CLOCK_STATUS_HPP

#include <QObject>
#include <QString>

class QTimer;

class PanelClockStatus : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString text READ text NOTIFY textChanged)
    Q_PROPERTY(QString month READ month NOTIFY textChanged)
    Q_PROPERTY(QString day READ day NOTIFY textChanged)
    Q_PROPERTY(QString hourMin READ hourMin NOTIFY textChanged)

public:
    explicit PanelClockStatus(QObject* parent = nullptr);
    QString text() const;
    QString month() const;
    QString day() const;
    QString hourMin() const;

signals:
    void textChanged();

private:
    void updateNow();

    QTimer* m_timer = nullptr;
    QString m_month;
    QString m_day;
    QString m_hourMin;
};

#endif
