#ifndef PLUGINMANAGER_P_H
#define PLUGINMANAGER_P_H

#include "dfm-framework/lifecycle/pluginsetting.h"
#include "dfm-framework/dfm_framework_global.h"

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
    QString pluginLoadIID;
    QStringList pluginLoadPaths;
    QStringList serviceLoadPaths;
    QList<PluginMetaObjectPointer> plugins;
    QQueue<PluginMetaObjectPointer> readQueue;
    QQueue<PluginMetaObjectPointer> loadQueue;
    PluginSetting setting;
public:
    explicit PluginManagerPrivate(PluginManager *qq);
    virtual ~PluginManagerPrivate();
    QString pluginIID() const;
    void setPluginIID(const QString &pluginIID);
    QStringList pluginPaths() const;
    void setPluginPaths(const QStringList &pluginPaths);
    QStringList servicePaths() const;
    void setServicePaths(const QStringList &servicePaths);
    void setPluginEnable(const PluginMetaObject& meta, bool enabled);
    PluginMetaObjectPointer pluginMetaObj(const QString &name,const QString &version = "");
    void loadPlugin(PluginMetaObjectPointer &pluginMetaObj);
    void initPlugin(PluginMetaObjectPointer &pluginMetaObj);
    void startPlugin(PluginMetaObjectPointer &pluginMetaObj);
    void stopPlugin(PluginMetaObjectPointer &pluginMetaObj);
    void readPlugins();
    void loadPlugins();
    void initPlugins();
    void startPlugins();
    void stopPlugins();
    static void scanfAllPlugin(QQueue<PluginMetaObjectPointer> *destQueue,
                               const QStringList& pluginPaths,
                               const QString &pluginIID);
    static void readJsonToMeta(const PluginMetaObjectPointer &metaObject);
    static void dependsSort(QQueue<PluginMetaObjectPointer>* dstQueue,
                            QQueue<PluginMetaObjectPointer>* srcQueue);
};

DPF_END_NAMESPACE

#endif // PLUGINMANAGER_P_H
