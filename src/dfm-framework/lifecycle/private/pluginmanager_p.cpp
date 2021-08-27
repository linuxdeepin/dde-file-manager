#include "pluginmanager_p.h"

#include "lifecycle/private/pluginmetaobject_p.h"
#include "lifecycle/plugincontext.h"
#include "lifecycle/plugin.h"

#include "log/frameworklog.h"

DPF_BEGIN_NAMESPACE

namespace GlobalPrivate {
    static QMutex mutex;
} //namespace GlobalPrivate

PluginManagerPrivate::PluginManagerPrivate(PluginManager *qq)
    : q_ptr(qq)
{
    dpfCheckTimeBegin();
    dpfCheckTimeEnd();
}

PluginManagerPrivate::~PluginManagerPrivate(){
    stopPlugins();
}

QString PluginManagerPrivate::pluginIID() const
{
    return pluginLoadIID;
}

void PluginManagerPrivate::setPluginIID(const QString &pluginIID)
{
    pluginLoadIID = pluginIID;
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
    return setting.setPluginEnable(meta,enabled);
}

//线程安全
PluginMetaObjectPointer PluginManagerPrivate::pluginMetaObj(const QString &name,
                                                            const QString &version)
{
    dpfCheckTimeBegin();

    auto controller = QtConcurrent::run([=](){
        QMutexLocker lock(&GlobalPrivate::mutex);
        int size = readQueue.size();
        int idx = 0;
        while (idx < size) {
            if (!version.isEmpty()) {
                if (readQueue[idx]->d_ptr->version == version
                        && readQueue[idx]->d_ptr->name == name)
                {
                    return readQueue[idx];
                }
            } else {
                if (readQueue[idx]->d_ptr->name == name)
                {
                    return readQueue[idx];
                }
            }
            idx++;
        }
        return PluginMetaObjectPointer(nullptr);
    });
    controller.waitForFinished();

    dpfCheckTimeBegin();

    return controller.result();
}

//加载一个插件，线程安全，可单独使用。
void PluginManagerPrivate::loadPlugin(PluginMetaObjectPointer &pluginMetaObj)
{
    dpfCheckTimeBegin();

    //流程互斥
    if (pluginMetaObj->d_ptr->state >= PluginMetaObject::State::Loaded)
        return;

    if (pluginMetaObj.isNull()) {
        dpfCritical() << "Failed load plugin is nullptr";
        return;
    }

    auto controller = QtConcurrent::run([=](){

        QMutexLocker lock(&GlobalPrivate::mutex);

        bool result = pluginMetaObj->d_ptr->loader->load();
        pluginMetaObj->d_ptr->plugin = QSharedPointer<Plugin>
                (qobject_cast<Plugin*>(pluginMetaObj->d_ptr->loader->instance()));
        if (!pluginMetaObj->d_ptr->plugin.isNull()) {
            pluginMetaObj->d_ptr->state = PluginMetaObject::State::Loaded;
            dpfCritical() << "Loaded plugin: " << pluginMetaObj->d_ptr->name;
        } else {
            dpfCritical() << "Failed get plugin instance is nullptr";
            pluginMetaObj->d_ptr->error = pluginMetaObj->d_ptr->loader->errorString();
        }
        return result;
    });
    controller.waitForFinished();

    if (!controller.result()) {
        dpfCritical() << "Failed load plugin: " << pluginMetaObj->name();
    }

    dpfCheckTimeEnd();
}

//初始化一个插件，线程安全，可单独使用
void PluginManagerPrivate::initPlugin(PluginMetaObjectPointer &pluginMetaObj)
{
    dpfCheckTimeBegin();

    //流程互斥
    if (pluginMetaObj->d_ptr->state >= PluginMetaObject::State::Initialized)
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
    QMutexLocker lock(&GlobalPrivate::mutex);

    pluginMetaObj->d_ptr->context = QSharedPointer<PluginContext>(new PluginContext());
    //    auto controller = QtConcurrent::run([=](){
    pluginMetaObj->d_ptr->plugin->initialize();
    dpfCritical() << "Initialized plugin: " << pluginMetaObj->d_ptr->name;
    //    });
    //    controller.waitForFinished();

    pluginMetaObj->d_ptr->state = PluginMetaObject::State::Initialized;

    dpfCheckTimeEnd();
}

//启动一个插件，线程安全，可单独使用
void PluginManagerPrivate::startPlugin(PluginMetaObjectPointer &pluginMetaObj)
{
    dpfCheckTimeBegin();

    //流程互斥
    if (pluginMetaObj->d_ptr->state >= PluginMetaObject::State::Started)
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
    QMutexLocker lock(&GlobalPrivate::mutex);

    pluginMetaObj->d_ptr->context = QSharedPointer<PluginContext>(new PluginContext());
    if (pluginMetaObj->d_ptr->plugin->start(pluginMetaObj->d_ptr->context)) {
        pluginMetaObj->d_ptr->state = PluginMetaObject::State::Started;
        dpfCritical() << "Started plugin: " << pluginMetaObj->d_ptr->name;
    } else {
        pluginMetaObj->d_ptr->error = "Failed start plugin in function start() logic";
        dpfCritical() << pluginMetaObj->d_ptr->error.toLocal8Bit().data();
    }

    dpfCheckTimeEnd();
}

//停止并卸载一个插件，线程安全，可单独使用
void PluginManagerPrivate::stopPlugin(PluginMetaObjectPointer &pluginMetaObj)
{
    dpfCheckTimeBegin();

    //流程互斥
    if (pluginMetaObj->d_ptr->state >= PluginMetaObject::State::Stoped)
        return;

    //线程互斥
    QMutexLocker lock(&GlobalPrivate::mutex);

    Plugin::ShutdownFlag stFlag = pluginMetaObj->d_ptr->plugin->stop();
    pluginMetaObj->d_ptr->state = PluginMetaObject::State::Stoped;

    if (stFlag == Plugin::ShutdownFlag::Asynch) {

        dpfCritical() << "asynch stop" << pluginMetaObj->d_ptr->plugin->
                         metaObject()->className();

        pluginMetaObj->d_ptr->state = PluginMetaObject::State::Stoped;

        QObject::connect(pluginMetaObj->d_ptr->plugin.data(), &Plugin::asyncStopFinished,
                         pluginMetaObj->d_ptr->plugin.data(), [=]()
        {
            pluginMetaObj->d_ptr->plugin = nullptr;

            if (!pluginMetaObj->d_ptr->loader->unload()) {
                dpfCritical() << pluginMetaObj->d_ptr->loader->errorString();
            }

            pluginMetaObj->d_ptr->state = PluginMetaObject::State::Shutdown;
            dpfCritical() << "shutdown" << pluginMetaObj->d_ptr->loader->fileName();

        },Qt::ConnectionType::DirectConnection); //同步信号直接调用无需等待

    } else {

        if (pluginMetaObj->d_ptr->plugin) {
            dpfCritical() << "synch stop" << pluginMetaObj->d_ptr->plugin->
                             metaObject()->className();

            pluginMetaObj->d_ptr->plugin = nullptr;
            pluginMetaObj->d_ptr->state = PluginMetaObject::State::Stoped;
        }

        if (!pluginMetaObj->d_ptr->loader->unload()) {
            dpfCritical() << pluginMetaObj->d_ptr->loader->errorString();
        }

        pluginMetaObj->d_ptr->state = PluginMetaObject::State::Shutdown;
        dpfCritical() << "shutdown" << pluginMetaObj->d_ptr->loader->fileName();
    }

    dpfCheckTimeEnd();
}

//读取所有插件的Json源数据
void PluginManagerPrivate::readPlugins()
{
    dpfCheckTimeBegin();

    int currMaxCountThread = QThreadPool::globalInstance()->maxThreadCount();
    if (currMaxCountThread < 4) {
        QThreadPool::globalInstance()->setMaxThreadCount(4);
    }

    //内部已有线程互斥
    QFuture<void> scanController = QtConcurrent::run(scanfAllPlugin,
                                                     &readQueue,
                                                     pluginLoadPaths,
                                                     pluginLoadIID);
    scanController.waitForFinished();

    QMutexLocker lock(&GlobalPrivate::mutex);

    QFuture<void> mapController = QtConcurrent::map(readQueue.begin(),
                                                    readQueue.end(),
                                                    readJsonToMeta);
    mapController.waitForFinished();

    dpfDebug() << readQueue;

    dpfCheckTimeEnd();
}

//扫描所有插件到目标队列
void PluginManagerPrivate::scanfAllPlugin(QQueue<PluginMetaObjectPointer> *destQueue,
                                          const QStringList &pluginPaths,
                                          const QString &pluginIID)
{
    dpfCheckTimeBegin();

    if (pluginIID.isEmpty())
        return;

    for(QString path : pluginPaths)
    {
        QDirIterator dirItera(path, {"*.so", "*.dll"},
                              QDir::Filter::Files,
                              QDirIterator::IteratorFlag::NoIteratorFlags);

        //线程安全
        QMutexLocker lock(&GlobalPrivate::mutex);

        while (dirItera.hasNext())
        {
            dirItera.next();

            PluginMetaObjectPointer metaObj(new PluginMetaObject);
            metaObj->d_ptr->loader->setFileName(dirItera.path() + "/" + dirItera.fileName());
            QJsonObject metaJson = metaObj->d_ptr->loader->metaData();
            QString IID = metaJson.value("IID").toString();
            if (pluginIID != IID) {
                continue;
            }

            destQueue->append(metaObj);
            metaObj->d_ptr->state = PluginMetaObject::Readed;
        }
    }

    dpfCheckTimeEnd();
}

//同步json到定义类型
void PluginManagerPrivate::readJsonToMeta(const PluginMetaObjectPointer &metaObject)
{
    dpfCheckTimeBegin();

    metaObject->d_ptr->state = PluginMetaObject::Reading;

    QJsonObject jsonObj = metaObject->d_ptr->loader->metaData();

    if (jsonObj.isEmpty()) return;

    QString iid = jsonObj.value("IID").toString();
    if (iid.isEmpty()) return;
    metaObject->d_ptr->iid = iid;

    QJsonObject metaData = jsonObj.value("MetaData").toObject();

    QString name = metaData.value(PLUGIN_NAME).toString();
    if (name.isEmpty()) return;
    metaObject->d_ptr->name = name;

    QString version = metaData.value(PLUGIN_VERSION).toString();
    metaObject->d_ptr->version = version;

    QString compatVersion = metaData.value(PLUGIN_COMPATVERSION).toString();
    metaObject->d_ptr->compatVersion = compatVersion;

    QString category = metaData.value(PLUGIN_CATEGORY).toString();
    metaObject->d_ptr->category = category;

    QJsonArray licenseArray = metaData.value(PLUGIN_LICENSE).toArray();
    auto licenItera = licenseArray.begin();
    while (licenItera != licenseArray.end()) {
        metaObject->d_ptr->license.append(licenItera->toString());
        licenItera ++;
    }

    QString copyright = metaData.value(PLUGIN_COPYRIGHT).toString();
    metaObject->d_ptr->copyright = copyright;

    QString vendor = metaData.value(PLUGIN_VENDOR).toString();
    metaObject->d_ptr->vendor = vendor;

    QString description = metaData.value(PLUGIN_DESCRIPTION).toString();
    metaObject->d_ptr->description = description;

    QString urlLink = metaData.value(PLUGIN_URLLINK).toString();
    metaObject->d_ptr->urlLink = urlLink;

    QJsonArray dependsArray = metaData.value(PLUGIN_DEPENDS).toArray();
    auto itera = dependsArray.begin();
    while (itera != dependsArray.end())
    {
        QJsonObject dependObj = itera->toObject();
        QString dependName = dependObj.value(PLUGIN_NAME).toString();
        QString dependVersion = dependObj.value(PLUGIN_VERSION).toString();
        PluginDepend depends;
        depends.pluginName = dependName;
        depends.pluginVersion = dependVersion;
        metaObject->d_ptr->depends.append(depends);
        itera ++;
    }

    metaObject->d_ptr->state = PluginMetaObject::Readed;

    dpfCheckTimeEnd();
}

//内部使用QPluginLoader加载所有插件
void PluginManagerPrivate::loadPlugins()
{
    dpfCheckTimeBegin();

    QFuture<void> sortController = QtConcurrent::run(dependsSort,
                                                     &loadQueue,
                                                     &readQueue);
    sortController.waitForFinished();

    QFuture<void> mapController = QtConcurrent::map(readQueue.begin(),
                                                    readQueue.end(),
                                                    [=](PluginMetaObjectPointer &pointer){
        //流程互斥
        if (pointer->d_ptr->state >= PluginMetaObject::State::Loaded) {
            dpfCritical() << "Is Loaded plugin: "
                          << pointer->d_ptr->name
                          << pointer->fileName();
            return;
        }

        //必须执行了读取操作
        if (pointer->d_ptr->state != PluginMetaObject::State::Readed) {
            dpfCritical() << "Failed load plugin: "
                          << pointer->d_ptr->name
                          << pointer->fileName();
            return;
        }

        //设置当前正在加载的标志位
        pointer->d_ptr->state = PluginMetaObject::State::Loading;

        if (!pointer->d_ptr->loader->load()) {
            pointer->d_ptr->error = "Failed load plugin: " + pointer->errorString();
            dpfCritical() << "Failed load plugin: "
                          << pointer->d_ptr->name
                          << pointer->fileName()
                          << pointer->errorString();
            return;
        }

        pointer->d_ptr->plugin = QSharedPointer<Plugin>
                (qobject_cast<Plugin*>(pointer->d_ptr->loader->instance()));

        if (!pointer.isNull()) {
            pointer->d_ptr->state = PluginMetaObject::State::Loaded;
            dpfCritical() << "Loaded plugin: " << pointer->d_ptr->name;
        } else {
            pointer->d_ptr->error = "Failed get plugin instance is nullptr";
            dpfCritical() << pointer->d_ptr->error;
        }
    });
    mapController.waitForFinished();

    dpfDebug() << loadQueue;

    dpfCheckTimeEnd();
}

//初始化所有插件
void PluginManagerPrivate::initPlugins()
{
    dpfCheckTimeBegin();

    QFuture<void> runController = QtConcurrent::run([=](){
        QQueue<PluginMetaObjectPointer> initQueue = loadQueue;
        QQueue<PluginMetaObjectPointer>::iterator itera = initQueue.begin();
        while(itera != initQueue.end())
        {
            auto pointer = *itera;
            //流程互斥
            if (pointer->d_ptr->state >= PluginMetaObject::State::Initialized) {
                dpfCritical() << "Is initialized plugin: "
                              << pointer->d_ptr->name
                              << pointer->fileName();
                itera ++;
                continue;
            }

            if (pointer->d_ptr->state != PluginMetaObject::State::Loaded) {
                dpfCritical() << "Failed initialized plugin"
                              << pointer->d_ptr->name
                              << pointer->fileName();
                itera ++;
                continue;
            }

            if (!pointer->d_ptr->plugin.isNull()) {
                pointer->d_ptr->plugin->initialize();
                dpfCritical() << "Initialized plugin: " << pointer->d_ptr->name;
                pointer->d_ptr->state = PluginMetaObject::State::Initialized;
            }
            itera ++;
        }
    });

    runController.waitForFinished();

    dpfCheckTimeEnd();
}

//拉起插件,仅主线程使用
void PluginManagerPrivate::startPlugins()
{
    dpfCheckTimeBegin();

    QQueue<PluginMetaObjectPointer> startQueue = loadQueue;
    QQueue<PluginMetaObjectPointer>::iterator itera = startQueue.begin();
    while(itera != startQueue.end())
    {
        PluginMetaObjectPointer pointer = *itera;
        //流程互斥
        if (pointer->d_ptr->state >= PluginMetaObject::State::Started) {
            dpfCritical() << "Is started plugin:"
                          << pointer->d_ptr->name
                          << pointer->fileName();
            itera ++;
            continue;
        }

        if (pointer->d_ptr->state != PluginMetaObject::State::Initialized) {
            dpfCritical() << "Failed start plugin:"
                          << pointer->d_ptr->name
                          << pointer->fileName();
            itera ++;
            continue;
        }

        pointer->d_ptr->context = QSharedPointer<PluginContext>(new PluginContext());

        if (pointer->d_ptr->plugin->start(pointer->d_ptr->context)) {
            dpfCritical() << "Started plugin: " << pointer->d_ptr->name;
            pointer->d_ptr->state = PluginMetaObject::State::Started;
        } else {
            pointer->d_ptr->error = "Failed start plugin in function start() logic";
            dpfCritical() << pointer->d_ptr->error.toLocal8Bit().data();
        }

        itera ++;
    }

    dpfCheckTimeEnd();
}

//停止插件,仅主线程
void PluginManagerPrivate::stopPlugins()
{
    dpfCheckTimeBegin();

    QQueue<PluginMetaObjectPointer> stopQueue = loadQueue;
    auto itera = stopQueue.rbegin();
    while(itera != stopQueue.rend())
    {
        PluginMetaObjectPointer pointer = *itera;
        if (pointer->d_ptr->state != PluginMetaObject::State::Started) {
            continue;
        }

        Plugin::ShutdownFlag stFlag = pointer->d_ptr->plugin->stop();
        pointer->d_ptr->state = PluginMetaObject::State::Stoped;

        if (stFlag == Plugin::ShutdownFlag::Asynch) {

            dpfCritical() << "Stoped asynch plugin: " << pointer->d_ptr->name;

            pointer->d_ptr->state = PluginMetaObject::State::Stoped;

            QObject::connect(pointer->d_ptr->plugin.data(), &Plugin::asyncStopFinished,
                             pointer->d_ptr->plugin.data(), [=]()
            {
                pointer->d_ptr->plugin = nullptr;

                if (!pointer->d_ptr->loader->unload()) {
                    dpfCritical() << pointer->d_ptr->loader->errorString();
                }

                pointer->d_ptr->state = PluginMetaObject::State::Shutdown;
                dpfCritical() << "shutdown" << pointer->d_ptr->loader->fileName();

            },Qt::ConnectionType::DirectConnection); //同步信号直接调用无需等待

        } else {

            if (pointer->d_ptr->plugin) {
                dpfCritical() << "Stoped synch plugin: " << pointer->d_ptr->name;
                pointer->d_ptr->plugin = nullptr;
                pointer->d_ptr->state = PluginMetaObject::State::Stoped;
            }

            if (!pointer->d_ptr->loader->unload()) {
                dpfCritical() << pointer->d_ptr->loader->errorString();
                continue;
            }

            pointer->d_ptr->state = PluginMetaObject::State::Shutdown;
            dpfCritical() << "shutdown" << pointer->d_ptr->loader->fileName();
        }

        itera ++;
    }

    dpfCheckTimeEnd();
}

//按照依赖排序
void PluginManagerPrivate::dependsSort(QQueue<PluginMetaObjectPointer> *dstQueue,
                                       QQueue<PluginMetaObjectPointer> *srcQueue)
{
    dpfCheckTimeBegin();

    QMutexLocker lock(&GlobalPrivate::mutex);

    *dstQueue = (*srcQueue);
    typedef PluginMetaObjectPointer EmleTPointer;
    std::sort(dstQueue->begin(), dstQueue->end(), [=](EmleTPointer after, EmleTPointer befor)
    {
        dpfDebug() << after->d_ptr->name << befor->d_ptr->name;
        if (after->depends().isEmpty()) {
            //前节点没有依赖则保持顺序
            return true;
        } else { //前节点存在依赖
            if(befor->depends().isEmpty()) {
                //后节点为空则调整顺序
                return false;
            } else { //后节点存在依赖
                //遍历后节点依赖
                for (auto depend : befor->d_ptr->depends)
                {
                    //后节点依赖存在前节点
                    if (depend.name() == after->name())
                        return true;
                }
                //遍历前节点依赖
                for (auto depend : after->d_ptr->depends)
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

    dpfCheckTimeEnd();
}

DPF_END_NAMESPACE
