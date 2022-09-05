// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
