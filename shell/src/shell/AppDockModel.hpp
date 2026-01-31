#ifndef APP_DOCK_MODEL_HPP
#define APP_DOCK_MODEL_HPP

#include <QObject>
#include <QPointer>
#include <QVariantList>
#include <memory>

class QWindow;
class PikselCoreClient;

class AppDockModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList apps READ apps NOTIFY appsChanged)
    Q_PROPERTY(QVariantList pinnedApps READ pinnedApps NOTIFY pinnedAppsChanged)

public:
    explicit AppDockModel(QObject* parent = nullptr);

    QVariantList apps() const;
    QVariantList pinnedApps() const;

    void registerLaunchedApp(const QString& appId,
                             const QString& displayName,
                             const QString& iconSource,
                             const QString& iconName,
                             const QString& exec,
                             qint64 pid = 0);
    void registerWindow(const QString& appId,
                        const QString& displayName,
                        const QString& iconSource,
                        const QString& iconName,
                        const QString& exec,
                        QWindow* window);
    void unregisterApp(const QString& appId);

    Q_INVOKABLE void activateApp(const QString& appId);
    Q_INVOKABLE void activatePinned(const QString& appId);
    Q_INVOKABLE void closeApp(const QString& appId);

    Q_INVOKABLE bool isPinned(const QString& appId) const;
    Q_INVOKABLE void pinApp(const QString& appId);
    Q_INVOKABLE void unpinApp(const QString& appId);

signals:
    void appsChanged();
    void pinnedAppsChanged();
    void requestOpenFileManager();

private:
    struct Entry {
        QString appId;
        QString displayName;
        QString iconSource;
        QString iconName;
        QString exec;
        qint64 pid = 0;
        QPointer<QWindow> window;
    };

    struct PinnedEntry {
        QString appId;
        QString displayName;
        QString iconSource;
        QString iconName;
        QString exec;
    };

    bool startPinnedDetached(const QString& exec, qint64* pidOut) const;
    void loadPinnedFromCore();
    void applyPinnedFromRaw(const QString& raw);
    void savePinnedToCore() const;

    void emitIfChanged();
    void emitPinnedIfChanged();

    QVariantList m_cachedApps;
    QHash<QString, Entry> m_entries;
    QStringList m_order;

    std::unique_ptr<PikselCoreClient> m_core;
    QHash<QString, PinnedEntry> m_pinned;
    QStringList m_pinnedOrder;
    QVariantList m_cachedPinnedApps;
};

#endif // APP_DOCK_MODEL_HPP
