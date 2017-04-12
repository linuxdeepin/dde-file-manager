#include "pluginmanager.h"
#include "preview/previewinterface.h"
#include <menu/menuinterface.h>
#include "view/viewinterface.h"
#include "interfaces/dfmglobal.h"
#include "interfaces/dfmstandardpaths.h"
#include <QDir>
#include <QPluginLoader>
#include <QDebug>
#include <QMetaEnum>

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

QString PluginManager::PluginDir()
{
    return DFMStandardPaths::standardLocation(DFMStandardPaths::PluginsPath);
}

void PluginManager::loadPlugin()
{
    Q_D(PluginManager);
    QStringList pluginChildDirs;
    d->menuInterfaces.clear();
    d->expandInfoInterfaces.clear();
    d->viewInterfaces.clear();
    d->previewInterfaces.clear();

    QStringList pluginDirs = DFMGlobal::PluginLibraryPaths;

    foreach (QString dir, pluginDirs) {
        QDir pluginDir(dir);
        qDebug() << dir;
        pluginChildDirs << "menu" << "view" << "preview";
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

                    PropertyDialogExpandInfoInterface *expandInfoInterface = qobject_cast<PropertyDialogExpandInfoInterface*>(plugin);
                    if(expandInfoInterface){
                        d->expandInfoInterfaces.append(expandInfoInterface);
                    }

                    ViewInterface *viewInterface = qobject_cast<ViewInterface *>(plugin);
                    if (viewInterface){
                        d->viewInterfaces.append(viewInterface);
                        d->viewInterfacesMap.insert(viewInterface->scheme(), viewInterface);
                    }

                    PreviewInterface* previewInterface = qobject_cast<PreviewInterface*> (plugin);
                    if(previewInterface){
                        static int counter = 0;
                        counter ++;
                        d->previewInterfaces << previewInterface;
                    }
                }
            }
        }
    }
    qDebug(  ) << "menu plugin size:" << d->menuInterfaces.size();
    qDebug(  ) << "expand info size:" << d->expandInfoInterfaces.size();
    qDebug(  ) << "view size:" << d->viewInterfaces.size();
}

QList<MenuInterface *> PluginManager::getMenuInterfaces()
{
    Q_D(PluginManager);
    return d->menuInterfaces;
}

QList<PropertyDialogExpandInfoInterface *> PluginManager::getExpandInfoInterfaces()
{
    Q_D(PluginManager);
    return d->expandInfoInterfaces;
}

QList<ViewInterface *> PluginManager::getViewInterfaces()
{
    Q_D(PluginManager);
    return d->viewInterfaces;
}

QMap<QString, ViewInterface *> PluginManager::getViewInterfacesMap()
{
    Q_D(PluginManager);
    return d->viewInterfacesMap;
}

QList<PreviewInterface *> PluginManager::getPreviewInterfaces()
{
    Q_D(PluginManager);
    return d->previewInterfaces;
}

ViewInterface *PluginManager::getViewInterfaceByScheme(const QString &scheme)
{
    Q_D(PluginManager);
    if (d->viewInterfacesMap.contains(scheme)){
        return d->viewInterfacesMap.value(scheme);
    }
    return NULL;
}
