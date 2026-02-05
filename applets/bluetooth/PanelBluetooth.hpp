#pragma once

#include "shell/PikselSystemClient.hpp"

#include <QObject>
#include <QVariantList>

class PanelBluetoothStatus : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool powered READ powered NOTIFY poweredChanged)
    Q_PROPERTY(QVariantList devices READ devices NOTIFY devicesChanged)

public:
    explicit PanelBluetoothStatus(QObject *parent = nullptr);

    bool powered() const { return m_powered; }
    QVariantList devices() const { return m_devices; }

public slots:
    void refresh();

signals:
    void poweredChanged();
    void devicesChanged();

private:
    void updateNow();

    PikselSystemClient m_core;
    bool m_powered = false;
    QVariantList m_devices;
};

