#ifndef PLUGINSSETTING_H
#define PLUGINSSETTING_H

#include "dfm-framework/lifecycle/pluginmetaobject.h"
#include "dfm-framework/dfm_framework_global.h"

#include <QCoreApplication>
#include <QSettings>

DPF_BEGIN_NAMESPACE

const char ORGANIZATION[] = "deepin";
const char ENABLED[] = "enabled";

class PluginSetting final : public QSettings
{
    Q_OBJECT

public:

    PluginSetting(Scope scope = Scope::UserScope,
                      const QString &organization = ORGANIZATION,
                      const QString &application = QCoreApplication::applicationName(),
                      QObject *parent = nullptr);

    void setPluginEnable(const PluginMetaObject &meta, bool enabled);

Q_SIGNALS:
    void pluginEnabled(const PluginMetaObject &meta, bool enabled);
};

DPF_END_NAMESPACE

#endif // PLUGINSSETTING_H
