// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginmetaobject_p.h"
#include "pluginmanager_p.h"

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
    scanfAllPlugin(&readQueue, pluginLoadPaths, pluginLoadIIDs, blackPlguinNames);
    qInfo() << "Lazy load plugin names: " << lazyLoadPluginsNames;
    std::for_each(readQueue.begin(), readQueue.end(), [this](PluginMetaObjectPointer obj) {
        readJsonToMeta(obj);
        if (!lazyLoadPluginsNames.contains(obj->name()))
            notLazyLoadQuene.append(obj);
        else
            qInfo() << "Skip load(lazy load): " << obj->name();
    });

#ifdef QT_DEBUG
    qDebug() << "Start traversing the meta information of all plugins: ";
    for (auto read : readQueue) {
        qDebug() << read;
    }
    qDebug() << "End traversal of meta information for all plugins!";
#endif

    return readQueue.isEmpty() ? false : true;
}

/*!
 * \brief 扫描所有插件到目标队列
 * \param destQueue
 * \param pluginPaths
 * \param pluginIID
 */
void PluginManagerPrivate::scanfAllPlugin(QQueue<PluginMetaObjectPointer> *destQueue,
                                          const QStringList &pluginPaths,
                                          const QStringList &pluginIIDs,
                                          const QStringList &blackList)
{
    Q_ASSERT(destQueue);

    if (pluginIIDs.isEmpty())
        return;

    for (const QString &path : pluginPaths) {
        QDirIterator dirItera(path, { "*.so" },
                              QDir::Filter::Files,
                              QDirIterator::IteratorFlag::NoIteratorFlags);

        while (dirItera.hasNext()) {
            dirItera.next();
            PluginMetaObjectPointer metaObj(new PluginMetaObject);
            const QString &fileName { dirItera.path() + "/" + dirItera.fileName() };
            metaObj->d->loader->setFileName(fileName);
            QJsonObject &&metaJson = metaObj->d->loader->metaData();
            QJsonObject &&dataJson = metaJson.value("MetaData").toObject();
            QString &&iid = metaJson.value("IID").toString();
            if (!pluginIIDs.contains(iid))
                continue;

            bool isVirtual = dataJson.contains(kVirtualPluginMeta) && dataJson.contains(kVirtualPluginList);
            if (isVirtual)
                scanfVirtualPlugin(destQueue, fileName, dataJson, blackList);
            else
                scanfRealPlugin(destQueue, metaObj, dataJson, blackList);
        }
    }
}

void PluginManagerPrivate::scanfRealPlugin(QQueue<PluginMetaObjectPointer> *destQueue, PluginMetaObjectPointer metaObj,
                                           const QJsonObject &dataJson, const QStringList &blackList)
{
    Q_ASSERT(destQueue);
    Q_ASSERT(metaObj);

    QString &&name = dataJson.value(kPluginName).toString();
    if (blackList.contains(name)) {
        qWarning() << "Black plugin: " << name << "don't load!";
        return;
    }

    metaObj->d->isVirtual = false;
    metaObj->d->name = name;
    destQueue->append(metaObj);
    metaObj->d->state = PluginMetaObject::kReaded;
}

void PluginManagerPrivate::scanfVirtualPlugin(QQueue<PluginMetaObjectPointer> *destQueue, const QString &fileName,
                                              const QJsonObject &dataJson, const QStringList &blackList)
{
    Q_ASSERT(destQueue);

    QJsonObject &&metaDataJson { dataJson.value(kVirtualPluginMeta).toObject() };
    QString &&realName { metaDataJson.value(kPluginName).toString() };
    if (blackList.contains(realName)) {
        qWarning() << "Black plugin: " << realName << "don't load!";
        return;
    }

    QJsonArray &&virtualJsonArray { dataJson.value(kVirtualPluginList).toArray() };
    for (auto iter = virtualJsonArray.begin(); iter != virtualJsonArray.end(); ++iter) {
        QJsonObject &&object { iter->toObject() };
        QString &&name { object.value(kPluginName).toString() };
        if (blackList.contains(name)) {
            qWarning() << "Black plugin: " << name << "don't load!";
            continue;
        }

        PluginMetaObjectPointer metaObj(new PluginMetaObject);
        metaObj->d->loader->setFileName(fileName);
        metaObj->d->isVirtual = true;
        metaObj->d->realName = realName;
        metaObj->d->name = name;
        destQueue->append(metaObj);
        metaObj->d->state = PluginMetaObject::kReaded;
    }
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
    metaObject->d->version = metaData.value(kPluginVersion).toString();
    metaObject->d->category = metaData.value(kPluginCategory).toString();
    metaObject->d->description = metaData.value(kPluginDescription).toString();
    metaObject->d->urlLink = metaData.value(kPluginUrlLink).toString();

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
        ++itera;
    }

    metaObject->d->state = PluginMetaObject::kReaded;
}

/*!
 * \brief 内部使用QPluginLoader加载所有插件
 */
bool PluginManagerPrivate::loadPlugins()
{
    qInfo() << "Start loading all plugins: ";
    dependsSort(&loadQueue, &notLazyLoadQuene);

    bool ret = true;
    std::for_each(loadQueue.begin(), loadQueue.end(), [&ret, this](PluginMetaObjectPointer pointer) {
        if (!PluginManagerPrivate::doLoadPlugin(pointer))
            ret = false;
    });
    qInfo() << "End loading all plugins.";

    return ret;
}

/*!
 * \brief 初始化所有插件
 */
bool PluginManagerPrivate::initPlugins()
{
    qInfo() << "Start initializing all plugins: ";
    bool ret = true;
    std::for_each(loadQueue.begin(), loadQueue.end(), [&ret, this](PluginMetaObjectPointer pointer) {
        if (!PluginManagerPrivate::doInitPlugin(pointer))
            ret = false;
    });
    qInfo() << "End initialization of all plugins.";

    emit Listener::instance()->pluginsInitialized();
    allPluginsInitialized = true;

    return ret;
}

/*!
 * \brief 拉起插件,仅主线程使用
 */
bool PluginManagerPrivate::startPlugins()
{
    qInfo() << "Start start all plugins: ";
    bool ret = true;
    std::for_each(loadQueue.begin(), loadQueue.end(), [&ret, this](PluginMetaObjectPointer pointer) {
        if (!PluginManagerPrivate::doStartPlugin(pointer))
            ret = false;
    });
    qInfo() << "End start of all plugins.";

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
                qInfo("Dependency `%s` <- `%s`", qUtf8Printable(name), qUtf8Printable(ptr->name()));
                dependGroup.append({ srcMap.value(name), ptr });
            } else {
                qWarning("Plugin `%s` cannot depend a unkonw plugin: `%s`", qUtf8Printable(ptr->name()), qUtf8Printable(name));
            }
        }
    });

    // sort
    dstQueue->clear();
    if (!doPluginSort(dependGroup, srcMap, dstQueue)) {
        qWarning() << "Sort depnd group failed!";
        *dstQueue = *srcQueue;
        return;
    }
}

bool PluginManagerPrivate::doLoadPlugin(PluginMetaObjectPointer pointer)
{
    Q_ASSERT(pointer);

    // 流程互斥
    if (pointer->d->state >= PluginMetaObject::State::kLoaded) {
        qInfo() << "Is Loaded plugin: "
                << pointer->d->name
                << pointer->fileName();
        return true;
    }

    // 必须执行了读取操作
    if (pointer->d->state != PluginMetaObject::State::kReaded) {
        qCritical() << "Failed load plugin: "
                    << pointer->d->name
                    << pointer->fileName();
        return false;
    }

    pointer->d->state = PluginMetaObject::State::kLoading;

    if (pointer->isVirtual() && loadedVirtualPlugins.contains(pointer->d->realName)) {
        auto creator = qobject_cast<PluginCreator *>(pointer->d->loader->instance());
        if (creator)
            pointer->d->plugin = creator->create(pointer->name());
        pointer->d->state = PluginMetaObject::State::kLoaded;
        qInfo() << "Virtual Plugin: " << pointer->d->name << " has been loaded";
        return true;
    }

    if (!pointer->d->loader->load()) {
        pointer->d->error = "Failed load plugin: " + pointer->d->loader->errorString();
        qCritical() << pointer->errorString() << pointer->d->name << pointer->d->loader->fileName();
        return false;
    }

    // resolve loader instance
    bool isNullPluginInstance { false };
    if (pointer->isVirtual()) {
        auto creator = qobject_cast<PluginCreator *>(pointer->d->loader->instance());
        if (creator)
            pointer->d->plugin = creator->create(pointer->name());
        else
            isNullPluginInstance = true;
    } else {
        pointer->d->plugin = QSharedPointer<Plugin>(qobject_cast<Plugin *>(pointer->d->loader->instance()));
        if (pointer->d->plugin.isNull())
            isNullPluginInstance = true;
    }

    if (isNullPluginInstance) {
        pointer->d->error = "Failed get plugin instance is nullptr";
        qCritical() << pointer->d->name << pointer->d->error;
        return false;
    }

    // load success
    pointer->d->state = PluginMetaObject::State::kLoaded;
    qInfo() << "Loaded plugin: " << pointer->d->name << pointer->d->loader->fileName();
    if (pointer->isVirtual())
        loadedVirtualPlugins.push_back(pointer->d->realName);

    return true;
}

bool PluginManagerPrivate::doInitPlugin(PluginMetaObjectPointer pointer)
{
    Q_ASSERT(pointer);

    if (pointer->d->state >= PluginMetaObject::State::kInitialized) {
        qInfo() << "Is initialized plugin: "
                << pointer->d->name
                << pointer->fileName();
        return true;
    }

    if (pointer->d->state != PluginMetaObject::State::kLoaded) {
        qCritical() << "Failed initialized plugin: "
                    << pointer->d->name
                    << pointer->fileName();
        return false;
    }

    if (pointer->d->plugin.isNull()) {
        pointer->d->error = "Failed initialized plugin, plugin instance is nullptr";
        qCritical() << pointer->d->name << pointer->d->error;
        return false;
    }

    pointer->d->state = PluginMetaObject::State::kInitialized;
    pointer->d->plugin->initialize();
    qInfo() << "Initialized plugin: " << pointer->d->name;
    emit Listener::instance()->pluginInitialized(pointer->d->iid, pointer->d->name);

    return true;
}

bool PluginManagerPrivate::doStartPlugin(PluginMetaObjectPointer pointer)
{
    Q_ASSERT(pointer);

    if (pointer->d->state >= PluginMetaObject::State::kStarted) {
        qInfo() << "Is started plugin:"
                << pointer->d->name
                << pointer->fileName();
        return true;
    }

    if (pointer->d->state != PluginMetaObject::State::kInitialized) {
        qCritical() << "Failed start plugin:"
                    << pointer->d->name
                    << pointer->fileName();
        return false;
    }

    if (pointer->d->plugin.isNull()) {
        pointer->d->error = "Failed start plugin, plugin instance is nullptr";
        qCritical() << pointer->d->name << pointer->d->error;
        return false;
    }

    if (pointer->d->plugin->start()) {
        qInfo() << "Started plugin: " << pointer->d->name;
        pointer->d->state = PluginMetaObject::State::kStarted;
        emit Listener::instance()->pluginStarted(pointer->d->iid, pointer->d->name);
        return true;
    }

    pointer->d->error = "Failed start plugin in function start() logic";
    qCritical() << pointer->d->error.toLocal8Bit().data();
    return false;
}

bool PluginManagerPrivate::doStopPlugin(PluginMetaObjectPointer pointer)
{
    Q_ASSERT(pointer);

    if (pointer->d->state >= PluginMetaObject::State::kStoped) {
        qInfo() << "Is stoped plugin:"
                << pointer->d->name
                << pointer->fileName();
        return true;
    }

    if (pointer->d->state != PluginMetaObject::State::kStarted) {
        qCritical() << "Failed stop plugin:"
                    << pointer->d->name
                    << pointer->fileName();
        return false;
    }

    if (pointer->d->plugin.isNull()) {
        pointer->d->error = "Failed stop plugin, plugin instance is nullptr";
        qCritical() << pointer->d->name << pointer->d->error;
        return false;
    }

    pointer->d->plugin->stop();
    pointer->d->plugin = nullptr;
    pointer->d->state = PluginMetaObject::State::kStoped;
    qInfo() << "stop" << pointer->d->loader->fileName();

    if (!pointer->d->loader->unload()) {
        qWarning() << "Unload plugin failed: " << pointer->d->loader->errorString();
        return false;
    }

    pointer->d->state = PluginMetaObject::State::kShutdown;
    qDebug() << "shutdown" << pointer->d->loader->fileName();
    return true;
}

bool PluginManagerPrivate::doPluginSort(const PluginDependGroup group, QMap<QString, PluginMetaObjectPointer> src, QQueue<PluginMetaObjectPointer> *dest)
{
    if (!group.isEmpty() && src.isEmpty()) {
        qWarning() << "Maybe circle depends occured";
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
        qWarning() << "Maybe circle depends occured, header circle";
        return false;
    }

    return doPluginSort(nextGroup, nextSrc, dest);
}

DPF_END_NAMESPACE
