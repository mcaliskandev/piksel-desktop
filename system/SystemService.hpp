#pragma once
#include <QObject>

class Config;

class SystemService : public QObject {
    Q_OBJECT
public:
    explicit SystemService(Config *config, QObject *parent = nullptr);

public slots:
    QString GetSetting(const QString &key);
    void SetSetting(const QString &key, const QString &value);

signals:
    void SettingChanged(const QString &key, const QString &value);

private:
    Config *m_config;
};
