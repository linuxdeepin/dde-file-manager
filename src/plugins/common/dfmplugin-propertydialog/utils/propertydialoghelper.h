#ifndef PROPERTYDIALOGHELPER_H
#define PROPERTYDIALOGHELPER_H

#include "dfmplugin_propertydialog_global.h"

#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"
#include "services/common/propertydialog/propertydialogservice.h"

DPPROPERTYDIALOG_BEGIN_NAMESPACE
class PropertyDialogHelper : public QObject
{
    Q_OBJECT
public:
    explicit PropertyDialogHelper(QObject *parent = nullptr) = delete;

    static DSC_NAMESPACE::PropertyDialogService *propertyServiceInstance();
};
DPPROPERTYDIALOG_END_NAMESPACE
#endif   // PROPERTYDIALOGHELPER_H
