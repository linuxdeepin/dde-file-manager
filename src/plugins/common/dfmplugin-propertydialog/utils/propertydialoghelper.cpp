#include "propertydialoghelper.h"

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
