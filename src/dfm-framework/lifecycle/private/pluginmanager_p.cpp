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
#include "listener/private/listener_p.h"
#include "pluginmanager_p.h"
#include "lifecycle/private/pluginmetaobject_p.h"
#include "lifecycle/plugin.h"
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

QStringList PluginManagerPrivate::pluginIIDs() const
{
    return pluginLoadIIDs;
}

void PluginManagerPrivate::addPluginIID(const QString &pluginIID)
{
    if (pluginLoadIIDs.contains(pluginIID))
        return;
    pluginLoadIIDs.push_back(pluginIID);
}

QStringList PluginManagerPrivate::pluginPaths() const
{
    return pluginLoadPaths;
}

void PluginManagerPrivate::setPluginPaths(const QStringList &pluginPaths)
{
    pluginLoadPaths = pluginPaths;
}

QStringList PluginManagerPrivate::servicePaths() const
{
    return serviceLoadPaths;
}

void PluginManagerPrivate::setServicePaths(const QStringList &servicePaths)
{
    serviceLoadPaths = servicePaths;
}

void PluginManagerPrivate::setPluginEnable(const PluginMetaObject &meta, bool enabled)
{
    return setting.setPluginEnable(meta, enabled);
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

    //流程互斥
    if (pluginMetaObj->d->state >= PluginMetaObject::State::Loaded) {
        qCritical() << "State error: " << pluginMetaObj->d->state;
        return false;
    }
    if (pluginMetaObj.isNull()) {
        qCritical() << "Failed load plugin is nullptr";
        return false;
    }

    QMutexLocker lock(PluginManagerPrivate::mutex());

    bool result = pluginMetaObj->d->loader->load();
    pluginMetaObj->d->plugin = QSharedPointer<Plugin>(qobject_cast<Plugin *>(pluginMetaObj->d->loader->instance()));
    if (!pluginMetaObj->d->plugin.isNull()) {
        pluginMetaObj->d->state = PluginMetaObject::State::Loaded;
        qDebug() << "Loaded plugin: " << pluginMetaObj->d->name;
    } else {
        pluginMetaObj->d->error = "Failed get plugin instance is nullptr: "
                + pluginMetaObj->d->loader->errorString();
        qCritical() << pluginMetaObj->d->error;
        result = false;
        qCritical() << "Failed load plugin: " << pluginMetaObj->name();
    }
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

    //流程互斥
    if (pluginMetaObj->d->state >= PluginMetaObject::State::Initialized) {
        qCritical() << "State error: " << pluginMetaObj->d->state;
        return false;
    }

    if (pluginMetaObj.isNull()) {
        qCritical() << "Failed init plugin is nullptr";
        return false;
    }

    auto pluginInterface = pluginMetaObj->plugin();

    if (pluginInterface.isNull()) {
        qCritical() << "Failed init plugin interface is nullptr";
        return false;
    }

    //线程互斥
    QMutexLocker lock(PluginManagerPrivate::mutex());

    pluginMetaObj->d->plugin->initialize();
    qDebug() << "Initialized plugin: " << pluginMetaObj->d->name;
    pluginMetaObj->d->state = PluginMetaObject::State::Initialized;

    dpfCheckTimeEnd();
    return true;
}

/*!
 * \brief 启动一个插件，线程安全，可单独使用
 * \param pluginMetaObj
 */
bool PluginManagerPrivate::startPlugin(PluginMetaObjectPointer &pluginMetaObj)
{
    dpfCheckTimeBegin();

    //流程互斥
    if (pluginMetaObj->d->state >= PluginMetaObject::State::Started) {
        qCritical() << "State error: " << pluginMetaObj->d->state;
        return false;
    }

    if (pluginMetaObj.isNull()) {
        qCritical() << "Failed start plugin is nullptr";
        return false;
    }

    auto pluginInterface = pluginMetaObj->plugin();

    if (pluginInterface.isNull()) {
        qCritical() << "Failed start plugin interface is nullptr";
        return false;
    }

    //线程互斥
    QMutexLocker lock(PluginManagerPrivate::mutex());

    if (pluginMetaObj->d->plugin->start()) {
        pluginMetaObj->d->state = PluginMetaObject::State::Started;
        qDebug() << "Started plugin: " << pluginMetaObj->d->name;
    } else {
        pluginMetaObj->d->error = "Failed start plugin in function start() logic";
        qCritical() << pluginMetaObj->d->error.toLocal8Bit().data();
    }

    dpfCheckTimeEnd();
    return true;
}

/*!
 * \brief 停止并卸载一个插件，线程安全，可单独使用
 * \param pluginMetaObj
 */
void PluginManagerPrivate::stopPlugin(PluginMetaObjectPointer &pluginMetaObj)
{
    dpfCheckTimeBegin();

    // 流程互斥
    if (pluginMetaObj->d->state >= PluginMetaObject::State::Stoped)
        return;

    // 线程互斥
    QMutexLocker lock(PluginManagerPrivate::mutex());

    Plugin::ShutdownFlag stFlag = pluginMetaObj->d->plugin->stop();
    pluginMetaObj->d->state = PluginMetaObject::State::Stoped;

    if (stFlag == Plugin::ShutdownFlag::kAsync) {

        qDebug() << "async stop" << pluginMetaObj->d->plugin->metaObject()->className();

        pluginMetaObj->d->state = PluginMetaObject::State::Stoped;

        QObject::connect(pluginMetaObj->d->plugin.data(), &Plugin::asyncStopFinished,
                         pluginMetaObj->d->plugin.data(), [=]() {
                             pluginMetaObj->d->plugin = nullptr;

                             if (!pluginMetaObj->d->loader->unload()) {
                                 qDebug() << pluginMetaObj->d->loader->errorString();
                             }

                             pluginMetaObj->d->state = PluginMetaObject::State::Shutdown;
                             qDebug() << "shutdown" << pluginMetaObj->d->loader->fileName();
                         },
                         Qt::ConnectionType::DirectConnection);   //同步信号直接调用无需等待

    } else {
        if (pluginMetaObj->d->plugin) {
            qDebug() << "sync stop" << pluginMetaObj->d->plugin->metaObject()->className();

            pluginMetaObj->d->plugin = nullptr;
            pluginMetaObj->d->state = PluginMetaObject::State::Stoped;
        }

        if (!pluginMetaObj->d->loader->unload()) {
            qDebug() << pluginMetaObj->d->loader->errorString();
        }

        pluginMetaObj->d->state = PluginMetaObject::State::Shutdown;
        qDebug() << "shutdown" << pluginMetaObj->d->loader->fileName();
    }

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

    qDebug() << readQueue;

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
    dpfCheckTimeBegin();

    if (pluginIIDs.isEmpty())
        return;

    for (const QString &path : pluginPaths) {
        QDirIterator dirItera(path, { "*.so", "*.dll" },
                              QDir::Filter::Files,
                              QDirIterator::IteratorFlag::NoIteratorFlags);

        //线程安全
        QMutexLocker lock(PluginManagerPrivate::mutex());

        while (dirItera.hasNext()) {
            dirItera.next();

            PluginMetaObjectPointer metaObj(new PluginMetaObject);
            QString fileName { dirItera.path() + "/" + dirItera.fileName() };
            metaObj->d->loader->setFileName(fileName);
            QJsonObject &&metaJson = metaObj->d->loader->metaData();
            QJsonObject &&dataJson = metaJson.value("MetaData").toObject();
            QString &&IID = metaJson.value("IID").toString();
            QString &&name = dataJson.value("Name").toString();
            if (!pluginIIDs.contains(IID))
                continue;
            if (blackList.contains(name)) {
                qInfo() << "Black plugin: " << name << "don't load!";
                continue;
            }

            destQueue->append(metaObj);
            metaObj->d->state = PluginMetaObject::Readed;
        }
    }

    dpfCheckTimeEnd();
}

/*!
 * \brief 同步json到定义类型
 * \param metaObject
 */
void PluginManagerPrivate::readJsonToMeta(const PluginMetaObjectPointer &metaObject)
{
    dpfCheckTimeBegin();

    metaObject->d->state = PluginMetaObject::Reading;

    QJsonObject &&jsonObj = metaObject->d->loader->metaData();

    if (jsonObj.isEmpty()) return;

    QString &&iid = jsonObj.value("IID").toString();
    if (iid.isEmpty()) return;
    metaObject->d->iid = iid;

    QJsonObject &&metaData = jsonObj.value("MetaData").toObject();

    QString &&name = metaData.value(kPluginName).toString();
    if (name.isEmpty()) return;
    metaObject->d->name = name;

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

    metaObject->d->state = PluginMetaObject::Readed;

    dpfCheckTimeEnd();
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
    QFuture<void> mapController = QtConcurrent::map(readQueue.begin(),
                                                    readQueue.end(),
                                                    [&ret, this](PluginMetaObjectPointer &pointer) {
                                                        ret = PluginManagerPrivate::doLoadPlugin(pointer);
                                                    });
    mapController.waitForFinished();

    dpfCheckTimeEnd();
    return ret;
}

/*!
 * \brief 初始化所有插件
 */
void PluginManagerPrivate::initPlugins()
{
    dpfCheckTimeBegin();

    QQueue<PluginMetaObjectPointer> initQueue = loadQueue;
    QQueue<PluginMetaObjectPointer>::iterator itera = initQueue.begin();
    while (itera != initQueue.end()) {
        auto pointer = *itera;
        //流程互斥
        if (pointer->d->state >= PluginMetaObject::State::Initialized) {
            qDebug() << "Is initialized plugin: "
                     << pointer->d->name
                     << pointer->fileName();
            ++itera;
            continue;
        }

        if (pointer->d->state != PluginMetaObject::State::Loaded) {
            qCritical() << "Failed initialized plugin"
                        << pointer->d->name
                        << pointer->fileName();
            ++itera;
            continue;
        }

        if (!pointer->d->plugin.isNull()) {
            pointer->d->plugin->initialize();
            qDebug() << "Initialized plugin: " << pointer->d->name;
            pointer->d->state = PluginMetaObject::State::Initialized;
        }
        ++itera;
    }

    // 私有类转发进行Sendler闭包
    emit Listener::instance()->d->pluginsInitialized();

    dpfCheckTimeEnd();
}

/*!
 * \brief 拉起插件,仅主线程使用
 */
void PluginManagerPrivate::startPlugins()
{
    dpfCheckTimeBegin();

    QQueue<PluginMetaObjectPointer> startQueue = loadQueue;
    QQueue<PluginMetaObjectPointer>::iterator itera = startQueue.begin();
    while (itera != startQueue.end()) {
        PluginMetaObjectPointer pointer = *itera;
        //流程互斥
        if (pointer->d->state >= PluginMetaObject::State::Started) {
            qDebug() << "Is started plugin:"
                     << pointer->d->name
                     << pointer->fileName();
            ++itera;
            continue;
        }

        if (pointer->d->state != PluginMetaObject::State::Initialized) {
            qCritical() << "Failed start plugin:"
                        << pointer->d->name
                        << pointer->fileName();
            ++itera;
            continue;
        }

        if (pointer->d->plugin->start()) {
            qDebug() << "Started plugin: " << pointer->d->name;
            pointer->d->state = PluginMetaObject::State::Started;
        } else {
            pointer->d->error = "Failed start plugin in function start() logic";
            qCritical() << pointer->d->error.toLocal8Bit().data();
        }

        ++itera;
    }

    // 私有类转发进行Sendler闭包
    emit Listener::instance()->d->pluginsStarted();

    dpfCheckTimeEnd();
}

/*!
 * \brief 停止插件,仅主线程
 */
void PluginManagerPrivate::stopPlugins()
{
    dpfCheckTimeBegin();

    QQueue<PluginMetaObjectPointer> stopQueue = loadQueue;
    for (auto itera = stopQueue.rbegin(); itera != stopQueue.rend(); ++itera) {
        PluginMetaObjectPointer pointer = *itera;
        if (pointer->d->state != PluginMetaObject::State::Started) {
            continue;
        }

        Plugin::ShutdownFlag stFlag = pointer->d->plugin->stop();
        pointer->d->state = PluginMetaObject::State::Stoped;

        if (stFlag == Plugin::ShutdownFlag::kAsync) {

            qDebug() << "Stoped async plugin: " << pointer->d->name;

            pointer->d->state = PluginMetaObject::State::Stoped;

            QObject::connect(pointer->d->plugin.data(), &Plugin::asyncStopFinished,
                             pointer->d->plugin.data(), [&pointer, this]() {
                                 this->doUnloadPlugin(pointer);
                             },
                             Qt::ConnectionType::DirectConnection);   //同步信号直接调用无需等待

        } else {
            if (pointer->d->plugin) {
                qDebug() << "Stoped sync plugin: " << pointer->d->name;
                pointer->d->plugin = nullptr;
                pointer->d->state = PluginMetaObject::State::Stoped;
            }

            if (!pointer->d->loader->unload()) {
                qDebug() << pointer->d->loader->errorString();
                continue;
            }

            pointer->d->state = PluginMetaObject::State::Shutdown;
            qDebug() << "shutdown" << pointer->d->loader->fileName();
        }
    }

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
                                       QQueue<PluginMetaObjectPointer> *srcQueue)
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

bool PluginManagerPrivate::doLoadPlugin(PluginMetaObjectPointer &pointer)
{   // 流程互斥
    if (pointer->d->state >= PluginMetaObject::State::Loaded) {
        qDebug() << "Is Loaded plugin: "
                 << pointer->d->name
                 << pointer->fileName();
        return true;
    }

    // 必须执行了读取操作
    if (pointer->d->state != PluginMetaObject::State::Readed) {
        qCritical() << "Failed load plugin: "
                    << pointer->d->name
                    << pointer->fileName();
        return false;
    }

    //设置当前正在加载的标志位
    pointer->d->state = PluginMetaObject::State::Loading;
    if (!pointer->d->loader->load()) {
        pointer->d->error = "Failed load plugin: " + pointer->d->loader->errorString();
        qCritical() << pointer->errorString();
        return false;
    }

    pointer->d->plugin = QSharedPointer<Plugin>(qobject_cast<Plugin *>(pointer->d->loader->instance()));

    if (pointer.isNull()) {
        pointer->d->error = "Failed get plugin instance is nullptr";
        qCritical() << pointer->d->error;
        return false;
    }

    pointer->d->state = PluginMetaObject::State::Loaded;
    qInfo() << "Loaded plugin: " << pointer->d->name;

    return true;
}

void PluginManagerPrivate::doUnloadPlugin(PluginMetaObjectPointer &pointer)
{
    pointer->d->plugin = nullptr;

    if (!pointer->d->loader->unload()) {
        qDebug() << pointer->d->loader->errorString();
    }

    pointer->d->state = PluginMetaObject::State::
            Shutdown;
    qDebug() << "shutdown" << pointer->d->loader->fileName();
}

bool PluginManagerPrivate::doPluginSort(const PluginDependGroup &group, QMap<QString, PluginMetaObjectPointer> src, QQueue<PluginMetaObjectPointer> *dest)
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
