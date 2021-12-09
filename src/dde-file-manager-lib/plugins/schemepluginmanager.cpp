
/*
* Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
*
* Author:      Xiao Zhiguo <xiaozhiguo@uniontech.com>
* Maintainer:  Xiao Zhiguo <xiaozhiguo@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
// NOTE [XIAO]
#include "schemepluginmanager.h"
#include "schemeplugininterface.h"
#include "interfaces/dfmstandardpaths.h"
#include <QPluginLoader>
#include <QDir>
Q_GLOBAL_STATIC(SchemePluginManager, manager);
SchemePluginManager::SchemePluginManager(QObject *parent)
    : QObject(parent)
{
}
SchemePluginManager *SchemePluginManager::instance()
{
    return manager;
}
QString SchemePluginManager::schemePluginDir()
{
    return QString("/usr/lib/dde-file-manager/addons");
}
void SchemePluginManager::loadSchemePlugin()
{
    qWarning() << schemePluginDir();
    QDir pluginDir(schemePluginDir());
    qWarning() << pluginDir.entryList(QDir::Files);
    for (auto pluginFile : pluginDir.entryList(QDir::Files)) {
        QPluginLoader pluginLoader(pluginDir.absoluteFilePath(pluginFile));
        QObject *plugin = pluginLoader.instance();
        if (plugin) {
            SchemePluginInterface *schemePlugin = qobject_cast<SchemePluginInterface *>(plugin);
            if (schemePlugin) {
                QString pluginName = schemePlugin->pluginName();
                schemePluginList << qMakePair<QString, SchemePluginInterface *>(pluginName, schemePlugin);
                schemePluginPahtList << qMakePair<QString, QString>(pluginName, pluginDir.absoluteFilePath(pluginFile));
                qInfo() << "Scheme plugin name:" << pluginName;
                qInfo() << "Scheme plugin path:" << pluginDir.absoluteFilePath(pluginFile);
            }
        } else {
            qInfo() << pluginLoader.errorString();
        }
    }
}
void SchemePluginManager::unloadSchemePlugin(QString schemeName)
{
    Q_UNUSED(schemeName);
}
SchemePluginList SchemePluginManager::schemePlugins()
{
    return schemePluginList;
}
