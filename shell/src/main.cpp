#include "../core/src/CoreService.hpp"
#include "../core/src/config/Config.hpp"
#include "shell/ShellManager.hpp"
#include <QApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QString>

#include "coreadaptor.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Config config;
    CoreService core(&config);
    new CoreAdaptor(&core);

    QDBusConnection bus = QDBusConnection::sessionBus();
    auto *iface = bus.interface();
    if (!iface) {
        qWarning() << "DBus session bus is unavailable; continuing without Core service.";
    } else {
        const bool replace = app.arguments().contains(QStringLiteral("--replace"));
        const auto qopt = replace ? QDBusConnectionInterface::ReplaceExistingService
                                  : QDBusConnectionInterface::DontQueueService;
        const auto ropt = QDBusConnectionInterface::AllowReplacement;
        const auto reply = iface->registerService(QStringLiteral("org.piksel.Core"), qopt, ropt);
        if (!reply.isValid() || reply.value() == QDBusConnectionInterface::ServiceNotRegistered) {
            qWarning() << "Failed to register org.piksel.Core on DBus; continuing without Core service.";
        } else {
            bus.registerObject(QStringLiteral("/org/piksel/Core"), &core);
        }
    }
    ShellManager manager;
    manager.start();

    return app.exec();
}
