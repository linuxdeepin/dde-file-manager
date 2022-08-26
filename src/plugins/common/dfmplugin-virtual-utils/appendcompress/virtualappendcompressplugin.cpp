#include "virtualappendcompressplugin.h"

using namespace dfmplugin_utils;

void VirtualAppendCompressPlugin::initialize()
{
    eventReceiver->initEventConnect();
}

bool VirtualAppendCompressPlugin::start()
{
    return true;
}
