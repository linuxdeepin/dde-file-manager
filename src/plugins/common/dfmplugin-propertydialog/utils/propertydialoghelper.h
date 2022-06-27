#ifndef PROPERTYDIALOGHELPER_H
#define PROPERTYDIALOGHELPER_H

#include "dfmplugin_propertydialog_global.h"

#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"
#include "services/common/propertydialog/propertydialogservice.h"

namespace dfmplugin_propertydialog {
class PropertyDialogHelper : public QObject
{
    Q_OBJECT
public:
    explicit PropertyDialogHelper(QObject *parent = nullptr) = delete;

    static DSC_NAMESPACE::PropertyDialogService *propertyServiceInstance();
};
}
#endif   // PROPERTYDIALOGHELPER_H
