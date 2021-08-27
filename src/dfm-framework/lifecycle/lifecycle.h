#ifndef LIFECYCLE_H
#define LIFECYCLE_H

#include "dfm-framework/lifecycle/pluginmetaobject.h"
#include "dfm-framework/lifecycle/plugin.h"
#include "dfm-framework/dfm_framework_global.h"

#include <QString>
#include <QObject>

DPF_BEGIN_NAMESPACE

class LifeCycle final : public QObject
{
    Q_OBJECT
public:
    LifeCycle() = delete;
    static void setPluginIID(const QString &pluginIID);
    static QString pluginIID();
    static QStringList pluginPaths();
    static void setPluginPaths(const QStringList &pluginPaths);
    static QStringList servicePaths();
    static void setServicePaths(const QStringList &servicePaths);
    static PluginMetaObjectPointer pluginMetaObj(const QString &pluginName,
                                             const QString version = "");
    static void readPlugins();
    static void loadPlugins();
    static void shutdownPlugins();

    static void loadPlugin(PluginMetaObjectPointer &pointer);
    static void shutdownPlugin(PluginMetaObjectPointer &pointer);
};

DPF_END_NAMESPACE

#endif // LIFECYCLE_H
