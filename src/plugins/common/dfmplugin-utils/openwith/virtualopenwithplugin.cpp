#include "virtualopenwithplugin.h"
#include "openwith/openwithhelper.h"

using namespace dfmplugin_utils;

using CustomViewExtensionView = std::function<QWidget *(const QUrl &url)>;
Q_DECLARE_METATYPE(CustomViewExtensionView)

void VirtualOpenWithPlugin::initialize()
{
    eventReceiver->initEventConnect();
}

bool VirtualOpenWithPlugin::start()
{
    CustomViewExtensionView func { OpenWithHelper::createOpenWithWidget };
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_ViewExtension_Register", func, 2);

    return true;
}
