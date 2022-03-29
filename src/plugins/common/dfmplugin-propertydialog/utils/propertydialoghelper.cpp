#include "propertydialoghelper.h"
DSB_FM_USE_NAMESPACE
DSC_USE_NAMESPACE
DPPROPERTYDIALOG_USE_NAMESPACE

PropertyDialogService *PropertyDialogHelper::propertyServiceInstance()
{
    static PropertyDialogService *propertyService = nullptr;
    if (propertyService == nullptr) {
        auto &ctx = dpfInstance.serviceContext();
        QString errStr;
        if (!ctx.load(PropertyDialogService::name(), &errStr)) {
            qCritical() << errStr;
            abort();
        }

        propertyService = ctx.service<PropertyDialogService>(PropertyDialogService::name());
        if (!propertyService) {
            qCritical() << "Failed, init sidebar \"propertyService\" is empty";
            abort();
        }
    }
    return propertyService;
}

WindowsService *PropertyDialogHelper::windowServiceInstance()
{
    static WindowsService *windowsService = nullptr;
    if (windowsService == nullptr) {
        auto &ctx = dpfInstance.serviceContext();
        QString errStr;
        if (!ctx.load(WindowsService::name(), &errStr)) {
            qCritical() << errStr;
            abort();
        }

        windowsService = ctx.service<WindowsService>(WindowsService::name());
        if (!windowsService) {
            qCritical() << "Failed, init windows \"sideBarService\" is empty";
            abort();
        }
    }
    return windowsService;
}
