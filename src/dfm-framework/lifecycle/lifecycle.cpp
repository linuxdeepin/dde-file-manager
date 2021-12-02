/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "lifecycle.h"
#include "pluginmanager.h"
#include "pluginmetaobject.h"

DPF_BEGIN_NAMESPACE

Q_GLOBAL_STATIC(PluginManager, pluginManager);

/*!
 * \brief Add a class of plug-in IID to the framework,
 * the framework will load the IID plug-in to achieve an
 * APP can load a variety of different IID plug-in capabilities
 * \param pluginIID: like org.deepin.plugin.[XXX]
 */
void LifeCycle::addPluginIID(const QString &pluginIID)
{
    return pluginManager->addPluginIID(pluginIID);
}

/*!
 * \brief LifeCycle::pluginIIDs Get plugin identity
 * \return all id list
 */
QStringList LifeCycle::pluginIIDs()
{
    return pluginManager->pluginIIDs();
}

/*!
 * \brief LifeCycle::pluginPaths get all plugin path list
 * \return plugin path list
 */
QStringList LifeCycle::pluginPaths()
{
    return pluginManager->pluginPaths();
}

/*!
 * \brief LifeCycle::setPluginPaths
 * \param pluginPaths
 */
void LifeCycle::setPluginPaths(const QStringList &pluginPaths)
{
    return pluginManager->setPluginPaths(pluginPaths);
}

/*!
 * \brief LifeCycle::servicePaths
 * \return service path list
 */
QStringList LifeCycle::servicePaths()
{
    return pluginManager->servicePaths();
}

/*!
 * \brief LifeCycle::setServicePaths
 * \param servicePaths
 */
void LifeCycle::setServicePaths(const QStringList &servicePaths)
{
    return pluginManager->setServicePaths(servicePaths);
}

PluginMetaObjectPointer LifeCycle::pluginMetaObj(const QString &pluginName,
                                                 const QString version)
{
    return pluginManager->pluginMetaObj(pluginName, version);
}

/*!
 * \brief LifeCycle::readPlugins read meta data of all plugins
 * \pre {
 * The setPluginIID and setPluginPaths functions should be called before using this function, otherwise there is no point in executing it
 * }
 * @details The execution of this function will automatically scan for all IID-compliant plugins under the setPluginPaths and read the plugin metadata at the same time.
 * When the plugin manager PluginManager reads the relevant plugin metadata, the
 * the PluginMetaObject metadata object will be generated internally.
 * See the file pluginmetaobject.h/.cpp
 * \return true if success
 */
bool LifeCycle::readPlugins()
{
    return pluginManager->readPlugins();
}

/*!
 * \brief LifeCycle::loadPlugins load all plugins
 * \pre {
 *  You need to execute readPlugins first, otherwise you will not be able to load any plugins
 * }
 * \details internally uses QPluginLoader to load plugins that have been read with plugin metadata
 *  This function will call the following function in the PluginManager
 * \code
 *  loadPlugins();
 *  initPlugins();
 *  startPlugins();
 * \endcode
 * For more details, see class PluginManager
 *
 * \return true if success
 */
bool LifeCycle::loadPlugins()
{
    if (!pluginManager->loadPlugins())
        return false;

    pluginManager->initPlugins();
    pluginManager->startPlugins();

    return true;
}

/*!
 * \brief LifeCycle::shutdownPlugins unload all plugins
 * \pre {
 *  Need to execute loadPlugins first, otherwise the call is meaningless
 * }
 * @details will internally execute the release Plugin interface pointer.
 *  Then the QPluginLoader unloader function will be executed internally, referring to the Qt feature
 * See the stopPlugins function in the class PluginManager for details
 *
 */
void LifeCycle::shutdownPlugins()
{
    pluginManager->stopPlugins();
}

bool LifeCycle::loadPlugin(PluginMetaObjectPointer &pointer)
{
    if (!pluginManager->loadPlugin(pointer))
        return false;
    if (!pluginManager->initPlugin(pointer))
        return false;
    if (!pluginManager->startPlugin(pointer))
        return false;

    return true;
}

void LifeCycle::shutdownPlugin(PluginMetaObjectPointer &pointer)
{
    pluginManager->stopPlugin(pointer);
}

DPF_END_NAMESPACE
