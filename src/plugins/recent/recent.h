#ifndef COREPLUGIN_H
#define COREPLUGIN_H

#include "dfm-framework/lifecycle/plugin.h"

class DFMWindowManagerService;
class Recent : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PLUGIN_INTERFACE FILE "recent.json")

public:
    virtual void initialize() override;
    virtual bool start(QSharedPointer<dpf::PluginContext> context) override;
    virtual ShutdownFlag stop() override;

    DFMWindowManagerService *windowManagerService;
};

#endif // COREPLUGIN_H
