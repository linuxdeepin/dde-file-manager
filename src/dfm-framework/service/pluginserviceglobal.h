#ifndef PLUGINSERVICEGLOBAL_H
#define PLUGINSERVICEGLOBAL_H

#include "dfm-framework/definitions/globaldefinitions.h"

#include <QString>
#include <QObject>
#include <QHash>
#include <QtConcurrent>

DPF_BEGIN_NAMESPACE

//全局私有，对内使用
class PluginService;

namespace GlobalPrivate {

typedef QString ServiceName,BundleName;

/**
 * @brief The PluginServiceGlobal class
 * 插件服务的全局注册表
 */
class PluginServiceGlobal : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PluginServiceGlobal)

    static QHash<ServiceName, PluginService*> services;/// 服务注册列表，1:1关联
    static QHash<ServiceName, BundleName> importers; ///服务导入者列表，1:1关联

public:

    PluginServiceGlobal() = delete;

    static void addImportInfo(const ServiceName &serviceName,
                              const BundleName &bundleName)
    {
        importers.insert(serviceName,bundleName);
    }

    static void delImportInfo(const BundleName &bundleName)
    {
        QFuture<void> removeController = QtConcurrent::run([=]()
        {
            QHash<ServiceName, BundleName>::iterator itera = importers.begin();
            while (itera != importers.end()) {
                if (itera.value() == bundleName) {
                    importers.remove(itera.key());
                }
                itera++;
            }
        });

        removeController.waitForFinished();
    }

    template<class T = PluginService>
    static T* findService(const QString &serviceName)
    {
        return qobject_cast<T*>(services.value(serviceName));
    }

    template<class T = PluginService>
    static bool importService(T * const serviceIns)
    {
        auto serviceName = serviceIns->metaObject()->className();
        auto service = findService(serviceName);
        if (nullptr == service) {
            services.insert(serviceName, serviceIns);
            return true;
        } else {
            return false;
        }
        return false;
    }

    template<class T = PluginService>
    static bool exportService(const QString &serverName)
    {
        if (qobject_cast<T*>(services.value(serverName))->parent() == nullptr) {
            delete services.take(serverName);
        }

        return services.remove(serverName);
    }

    static QStringList getServices()
    {
        QStringList retServices = services.keys();
        return retServices;
    }

}; //class PluginServiceGlobal

} //namespace GlobalPrivate

DPF_END_NAMESPACE

#endif // PLUGINSERVICEGLOBAL_H
