#ifndef PIKSEL_LAUNCHER_HPP
#define PIKSEL_LAUNCHER_HPP

#include <QQuickWidget>
#include <QPointer>
#include <QWidget>
#include <QWindow>
#include <memory>
#include <string>

#include "../../shell/ShellComponent.hpp"

class AppDockModel;
class LauncherAppsModel;

class PikselLauncher : public QQuickWidget, public ShellComponent {
    Q_OBJECT
public:
    explicit PikselLauncher(QWidget* parent = nullptr);
    ~PikselLauncher() override;
    void setDockModel(AppDockModel* dockModel);
    virtual ComponentType id() const override { return ComponentType::LAUNCHER; }
    virtual QWidget* widget() { return this; }

private:
    QPointer<QWidget> m_fileManagerWidget;
    QPointer<QWindow> m_fileManagerWindow;
    AppDockModel* m_dockModel = nullptr;
    std::unique_ptr<LauncherAppsModel> m_appsModel;

public slots:
    void openFileManager();
    Q_INVOKABLE void launchEntry(const QString& appAction,
                                 const QString& appExec,
                                 const QString& appId,
                                 const QString& appName,
                                 const QString& appIconSource,
                                 const QString& appIconName);
    Q_INVOKABLE void requestHide();
    Q_INVOKABLE void lockSystem();
    Q_INVOKABLE void suspendSystem();
    Q_INVOKABLE void powerOffSystem();

private:
    void openFileManagerWithDock(const QString& appId, const QString& appName, const QString& appIconSource);
    bool launchExecDetached(const QString& exec, qint64* pidOut) const;
};

#endif // PIKSEL_LAUNCHER_HPP
