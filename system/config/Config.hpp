#pragma once
#include <QObject>
#include <QJsonObject>
#include <QString>

class Config : public QObject {
    Q_OBJECT
public:
    explicit Config(QObject *parent = nullptr);

    QString get(const QString &key);
    void set(const QString &key, const QString &value);

private:
    QString path;
    QJsonObject data;

    void load();
    void save();
};
