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
#include "listener/listener.h"
#include "pluginmanager_p.h"
#include "lifecycle/private/pluginmetaobject_p.h"
#include "lifecycle/plugin.h"
#include "lifecycle/plugincreator.h"
#include "log/codetimecheck.h"

DPF_BEGIN_NAMESPACE

namespace GlobalPrivate {
static QMutex kMutex;
}   // namespace GlobalPrivate

PluginManagerPrivate::PluginManagerPrivate(PluginManager *qq)
    : q(qq)
{
    dpfCheckTimeBegin();
    dpfCheckTimeEnd();
}

PluginManagerPrivate::~PluginManagerPrivate()
{
}

/*!
 * \brief 获取插件的元数据，线程安全
 * \param name
 * \param version
 * \return
 */
PluginMetaObjectPointer PluginManagerPrivate::pluginMetaObj(const QString &name,
                                                            const QString &version)
{
    dpfCheckTimeBegin();
    QMutexLocker lock(PluginManagerPrivate::mutex());
    int size = readQueue.size();
    int idx = 0;
    while (idx < size) {
        if (!version.isEmpty()) {
            if (readQueue[idx]->d->version == version
                && readQueue[idx]->d->name == name) {
                return readQueue[idx];
            }
        } else {
            if (readQueue[idx]->d->name == name) {
                return readQueue[idx];
            }
        }
        idx++;
    }
    dpfCheckTimeBegin();

    return PluginMetaObjectPointer(nullptr);
}

/*!
 * \brief 加载一个插件，线程安全，可单独使用
 * \param pluginMetaObj
 */
bool PluginManagerPrivate::loadPlugin(PluginMetaObjectPointer &pluginMetaObj)
{
    dpfCheckTimeBegin();

    QMutexLocker lock(PluginManagerPrivate::mutex());
    bool result = doLoadPlugin(pluginMetaObj);

    dpfCheckTimeEnd();
    return result;
}

/*!
 * \brief 初始化一个插件，线程安全，可单独使用
 * \param pluginMetaObj
 */
bool PluginManagerPrivate::initPlugin(PluginMetaObjectPointer &pluginMetaObj)
{
    dpfCheckTimeBegin();

    QMutexLocker lock(PluginManagerPrivate::mutex());
    bool result = doInitPlugin(pluginMetaObj);

    dpfCheckTimeEnd();
    return result;
}

/*!
 * \brief 启动一个插件，线程安全，可单独使用
 * \param pluginMetaObj
 */
bool PluginManagerPrivate::startPlugin(PluginMetaObjectPointer &pluginMetaObj)
{
    dpfCheckTimeBegin();

    //线程互斥
    QMutexLocker lock(PluginManagerPrivate::mutex());
    bool result = doStartPlugin(pluginMetaObj);

    dpfCheckTimeEnd();
    return result;
}

/*!
 * \brief 停止并卸载一个插件，线程安全，可单独使用
 * \param pluginMetaObj
 */
void PluginManagerPrivate::stopPlugin(PluginMetaObjectPointer &pluginMetaObj)
{
    dpfCheckTimeBegin();

    // 线程互斥
    QMutexLocker lock(PluginManagerPrivate::mutex());
    doStopPlugin(pluginMetaObj);

    dpfCheckTimeEnd();
}

/*!
 * \brief 读取所有插件的Json源数据
 * \return
 */
bool PluginManagerPrivate::readPlugins()
{
    dpfCheckTimeBegin();

    int currMaxCountThread = QThreadPool::globalInstance()->maxThreadCount();
    if (currMaxCountThread < 4) {
        QThreadPool::globalInstance()->setMaxThreadCount(4);
    }

    // 内部已有线程互斥
    QFuture<void> scanController = QtConcurrent::run(scanfAllPlugin,
                                                     &readQueue,
                                                     pluginLoadPaths,
                                                     pluginLoadIIDs,
                                                     blackPlguinNames);
    scanController.waitForFinished();

    QMutexLocker lock(PluginManagerPrivate::mutex());

    QFuture<void> mapController = QtConcurrent::map(readQueue.begin(),
                                                    readQueue.end(),
                                                    readJsonToMeta);
    mapController.waitForFinished();

#ifdef QT_DEBUG
    for (auto read : readQueue)
        qDebug() << read;
#endif

    dpfCheckTimeEnd();
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
    dpfCheckTimeBegin();

    if (pluginIIDs.isEmpty())
        return;

    for (const QString &path : pluginPaths) {
        QDirIterator dirItera(path, { "*.so" },
                              QDir::Filter::Files,
                              QDirIterator::IteratorFlag::NoIteratorFlags);

        QMutexLocker lock(PluginManagerPrivate::mutex());

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

    dpfCheckTimeEnd();
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
    dpfCheckTimeBegin();

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

    dpfCheckTimeEnd();
}

void PluginManagerPrivate::jsonToMeta(PluginMetaObjectPointer metaObject, const QJsonObject &metaData)
{
    QString &&version = metaData.value(kPluginVersion).toString();
    metaObject->d->version = version;

    QString &&compatVersion = metaData.value(kPluginCompatversion).toString();
    metaObject->d->compatVersion = compatVersion;

    QString &&category = metaData.value(kPluginCategory).toString();
    metaObject->d->category = category;

    QJsonArray &&licenseArray = metaData.value(kPluginLicense).toArray();
    auto licenItera = licenseArray.begin();
    while (licenItera != licenseArray.end()) {
        metaObject->d->license.append(licenItera->toString());
        ++licenItera;
    }

    QString &&copyright = metaData.value(kPluginCopyright).toString();
    metaObject->d->copyright = copyright;

    QString &&vendor = metaData.value(kPluginVendor).toString();
    metaObject->d->vendor = vendor;

    QString &&description = metaData.value(kPluginDescription).toString();
    metaObject->d->description = description;

    QString &&urlLink = metaData.value(kPluginUrlLink).toString();
    metaObject->d->urlLink = urlLink;

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
    dpfCheckTimeBegin();

    QFuture<void> sortController = QtConcurrent::run(dependsSort,
                                                     &loadQueue,
                                                     &readQueue);
    sortController.waitForFinished();

    bool ret = true;
    std::for_each(loadQueue.begin(), loadQueue.end(), [&ret, this](PluginMetaObjectPointer pointer) {
        if (!PluginManagerPrivate::doLoadPlugin(pointer))
            ret = false;
    });

    dpfCheckTimeEnd();
    return ret;
}

/*!
 * \brief 初始化所有插件
 */
bool PluginManagerPrivate::initPlugins()
{
    dpfCheckTimeBegin();

    bool ret = true;
    std::for_each(loadQueue.begin(), loadQueue.end(), [&ret, this](PluginMetaObjectPointer pointer) {
        if (!PluginManagerPrivate::doInitPlugin(pointer))
            ret = false;
    });

    emit Listener::instance()->pluginsInitialized();
    dpfCheckTimeEnd();

    return ret;
}

/*!
 * \brief 拉起插件,仅主线程使用
 */
bool PluginManagerPrivate::startPlugins()
{
    dpfCheckTimeBegin();

    bool ret = true;
    std::for_each(loadQueue.begin(), loadQueue.end(), [&ret, this](PluginMetaObjectPointer pointer) {
        if (!PluginManagerPrivate::doStartPlugin(pointer))
            ret = false;
    });

    emit Listener::instance()->pluginsStarted();
    dpfCheckTimeEnd();

    return ret;
}

/*!
 * \brief 停止插件,仅主线程
 */
void PluginManagerPrivate::stopPlugins()
{
    dpfCheckTimeBegin();
    // reverse queue
    std::for_each(loadQueue.rbegin(), loadQueue.rend(), [this](PluginMetaObjectPointer pointer) {
        PluginManagerPrivate::doStopPlugin(pointer);
    });

    dpfCheckTimeEnd();
}

QMutex *PluginManagerPrivate::mutex()
{
    static QMutex m;
    return &m;
}

/*!
 * \brief 按照依赖排序
 * \param dstQueue
 * \param srcQueue
 */
void PluginManagerPrivate::dependsSort(QQueue<PluginMetaObjectPointer> *dstQueue,
                                       const QQueue<PluginMetaObjectPointer> *srcQueue)
{
    dpfCheckTimeBegin();
    Q_ASSERT(dstQueue);
    Q_ASSERT(srcQueue);

    QMutexLocker lock(PluginManagerPrivate::mutex());

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
                qInfo() << name << "->" << ptr->name();
                dependGroup.append({ srcMap.value(name), ptr });
            } else {
                qWarning("Plugin `%s` cannot depend a unkonw plugin: `%s`", qUtf8Printable(ptr->name()), qUtf8Printable(name));
            }
        }
    });

    // sort
    dstQueue->clear();
    if (!doPluginSort(dependGroup, srcMap, dstQueue)) {
        qCritical() << "Sort depnd group failed";
        *dstQueue = *srcQueue;
        dpfCheckTimeEnd();
        return;
    }

    dpfCheckTimeEnd();
}

bool PluginManagerPrivate::doLoadPlugin(PluginMetaObjectPointer pointer)
{
    Q_ASSERT(pointer);

    // 流程互斥
    if (pointer->d->state >= PluginMetaObject::State::kLoaded) {
        qDebug() << "Is Loaded plugin: "
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
        qDebug() << "Is initialized plugin: "
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
        qDebug() << "Is started plugin:"
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
    } else {
        pointer->d->error = "Failed start plugin in function start() logic";
        qCritical() << pointer->d->error.toLocal8Bit().data();
    }

    return true;
}

void PluginManagerPrivate::doStopPlugin(PluginMetaObjectPointer pointer)
{
    Q_ASSERT(pointer);

    if (pointer->d->state >= PluginMetaObject::State::kStoped) {
        qDebug() << "Is stoped plugin:"
                 << pointer->d->name
                 << pointer->fileName();
        return;
    }

    if (pointer->d->state != PluginMetaObject::State::kStarted) {
        qCritical() << "Failed stop plugin:"
                    << pointer->d->name
                    << pointer->fileName();
        return;
    }

    if (pointer->d->plugin.isNull()) {
        pointer->d->error = "Failed stop plugin, plugin instance is nullptr";
        qCritical() << pointer->d->name << pointer->d->error;
        return;
    }

    pointer->d->plugin->stop();
    pointer->d->plugin = nullptr;
    pointer->d->state = PluginMetaObject::State::kStoped;
    qInfo() << "stop" << pointer->d->loader->fileName();

    if (!pointer->d->loader->unload()) {
        qDebug() << pointer->d->loader->errorString();
        return;
    }

    pointer->d->state = PluginMetaObject::State::kShutdown;
    qDebug() << "shutdown" << pointer->d->loader->fileName();
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
