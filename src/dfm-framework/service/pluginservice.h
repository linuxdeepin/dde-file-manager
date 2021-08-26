#ifndef PLUGINSERVICE_H
#define PLUGINSERVICE_H

#include "pluginserviceglobal.h"

#include "dfm-framework/log/frameworklog.h"

#include "dfm-framework/definitions/globaldefinitions.h"

#include <QObject>
#include <QMetaObject>
#include <QHash>
#include <QDebug>

DPF_BEGIN_NAMESPACE

/**
 * @brief The PluginService class
 * 插件服务接口类
 */

class PluginService : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PluginService)
    friend class PluginServiceGlobal;

public:

    explicit PluginService(QObject * parent = nullptr): QObject(parent)
    {

    }

};

/// @brief PLUGIN_SERVICE 服务类声明宏，与Q_DISABLE_COPY类似
#define PLUGIN_SERVICE(x) \
    public: \
    explicit x(QObject *parent = nullptr) : PluginService(parent) \
{\
    dpf::GlobalPrivate::PluginServiceGlobal::importService<x>(this); \
    dpfCritical() << "IMPORT_SERVICE: " \
    << #x ;\
    if (parent != nullptr) { \
    dpfCritical() << "Importer class: " \
    << parent->metaObject()->className(); \
    }\
    QObject::connect(this, &QObject::destroyed, this, [=]() \
{ \
    dpfCritical() << "EXPORT_SERVICE: " << #x ;\
    if (parent != nullptr) { \
    dpfCritical() << "Exporter class: " \
    << parent->metaObject()->className(); \
    }\
    });\
    }

/// @brief IMPORT_SERVICE 服务导入接口宏
#define IMPORT_SERVICE(x)\
    nullptr == DPF_NAMESPACE::GlobalPrivate::PluginServiceGlobal::findService<x>(#x) ? \
    new x : DPF_NAMESPACE::GlobalPrivate::PluginServiceGlobal::findService<x>(#x); \
    DPF_NAMESPACE::GlobalPrivate::PluginServiceGlobal::addImportInfo(#x,this->metaObject()->className());

/// @brief IMPORT_SERVICE 服务导出接口宏
#define EXPORT_SERVICE(x)\
    nullptr == DPF_NAMESPACE::GlobalPrivate::PluginServiceGlobal::findService<x>(#x) ? \
    false : DPF_NAMESPACE::GlobalPrivate::PluginServiceGlobal::exportService<x>(#x); \
    DPF_NAMESPACE::GlobalPrivate::PluginServiceGlobal::delImportInfo(#x);

DPF_END_NAMESPACE

#endif // PLUGINSERVICE_H
