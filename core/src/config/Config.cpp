#include "Config.hpp"
#include <QFile>
#include <QDir>
#include <QJsonDocument>

Config::Config(QObject *parent)
    : QObject(parent)
{
    path = QDir::homePath() + "/.config/piksel/config.json";
    load();
}

void Config::load() {
    QFile f(path);
    if (!f.exists()) {
        QDir().mkpath(QDir::homePath() + "/.config/piksel");
        data = QJsonObject();
        save();
        return;
    }

    if (!f.open(QIODevice::ReadOnly))
        return;

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    data = doc.object();
}

void Config::save() {
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly))
        return;

    QJsonDocument doc(data);
    f.write(doc.toJson());
}

QString Config::get(const QString &key) {
    return data.value(key).toString();
}

void Config::set(const QString &key, const QString &value) {
    data.insert(key, value);
    save();
}
