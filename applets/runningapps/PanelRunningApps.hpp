#ifndef PANEL_RUNNING_APPS_HPP
#define PANEL_RUNNING_APPS_HPP

#include <QObject>
#include <QHash>
#include <QTimer>
#include <QVariantList>

class PanelRunningApps : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList apps READ apps NOTIFY appsChanged)

public:
    explicit PanelRunningApps(QObject* parent = nullptr);

    QVariantList apps() const;

    Q_INVOKABLE void activate(qulonglong windowId) const;
    Q_INVOKABLE void activateLocal(qulonglong windowPtr) const;

signals:
    void appsChanged();

private slots:
    void refresh();

private:
    struct DesktopEntry {
        QString name;
        QString iconName;
    };

    void rebuildDesktopCache();
    DesktopEntry entryForWmClass(const QString& wmClass) const;

    QVariantList m_apps;
    QTimer m_refreshTimer;
    bool m_hasWmctrl = false;

    QHash<QString, DesktopEntry> m_wmClassToEntry;
};

#endif // PANEL_RUNNING_APPS_HPP
