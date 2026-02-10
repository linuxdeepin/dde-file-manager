// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginmetaobject_p.h"
#include "pluginmanager_p.h"
#include "pluginquickmetadata_p.h"

#include <dfm-framework/listener/listener.h>
#include <dfm-framework/lifecycle/plugin.h>
#include <dfm-framework/lifecycle/plugincreator.h>

DPF_BEGIN_NAMESPACE

PluginManagerPrivate::PluginManagerPrivate(PluginManager *qq)
    : q(qq)
{
}

PluginManagerPrivate::~PluginManagerPrivate()
{
}

/*!
 * \brief 获取插件的元数据
 * \param name
 * \param version
 * \return
 */
PluginMetaObjectPointer PluginManagerPrivate::pluginMetaObj(const QString &name)
{
    auto result = std::find_if(readQueue.begin(), readQueue.end(), [name](PluginMetaObjectPointer ptr) {
        return ptr->name() == name;
    });
    if (result != readQueue.end())
        return *result;
    return {};
}

/*!
 * \brief 加载一个插件，可单独使用
 * \param pluginMetaObj
 */
bool PluginManagerPrivate::loadPlugin(PluginMetaObjectPointer &pluginMetaObj)
{
    bool result = doLoadPlugin(pluginMetaObj);

    return result;
}

/*!
 * \brief 初始化一个插件，可单独使用
 * \param pluginMetaObj
 */
bool PluginManagerPrivate::initPlugin(PluginMetaObjectPointer &pluginMetaObj)
{
    bool result = doInitPlugin(pluginMetaObj);

    return result;
}

/*!
 * \brief 启动一个插件，可单独使用
 * \param pluginMetaObj
 */
bool PluginManagerPrivate::startPlugin(PluginMetaObjectPointer &pluginMetaObj)
{
    bool result = doStartPlugin(pluginMetaObj);

    return result;
}

/*!
 * \brief 停止并卸载一个插件，可单独使用
 * \param pluginMetaObj
 */
bool PluginManagerPrivate::stopPlugin(PluginMetaObjectPointer &pluginMetaObj)
{
    return doStopPlugin(pluginMetaObj);
}

/*!
 * \brief 读取所有插件的Json源数据
 * \return
 */
bool PluginManagerPrivate::readPlugins()
{
    scanfAllPlugin();
    std::for_each(readQueue.begin(), readQueue.end(), [this](PluginMetaObjectPointer obj) {
        readJsonToMeta(obj);
        const QString &pluginName { obj->name() };
        if (lazyLoadPluginNames.contains(pluginName)) {
            qCDebug(logDPF) << "Skip load(lazy load): " << pluginName;
            return;
        }

        if (lazyPluginFilter && lazyPluginFilter(pluginName)) {
            lazyLoadPluginNames.append(pluginName);
            qCDebug(logDPF) << "Skip load(lazy load by filter): " << pluginName;
            return;
        }

        pluginsToLoad.append(obj);
    });

#ifdef QT_DEBUG
    qCDebug(logDPF) << "Start traversing the meta information of all plugins: ";
    for (auto read : readQueue) {
        qCDebug(logDPF) << read;
    }
    qCDebug(logDPF) << "End traversal of meta information for all plugins!";
#endif

    return readQueue.isEmpty() ? false : true;
}

/*!
 * \brief 扫描所有插件到目标队列
 * \param destQueue
 * \param pluginPaths
 * \param pluginIID
 */
void PluginManagerPrivate::scanfAllPlugin()
{
    if (pluginLoadIIDs.isEmpty()) {
        qCWarning(logDPF) << "PluginManagerPrivate: no plugin IIDs configured, skipping scan";
        return;
    }

    qCInfo(logDPF) << "PluginManagerPrivate: starting plugin scan in" << pluginLoadPaths.size() << "paths";
    int totalScanned = 0;
    int validPlugins = 0;

    for (const QString &path : pluginLoadPaths) {
        qCDebug(logDPF) << "PluginManagerPrivate: scanning path:" << path;
        QDirIterator dirItera(path, { "*.so" },
                              QDir::Filter::Files,
                              QDirIterator::IteratorFlag::NoIteratorFlags);

        int pathScanned = 0;
        while (dirItera.hasNext()) {
            dirItera.next();
            totalScanned++;
            pathScanned++;
            PluginMetaObjectPointer metaObj(new PluginMetaObject);
            const QString &fileName { dirItera.path() + "/" + dirItera.fileName() };
            qCDebug(logDPF) << "scan plugin:" << fileName;
            metaObj->d->loader->setFileName(fileName);
            QJsonObject &&metaJson = metaObj->d->loader->metaData();
            QJsonObject &&dataJson = metaJson.value("MetaData").toObject();
            QString &&iid = metaJson.value("IID").toString();
            if (!pluginLoadIIDs.contains(iid)) {
                qCWarning(logDPF) << "Invalid iid:" << fileName << iid;
                continue;
            }

            bool isVirtual = dataJson.contains(kVirtualPluginMeta) && dataJson.contains(kVirtualPluginList);
            if (isVirtual) {
                qCDebug(logDPF) << "PluginManagerPrivate: found virtual plugin:" << fileName;
                scanfVirtualPlugin(fileName, dataJson);
            } else {
                qCDebug(logDPF) << "PluginManagerPrivate: found real plugin:" << fileName;
                scanfRealPlugin(metaObj, dataJson);
            }
            validPlugins++;
        }
        qCDebug(logDPF) << "PluginManagerPrivate: scanned" << pathScanned << "files in path:" << path;
    }
    
    qCInfo(logDPF) << "PluginManagerPrivate: plugin scan completed - total scanned:" << totalScanned 
                   << "valid plugins:" << validPlugins << "in read queue:" << readQueue.size();
}

void PluginManagerPrivate::scanfRealPlugin(PluginMetaObjectPointer metaObj,
                                           const QJsonObject &dataJson)
{
    Q_ASSERT(metaObj);

    QString &&name = dataJson.value(kPluginName).toString();
    if (isBlackListed(name))
        return;

    metaObj->d->isVirtual = false;
    metaObj->d->name = name;
    readQueue.append(metaObj);
    metaObj->d->state = PluginMetaObject::kReaded;
}

void PluginManagerPrivate::scanfVirtualPlugin(const QString &fileName,
                                              const QJsonObject &dataJson)
{
    QJsonObject &&metaDataJson { dataJson.value(kVirtualPluginMeta).toObject() };
    QString &&realName { metaDataJson.value(kPluginName).toString() };
    if (isBlackListed(realName))
        return;

    QJsonArray &&virtualJsonArray { dataJson.value(kVirtualPluginList).toArray() };
    for (auto iter = virtualJsonArray.begin(); iter != virtualJsonArray.end(); ++iter) {
        QJsonObject &&object { iter->toObject() };
        QString &&name { object.value(kPluginName).toString() };
        if (isBlackListed(name))
            return;

        PluginMetaObjectPointer metaObj(new PluginMetaObject);
        metaObj->d->loader->setFileName(fileName);
        metaObj->d->isVirtual = true;
        metaObj->d->realName = realName;
        metaObj->d->name = name;
        readQueue.append(metaObj);
        metaObj->d->state = PluginMetaObject::kReaded;
    }
}

bool PluginManagerPrivate::isBlackListed(const QString &name)
{
    if (blackPluginNames.contains(name)) {
        qCWarning(logDPF) << "Black plugin: " << name << "don't load!";
        return true;
    }

    if (blackListFilter && blackListFilter(name)) {
        qCWarning(logDPF) << "Black plugin(filter): " << name << "don't load!";
        return true;
    }

    return false;
}

/*!
 * \brief 同步json到定义类型
 * \param metaObject
 */
void PluginManagerPrivate::readJsonToMeta(PluginMetaObjectPointer metaObject)
{
    metaObject->d->state = PluginMetaObject::kReading;

    QJsonObject &&jsonObj = metaObject->d->loader->metaData();
    if (jsonObj.isEmpty())
        return;

    QString &&iid = jsonObj.value("IID").toString();
    if (iid.isEmpty())
        return;
    metaObject->d->iid = iid;

    QJsonObject &&metaData = jsonObj.value("MetaData").toObject();
    if (metaObject->isVirtual()) {
        QJsonArray &&virtualJsonArray { metaData.value(kVirtualPluginList).toArray() };
        for (auto iter = virtualJsonArray.begin(); iter != virtualJsonArray.end(); ++iter) {
            QJsonObject &&object { iter->toObject() };
            QString &&name = object.value(kPluginName).toString();
            if (name.isEmpty() || metaObject->d->name != name)
                continue;
            jsonToMeta(metaObject, object);
        }
    } else {
        jsonToMeta(metaObject, metaData);
    }
}

void PluginManagerPrivate::jsonToMeta(PluginMetaObjectPointer metaObject, const QJsonObject &metaData)
{
    qCDebug(logDPF) << "PluginManagerPrivate: parsing JSON metadata for plugin:" << metaObject->d->name;
    
    metaObject->d->version = metaData.value(kPluginVersion).toString();
    metaObject->d->category = metaData.value(kPluginCategory).toString();
    metaObject->d->description = metaData.value(kPluginDescription).toString();
    metaObject->d->urlLink = metaData.value(kPluginUrlLink).toString();
    metaObject->d->customData = metaData.value(kCustomData).toVariant().toMap();

    QJsonArray &&dependsArray = metaData.value(kPluginDepends).toArray();
    auto itera = dependsArray.begin();
    while (itera != dependsArray.end()) {
        QJsonObject &&dependObj = itera->toObject();
        QString &&dependName = dependObj.value(kPluginName).toString();
        QString &&dependVersion = dependObj.value(kPluginVersion).toString();
        PluginDepend depends;
        depends.pluginName = dependName;
        depends.pluginVersion = dependVersion;
        metaObject->d->depends.append(depends);
        qCDebug(logDPF) << "PluginManagerPrivate: added dependency:" << dependName << "version:" << dependVersion;
        ++itera;
    }

    metaObject->d->state = PluginMetaObject::kReaded;

    // QML 组件信息
    QJsonArray &&quickArray = metaData.value(kQuick).toArray();
    qCDebug(logDPF) << "PluginManagerPrivate: processing" << quickArray.size() << "Quick components for plugin:" << metaObject->d->name;
    
    for (const auto &quickItr : quickArray) {
        const QJsonObject &quick = quickItr.toObject();

        QString quickParent = quick.value(kQuickParent).toString();
        // Quick plugin 的 parent 必须在 Depends 字段存在
        if (!quickParent.isEmpty() && quickParent.contains(".")) {
            QString parentPlugin = quickParent.split('.').first();
            const QList<PluginDepend> &depends = metaObject->d->depends;
            auto findItr = std::find_if(depends.cbegin(), depends.cend(), [&parentPlugin](const PluginDepend &depend) {
                return depend.pluginName == parentPlugin;
            });

            if (findItr == metaObject->d->depends.end()) {
                qCWarning(logDPF) << QString("Quick plugin %1 not find parent %2 plugin name on Depends field!")
                                             .arg(metaObject->d->name)
                                             .arg(quickParent);
                continue;
            }
        }

        // id url 不为空
        QString url = quick.value(kQuickUrl).toString();
        QString id = quick.value(kQuickId).toString();
        if (url.isEmpty() || id.isEmpty()) {
            qCWarning(logDPF) << QString("Quick plugin's id %1 or url %2 is empty").arg(id).arg(url);
            continue;
        }

        PluginQuickMetaPtr quickMeta = PluginQuickMetaPtr::create();
        // QML Url = [插件绝对路径 plugin.path] / [插件名称 plugin.name] / [url]
        QString pluginPath = QFileInfo(metaObject->fileName()).absolutePath();
        QString fullPath = pluginPath + QDir::separator() + metaObject->name() + QDir::separator() + url;
        quickMeta->d->quickUrl = QUrl::fromLocalFile(fullPath);
        quickMeta->d->quickId = id;
        quickMeta->d->plugin = metaObject->name();
        quickMeta->d->quickType = quick.value(kQuickType).toString();
        quickMeta->d->quickApplet = quick.value(kQuickApplet).toString();
        quickMeta->d->quickParent = quickParent;

        metaObject->d->quickMetaList.append(quickMeta);
        qCDebug(logDPF) << "PluginManagerPrivate: added Quick component:" << id << "url:" << fullPath;
    }
    
    qCDebug(logDPF) << "PluginManagerPrivate: JSON metadata parsing completed for plugin:" << metaObject->d->name;
}

/*!
 * \brief 内部使用QPluginLoader加载所有插件
 */
bool PluginManagerPrivate::loadPlugins()
{
    qCInfo(logDPF) << "Start loading all plugins: ";
    dependsSort(&loadQueue, &pluginsToLoad);

    bool ret = true;
    for (auto iter = loadQueue.begin(); iter != loadQueue.end();) {
        if (!PluginManagerPrivate::doLoadPlugin(*iter)) {
            qCWarning(logDPF) << "Failed to load plugin:" << (*iter)->name() << ", removing from queue";
            iter = loadQueue.erase(iter);   // 移除失败的插件并获取下一个迭代器
            ret = false;
        } else {
            ++iter;   // 加载成功,继续下一个
        }
    }
    qCInfo(logDPF) << "End loading all plugins.";

    return ret;
}

/*!
 * \brief 初始化所有插件
 */
bool PluginManagerPrivate::initPlugins()
{
    qCInfo(logDPF) << "Start initializing all plugins: ";
    bool ret = true;
    std::for_each(loadQueue.begin(), loadQueue.end(), [&ret, this](PluginMetaObjectPointer pointer) {
        if (!PluginManagerPrivate::doInitPlugin(pointer))
            ret = false;
    });
    qCInfo(logDPF) << "End initialization of all plugins.";

    emit Listener::instance()->pluginsInitialized();
    allPluginsInitialized = true;

    return ret;
}

/*!
 * \brief 拉起插件,仅主线程使用
 */
bool PluginManagerPrivate::startPlugins()
{
    qCInfo(logDPF) << "Start start all plugins: ";
    bool ret = true;
    std::for_each(loadQueue.begin(), loadQueue.end(), [&ret, this](PluginMetaObjectPointer pointer) {
        if (!PluginManagerPrivate::doStartPlugin(pointer))
            ret = false;
    });
    qCInfo(logDPF) << "End start of all plugins.";

    emit Listener::instance()->pluginsStarted();
    allPluginsStarted = true;

    return ret;
}

/*!
 * \brief 停止插件,仅主线程
 */
void PluginManagerPrivate::stopPlugins()
{
    // reverse queue
    std::for_each(loadQueue.rbegin(), loadQueue.rend(), [this](PluginMetaObjectPointer pointer) {
        PluginManagerPrivate::doStopPlugin(pointer);
    });
}

/*!
 * \brief 按照依赖排序
 * \param dstQueue
 * \param srcQueue
 */
void PluginManagerPrivate::dependsSort(QQueue<PluginMetaObjectPointer> *dstQueue,
                                       const QQueue<PluginMetaObjectPointer> *srcQueue)
{
    Q_ASSERT(dstQueue);
    Q_ASSERT(srcQueue);

    PluginDependGroup dependGroup;   // list of pair<depended plugin, plugin>
    QMap<QString, PluginMetaObjectPointer> srcMap;   // key: plugin name

    std::for_each(srcQueue->cbegin(), srcQueue->cend(), [&srcMap](PluginMetaObjectPointer ptr) {
        srcMap[ptr->name()] = ptr;
    });

    // make depends pair group
    std::for_each(srcQueue->begin(), srcQueue->end(), [&dependGroup, srcMap](PluginMetaObjectPointer ptr) {
        for (const PluginDepend &depend : ptr->depends()) {
            QString &&name { depend.name() };
            if (srcMap.contains(name)) {
                qCInfo(logDPF, "Dependency `%s` <- `%s`", qUtf8Printable(name), qUtf8Printable(ptr->name()));
                dependGroup.append({ srcMap.value(name), ptr });
            } else {
                qCWarning(logDPF, "Plugin `%s` cannot depend a unkonw plugin: `%s`", qUtf8Printable(ptr->name()), qUtf8Printable(name));
            }
        }
    });

    // sort
    dstQueue->clear();
    if (!doPluginSort(dependGroup, srcMap, dstQueue)) {
        qCWarning(logDPF) << "Sort depnd group failed!";
        *dstQueue = *srcQueue;
        return;
    }
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
bool PluginManagerPrivate::checkPluginQtVersion(PluginMetaObjectPointer pointer)
{
    Q_ASSERT(pointer && pointer->d->loader);

    auto name { pointer->d->name };
    if (qtVersionInsensitivePluginNames.contains(name)) {
        qCDebug(logDPF) << "Skip Qt version check" << name;
        return true;
    }

    // Create QLibrary instance using the plugin's file path
    QLibrary lib(pointer->d->loader->fileName());
    if (!lib.load()) {
        pointer->d->error = QString("Failed to load library for version check: %1").arg(lib.errorString());
        return false;
    }

    // Use QLibrary to resolve qVersion symbol
    using QVersionFunction = const char *(*)();
    auto qVersionFunc = reinterpret_cast<QVersionFunction>(lib.resolve("qVersion"));

    if (!qVersionFunc) {
        pointer->d->error = QString("Plugin '%1' does not link against Qt").arg(pointer->d->name);
        lib.unload();
        return false;
    }

    const QString pluginQtVersion = QString::fromLatin1(qVersionFunc());
    lib.unload();

    if (!pluginQtVersion.startsWith('6')) {
        pointer->d->error = QString("Qt version compatibility check failed:\n"
                                    "- Plugin name: %1\n"
                                    "- Plugin path: %2\n"
                                    "- Plugin Qt version: %3\n"
                                    "- Application Qt version: %4\n"
                                    "Error: Qt6 application cannot load Qt%3 plugins.")
                                    .arg(pointer->d->name)
                                    .arg(pointer->fileName())
                                    .arg(pluginQtVersion)
                                    .arg(QString::fromLatin1(qVersion()));
        return false;
    }

    return true;
}
#endif

bool PluginManagerPrivate::doLoadPlugin(PluginMetaObjectPointer pointer)
{
    Q_ASSERT(pointer);

    // 流程互斥
    if (pointer->d->state >= PluginMetaObject::State::kLoaded) {
        qCInfo(logDPF) << "Is Loaded plugin: "
                       << pointer->d->name
                       << pointer->fileName();
        return true;
    }

    // 必须执行了读取操作
    if (pointer->d->state != PluginMetaObject::State::kReaded) {
        qCCritical(logDPF) << "Failed load plugin: "
                           << pointer->d->name
                           << pointer->fileName();
        return false;
    }

    qCInfo(logDPF) << "PluginManagerPrivate: starting to load plugin:" << pointer->d->name;
    pointer->d->state = PluginMetaObject::State::kLoading;

    if (pointer->isVirtual() && loadedVirtualPlugins.contains(pointer->d->realName)) {
        auto creator = qobject_cast<PluginCreator *>(pointer->d->loader->instance());
        if (creator) {
            pointer->d->plugin = creator->create(pointer->name());
            qCDebug(logDPF) << "PluginManagerPrivate: created virtual plugin instance for:" << pointer->d->name;
        } else {
            qCWarning(logDPF) << "PluginManagerPrivate: failed to get creator for virtual plugin:" << pointer->d->name;
        }
        pointer->d->state = PluginMetaObject::State::kLoaded;
        qCInfo(logDPF) << "Virtual Plugin: " << pointer->d->name << " has been loaded";
        return true;
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // Check Qt version compatibility after plugin is loaded
    if (!checkPluginQtVersion(pointer)) {
        qCCritical(logDPF) << pointer->d->error;
        pointer->d->loader->unload();
        return false;
    }
#endif

    if (!pointer->d->loader->load()) {
        pointer->d->error = "Failed load plugin: " + pointer->d->loader->errorString();
        qCCritical(logDPF) << pointer->errorString() << pointer->d->name << pointer->d->loader->fileName();
        return false;
    }

    // resolve loader instance
    bool isNullPluginInstance { false };
    if (pointer->isVirtual()) {
        auto creator = qobject_cast<PluginCreator *>(pointer->d->loader->instance());
        if (creator) {
            pointer->d->plugin = creator->create(pointer->name());
            qCDebug(logDPF) << "PluginManagerPrivate: created virtual plugin instance:" << pointer->d->name;
        } else {
            isNullPluginInstance = true;
            qCWarning(logDPF) << "PluginManagerPrivate: failed to get creator for virtual plugin:" << pointer->d->name;
        }
    } else {
        pointer->d->plugin = QSharedPointer<Plugin>(qobject_cast<Plugin *>(pointer->d->loader->instance()));
        if (pointer->d->plugin.isNull()) {
            isNullPluginInstance = true;
            qCWarning(logDPF) << "PluginManagerPrivate: failed to get plugin instance for:" << pointer->d->name;
        } else {
            qCDebug(logDPF) << "PluginManagerPrivate: got plugin instance for:" << pointer->d->name;
        }
    }

    if (isNullPluginInstance) {
        pointer->d->error = "Failed get plugin instance is nullptr";
        qCCritical(logDPF) << pointer->d->name << pointer->d->error;
        return false;
    }

    // load success
    pointer->d->state = PluginMetaObject::State::kLoaded;
    qCInfo(logDPF) << "Loaded plugin: " << pointer->d->name << pointer->d->loader->fileName();
    if (pointer->isVirtual())
        loadedVirtualPlugins.push_back(pointer->d->realName);

    return true;
}

bool PluginManagerPrivate::doInitPlugin(PluginMetaObjectPointer pointer)
{
    Q_ASSERT(pointer);

    if (pointer->d->state >= PluginMetaObject::State::kInitialized) {
        qCInfo(logDPF) << "Is initialized plugin: "
                       << pointer->d->name
                       << pointer->fileName();
        return true;
    }

    if (pointer->d->state != PluginMetaObject::State::kLoaded) {
        qCCritical(logDPF) << "Failed initialized plugin: "
                           << pointer->d->name
                           << pointer->fileName();
        return false;
    }

    if (pointer->d->plugin.isNull()) {
        pointer->d->error = "Failed initialized plugin, plugin instance is nullptr";
        qCCritical(logDPF) << pointer->d->name << pointer->d->error;
        return false;
    }

    pointer->d->state = PluginMetaObject::State::kInitialized;
    pointer->d->plugin->initialize();
    qCInfo(logDPF) << "Initialized plugin: " << pointer->d->name;
    emit Listener::instance()->pluginInitialized(pointer->d->iid, pointer->d->name);

    return true;
}

bool PluginManagerPrivate::doStartPlugin(PluginMetaObjectPointer pointer)
{
    Q_ASSERT(pointer);

    if (pointer->d->state >= PluginMetaObject::State::kStarted) {
        qCInfo(logDPF) << "Is started plugin:"
                       << pointer->d->name
                       << pointer->fileName();
        return true;
    }

    if (pointer->d->state != PluginMetaObject::State::kInitialized) {
        qCCritical(logDPF) << "Failed start plugin:"
                           << pointer->d->name
                           << pointer->fileName();
        return false;
    }

    if (pointer->d->plugin.isNull()) {
        pointer->d->error = "Failed start plugin, plugin instance is nullptr";
        qCCritical(logDPF) << pointer->d->name << pointer->d->error;
        return false;
    }

    if (pointer->d->plugin->start()) {
        qCInfo(logDPF) << "Started plugin: " << pointer->d->name;
        pointer->d->state = PluginMetaObject::State::kStarted;
        emit Listener::instance()->pluginStarted(pointer->d->iid, pointer->d->name);
        return true;
    }

    pointer->d->error = "Failed start plugin in function start() logic";
    qCCritical(logDPF) << pointer->d->error.toLocal8Bit().data();
    return false;
}

bool PluginManagerPrivate::doStopPlugin(PluginMetaObjectPointer pointer)
{
    Q_ASSERT(pointer);

    if (pointer->d->state >= PluginMetaObject::State::kStoped) {
        qCInfo(logDPF) << "Is stoped plugin:"
                       << pointer->d->name
                       << pointer->fileName();
        return true;
    }

    if (pointer->d->state != PluginMetaObject::State::kStarted) {
        qCCritical(logDPF) << "Failed stop plugin:"
                           << pointer->d->name
                           << pointer->fileName();
        return false;
    }

    if (pointer->d->plugin.isNull()) {
        pointer->d->error = "Failed stop plugin, plugin instance is nullptr";
        qCCritical(logDPF) << pointer->d->name << pointer->d->error;
        return false;
    }

    pointer->d->plugin->stop();
    pointer->d->plugin = nullptr;
    pointer->d->state = PluginMetaObject::State::kStoped;
    qCInfo(logDPF) << "stop" << pointer->d->loader->fileName();

    if (!pointer->d->isVirtual && !pointer->d->loader->unload()) {
        qCWarning(logDPF) << "Unload plugin failed: " << pointer->d->name
                          << pointer->d->loader->errorString();
        return false;
    }

    pointer->d->state = PluginMetaObject::State::kShutdown;
    qCDebug(logDPF) << "shutdown" << pointer->d->loader->fileName();
    return true;
}

bool PluginManagerPrivate::doPluginSort(const PluginDependGroup group, QMap<QString, PluginMetaObjectPointer> src, QQueue<PluginMetaObjectPointer> *dest)
{
    if (!group.isEmpty() && src.isEmpty()) {
        qCWarning(logDPF) << "Maybe circle depends occured";
        return false;
    }

    if (group.isEmpty() && src.isEmpty())
        return true;

    PluginDependGroup nextGroup;
    QMap<QString, PluginMetaObjectPointer> nextSrc;

    for (const auto &pair : group) {
        const QString &rname = pair.second->name();
        if (src.contains(rname)) {
            src.remove(rname);
            if (!nextSrc.contains(rname))
                nextSrc.insert(rname, pair.second);
        }
    }

    for (auto itor = src.cbegin(); itor != src.cend(); ++itor)
        dest->push_back(itor.value());

    const QStringList &keys = src.keys();
    for (const auto &pair : group) {
        const QString &lname = pair.first->name();
        if (!keys.contains(lname))
            nextGroup.push_back(pair);
    }

    if (!nextGroup.isEmpty() && nextGroup.size() == group.size()) {
        qCWarning(logDPF) << "Maybe circle depends occured, header circle";
        return false;
    }

    return doPluginSort(nextGroup, nextSrc, dest);
}

DPF_END_NAMESPACE
