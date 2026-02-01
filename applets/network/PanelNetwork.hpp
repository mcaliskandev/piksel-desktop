#pragma once

#include "shell/PikselCoreClient.hpp"

#include <QObject>
#include <QVariantList>

class PanelNetworkStatus : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList networks READ networks NOTIFY networksChanged)

public:
    explicit PanelNetworkStatus(QObject *parent = nullptr);

    QVariantList networks() const { return m_networks; }

public slots:
    void refresh();

signals:
    void networksChanged();

private:
    void updateNow();

    PikselCoreClient m_core;
    QVariantList m_networks;
};
