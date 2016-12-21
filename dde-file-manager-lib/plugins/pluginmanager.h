#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QObject>
#include <QScopedPointer>
#include <QMap>

class MenuInterface;
class PluginManager;
class PropertyDialogExpandInfoInterface;
class ViewInterface;

class PluginManagerPrivate {

public:
    PluginManagerPrivate(PluginManager* parent);

    QList<MenuInterface*> menuInterfaces;
    QList<PropertyDialogExpandInfoInterface*> expandInfoInterfaces;
    QList<ViewInterface*> viewInterfaces;
    QMap<QString, ViewInterface*> viewInterfacesMap;

private:
    PluginManager* q_ptr=NULL;
    Q_DECLARE_PUBLIC(PluginManager)
};


class PluginManager : public QObject
{
    Q_OBJECT

public:
    inline static PluginManager* instance(){
        static PluginManager* instance = new PluginManager();
        return instance;
    }

    inline static QString PluginDir(){
        return PLUGINDIR;
    }

    void loadPlugin();
    QList<MenuInterface*> getMenuInterfaces();
    QList<PropertyDialogExpandInfoInterface*> getExpandInfoInterfaces();
    QList<ViewInterface*> getViewInterfaces();
    QMap<QString, ViewInterface*> getViewInterfacesMap();
    ViewInterface* getViewInterfaceByScheme(const QString& scheme);

signals:

public slots:

private:
    explicit PluginManager(QObject *parent = 0);
    ~PluginManager();

    QScopedPointer<PluginManagerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(PluginManager)
};

#endif // PLUGINMANAGER_H
