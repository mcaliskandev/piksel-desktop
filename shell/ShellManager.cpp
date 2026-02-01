#include "ShellManager.hpp"
#include "AppDockModel.hpp"
#include <sstream>
#include <cstdlib>
#include <QTimer>
#include <algorithm>
#include <iostream>

ShellManager::ShellManager()
{
}

ShellManager::~ShellManager()
{
}

void ShellManager::setupUI()
{
    m_screen = QGuiApplication::primaryScreen();
    if (!m_screen) {
        qCritical() << "No primary screen available";
        QCoreApplication::exit(EXIT_FAILURE);
        return;
    }
    connect(m_screen, &QScreen::geometryChanged, this, [this](const QRect &) { applyComponentGeometries(); });
    connect(m_screen, &QScreen::availableGeometryChanged, this, [this](const QRect &) { applyComponentGeometries(); });

    auto wallpaper = std::make_unique<PikselWallpaper>();
    auto panel = std::make_unique<PikselPanel>(wallpaper.get());
    auto launcher = std::make_unique<PikselLauncher>(wallpaper.get());
    m_dockApps = std::make_unique<AppDockModel>(this);

    panel->setDockModel(m_dockApps.get());
    launcher->setDockModel(m_dockApps.get());
    connect(m_dockApps.get(), &AppDockModel::requestOpenFileManager, launcher.get(), &PikselLauncher::openFileManager);
    connect(panel.get(), &PikselPanel::wallpaperBackgroundColorChanged, wallpaper.get(), &PikselWallpaper::applyColor);

    m_components.emplace_back(std::move(panel));
    m_components.emplace_back(std::move(wallpaper));
    m_components.emplace_back(std::move(launcher));

    for(auto &component : m_components) {
        if(!component) continue;
        else m_componentsById[component->id()] = component.get();
    }

    for(auto &component : m_components) {
        QObject *obj = component->widget();
        const char *sig = "requestShow(ComponentType)";
        if(obj->metaObject()->indexOfSignal(sig) != -1) {
            connect(obj, SIGNAL(requestShow(ComponentType)), this, SLOT(onRequestShow(ComponentType)));
        }
        else {
            std::clog << "ShellManager: component " << static_cast<int>(component->id()) << " does not expose requestShow signal; skipping connect" << std::endl;
        }
    }

    applyComponentGeometries();
}

void ShellManager::applyComponentGeometries()
{
    if (!m_screen) return;

    const QRect geo = m_screen->geometry();
    const int screenWidth = geo.width();
    const int screenHeight = geo.height();
    if (screenWidth <= 0 || screenHeight <= 0) return;

    constexpr int kMinPanelHeightPx = 44;
    const int panelHeight = std::min(screenHeight, std::max(kMinPanelHeightPx, screenHeight / 30));
    const int panelY = screenHeight - panelHeight;

    if (auto it = m_componentsById.find(ComponentType::PANEL); it != m_componentsById.end()) {
        if (auto *panel = static_cast<PikselPanel*>(it->second)) {
            panel->setGeometry(0, panelY, screenWidth, panelHeight);
            panel->setFixedSize(screenWidth, panelHeight);
        }
    }
    if (auto it = m_componentsById.find(ComponentType::WALLPAPER); it != m_componentsById.end()) {
        if (auto *wallpaper = static_cast<PikselWallpaper*>(it->second)) {
            wallpaper->setGeometry(geo.x(), geo.y(), screenWidth, screenHeight);
            wallpaper->setFixedSize(screenWidth, screenHeight);
        }
    }
    if (auto it = m_componentsById.find(ComponentType::LAUNCHER); it != m_componentsById.end()) {
        if (auto *launcher = static_cast<PikselLauncher*>(it->second)) {
            launcher->setFixedSize(screenWidth, screenHeight);
            launcher->move(0, 0);
        }
    }
}

void ShellManager::showComponentById(const ComponentType& id)
{
    auto it = m_componentsById.find(id);
    if(it == m_componentsById.end()) 
    {
        std::cout << "WARNING : Component not exist !! component id : " << static_cast<int>(id) << std::endl;
        return;
    }
    QWidget* componentWindow = it->second->widget();
    if (id == ComponentType::WALLPAPER) 
    {
        componentWindow->showFullScreen();
    }
    else 
    {
        componentWindow->show();
    }
    componentWindow->raise();
    componentWindow->activateWindow();
}

void ShellManager::hideComponentById(const ComponentType& id)
{
    auto it = m_componentsById.find(id);
    if(it == m_componentsById.end()) return;
    it->second->hideComponent();
}

void ShellManager::onRequestShow(ComponentType componentId)
{
    showComponentById(componentId);
}

void ShellManager::start()
{
    setupUI();

    showComponentById(ComponentType::WALLPAPER);
    showComponentById(ComponentType::PANEL);
    if (auto it = m_componentsById.find(ComponentType::PANEL); it != m_componentsById.end()) {
        if (auto *panel = it->second->widget()) panel->raise();
    }
    QTimer::singleShot(0, this, [this]() { applyComponentGeometries(); });
}
