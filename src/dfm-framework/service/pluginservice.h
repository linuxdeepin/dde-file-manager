#ifndef PLUGINSERVICE_H
#define PLUGINSERVICE_H

#include "pluginserviceglobal.h"

#include "dfm-framework/log/frameworklog.h"

#include "dfm-framework/dfm_framework_global.h"

#include <QObject>
#include <QMetaObject>
#include <QHash>
#include <QDebug>

DPF_BEGIN_NAMESPACE

/**
 * @brief The PluginService class
 * 插件服务接口类
 * @code
 * class PluginService : public QObject
 * {
 *      Q_OBJECT
 *      Q_DISABLE_COPY(PluginService)
 *      friend class PluginServiceGlobal;
 * public:
 *      explicit PluginService(){}
 *      virtual ~PluginService(){}
 * };
 * @endcode
 */

class PluginService;

#define PLUGIN_SERVICE(x) \
   static bool x##_regResult = dpf::GlobalPrivate::PluginServiceGlobal::instance().regClass<x>(#x);

#define IMPORT_SERVICE(x) \
    QString errorString; \
    auto x##_ins = dpf::GlobalPrivate::PluginServiceGlobal::instance().create(#x,&errorString); \
    if (!x##_ins) \
        dpfCritical() << errorString; \
    else { \
        dpfCritical() << "IMPORT_SERVICE:" << #x;\
        auto appendRes = dpf::GlobalPrivate::PluginServiceGlobal::instance().append(#x, x##_ins, &errorString);\
        if (!appendRes) dpfCritical()<< errorString;\
    }

#define EXPORT_SERVICE(x) \
    dpf::GlobalPrivate::PluginServiceGlobal::instance().remove(#x);\
    dpfCritical() << "EXPORT_SERVICE:" << #x;

DPF_END_NAMESPACE

#endif // PLUGINSERVICE_H
