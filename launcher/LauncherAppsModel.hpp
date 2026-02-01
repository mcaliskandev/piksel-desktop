#pragma once

#include "shell/PikselCoreClient.hpp"

#include <QObject>
#include <QVariantList>

class LauncherAppsModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList apps READ apps NOTIFY appsChanged)

public:
    explicit LauncherAppsModel(QObject *parent = nullptr);

    QVariantList apps() const;

public slots:
    void refresh();

signals:
    void appsChanged();

private:
    static QVariantList parseAppsJson(const QString &json);
    static QVariantList scanInstalledDesktopApps();
    static QVariantMap makeFileManagerEntry();
    static QString normalizeId(const QString &s);
    static QString execToProgramKey(const QString &exec);

    void setApps(QVariantList next);
    void updateFromCoreOrFallback(const QString &json);

    PikselCoreClient m_core;
    QVariantList m_apps;
};
