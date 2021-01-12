/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
    return DFMStandardPaths::location(DFMStandardPaths::PluginsPath);
}

void PluginManager::loadPlugin()
{
    Q_D(PluginManager);
    QStringList pluginChildDirs;
    d->expandInfoInterfaces.clear();
    d->viewInterfaces.clear();
    d->previewInterfaces.clear();

    QStringList pluginDirs = DFMGlobal::PluginLibraryPaths;

    foreach (QString dir, pluginDirs) {
        QDir pluginDir(dir);
        qDebug() << dir;
        pluginChildDirs << "view" << "preview";
        foreach (QString childDir, pluginChildDirs) {
            QDir childPluginDir(pluginDir.absoluteFilePath(childDir));
            qDebug() << "load plugin in: " << childPluginDir.absolutePath();
            foreach (QString fileName, childPluginDir.entryList(QDir::Files)) {
                QPluginLoader pluginLoader(childPluginDir.absoluteFilePath(fileName));
                QObject *plugin = pluginLoader.instance();
                if (plugin) {
                    qDebug() << plugin;

                    PropertyDialogExpandInfoInterface *expandInfoInterface = qobject_cast<PropertyDialogExpandInfoInterface *>(plugin);
                    if (expandInfoInterface) {
                        d->expandInfoInterfaces.append(expandInfoInterface);
                    }

                    ViewInterface *viewInterface = qobject_cast<ViewInterface *>(plugin);
                    if (viewInterface) {
                        d->viewInterfaces.append(viewInterface);
                        d->viewInterfacesMap.insert(viewInterface->scheme(), viewInterface);
                    }

                    PreviewInterface *previewInterface = qobject_cast<PreviewInterface *> (plugin);
                    if (previewInterface) {
                        static int counter = 0;
                        counter ++;
                        d->previewInterfaces << previewInterface;
                    }
                }
            }
        }
    }
    qDebug() << "expand info size:" << d->expandInfoInterfaces.size();
    qDebug() << "view size:" << d->viewInterfaces.size();
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
    if (d->viewInterfacesMap.contains(scheme)) {
        return d->viewInterfacesMap.value(scheme);
    }
    return nullptr;
}
