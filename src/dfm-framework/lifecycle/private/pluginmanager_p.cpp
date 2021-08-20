#include "pluginmanager_p.h"

#include "lifecycle/plugincontext.h"
#include "lifecycle/plugin.h"

#include "log/frameworklog.h"

DPF_USE_NAMESPACE

PluginManagerPrivate::PluginManagerPrivate(PluginManager *qq)
    : q_ptr(qq)
{
    dpfCheckTimeBegin();
    dpfCheckTimeEnd();
}

QString PluginManagerPrivate::pluginIID() const
{
    return m_pluginIID;
}

void PluginManagerPrivate::setPluginIID(const QString &pluginIID)
{
    m_pluginIID = pluginIID;
}

QStringList PluginManagerPrivate::pluginPaths() const
{
    return m_pluginPaths;
}

void PluginManagerPrivate::setPluginPaths(const QStringList &pluginPaths)
{
    m_pluginPaths = pluginPaths;
}

QStringList PluginManagerPrivate::servicePaths() const
{
    return m_servicePaths;
}

void PluginManagerPrivate::setServicePaths(const QStringList &servicePaths)
{
    m_servicePaths = servicePaths;
}

void PluginManagerPrivate::setPluginEnable(const PluginMetaObject &meta, bool enabled)
{
    return m_setting.setPluginEnable(meta,enabled);
}

PluginMetaObjectPointer PluginManagerPrivate::pluginMetaObj(const QString &name,
                                                            const QString &version)
{
    auto controller = QtConcurrent::run([=](){
        static QMutex mutex;
        mutex.lock();
        int size = m_readQueue.size();
        int idx = 0;
        while (idx < size) {
            if (!version.isEmpty()) {
                if (m_readQueue[idx]->m_version == version
                        && m_readQueue[idx]->m_name == name)
                {
                    mutex.unlock();
                    return m_readQueue[idx];
                }
            } else {
                if (m_readQueue[idx]->m_name == name)
                {
                    mutex.unlock();
                    return m_readQueue[idx];
                }
            }
            idx++;
        }
        mutex.unlock();
        return PluginMetaObjectPointer(nullptr);
    });
    controller.waitForFinished();

    return controller.result();
}

void PluginManagerPrivate::loadPlugin(PluginMetaObjectPointer &pluginMetaObj)
{
    //流程互斥
    if (pluginMetaObj->m_state >= PluginMetaObject::State::Loaded)
        return;

    if (pluginMetaObj.isNull()) {
        dpfCritical() << "Failed load plugin is nullptr";
        return;
    }

    auto controller = QtConcurrent::run([=](){

        static QMutex mutex;
        mutex.lock();

        bool result = pluginMetaObj->m_loader->load();
        pluginMetaObj->m_plugin = QSharedPointer<Plugin>(qobject_cast<Plugin*>(pluginMetaObj->m_loader->instance()));
        if (!pluginMetaObj->m_plugin.isNull()) {
            pluginMetaObj->m_state = PluginMetaObject::State::Loaded;
            dpfCritical() << "Loaded plugin: " << pluginMetaObj->m_name;
        }
        else
            dpfCritical() << "Failed get plugin instance is nullptr";

        mutex.unlock();

        return result;
    });
    controller.waitForFinished();

    if (!controller.result()) {
        dpfCritical() << "Failed load plugin: " << pluginMetaObj->name();
    }
}

void PluginManagerPrivate::initPlugin(PluginMetaObjectPointer &pluginMetaObj)
{
    //流程互斥
    if (pluginMetaObj->m_state >= PluginMetaObject::State::Initialized)
        return;


    if (pluginMetaObj.isNull()) {
        dpfCritical() << "Failed init plugin is nullptr";
    }

    auto pluginInterface = pluginMetaObj->plugin();

    if (pluginInterface.isNull()) {
        dpfCritical() << "Failed init plugin interface is nullptr";
        return;
    }

    //线程互斥
    static QMutex mutex;
    mutex.lock();

    pluginMetaObj->m_context = QSharedPointer<PluginContext>(new PluginContext());
    //    auto controller = QtConcurrent::run([=](){
    pluginMetaObj->m_plugin->initialize();
    dpfCritical() << "Initialized plugin: " << pluginMetaObj->m_name;
    //    });
    //    controller.waitForFinished();

    pluginMetaObj->m_state = PluginMetaObject::State::Initialized;

    mutex.unlock();
}

void PluginManagerPrivate::startPlugin(PluginMetaObjectPointer &pluginMetaObj)
{
    //流程互斥
    if (pluginMetaObj->m_state >= PluginMetaObject::State::Started)
        return;

    if (pluginMetaObj.isNull()) {
        dpfCritical() << "Failed start plugin is nullptr";
    }

    auto pluginInterface = pluginMetaObj->plugin();

    if (pluginInterface.isNull()) {
        dpfCritical() << "Failed start plugin interface is nullptr";
        return;
    }

    //线程互斥
    static QMutex mutex;
    mutex.lock();

    pluginMetaObj->m_context = QSharedPointer<PluginContext>(new PluginContext());
    pluginMetaObj->m_plugin->start(pluginMetaObj->m_context);

    dpfCritical() << "Started plugin: " << pluginMetaObj->m_name;

    pluginMetaObj->m_state = PluginMetaObject::State::Started;

    mutex.unlock();
}

void PluginManagerPrivate::stopPlugin(PluginMetaObjectPointer &pluginMetaObj)
{
    //流程互斥
    if (pluginMetaObj->m_state >= PluginMetaObject::State::Stoped)
        return;

    //线程互斥
    static QMutex mutex;
    mutex.lock();

    Plugin::ShutdownFlag stFlag = pluginMetaObj->m_plugin->stop();
    pluginMetaObj->m_state = PluginMetaObject::State::Stoped;

    if (stFlag == Plugin::ShutdownFlag::Asynch) {

        dpfCritical() << "asynch stop" << pluginMetaObj->m_plugin->
                         metaObject()->className();

        pluginMetaObj->m_state = PluginMetaObject::State::Stoped;

        QObject::connect(pluginMetaObj->m_plugin.data(), &Plugin::asyncStopFinished,
                         pluginMetaObj->m_plugin.data(), [=]()
        {
            pluginMetaObj->m_plugin = nullptr;

            if (!pluginMetaObj->m_loader->unload()) {
                dpfCritical() << pluginMetaObj->m_loader->errorString();
            }

            pluginMetaObj->m_state = PluginMetaObject::State::Shutdown;
            dpfCritical() << "shutdown" << pluginMetaObj->m_loader->fileName();

        },Qt::ConnectionType::DirectConnection); //同步信号直接调用无需等待

    } else {

        if (pluginMetaObj->m_plugin) {
            dpfCritical() << "synch stop" << pluginMetaObj->m_plugin->
                             metaObject()->className();

            pluginMetaObj->m_plugin = nullptr;
            pluginMetaObj->m_state = PluginMetaObject::State::Stoped;
        }

        if (!pluginMetaObj->m_loader->unload()) {
            dpfCritical() << pluginMetaObj->m_loader->errorString();
        }

        pluginMetaObj->m_state = PluginMetaObject::State::Shutdown;
        dpfCritical() << "shutdown" << pluginMetaObj->m_loader->fileName();
    }

    mutex.unlock();
}

void PluginManagerPrivate::readPlugins()
{
    dpfCheckTimeBegin();

    int currMaxCountThread = QThreadPool::globalInstance()->maxThreadCount();
    if (currMaxCountThread < 4) {
        QThreadPool::globalInstance()->setMaxThreadCount(4);
    }

    QFuture<void> scanController = QtConcurrent::run(scanfAllPlugin,
                                                     &m_readQueue,
                                                     m_pluginPaths,
                                                     m_pluginIID);
    scanController.waitForFinished();

    QFuture<void> mapController = QtConcurrent::map(m_readQueue.begin(),
                                                    m_readQueue.end(),
                                                    readJsonToMeta);
    mapController.waitForFinished();
    dpfDebug() << m_readQueue;

    dpfCheckTimeEnd();
}

void PluginManagerPrivate::scanfAllPlugin(QQueue<PluginMetaObjectPointer> *destQueue,
                                          const QStringList &pluginPaths,
                                          const QString &pluginIID)
{
    if (pluginIID.isEmpty())
        return;

    for(QString path : pluginPaths)
    {
        QDirIterator dirItera(path, {"*.so", "*.dll"},
                              QDir::Filter::Files,
                              QDirIterator::IteratorFlag::NoIteratorFlags);

        //线程安全
        static QMutex mutex;
        mutex.lock();
        while (dirItera.hasNext())
        {
            dirItera.next();

            PluginMetaObjectPointer metaObj(new PluginMetaObject);
            metaObj->m_loader->setFileName(dirItera.path() + "/" + dirItera.fileName());
            QJsonObject metaJson = metaObj->m_loader->metaData();
            QString IID = metaJson.value("IID").toString();
            if (pluginIID != IID) {
                continue;
            }

            destQueue->append(metaObj);
            metaObj->m_state = PluginMetaObject::Readed;
        }
        mutex.unlock();
    }
}

void PluginManagerPrivate::readJsonToMeta(const PluginMetaObjectPointer &metaObject)
{
    metaObject->m_state = PluginMetaObject::Reading;

    QJsonObject jsonObj = metaObject->m_loader->metaData();

    if (jsonObj.isEmpty()) return;

    QString iid = jsonObj.value("IID").toString();
    if (iid.isEmpty()) return;
    metaObject->m_iid = iid;

    QJsonObject metaData = jsonObj.value("MetaData").toObject();

    QString name = metaData.value(PLUGIN_NAME).toString();
    if (name.isEmpty()) return;
    metaObject->m_name = name;

    QString version = metaData.value(PLUGIN_VERSION).toString();
    metaObject->m_version = version;

    QString compatVersion = metaData.value(PLUGIN_COMPATVERSION).toString();
    metaObject->m_compatVersion = compatVersion;

    QString category = metaData.value(PLUGIN_CATEGORY).toString();
    metaObject->m_category = category;

    QJsonArray licenseArray = metaData.value(PLUGIN_LICENSE).toArray();
    auto licenItera = licenseArray.begin();
    while (licenItera != licenseArray.end()) {
        metaObject->m_license.append(licenItera->toString());
        licenItera ++;
    }

    QString copyright = metaData.value(PLUGIN_COPYRIGHT).toString();
    metaObject->m_copyright = copyright;

    QString vendor = metaData.value(PLUGIN_VENDOR).toString();
    metaObject->m_vendor = vendor;

    QString description = metaData.value(PLUGIN_DESCRIPTION).toString();
    metaObject->m_description = description;

    QString urlLink = metaData.value(PLUGIN_URLLINK).toString();
    metaObject->m_urlLink = urlLink;

    QJsonArray dependsArray = metaData.value(PLUGIN_DEPENDS).toArray();
    auto itera = dependsArray.begin();
    while (itera != dependsArray.end())
    {
        QJsonObject dependObj = itera->toObject();
        QString dependName = dependObj.value(PLUGIN_NAME).toString();
        QString dependVersion = dependObj.value(PLUGIN_VERSION).toString();
        PluginDepend depends;
        depends.m_name = dependName;
        depends.m_version = dependVersion;
        metaObject->m_depends.append(depends);
        itera ++;
    }

    metaObject->m_state = PluginMetaObject::Readed;
}

void PluginManagerPrivate::loadPlugins()
{
    dpfCheckTimeBegin();

    QFuture<void> sortController = QtConcurrent::run(dependsSort,
                                                     &m_loadQueue,
                                                     &m_readQueue);
    sortController.waitForFinished();

    QFuture<void> mapController = QtConcurrent::map(m_readQueue.begin(),
                                                    m_readQueue.end(),
                                                    [=](PluginMetaObjectPointer &pointer){
        //流程互斥
        if (pointer->m_state >= PluginMetaObject::State::Loaded) {
            dpfCritical() << "Is Loaded plugin: "
                          << pointer->m_name
                          << pointer->fileName();
            return;
        }

        if (pointer->m_state != PluginMetaObject::State::Readed) {
            dpfCritical() << "Failed load plugin: "
                          << pointer->m_name
                          << pointer->fileName();
            return;
        }

        pointer->m_state = PluginMetaObject::State::Loading;

        if (!pointer->m_loader->load()) {
            dpfCritical() << "Failed load plugin: "
                          << pointer->m_name
                          << pointer->fileName()
                          << pointer->loaderErrorString();
            return;
        }

        pointer->m_plugin = QSharedPointer<Plugin>
                (qobject_cast<Plugin*>(pointer->m_loader->instance()));

        if (!pointer.isNull()) {
            pointer->m_state = PluginMetaObject::State::Loaded;
            dpfCritical() << "Loaded plugin: " << pointer->m_name;
        } else {
            dpfCritical() << "Failed get plugin instance is nullptr";
        }
    });
    mapController.waitForFinished();

    dpfDebug() << m_loadQueue;

    dpfCheckTimeEnd();
}

void PluginManagerPrivate::initPlugins()
{
    dpfCheckTimeBegin();

    QFuture<void> runController = QtConcurrent::run([=](){
        QQueue<PluginMetaObjectPointer> initQueue = m_loadQueue;
        QQueue<PluginMetaObjectPointer>::iterator itera = initQueue.begin();
        while(itera != initQueue.end())
        {
            auto pointer = *itera;
            //流程互斥
            if (pointer->m_state >= PluginMetaObject::State::Initialized) {
                dpfCritical() << "Is initialized plugin: "
                              << pointer->m_name
                              << pointer->fileName();
                itera ++;
                continue;
            }

            if (pointer->m_state != PluginMetaObject::State::Loaded) {
                dpfCritical() << "Failed initialized plugin"
                              << pointer->m_name
                              << pointer->fileName();
                itera ++;
                continue;
            }

            if (!pointer->m_plugin.isNull()) {
                pointer->m_plugin->initialize();
                dpfCritical() << "Initialized plugin: " << pointer->m_name;
                pointer->m_state = PluginMetaObject::State::Initialized;
            }
            itera ++;
        }
    });

    runController.waitForFinished();

    dpfCheckTimeEnd();
}

void PluginManagerPrivate::startPlugins()
{
    dpfCheckTimeBegin();

    QQueue<PluginMetaObjectPointer> startQueue = m_loadQueue;
    QQueue<PluginMetaObjectPointer>::iterator itera = startQueue.begin();
    while(itera != startQueue.end())
    {
        PluginMetaObjectPointer pointer = *itera;
        //流程互斥
        if (pointer->m_state >= PluginMetaObject::State::Started) {
            dpfCritical() << "Is started plugin:"
                          << pointer->m_name
                          << pointer->fileName();
            itera ++;
            continue;
        }

        if (pointer->m_state != PluginMetaObject::State::Initialized) {
            dpfCritical() << "Failed start plugin:"
                          << pointer->m_name
                          << pointer->fileName();
            itera ++;
            continue;
        }

        pointer->m_context = QSharedPointer<PluginContext>(new PluginContext());
        pointer->m_plugin->start(pointer->m_context);
        dpfCritical() << "Started plugin: " << pointer->m_name;
        pointer->m_state = PluginMetaObject::State::Started;

        itera ++;
    }

    dpfCheckTimeEnd();
}

void PluginManagerPrivate::stopPlugins()
{
    dpfCheckTimeBegin();

    QQueue<PluginMetaObjectPointer> stopQueue = m_loadQueue;
    auto itera = stopQueue.rbegin();
    while(itera != stopQueue.rend())
    {
        PluginMetaObjectPointer pointer = *itera;
        if (pointer->m_state != PluginMetaObject::State::Started) {
            continue;
        }

        Plugin::ShutdownFlag stFlag = pointer->m_plugin->stop();
        pointer->m_state = PluginMetaObject::State::Stoped;

        if (stFlag == Plugin::ShutdownFlag::Asynch) {

            dpfCritical() << "Stoped asynch plugin: " << pointer->m_name;

            pointer->m_state = PluginMetaObject::State::Stoped;

            QObject::connect(pointer->m_plugin.data(), &Plugin::asyncStopFinished,
                             pointer->m_plugin.data(), [=]()
            {
                pointer->m_plugin = nullptr;

                if (!pointer->m_loader->unload()) {
                    dpfCritical() << pointer->m_loader->errorString();
                }

                pointer->m_state = PluginMetaObject::State::Shutdown;
                dpfCritical() << "shutdown" << pointer->m_loader->fileName();

            },Qt::ConnectionType::DirectConnection); //同步信号直接调用无需等待

        } else {

            if (pointer->m_plugin) {
                dpfCritical() << "Stoped synch plugin: " << pointer->m_name;
                pointer->m_plugin = nullptr;
                pointer->m_state = PluginMetaObject::State::Stoped;
            }

            if (!pointer->m_loader->unload()) {
                dpfCritical() << pointer->m_loader->errorString();
                continue;
            }

            pointer->m_state = PluginMetaObject::State::Shutdown;
            dpfCritical() << "shutdown" << pointer->m_loader->fileName();
        }

        itera ++;
    }

    dpfCheckTimeEnd();
}

void PluginManagerPrivate::dependsSort(QQueue<PluginMetaObjectPointer> *dstQueue,
                                       QQueue<PluginMetaObjectPointer> *srcQueue)
{
    static QMutex mutex;
    mutex.lock();
    *dstQueue = (*srcQueue);
    typedef PluginMetaObjectPointer EmleTPointer;
    std::sort(dstQueue->begin(), dstQueue->end(), [=](EmleTPointer after, EmleTPointer befor)
    {
        dpfDebug() << after->m_name << befor->m_name;
        if (after->depends().isEmpty()) {
            //前节点没有依赖则保持顺序
            return true;
        } else { //前节点存在依赖
            if(befor->depends().isEmpty()) {
                //后节点为空则调整顺序
                return false;
            } else { //后节点存在依赖
                //遍历后节点依赖
                for (auto depend : befor->m_depends)
                {
                    //后节点依赖存在前节点
                    if (depend.name() == after->name())
                        return true;
                }
                //遍历前节点依赖
                for (auto depend : after->m_depends)
                {
                    //前节点依赖存在后节点
                    if (depend.name() == befor->name())
                        return false;
                }
            }
        }
        dpfCritical() << "Unknown Error from" << Q_FUNC_INFO;
        return false;
    });
    mutex.unlock();
}
