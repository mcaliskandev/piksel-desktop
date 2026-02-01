#include "SettingsWindow.hpp"

#include <QAbstractButton>
#include <QColor>
#include <QColorDialog>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

#include "ui_SettingsWallpaper.h"
#include "ui_SettingsWindow.h"

SettingsWindow::SettingsWindow(QWidget *parent)
    : QWidget(parent)
    , m_ui(new Ui::SettingsWindow)
    , m_wallpaperUi(std::make_unique<Ui::Form>())
{
    m_ui->setupUi(this);

    m_wallpaperUi->setupUi(m_ui->pageWallpaper);
    m_ui->stackedWidgetSettings->setCurrentWidget(m_ui->pageWallpaper);

    m_wallpaperUi->hexEdit->setValidator(new QRegularExpressionValidator(
        QRegularExpression(QStringLiteral("^#([0-9A-Fa-f]{6})$")),
        m_wallpaperUi->hexEdit));

    connect(m_wallpaperUi->chooseButton, &QPushButton::clicked, this, &SettingsWindow::onChooseColor);
    connect(m_wallpaperUi->applyButton, &QPushButton::clicked, this, &SettingsWindow::onApplyColor);
    connect(m_ui->btnWallpaper, &QPushButton::clicked, this, [this] {
        m_ui->stackedWidgetSettings->setCurrentWidget(m_ui->pageWallpaper);
    });

    for (auto *button : m_ui->stackedWidgetSettings->findChildren<QAbstractButton*>(
             QString(), Qt::FindDirectChildrenOnly)) {
        button->hide();
    }

    loadFromCore();
}

SettingsWindow::~SettingsWindow()
{
    delete m_ui;
}

void SettingsWindow::loadFromCore()
{
    const auto defaultColor = QStringLiteral("#0081CD");
    m_currentHex = defaultColor;
    m_wallpaperUi->hexEdit->setText(m_currentHex);
    updatePreview(QColor(m_currentHex));

    connect(&m_core, &PikselCoreClient::settingFetched, this, [this](const QString &key, const QString &value) {
        if (key != QStringLiteral("wallpaper/backgroundColor"))
            return;
        m_currentHex = value;
        m_wallpaperUi->hexEdit->setText(m_currentHex);
        updatePreview(QColor(m_currentHex));
    });
    m_core.getSettingAsyncDeferred(QStringLiteral("wallpaper/backgroundColor"), defaultColor);
}

void SettingsWindow::applyToCore()
{
    const auto hex = m_wallpaperUi->hexEdit->text().trimmed();
    const QColor color(hex);
    if (!color.isValid())
        return;

    m_currentHex = hex;
    updatePreview(color);
    m_core.setSetting(QStringLiteral("wallpaper/backgroundColor"), m_currentHex);
    emit wallpaperBackgroundColorChanged(m_currentHex);
}

void SettingsWindow::updatePreview(const QColor &color)
{
    if (!color.isValid())
        return;

    m_wallpaperUi->previewFrame->setStyleSheet(QStringLiteral("background-color: %1;").arg(color.name()));
}

void SettingsWindow::onChooseColor()
{
    const QColor initial(m_wallpaperUi->hexEdit->text());
    const QColor picked = QColorDialog::getColor(initial.isValid() ? initial : QColor(QStringLiteral("#0081CD")), this);
    if (!picked.isValid())
        return;

    m_wallpaperUi->hexEdit->setText(picked.name());
    updatePreview(picked);
}

void SettingsWindow::onApplyColor()
{
    applyToCore();
}
