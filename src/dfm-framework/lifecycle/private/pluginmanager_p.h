#ifndef PLUGINMANAGER_P_H
#define PLUGINMANAGER_P_H

#include "dfm-framework/lifecycle/pluginsetting.h"

#include "dfm-framework/definitions/globaldefinitions.h"

#include <QQueue>
#include <QStringList>
#include <QPluginLoader>
#include <QObject>
#include <QJsonArray>
#include <QSettings>
#include <QDirIterator>
#include <QDebug>
#include <QWriteLocker>
#include <QtConcurrent>

DPF_BEGIN_NAMESPACE

class PluginMetaObject;
class PluginManager;

class PluginManagerPrivate : public QSharedData
{
    Q_DISABLE_COPY(PluginManagerPrivate)
    Q_DECLARE_PUBLIC(PluginManager)
    PluginManager * const q_ptr;

public:
    explicit PluginManagerPrivate(PluginManager *qq);

    QString pluginIID() const;

    void setPluginIID(const QString &pluginIID);

    QStringList pluginPaths() const;

    void setPluginPaths(const QStringList &pluginPaths);

    QStringList servicePaths() const;

    void setServicePaths(const QStringList &servicePaths);

    void setPluginEnable(const PluginMetaObject& meta, bool enabled);

    //线程安全
    PluginMetaObjectPointer pluginMetaObj(const QString &name,
                                             const QString &version = "");

    //加载一个插件，线程安全，可单独使用。
    void loadPlugin(PluginMetaObjectPointer &pluginMetaObj);

    //初始化一个插件，线程安全，可单独使用
    void initPlugin(PluginMetaObjectPointer &pluginMetaObj);

    //启动一个插件，线程安全，可单独使用
    void startPlugin(PluginMetaObjectPointer &pluginMetaObj);

    //停止并卸载一个插件，线程安全，可单独使用
    void stopPlugin(PluginMetaObjectPointer &pluginMetaObj);

    //读取所有插件的Json源数据
    void readPlugins();

    //内部使用QPluginLoader加载所有插件
    void loadPlugins();

    //初始化所有插件
    void initPlugins();

    //拉起插件,仅主线程使用
    void startPlugins();

    //停止插件,仅主线程
    void stopPlugins();

    //扫描所有插件到目标队列
    static void scanfAllPlugin(QQueue<PluginMetaObjectPointer> *destQueue,
                               const QStringList& pluginPaths,
                               const QString &pluginIID);
    //同步json到定义类型
    static void readJsonToMeta(const PluginMetaObjectPointer &metaObject);

    //按照依赖排序
    static void dependsSort(QQueue<PluginMetaObjectPointer>* dstQueue,
                            QQueue<PluginMetaObjectPointer>* srcQueue);

private:
    QString m_pluginIID;
    QStringList m_pluginPaths;
    QStringList m_servicePaths;
    QList<PluginMetaObjectPointer> m_plugins;
    QQueue<PluginMetaObjectPointer> m_readQueue;
    QQueue<PluginMetaObjectPointer> m_loadQueue;
    PluginSetting m_setting;
};

DPF_END_NAMESPACE

#endif // PLUGINMANAGER_P_H
