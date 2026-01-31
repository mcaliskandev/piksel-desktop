#ifndef PANEL_BATTERY_STATUS_HPP
#define PANEL_BATTERY_STATUS_HPP

#include <QObject>

class QTimer;

class PanelBatteryStatus : public QObject {
    Q_OBJECT
    Q_PROPERTY(int percentage READ percentage NOTIFY percentageChanged)
    Q_PROPERTY(bool available READ available NOTIFY availableChanged)

public:
    explicit PanelBatteryStatus(QObject* parent = nullptr);

    int percentage() const { return m_percentage; }
    bool available() const { return m_available; }

public slots:
    void refresh();

signals:
    void percentageChanged();
    void availableChanged();

private:
    void updateNow();

    QTimer* m_timer = nullptr;
    int m_percentage = -1;
    bool m_available = false;
};

#endif
