#pragma once

#include <QObject>
#include <QString>

class PikselCoreClient : public QObject
{
    Q_OBJECT
public:
    explicit PikselCoreClient(QObject *parent = nullptr);

    bool isAvailable() const;
    QString getSetting(const QString &key, const QString &fallback = {}) const;
    bool setSetting(const QString &key, const QString &value) const;
    Q_INVOKABLE void getSettingAsync(const QString &key, const QString &fallback = {});
    void getSettingAsyncDeferred(const QString &key, const QString &fallback = {});

signals:
    void settingChanged(const QString &key, const QString &value);
    void settingFetched(const QString &key, const QString &value);

private slots:
    void onSettingChanged(const QString &key, const QString &value);

private:
    const QString m_service;
    const QString m_path;
    const QString m_interface;
};
