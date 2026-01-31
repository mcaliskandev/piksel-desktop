#include "Launcher.hpp"
#include <QQmlContext>
#include <QDebug>
#include <QDir>
#include <QProcess>
#include <QRegularExpression>
#include <QTimer>
#include <QDesktopServices>
#include <QUrl>

#ifdef PIKSEL_WITH_PUSULA
#include <pusula/pusula.hpp>
#endif

#include "LauncherAppsModel.hpp"
#include "../../shell/AppDockModel.hpp"

static bool runDetachedShellCommand(const QString& command)
{
    return QProcess::startDetached(QStringLiteral("/bin/sh"), {QStringLiteral("-c"), command});
}

PikselLauncher::PikselLauncher(QWidget* parent)
    : QQuickWidget(parent)
{
    if (!parent) {
        setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    }
    setResizeMode(QQuickWidget::SizeRootObjectToView);
    rootContext()->setContextProperty("launcher", this);
    m_appsModel = std::make_unique<LauncherAppsModel>(this);
    rootContext()->setContextProperty("launcherApps", m_appsModel.get());
    setSource(QUrl(QStringLiteral("qrc:/qml/Launcher/PikselLauncher.qml")));

    if (status() != QQuickWidget::Ready) {
        qCritical() << "Failed to load QML launcher:" << errors();
    } else {
        qDebug() << "Launcher QML loaded successfully.";
    }

    hide();
}

PikselLauncher::~PikselLauncher() = default;

void PikselLauncher::setDockModel(AppDockModel* dockModel)
{
    m_dockModel = dockModel;
    rootContext()->setContextProperty("dockApps", m_dockModel);
}

void PikselLauncher::requestHide()
{
    hide();
}

void PikselLauncher::lockSystem()
{
    const bool started = runDetachedShellCommand(
        QStringLiteral("loginctl lock-session || loginctl lock-sessions || dm-tool lock || xdg-screensaver lock"));
    if (!started)
        qWarning() << "Launcher: failed to start lock command";
}

void PikselLauncher::suspendSystem()
{
    const bool started = runDetachedShellCommand(QStringLiteral("systemctl suspend || loginctl suspend"));
    if (!started)
        qWarning() << "Launcher: failed to start suspend command";
}

void PikselLauncher::powerOffSystem()
{
    const bool started = runDetachedShellCommand(QStringLiteral("systemctl poweroff || loginctl poweroff"));
    if (!started)
        qWarning() << "Launcher: failed to start poweroff command";
}

void PikselLauncher::openFileManager()
{
    openFileManagerWithDock(QStringLiteral("fileManager"),
                            QStringLiteral("File Manager"),
                            QStringLiteral("qrc:/resources/icons/folder.png"));
}

static QString sanitizeDesktopExec(QString exec)
{
    exec = exec.trimmed();
    if (exec.isEmpty())
        return exec;

    exec.replace(QStringLiteral("%%"), QStringLiteral("%"));

    // Remove Desktop Entry field codes like %U, %f, %i, etc.
    exec.replace(QRegularExpression(QStringLiteral(R"(%[a-zA-Z])")), QString());

    // Remove redundant whitespace.
    exec = exec.simplified();
    return exec;
}

bool PikselLauncher::launchExecDetached(const QString& exec, qint64* pidOut) const
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

    // Fallback for entries that rely on shell behavior.
    pid = 0;
    const bool started = QProcess::startDetached(QStringLiteral("/bin/sh"), {QStringLiteral("-c"), cleaned}, QString(), &pid);
    if (started && pidOut)
        *pidOut = pid;
    return started;
}

void PikselLauncher::launchEntry(const QString& appAction,
                               const QString& appExec,
                               const QString& appId,
                               const QString& appName,
                               const QString& appIconSource,
                               const QString& appIconName)
{
    if (appAction == QStringLiteral("fileManager")) {
        if (m_dockModel)
            m_dockModel->registerLaunchedApp(appId.isEmpty() ? QStringLiteral("fileManager") : appId,
                                             appName,
                                             appIconSource,
                                             appIconName,
                                             QString());
        openFileManagerWithDock(appId.isEmpty() ? appAction : appId, appName, appIconSource);
        return;
    }

    if (appAction == QStringLiteral("exec")) {
        qint64 pid = 0;
        const bool started = launchExecDetached(appExec, &pid);
        if (!started)
            qWarning().noquote() << "Launcher: failed to start app:" << appId << appName << "exec=" << appExec;
        if (started && m_dockModel) {
            m_dockModel->registerLaunchedApp(appId, appName, appIconSource, appIconName, appExec, pid);
        }
        return;
    }

    qWarning().noquote() << "Launcher: unknown appAction:" << appAction << "for" << appId;
}

void PikselLauncher::openFileManagerWithDock(const QString& appId, const QString& appName, const QString& appIconSource)
{
    if (m_fileManagerWidget) {
        m_fileManagerWidget->show();
        m_fileManagerWidget->raise();
        m_fileManagerWidget->activateWindow();
        if (m_dockModel && m_fileManagerWindow)
            m_dockModel->registerWindow(appId, appName, appIconSource, QString(), QString(), m_fileManagerWindow);
        return;
    }

#ifdef PIKSEL_WITH_PUSULA
    auto* fileManager = new Pusula();
    fileManager->setAttribute(Qt::WA_DeleteOnClose, true);
    fileManager->setWindowTitle(appName);
    m_fileManagerWidget = fileManager;

    connect(fileManager, &QObject::destroyed, this, [this, appId] {
        if (m_dockModel)
            m_dockModel->unregisterApp(appId);
        m_fileManagerWindow = nullptr;
        m_fileManagerWidget = nullptr;
    });

    fileManager->show();
    fileManager->raise();
    fileManager->activateWindow();

    auto registerWithDock = [this, appId, appName, appIconSource] {
        if (!m_dockModel || !m_fileManagerWidget)
            return;

        if (!m_fileManagerWindow)
            m_fileManagerWindow = m_fileManagerWidget->windowHandle();

        if (m_fileManagerWindow)
            m_dockModel->registerWindow(appId, appName, appIconSource, QString(), QString(), m_fileManagerWindow);
    };

    registerWithDock();
    if (!m_fileManagerWindow)
        QTimer::singleShot(0, this, registerWithDock);
#else
    const QUrl homeUrl = QUrl::fromLocalFile(QDir::homePath());
    if (!QDesktopServices::openUrl(homeUrl))
        qWarning() << "Launcher: failed to open file manager for" << homeUrl;
#endif
}
