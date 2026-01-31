#include "PanelRunningApps.hpp"

#include <QDebug>
#include <QDirIterator>
#include <QFileInfo>
#include <QList>
#include <QProcess>
#include <QGuiApplication>
#include <QRegularExpression>
#include <QSettings>
#include <QSet>
#include <QStandardPaths>
#include <QWindow>

namespace {
QString normalizeKey(QString s) {
    return s.trimmed().toLower();
}

QStringList wmClassCandidates(const QString& wmClassToken) {
    const QString token = wmClassToken.trimmed();
    if (token.isEmpty())
        return {};

    QStringList out;
    out << token;

    const int dot = token.indexOf('.');
    if (dot > 0 && dot < token.size() - 1) {
        out << token.left(dot);
        out << token.mid(dot + 1);
    }

    const int lastDot = token.lastIndexOf('.');
    if (lastDot > 0 && lastDot < token.size() - 1) {
        out << token.left(lastDot);
        out << token.mid(lastDot + 1);
    }

    for (QString& s : out)
        s = normalizeKey(s);
    out.removeDuplicates();
    return out;
}

QString desktopIdFromPath(const QString& path) {
    const QString file = QFileInfo(path).fileName();
    if (file.endsWith(".desktop", Qt::CaseInsensitive))
        return normalizeKey(file.left(file.size() - 8));
    return normalizeKey(file);
}
} // namespace

PanelRunningApps::PanelRunningApps(QObject* parent)
    : QObject(parent)
{
    m_hasWmctrl = !QStandardPaths::findExecutable(QStringLiteral("wmctrl")).isEmpty();
    rebuildDesktopCache();

    connect(&m_refreshTimer, &QTimer::timeout, this, &PanelRunningApps::refresh);
    m_refreshTimer.setInterval(1500);
    m_refreshTimer.start();

    refresh();
}

QVariantList PanelRunningApps::apps() const {
    return m_apps;
}

void PanelRunningApps::activate(qulonglong windowId) const {
    if (!m_hasWmctrl || windowId == 0)
        return;

    const QString id = QStringLiteral("0x") + QString::number(windowId, 16);
    QProcess::startDetached(QStringLiteral("wmctrl"), {QStringLiteral("-i"), QStringLiteral("-a"), id});
}

void PanelRunningApps::activateLocal(qulonglong windowPtr) const {
    if (windowPtr == 0)
        return;

    auto* w = reinterpret_cast<QWindow*>(static_cast<quintptr>(windowPtr));
    if (!w)
        return;

    if (!w->isVisible())
        w->show();
    w->raise();
    w->requestActivate();
}

void PanelRunningApps::rebuildDesktopCache() {
    m_wmClassToEntry.clear();

    const QStringList appDirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    for (const QString& dir : appDirs) {
        QDirIterator it(dir, QStringList() << QStringLiteral("*.desktop"), QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            const QString path = it.next();
            QSettings desktop(path, QSettings::IniFormat);
            desktop.beginGroup(QStringLiteral("Desktop Entry"));

            if (desktop.value(QStringLiteral("NoDisplay"), false).toBool()) {
                desktop.endGroup();
                continue;
            }

            DesktopEntry entry;
            entry.name = desktop.value(QStringLiteral("Name")).toString().trimmed();
            entry.iconName = desktop.value(QStringLiteral("Icon")).toString().trimmed();
            const QString startupWmClass = desktop.value(QStringLiteral("StartupWMClass")).toString().trimmed();
            desktop.endGroup();

            if (entry.name.isEmpty() && entry.iconName.isEmpty())
                continue;

            const QString desktopId = desktopIdFromPath(path);
            if (!desktopId.isEmpty())
                m_wmClassToEntry.insert(desktopId, entry);

            if (!startupWmClass.isEmpty())
                m_wmClassToEntry.insert(normalizeKey(startupWmClass), entry);
        }
    }
}

PanelRunningApps::DesktopEntry PanelRunningApps::entryForWmClass(const QString& wmClass) const {
    for (const QString& key : wmClassCandidates(wmClass)) {
        const auto it = m_wmClassToEntry.find(key);
        if (it != m_wmClassToEntry.end())
            return it.value();
    }
    return DesktopEntry{};
}

void PanelRunningApps::refresh() {
    QVariantList next;
    QSet<QString> seen;

    // Wayland-friendly fallback: windows created by this process (e.g. Pusula).
    const auto windows = QGuiApplication::allWindows();
    for (QWindow* w : windows) {
        if (!w || !w->isVisible())
            continue;

        const QString title = w->title().trimmed();
        if (title.isEmpty())
            continue;

        QVariantMap m;
        m.insert(QStringLiteral("text"), title);

        // Prefer known in-shell apps (since theme icons may be missing).
        if (title.contains(QStringLiteral("Piksel File Manager"), Qt::CaseInsensitive)) {
            m.insert(QStringLiteral("iconSource"), QStringLiteral("qrc:/resources/icons/folder.png"));
            seen.insert(QStringLiteral("pusula"));
        } else if (title.contains(QStringLiteral("Settings"), Qt::CaseInsensitive)) {
            m.insert(QStringLiteral("iconSource"), QStringLiteral("qrc:/resources/icons/settings.png"));
            seen.insert(QStringLiteral("settings"));
        }

        const qulonglong ptr = static_cast<qulonglong>(reinterpret_cast<quintptr>(w));
        m.insert(QStringLiteral("localWindowPtr"), QVariant::fromValue<qulonglong>(ptr));

        const QString key = normalizeKey(title);
        if (seen.contains(key))
            continue;
        seen.insert(key);

        next.push_back(m);
    }

    // X11: also include global windows (requires wmctrl).
    if (!m_hasWmctrl) {
        if (next != m_apps) {
            m_apps = next;
            emit appsChanged();
        }
        return;
    }

    QProcess proc;
    proc.start(QStringLiteral("wmctrl"), {QStringLiteral("-l"), QStringLiteral("-x")});
    if (!proc.waitForFinished(500)) {
        proc.kill();
        return;
    }

    const QString out = QString::fromLocal8Bit(proc.readAllStandardOutput());

    // Format (wmctrl -lx):
    // 0x01200003  0 hostname WM_CLASS title...
    const QRegularExpression lineRe(
        QStringLiteral(R"(^(0x[0-9a-fA-F]+)\s+(-?\d+)\s+\S+\s+(\S+)\s+(.*)$)"));

    struct AppRow {
        QString displayName;
        QString iconName;
        QString iconSource;
        qulonglong windowId = 0;
    };
    QList<AppRow> ordered;

    const QStringList lines = out.split('\n', Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        const QRegularExpressionMatch m = lineRe.match(line);
        if (!m.hasMatch())
            continue;

        const QString winIdHex = m.captured(1);
        const QString wmClass = m.captured(3);
        const QString title = m.captured(4).trimmed();

        QString normalizedWinId = winIdHex;
        if (normalizedWinId.startsWith(QStringLiteral("0x"), Qt::CaseInsensitive))
            normalizedWinId = normalizedWinId.mid(2);

        bool ok = false;
        const qulonglong winId = normalizedWinId.toULongLong(&ok, 16);
        if (!ok || winId == 0)
            continue;

        if (wmClass.contains(QStringLiteral("PikselPanel"), Qt::CaseInsensitive))
            continue;

        const DesktopEntry entry = entryForWmClass(wmClass);

        AppRow row;
        row.windowId = winId;
        row.displayName = !entry.name.isEmpty() ? entry.name : (!title.isEmpty() ? title : wmClass);
        row.iconName = entry.iconName;
        if (row.iconName.isEmpty()) {
            const QStringList candidates = wmClassCandidates(wmClass);
            row.iconName = candidates.isEmpty() ? QString() : candidates.first();
        }
        if (row.displayName.contains(QStringLiteral("Piksel File Manager"), Qt::CaseInsensitive) ||
            wmClass.contains(QStringLiteral("Pusula"), Qt::CaseInsensitive)) {
            row.iconSource = QStringLiteral("qrc:/resources/icons/folder.png");
        }

        const QStringList candidates = wmClassCandidates(wmClass);
        const QString appKey = candidates.isEmpty()
            ? normalizeKey(row.iconName.isEmpty() ? row.displayName : row.iconName)
            : candidates.first();

        if (seen.contains(appKey))
            continue;
        seen.insert(appKey);
        ordered.push_back(row);
    }

    next.reserve(ordered.size());
    for (const AppRow& row : ordered) {
        QVariantMap m;
        m.insert(QStringLiteral("text"), row.displayName);
        m.insert(QStringLiteral("iconName"), row.iconName);
        if (!row.iconSource.isEmpty())
            m.insert(QStringLiteral("iconSource"), row.iconSource);
        m.insert(QStringLiteral("windowId"), QVariant::fromValue<qulonglong>(row.windowId));
        next.push_back(m);
    }

    if (next != m_apps) {
        m_apps = next;
        emit appsChanged();
    }
}
