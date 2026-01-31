#ifndef SHELLCOMPONENT_HPP
#define SHELLCOMPONENT_HPP

#include <string>
#include <QWidget>
#include <QVariant>

enum class ComponentType {
    PANEL,
    LAUNCHER,
    WALLPAPER
};

/*!
 * \brief base class for all shell components
 * \details every component represented by an id 
 */
class ShellComponent {
public:
    ShellComponent() = default;
    virtual ~ShellComponent() = default;
    virtual ComponentType id() const = 0;
    virtual QWidget* widget() { return nullptr; }
    void hideComponent() { widget()->hide(); }
};

#endif // SHELLCOMPONENT_HPP
