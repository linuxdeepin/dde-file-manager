// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-framework/lifecycle/lifecycle.h>
#include <dfm-framework/lifecycle/pluginmanager.h>
#include <dfm-framework/lifecycle/pluginmetaobject.h>
#include <mutex>

DPF_BEGIN_NAMESPACE
namespace LifeCycle {

/*!
 * \brief getPluginManager Get the global plugin manager instance
 * \details Uses Leaky Singleton pattern to avoid static destruction order issues.
 * The instance is created once and never destroyed, which is safe for system-level
 * plugin managers that need to exist throughout the entire program lifecycle.
 * \return Pointer to the global PluginManager instance
 */
static PluginManager* getPluginManager()
{
    // C++11 guarantees thread-safe initialization of function static variables
    // The instance is created on first call and never destroyed
    static PluginManager* instance = new PluginManager();
    return instance;
}

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
    qCInfo(logDPF) << "LifeCycle: initializing with" << IIDs.size() << "IIDs and" << paths.size() << "paths";
    qCDebug(logDPF) << "LifeCycle: IIDs:" << IIDs;
    qCDebug(logDPF) << "LifeCycle: paths:" << paths;
    
    for (const QString &id : IIDs)
        getPluginManager()->addPluginIID(id);
    getPluginManager()->setPluginPaths(paths);
    
    qCInfo(logDPF) << "LifeCycle: initialization completed";
}

void initialize(const QStringList &IIDs, const QStringList &paths, const QStringList &blackNames)
{
    qCInfo(logDPF) << "LifeCycle: initializing with blacklist," << blackNames.size() << "blacklisted plugins";
    qCDebug(logDPF) << "LifeCycle: blacklisted plugins:" << blackNames;
    
    for (const QString &name : blackNames)
        getPluginManager()->addBlackPluginName(name);
    initialize(IIDs, paths);
}

void initialize(const QStringList &IIDs, const QStringList &paths, const QStringList &blackNames, const QStringList &lazyNames)
{
    qCInfo(logDPF) << "LifeCycle: initializing with lazy loading," << lazyNames.size() << "lazy plugins";
    qCDebug(logDPF) << "LifeCycle: lazy plugins:" << lazyNames;
    
    for (const QString &name : lazyNames)
        getPluginManager()->addLazyLoadPluginName(name);
    initialize(IIDs, paths, blackNames);
}

void registerQtVersionInsensitivePlugins(const QStringList &names)
{
    qCInfo(logDPF) << "LifeCycle: registering" << names.size() << "Qt version insensitive plugins";
    qCDebug(logDPF) << "LifeCycle: Qt version insensitive plugins:" << names;
    getPluginManager()->setQtVersionInsensitivePluginNames(names);
}

/*!
 * \brief LifeCycle::pluginIIDs Get plugin identity
 * \return all id list
 */
QStringList pluginIIDs()
{
    return getPluginManager()->pluginIIDs();
}

/*!
 * \brief LifeCycle::pluginPaths get all plugin path list
 * \return plugin path list
 */
QStringList pluginPaths()
{
    return getPluginManager()->pluginPaths();
}

QStringList blackList()
{
    return getPluginManager()->blackList();
}

QStringList lazyLoadList()
{
    return getPluginManager()->lazyLoadList();
}

PluginMetaObjectPointer pluginMetaObj(const QString &pluginName,
                                      const QString version)
{
    return getPluginManager()->pluginMetaObj(pluginName, version);
}

QList<PluginMetaObjectPointer> pluginMetaObjs(const std::function<bool(PluginMetaObjectPointer)> &cond)
{
    const auto &queue { getPluginManager()->readQueue() };
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
    const auto &queue { getPluginManager()->loadQueue() };
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
    qCInfo(logDPF) << "LifeCycle: starting to read plugins";
    bool result = getPluginManager()->readPlugins();
    qCInfo(logDPF) << "LifeCycle: read plugins completed, success:" << result;
    return result;
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
    qCInfo(logDPF) << "LifeCycle: starting plugin loading sequence";
    
    bool result { getPluginManager()->loadPlugins() };
    qCInfo(logDPF) << "LifeCycle: plugin loading phase completed, success:" << result;

    getPluginManager()->initPlugins();
    qCInfo(logDPF) << "LifeCycle: plugin initialization phase completed";
    
    getPluginManager()->startPlugins();
    qCInfo(logDPF) << "LifeCycle: plugin startup phase completed";

    return result;
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
    static std::once_flag shutdownOnce;
    
    std::call_once(shutdownOnce, []() {
        qCInfo(logDPF) << "LifeCycle: starting plugin shutdown";
        // Now this call is always safe because the instance is never destroyed
        getPluginManager()->stopPlugins();
        qCInfo(logDPF) << "LifeCycle: plugin shutdown completed";
    });
}

bool loadPlugin(PluginMetaObjectPointer &pointer)
{
    if (!pointer) {
        qCWarning(logDPF) << "LifeCycle: attempted to load null plugin pointer";
        return false;
    }
    
    qCInfo(logDPF) << "LifeCycle: loading single plugin:" << pointer->name();
    
    if (!getPluginManager()->loadPlugin(pointer)) {
        qCWarning(logDPF) << "LifeCycle: failed to load plugin:" << pointer->name();
        return false;
    }
    if (!getPluginManager()->initPlugin(pointer)) {
        qCWarning(logDPF) << "LifeCycle: failed to initialize plugin:" << pointer->name();
        return false;
    }
    if (!getPluginManager()->startPlugin(pointer)) {
        qCWarning(logDPF) << "LifeCycle: failed to start plugin:" << pointer->name();
        return false;
    }

    qCInfo(logDPF) << "LifeCycle: successfully loaded plugin:" << pointer->name();
    return true;
}

void shutdownPlugin(PluginMetaObjectPointer &pointer)
{
    if (!pointer) {
        qCWarning(logDPF) << "LifeCycle: attempted to shutdown null plugin pointer";
        return;
    }
    
    qCInfo(logDPF) << "LifeCycle: shutting down plugin:" << pointer->name();
    getPluginManager()->stopPlugin(pointer);
    qCInfo(logDPF) << "LifeCycle: plugin shutdown completed:" << pointer->name();
}

bool isAllPluginsInitialized()
{
    bool result = getPluginManager()->isAllPluginsInitialized();
    qCDebug(logDPF) << "LifeCycle: all plugins initialized:" << result;
    return result;
}

bool isAllPluginsStarted()
{
    bool result = getPluginManager()->isAllPluginsStarted();
    qCDebug(logDPF) << "LifeCycle: all plugins started:" << result;
    return result;
}

void setLazyloadFilter(std::function<bool(const QString &)> filter)
{
    qCInfo(logDPF) << "LifeCycle: setting lazy load filter";
    getPluginManager()->setLazyLoadFilter(filter);
}

void setBlackListFilter(std::function<bool(const QString &)> filter)
{
    qCInfo(logDPF) << "LifeCycle: setting blacklist filter";
    getPluginManager()->setBlackListFilter(filter);
}

}   // namespace LifeCycle
DPF_END_NAMESPACE
