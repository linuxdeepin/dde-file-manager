#ifndef COREPLUGIN_H
#define COREPLUGIN_H

#include "dfmplugin.h"

class DFMWindowManagerService;
class Recent : public DFMPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DFMPLUGIN_INTERFACE FILE "recent.json")

public:
    virtual void initialize() override;
    virtual bool start(QSharedPointer<DFMPluginContext> context) override;
    virtual ShutdownFlag stop() override;

    DFMWindowManagerService *windowManagerService;
};

#endif // COREPLUGIN_H
