#include "AppDockModel.hpp"

#include "shell/PikselSystemClient.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QRegularExpression>
#include <QWindow>

#include <signal.h>

namespace {
constexpr auto kPinnedAppsKey = "dock/pinnedApps";
} // namespace

AppDockModel::AppDockModel(QObject* parent)
    : QObject(parent)
{
    m_core = std::make_unique<PikselSystemClient>(this);
    loadPinnedFromCore();

    connect(m_core.get(), &PikselSystemClient::settingFetched, this, [this](const QString& key, const QString& value) {
        if (key == QString::fromLatin1(kPinnedAppsKey))
            applyPinnedFromRaw(value);
    });
    connect(m_core.get(), &PikselSystemClient::settingChanged, this, [this](const QString& key, const QString&) {
        if (key == QString::fromLatin1(kPinnedAppsKey))
            loadPinnedFromCore();
    });
}

QVariantList AppDockModel::apps() const {
    return m_cachedApps;
}

QVariantList AppDockModel::pinnedApps() const {
    return m_cachedPinnedApps;
}

static QString sanitizeDesktopExec(QString exec)
{
    exec = exec.trimmed();
    if (exec.isEmpty())
        return exec;

    exec.replace(QStringLiteral("%%"), QStringLiteral("%"));
    exec.replace(QRegularExpression(QStringLiteral(R"(%[a-zA-Z])")), QString());
    return exec.simplified();
}

static bool startExecDetachedWithPid(const QString& exec, qint64* pidOut)
{
    const QString cleaned = sanitizeDesktopExec(exec);
    if (cleaned.isEmpty())
        return false;

    const QStringList parts = QProcess::splitCommand(cleaned);
    if (parts.isEmpty())
        return false;

    const QString program = parts.first();
    const QStringList args = parts.mid(1);
    qint64 pid = 0;
    if (QProcess::startDetached(program, args, QString(), &pid)) {
        if (pidOut)
            *pidOut = pid;
        return true;
    }

    pid = 0;
    const bool started = QProcess::startDetached(QStringLiteral("/bin/sh"), {QStringLiteral("-c"), cleaned}, QString(), &pid);
    if (started && pidOut)
        *pidOut = pid;
    return started;
}

bool AppDockModel::startPinnedDetached(const QString& exec, qint64* pidOut) const
{
    return startExecDetachedWithPid(exec, pidOut);
}

void AppDockModel::registerLaunchedApp(const QString& appId,
                                      const QString& displayName,
                                      const QString& iconSource,
                                      const QString& iconName,
                                      const QString& exec,
                                      qint64 pid)
{
    if (appId.trimmed().isEmpty())
        return;

    const bool isNew = !m_entries.contains(appId);
    Entry& entry = m_entries[appId];
    entry.appId = appId;
    entry.displayName = displayName;
    entry.iconSource = iconSource;
    entry.iconName = iconName;
    entry.exec = exec;
    if (pid > 0)
        entry.pid = pid;

    if (isNew)
        m_order.push_back(appId);

    emitIfChanged();
}

void AppDockModel::registerWindow(const QString& appId,
                                 const QString& displayName,
                                 const QString& iconSource,
                                 const QString& iconName,
                                 const QString& exec,
                                 QWindow* window) {
    if (appId.trimmed().isEmpty() || !window)
        return;

    const bool isNew = !m_entries.contains(appId);
    Entry& entry = m_entries[appId];
    entry.appId = appId;
    entry.displayName = displayName;
    entry.iconSource = iconSource;
    entry.iconName = iconName;
    entry.exec = exec;

    if (entry.window != window) {
        entry.window = window;
        connect(window, &QObject::destroyed, this, [this, appId] { unregisterApp(appId); });
    }

    if (isNew)
        m_order.push_back(appId);

    emitIfChanged();
}

void AppDockModel::unregisterApp(const QString& appId) {
    if (!m_entries.contains(appId))
        return;

    m_entries.remove(appId);
    m_order.removeAll(appId);
    emitIfChanged();
}

void AppDockModel::activateApp(const QString& appId) {
    const auto it = m_entries.find(appId);
    if (it == m_entries.end())
        return;

    QWindow* w = it->window;
    if (!w) {
        if (it->exec.trimmed().isEmpty()) {
            if (it->appId == QStringLiteral("fileManager"))
                emit requestOpenFileManager();
            return;
        }
        startExecDetachedWithPid(it->exec, nullptr);
        return;
    }

    if (!w->isVisible())
        w->show();
    w->raise();
    w->requestActivate();
}

void AppDockModel::activatePinned(const QString& appId)
{
    if (appId.trimmed().isEmpty())
        return;

    if (m_entries.contains(appId)) {
        activateApp(appId);
        return;
    }

    const auto it = m_pinned.find(appId);
    if (it == m_pinned.end())
        return;

    if (it->exec.trimmed().isEmpty()) {
        if (it->appId == QStringLiteral("fileManager"))
            emit requestOpenFileManager();
        return;
    }

    qint64 pid = 0;
    const bool started = startPinnedDetached(it->exec, &pid);
    if (!started)
        return;

    registerLaunchedApp(it->appId, it->displayName, it->iconSource, it->iconName, it->exec, pid);
}

void AppDockModel::closeApp(const QString& appId)
{
    const auto it = m_entries.find(appId);
    if (it == m_entries.end())
        return;

    if (it->window) {
        it->window->close();
        unregisterApp(appId);
        return;
    }

    if (it->pid > 0) {
        ::kill(static_cast<pid_t>(it->pid), SIGTERM);
        unregisterApp(appId);
        return;
    }
}

bool AppDockModel::isPinned(const QString& appId) const
{
    return m_pinned.contains(appId);
}

void AppDockModel::pinApp(const QString& appId)
{
    if (appId.trimmed().isEmpty() || m_pinned.contains(appId))
        return;

    const auto it = m_entries.find(appId);
    if (it == m_entries.end())
        return;

    PinnedEntry p;
    p.appId = it->appId;
    p.displayName = it->displayName;
    p.iconSource = it->iconSource;
    p.iconName = it->iconName;
    p.exec = it->exec;

    m_pinned.insert(appId, p);
    m_pinnedOrder.push_back(appId);
    emitPinnedIfChanged();
    savePinnedToCore();
}

void AppDockModel::unpinApp(const QString& appId)
{
    if (!m_pinned.contains(appId))
        return;

    m_pinned.remove(appId);
    m_pinnedOrder.removeAll(appId);
    emitPinnedIfChanged();
    savePinnedToCore();
}

void AppDockModel::emitIfChanged() {
    QVariantList next;
    next.reserve(m_order.size());

    for (const QString& appId : m_order) {
        const auto it = m_entries.find(appId);
        if (it == m_entries.end())
            continue;

        QVariantMap m;
        m.insert(QStringLiteral("appId"), it->appId);
        m.insert(QStringLiteral("text"), it->displayName);
        m.insert(QStringLiteral("iconSource"), it->iconSource);
        m.insert(QStringLiteral("iconName"), it->iconName);
        next.push_back(m);
    }

    if (next != m_cachedApps) {
        m_cachedApps = next;
        emit appsChanged();
    }
}

void AppDockModel::emitPinnedIfChanged()
{
    QVariantList next;
    next.reserve(m_pinnedOrder.size());

    for (const QString& appId : m_pinnedOrder) {
        const auto it = m_pinned.find(appId);
        if (it == m_pinned.end())
            continue;

        QVariantMap m;
        m.insert(QStringLiteral("appId"), it->appId);
        m.insert(QStringLiteral("text"), it->displayName);
        m.insert(QStringLiteral("iconSource"), it->iconSource);
        m.insert(QStringLiteral("iconName"), it->iconName);
        next.push_back(m);
    }

    if (next != m_cachedPinnedApps) {
        m_cachedPinnedApps = next;
        emit pinnedAppsChanged();
    }
}

void AppDockModel::loadPinnedFromCore()
{
    if (!m_core)
        return;

    m_core->getSettingAsyncDeferred(QString::fromLatin1(kPinnedAppsKey), QStringLiteral("[]"));
}

void AppDockModel::applyPinnedFromRaw(const QString& raw)
{
    const QJsonDocument doc = QJsonDocument::fromJson(raw.toUtf8());
    if (!doc.isArray())
        return;

    QHash<QString, PinnedEntry> pinned;
    QStringList order;

    const QJsonArray arr = doc.array();
    order.reserve(arr.size());
    for (const QJsonValue& v : arr) {
        if (!v.isObject())
            continue;
        const QJsonObject o = v.toObject();

        const QString appId = o.value(QStringLiteral("appId")).toString().trimmed();
        if (appId.isEmpty() || pinned.contains(appId))
            continue;

        PinnedEntry p;
        p.appId = appId;
        p.displayName = o.value(QStringLiteral("text")).toString();
        p.iconSource = o.value(QStringLiteral("iconSource")).toString();
        p.iconName = o.value(QStringLiteral("iconName")).toString();
        p.exec = o.value(QStringLiteral("exec")).toString();

        pinned.insert(appId, p);
        order.push_back(appId);
    }

    m_pinned = std::move(pinned);
    m_pinnedOrder = std::move(order);
    emitPinnedIfChanged();
}

void AppDockModel::savePinnedToCore() const
{
    if (!m_core)
        return;

    QJsonArray arr;
    for (const QString& appId : m_pinnedOrder) {
        const auto it = m_pinned.find(appId);
        if (it == m_pinned.end())
            continue;

        QJsonObject o;
        o.insert(QStringLiteral("appId"), it->appId);
        o.insert(QStringLiteral("text"), it->displayName);
        o.insert(QStringLiteral("iconSource"), it->iconSource);
        o.insert(QStringLiteral("iconName"), it->iconName);
        o.insert(QStringLiteral("exec"), it->exec);
        arr.push_back(o);
    }

    const QString json = QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact));
    m_core->setSetting(QString::fromLatin1(kPinnedAppsKey), json);
}
