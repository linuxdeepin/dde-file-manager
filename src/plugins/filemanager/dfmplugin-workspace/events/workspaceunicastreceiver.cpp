#include "workspaceunicastreceiver.h"
#include "views/workspacewidget.h"
#include "utils/workspacehelper.h"
#include "utils/customtopwidgetinterface.h"
#include "services/filemanager/dfm_filemanager_service_global.h"

#include <dfm-framework/framework.h>

DPWORKSPACE_BEGIN_NAMESPACE

#define STR1(s) #s
#define STR2(s) STR1(s)

/*!
 * \brief topic is defined in SideBarService
 * \param func
 * \return
 */
inline QString topic(const QString &func)
{
    return QString(STR2(DSB_FM_NAMESPACE)) + "::" + func;
}

WorkspaceUnicastReceiver *WorkspaceUnicastReceiver::instance()
{
    static WorkspaceUnicastReceiver receiver;
    return &receiver;
}

void WorkspaceUnicastReceiver::connectService()
{
    dpfInstance.eventUnicast().connect(topic("WorkspaceService::addScheme"), this, &WorkspaceUnicastReceiver::invokeAddScheme);
    dpfInstance.eventUnicast().connect(topic("WorkspaceService::tabAddable"), this, &WorkspaceUnicastReceiver::invokeTabAddable);
    dpfInstance.eventUnicast().connect(topic("WorkspaceService::addCustomTopWidget"), this, &WorkspaceUnicastReceiver::invokeAddCustomTopWidget);
    dpfInstance.eventUnicast().connect(topic("WorkspaceService::getCustomTopWidgetVisible"), this, &WorkspaceUnicastReceiver::invokeGetCustomTopWidgetVisible);
}

void WorkspaceUnicastReceiver::invokeAddScheme(const QString &scheme)
{
    WorkspaceHelper::instance()->addScheme(scheme);
}

bool WorkspaceUnicastReceiver::invokeTabAddable(const quint64 windowID)
{
    auto widget = WorkspaceHelper::instance()->findWorkspaceByWindowId(windowID);
    if (!widget)
        return false;

    return widget->canAddNewTab();
}

bool WorkspaceUnicastReceiver::invokeAddCustomTopWidget(const dfm_service_filemanager::Workspace::CustomTopWidgetInfo &info)
{
    if (WorkspaceHelper::instance()->isRegistedTopWidget(info.scheme)) {
        qWarning() << "custom top widget sechme " << info.scheme << "has been resigtered!";
        return false;
    }

    WorkspaceHelper::instance()->registerTopWidgetCreator(info.scheme, [=]() {
        CustomTopWidgetInterface *interface { new CustomTopWidgetInterface };
        interface->setKeepShow(info.keepShow);
        interface->registeCreateTopWidgetCallback(info.createTopWidgetCb);
        return interface;
    });
    return true;
}

bool WorkspaceUnicastReceiver::invokeGetCustomTopWidgetVisible(const quint64 windowID, const QString &scheme)
{
    WorkspaceWidget *workspaceWidget = WorkspaceHelper::instance()->findWorkspaceByWindowId(windowID);
    if (workspaceWidget) {
        return workspaceWidget->getCustomTopWidgetVisible(scheme);
    }
    return false;
}

WorkspaceUnicastReceiver::WorkspaceUnicastReceiver(QObject *parent)
    : QObject(parent)
{
}
DPWORKSPACE_END_NAMESPACE
