#ifndef SHELLMANAGER_HPP
#define SHELLMANAGER_HPP

#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <QVariant> 
#include <QGuiApplication>
#include <QScreen>

#include "../components/panel/Panel.hpp"
#include "../components/wallpaper/Wallpaper.hpp"
#include "../components/launcher/Launcher.hpp"
#include "ShellComponent.hpp"

class AppDockModel;

/*!
 * \brief Create and manage all components
 * \details Components are top-level widgets, sized/positioned by ShellManager.
 */
class ShellManager : public QObject
{
    Q_OBJECT

public:
    ShellManager();
    ~ShellManager();

    void start();

    // Direct API
    void showComponentById(const ComponentType& id);
    void hideComponentById(const ComponentType& id);

private:
    void setupUI();
    void applyComponentGeometries();

public slots:
    void onRequestShow(ComponentType componentId);

private:
    QScreen* m_screen = nullptr;

    std::vector<std::unique_ptr<ShellComponent>> m_components;
    std::unordered_map<ComponentType, ShellComponent*> m_componentsById;
    std::unique_ptr<AppDockModel> m_dockApps;
};

#endif // SHELLMANAGER_HPP
