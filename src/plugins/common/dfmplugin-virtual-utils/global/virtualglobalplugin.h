#ifndef VIRTUALGLOBALPLUGIN_H
#define VIRTUALGLOBALPLUGIN_H

#include "dfmplugin_utils_global.h"
#include "global/globaleventreceiver.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_utils {

class VirtualGlobalPlugin : public dpf::Plugin
{
    Q_OBJECT
    DPF_EVENT_NAMESPACE(DPUTILS_NAMESPACE)

    DPF_EVENT_REG_HOOK(hook_UrlsTransform)

public:
    virtual void initialize() override;
    virtual bool start() override;

private:
    QScopedPointer<GlobalEventReceiver> eventReceiver { new GlobalEventReceiver };
};

}   // dfmplugin_utils

#endif   // VIRTUALGLOBALPLUGIN_H
