#include "Wallpaper.hpp"
#include <QApplication>
#include <QDebug>
#include <QQuickItem>
#include <QQmlContext>
#include <QUrl>

PikselWallpaper::PikselWallpaper(QWidget* parent)
    : QQuickWidget(parent)
{
    if (!parent)
        setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    setResizeMode(QQuickWidget::SizeRootObjectToView);
    rootContext()->setContextProperty("wallpaper", this);
    setSource(QUrl(QStringLiteral("qrc:/surfaces/desktop/PikselWallpaper.qml")));
    if (status() != QQuickWidget::Ready) {
        qCritical() << "Failed to load QML wallpaper:" << errors();
    }
    m_rootItem = rootObject();

    connect(&m_core, &PikselSystemClient::settingChanged, this, &PikselWallpaper::onCoreSettingChanged);

    // Load persisted color from PikselSystem (DBus) on startup without blocking the UI thread.
    const QString defaultColor = QStringLiteral("#0081CD");
    applyColor(defaultColor);
    connect(&m_core, &PikselSystemClient::settingFetched, this, [this](const QString &key, const QString &value) {
        if (key == QStringLiteral("wallpaper/backgroundColor"))
            applyColor(value);
    });
    m_core.getSettingAsyncDeferred(QStringLiteral("wallpaper/backgroundColor"), defaultColor);
}

void PikselWallpaper::applyColor(const QString &hexColor)
{
    const QColor color(hexColor);
    if (!color.isValid())
        return;

    m_backgroundColor = color;
    syncColorToQml();
}

void PikselWallpaper::onCoreSettingChanged(const QString &key, const QString &value)
{
    if (key == QStringLiteral("wallpaper/backgroundColor"))
        applyColor(value);
}

void PikselWallpaper::syncColorToQml()
{
    if (!m_rootItem)
        m_rootItem = rootObject();
    if (!m_rootItem)
        return;

    m_rootItem->setProperty("backgroundColor", m_backgroundColor);
}

#ifdef TEST_WALLPAPER
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    PikselWallpaper wallpaper(500, 400, 10);
    wallpaper.show();
    return app.exec();
}
#endif
