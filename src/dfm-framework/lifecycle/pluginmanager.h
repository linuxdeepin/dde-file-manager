#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "dfm-framework/lifecycle/plugin.h"
#include "dfm-framework/lifecycle/pluginsetting.h"

#include "dfm-framework/definitions/globaldefinitions.h"

#include <QPluginLoader>
#include <QSettings>
#include <QObject>
#include <QQueue>
#include <QSharedData>
#include <QDirIterator>
#include <QScopedPointer>

DPF_BEGIN_NAMESPACE

class PluginManagerPrivate;

/*
 * @class PluginManager 插件管理器
 * @details 提供插件加载与卸载
 * 其中重要的特性为：plugin IID (插件身份标识) 可参阅Qt插件规范；
 * 此处目前只支持Plugin接口插件的动态库形式插件加载
 */
class PluginManager: public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PluginManager)
    Q_DECLARE_PRIVATE(PluginManager)
    QSharedPointer<PluginManagerPrivate> d;

public:
    explicit PluginManager();
    QString pluginIID() const;
    QStringList pluginPaths() const;
    QStringList servicePaths() const;
    void setPluginIID(const QString &pluginIID);
    void setPluginPaths(const QStringList &pluginPaths);
    void setServicePaths(const QStringList &servicePaths);

    void readPlugins();

    void loadPlugins();
    void initPlugins();
    void startPlugins();
    void stopPlugins();

    PluginMetaObjectPointer pluginMetaObj(const QString &pluginName,
                                             const QString version = "") const;

    void loadPlugin(PluginMetaObjectPointer &pointer);
    void initPlugin(PluginMetaObjectPointer &pointer);
    void startPlugin(PluginMetaObjectPointer &pointer);
    void stopPlugin(PluginMetaObjectPointer &pointer);
};

/*
 * @class PluginManagerGlobal
 * @details 全局静态的插件管理器接口，提供一个可以访问Plugin的插件接口静态类
 * 内部使用Q_GLOBAL_STATIC创建静态PluginManager对象
 */
class PluginManagerGlobal
{
public:

};

DPF_END_NAMESPACE

#endif // PLUGINMANAGER_H
