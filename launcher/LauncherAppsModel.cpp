#include "LauncherAppsModel.hpp"

#include <QDirIterator>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>

namespace {
const QString kCoreAppsKey = QStringLiteral("launcher/apps");

bool isProbablyPath(const QString &s)
{
    return s.contains(QLatin1Char('/')) || s.contains(QLatin1Char('.'));
}

QVariantMap makeEntry(QString id,
                      QString name,
                      QString action,
                      QString exec,
                      QString iconSource,
                      QString iconName)
{
    QVariantMap m;
    m.insert(QStringLiteral("appId"), std::move(id));
    m.insert(QStringLiteral("appName"), std::move(name));
    m.insert(QStringLiteral("appAction"), std::move(action));
    m.insert(QStringLiteral("appExec"), std::move(exec));
    m.insert(QStringLiteral("appIconSource"), std::move(iconSource));
    m.insert(QStringLiteral("appIconName"), std::move(iconName));
    return m;
}

QString normalizeDesktopIdFromPath(const QString &path)
{
    QString base = QFileInfo(path).fileName();
    if (base.endsWith(QStringLiteral(".desktop"), Qt::CaseInsensitive))
        base.chop(8);
    return base.trimmed().toLower();
}

QString extractDesktopEntryString(QSettings &desktop, const QString &key)
{
    const QVariant v = desktop.value(key);
    const QString s = v.toString().trimmed();
    return s;
}
} // namespace

LauncherAppsModel::LauncherAppsModel(QObject *parent)
    : QObject(parent)
    , m_core(this)
{
    connect(&m_core, &PikselSystemClient::settingFetched, this, [this](const QString &key, const QString &value) {
        if (key != kCoreAppsKey)
            return;
        updateFromCoreOrFallback(value);
    });
    connect(&m_core, &PikselSystemClient::settingChanged, this, [this](const QString &key, const QString &value) {
        if (key != kCoreAppsKey)
            return;
        updateFromCoreOrFallback(value);
    });

    refresh();
}

QVariantList LauncherAppsModel::apps() const
{
    return m_apps;
}

void LauncherAppsModel::refresh()
{
    m_core.getSettingAsyncDeferred(kCoreAppsKey, QStringLiteral("[]"));
}

void LauncherAppsModel::setApps(QVariantList next)
{
    if (next != m_apps) {
        m_apps = std::move(next);
        emit appsChanged();
    }
}

void LauncherAppsModel::updateFromCoreOrFallback(const QString &json)
{
    QVariantList next = parseAppsJson(json);
    if (next.size() <= 1) {
        // If system service doesn't provide apps yet (likely empty), fall back to local scan.
        next = scanInstalledDesktopApps();
    }
    setApps(std::move(next));
}

QVariantMap LauncherAppsModel::makeFileManagerEntry()
{
    return makeEntry(QStringLiteral("fileManager"),
                     QStringLiteral("File Manager"),
                     QStringLiteral("fileManager"),
                     QString(),
                     QStringLiteral("qrc:/resources/icons/folder.png"),
                     QString());
}

QString LauncherAppsModel::normalizeId(const QString &s)
{
    QString out = s.trimmed().toLower();
    out.replace(QRegularExpression(QStringLiteral(R"([^a-z0-9._-]+)")), QStringLiteral("-"));
    out = out.trimmed();
    if (out.isEmpty())
        out = QStringLiteral("app");
    return out;
}

QString LauncherAppsModel::execToProgramKey(const QString &exec)
{
    // Best-effort: use the first token as a stable-ish key.
    const QString trimmed = exec.trimmed();
    if (trimmed.isEmpty())
        return {};

    QString prog = trimmed;
    const int sp = prog.indexOf(QRegularExpression(QStringLiteral(R"(\\s+)")));
    if (sp > 0)
        prog = prog.left(sp);

    if (prog.startsWith(QLatin1Char('"')) && prog.endsWith(QLatin1Char('"')) && prog.size() >= 2)
        prog = prog.mid(1, prog.size() - 2);

    return normalizeId(QFileInfo(prog).fileName());
}

QVariantList LauncherAppsModel::parseAppsJson(const QString &json)
{
    QVariantList out;
    out.push_back(makeFileManagerEntry());

    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (!doc.isArray())
        return out;

    const QJsonArray arr = doc.array();
    out.reserve(out.size() + arr.size());

    for (const QJsonValue &v : arr) {
        if (!v.isObject())
            continue;

        const QJsonObject o = v.toObject();

        const QString name = o.value(QStringLiteral("name")).toString().trimmed();
        const QString exec = o.value(QStringLiteral("exec")).toString().trimmed();
        QString id = o.value(QStringLiteral("id")).toString().trimmed();
        if (id.isEmpty())
            id = execToProgramKey(exec);
        if (id.isEmpty())
            id = normalizeId(name);

        QString iconSource = o.value(QStringLiteral("iconSource")).toString().trimmed();
        QString iconName = o.value(QStringLiteral("iconName")).toString().trimmed();
        const QString icon = o.value(QStringLiteral("icon")).toString().trimmed();
        if (iconSource.isEmpty() && iconName.isEmpty() && !icon.isEmpty()) {
            if (QFileInfo::exists(icon) || isProbablyPath(icon))
                iconSource = QUrl::fromLocalFile(icon).toString();
            else
                iconName = icon;
        }

        if (name.isEmpty())
            continue;

        out.push_back(makeEntry(id,
                                name,
                                QStringLiteral("exec"),
                                exec,
                                iconSource,
                                iconName));
    }

    return out;
}

QVariantList LauncherAppsModel::scanInstalledDesktopApps()
{
    QVariantList out;
    out.push_back(makeFileManagerEntry());

    struct Row {
        QString id;
        QString name;
        QString exec;
        QString iconSource;
        QString iconName;
    };

    QHash<QString, Row> bestById;

    const QStringList appDirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    for (const QString &dir : appDirs) {
        QDirIterator it(dir, QStringList() << QStringLiteral("*.desktop"), QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            const QString path = it.next();
            QSettings desktop(path, QSettings::IniFormat);
            desktop.beginGroup(QStringLiteral("Desktop Entry"));

            const QString type = extractDesktopEntryString(desktop, QStringLiteral("Type"));
            const bool hidden = desktop.value(QStringLiteral("Hidden"), false).toBool();
            const bool noDisplay = desktop.value(QStringLiteral("NoDisplay"), false).toBool();
            const bool terminal = desktop.value(QStringLiteral("Terminal"), false).toBool();

            if (!type.isEmpty() && type.compare(QStringLiteral("Application"), Qt::CaseInsensitive) != 0) {
                desktop.endGroup();
                continue;
            }
            if (hidden || noDisplay || terminal) {
                desktop.endGroup();
                continue;
            }

            const QString name = extractDesktopEntryString(desktop, QStringLiteral("Name"));
            const QString exec = extractDesktopEntryString(desktop, QStringLiteral("Exec"));
            const QString icon = extractDesktopEntryString(desktop, QStringLiteral("Icon"));
            desktop.endGroup();

            if (name.isEmpty() || exec.isEmpty())
                continue;

            Row row;
            row.id = normalizeDesktopIdFromPath(path);
            row.name = name;
            row.exec = exec;

            if (!icon.isEmpty()) {
                if (QFileInfo::exists(icon) || isProbablyPath(icon))
                    row.iconSource = QUrl::fromLocalFile(icon).toString();
                else
                    row.iconName = icon;
            }

            if (row.id.isEmpty())
                row.id = execToProgramKey(exec);
            if (row.id.isEmpty())
                row.id = normalizeId(name);

            const auto itBest = bestById.constFind(row.id);
            if (itBest == bestById.cend()) {
                bestById.insert(row.id, row);
            } else {
                // Prefer entries with an icon and a nicer name.
                const bool currentHasIcon = !itBest->iconName.isEmpty() || !itBest->iconSource.isEmpty();
                const bool nextHasIcon = !row.iconName.isEmpty() || !row.iconSource.isEmpty();
                const bool preferNext = (!currentHasIcon && nextHasIcon) || (itBest->name.size() < row.name.size());
                if (preferNext)
                    bestById[row.id] = row;
            }
        }
    }

    QList<Row> rows = bestById.values();
    std::sort(rows.begin(), rows.end(), [](const Row &a, const Row &b) {
        return QString::localeAwareCompare(a.name.toLower(), b.name.toLower()) < 0;
    });

    out.reserve(out.size() + rows.size());
    for (const Row &r : rows) {
        out.push_back(makeEntry(r.id,
                                r.name,
                                QStringLiteral("exec"),
                                r.exec,
                                r.iconSource,
                                r.iconName));
    }

    return out;
}
