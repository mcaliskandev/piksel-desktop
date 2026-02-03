#include "system/SystemService.hpp"
#include "system/config/Config.hpp"
#include "shell/ShellManager.hpp"
#include <QApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QString>

#include "systemadaptor.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Config config;
    SystemService systemService(&config);
    new SystemAdaptor(&systemService);

    QDBusConnection bus = QDBusConnection::sessionBus();
    auto *iface = bus.interface();
    if (!iface) {
        qWarning() << "DBus session bus is unavailable; continuing without System service.";
    } else {
        const bool replace = app.arguments().contains(QStringLiteral("--replace"));
        const auto qopt = replace ? QDBusConnectionInterface::ReplaceExistingService
                                  : QDBusConnectionInterface::DontQueueService;
        const auto ropt = QDBusConnectionInterface::AllowReplacement;
        const auto reply = iface->registerService(QStringLiteral("org.piksel.System"), qopt, ropt);
        if (!reply.isValid() || reply.value() == QDBusConnectionInterface::ServiceNotRegistered) {
            qWarning() << "Failed to register org.piksel.System on DBus; continuing without System service.";
        } else {
            bus.registerObject(QStringLiteral("/org/piksel/System"), &systemService);
        }
    }
    ShellManager manager;
    manager.start();

    return app.exec();
}
