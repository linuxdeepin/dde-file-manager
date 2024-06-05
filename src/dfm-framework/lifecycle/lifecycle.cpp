// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-framework/lifecycle/lifecycle.h>
#include <dfm-framework/lifecycle/pluginmanager.h>
#include <dfm-framework/lifecycle/pluginmetaobject.h>

DPF_BEGIN_NAMESPACE
namespace LifeCycle {

Q_GLOBAL_STATIC(PluginManager, pluginManager);

/*!
 * \brief LifeCycle::initialize
 * \param IIDs Add a class of plug-in IID to the framework,
 * the framework will load the IID plug-in to achieve an
 * APP can load a variety of different IID plug-in capabilities,
 * like org.deepin.plugin.[XXX]
 * \param paths
 */
void initialize(const QStringList &IIDs, const QStringList &paths)
{
    for (const QString &id : IIDs)
        pluginManager->addPluginIID(id);
    pluginManager->setPluginPaths(paths);
}

void initialize(const QStringList &IIDs, const QStringList &paths, const QStringList &blackNames)
{
    for (const QString &name : blackNames)
        pluginManager->addBlackPluginName(name);
    initialize(IIDs, paths);
}

void initialize(const QStringList &IIDs, const QStringList &paths, const QStringList &blackNames, const QStringList &lazyNames)
{
    for (const QString &name : lazyNames)
        pluginManager->addLazyLoadPluginName(name);
    initialize(IIDs, paths, blackNames);
}

/*!
 * \brief LifeCycle::pluginIIDs Get plugin identity
 * \return all id list
 */
QStringList pluginIIDs()
{
    return pluginManager->pluginIIDs();
}

/*!
 * \brief LifeCycle::pluginPaths get all plugin path list
 * \return plugin path list
 */
QStringList pluginPaths()
{
    return pluginManager->pluginPaths();
}

QStringList blackList()
{
    return pluginManager->blackList();
}

QStringList lazyLoadList()
{
    return pluginManager->lazyLoadList();
}

PluginMetaObjectPointer pluginMetaObj(const QString &pluginName,
                                      const QString version)
{
    return pluginManager->pluginMetaObj(pluginName, version);
}

QList<PluginMetaObjectPointer> pluginMetaObjs(const std::function<bool(PluginMetaObjectPointer)> &cond)
{
    const auto &queue { pluginManager->readQueue() };
    if (!cond)
        return queue;

    QList<PluginMetaObjectPointer> ptrs;
    std::copy_if(queue.begin(), queue.end(), std::back_inserter(ptrs),
                 [cond](const PluginMetaObjectPointer &ptr) {
                     return cond(ptr);
                 });
    return ptrs;
}

QList<PluginMetaObjectPointer> pluginSortedMetaObjs(const std::function<bool(PluginMetaObjectPointer)> &cond)
{
    const auto &queue { pluginManager->loadQueue() };
    if (!cond)
        return queue;

    QList<PluginMetaObjectPointer> ptrs;
    std::copy_if(queue.begin(), queue.end(), std::back_inserter(ptrs),
                 [cond](const PluginMetaObjectPointer &ptr) {
                     return cond(ptr);
                 });
    return ptrs;
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
bool readPlugins()
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
bool loadPlugins()
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
void shutdownPlugins()
{
    pluginManager->stopPlugins();
}

bool loadPlugin(PluginMetaObjectPointer &pointer)
{
    if (!pluginManager->loadPlugin(pointer))
        return false;
    if (!pluginManager->initPlugin(pointer))
        return false;
    if (!pluginManager->startPlugin(pointer))
        return false;

    return true;
}

void shutdownPlugin(PluginMetaObjectPointer &pointer)
{
    pluginManager->stopPlugin(pointer);
}

bool isAllPluginsInitialized()
{
    return pluginManager->isAllPluginsInitialized();
}

bool isAllPluginsStarted()
{
    return pluginManager->isAllPluginsStarted();
}

void setLazyloadFilter(std::function<bool(const QString &)> filter)
{
    pluginManager->setLazyLoadFilter(filter);
}

void setBlackListFilter(std::function<bool(const QString &)> filter)
{
    pluginManager->setBlackListFilter(filter);
}

}   // namespace LifeCycle
DPF_END_NAMESPACE
