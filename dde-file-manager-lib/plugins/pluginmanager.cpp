#include "pluginmanager.h"
#include "menu/menuinterface.h"
#include "interfaces/dfmglobal.h"
#include <QDir>
#include <QPluginLoader>
#include <QDebug>

PluginManagerPrivate::PluginManagerPrivate(PluginManager *parent):
    q_ptr(parent)
{

}


PluginManager::PluginManager(QObject *parent) :
    QObject(parent),
    d_ptr(new PluginManagerPrivate(this))
{

}

PluginManager::~PluginManager()
{

}

void PluginManager::loadPlugin()
{
    Q_D(PluginManager);
    QStringList pluginChildDirs;
    d->menuInterfaces.clear();

    QStringList pluginDirs = DFMGlobal::PluginLibraryPaths;

    foreach (QString dir, pluginDirs) {
        QDir pluginDir(dir);
        qDebug() << dir;
        pluginChildDirs << "menu" << "view";
        foreach (QString childDir, pluginChildDirs) {
            QDir childPluginDir(pluginDir.absoluteFilePath(childDir));
            qDebug() << "load plugin in: " << childPluginDir.absolutePath();
            foreach (QString fileName, childPluginDir.entryList(QDir::Files))
            {
                QPluginLoader pluginLoader(childPluginDir.absoluteFilePath(fileName));
                QObject *plugin = pluginLoader.instance();
                if (plugin)
                {
                    qDebug() << plugin;
                    MenuInterface *menuInterface = qobject_cast<MenuInterface *>(plugin);
                    if (menuInterface)
                    {
                        d->menuInterfaces.append(menuInterface);
                    }
                }
            }
        }
    }
    qDebug(  ) << "loadPlugin size:" << d->menuInterfaces.size();
}

QList<MenuInterface *> PluginManager::getMenuInterfaces()
{
    Q_D(PluginManager);
    return d->menuInterfaces;
}
