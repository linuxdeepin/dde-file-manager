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
    dpfInstance.eventUnicast().connect(topic("WorkspaceService::setFileViewFilterData"), this, &WorkspaceUnicastReceiver::invokeSetFileViewFilterData);
    dpfInstance.eventUnicast().connect(topic("WorkspaceService::setFileViewFilterCallback"), this, &WorkspaceUnicastReceiver::invokeSetFileViewFilterCallback);
    dpfInstance.eventUnicast().connect(topic("WorkspaceService::setWorkspaceMenuScene"), this, &WorkspaceUnicastReceiver::invokeSetWorkspaceMenuScene);
    dpfInstance.eventUnicast().connect(topic("WorkspaceService::setDefaultViewMode"), this, &WorkspaceUnicastReceiver::invokeSetDefaultViewMode);
    dpfInstance.eventUnicast().connect(topic("WorkspaceService::getDefaultViewMode"), this, &WorkspaceUnicastReceiver::invokeGetDefaultViewMode);
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

bool WorkspaceUnicastReceiver::invokeAddCustomTopWidget(const DSB_FM_NAMESPACE::Workspace::CustomTopWidgetInfo &info)
{
    if (WorkspaceHelper::instance()->isRegistedTopWidget(info.scheme)) {
        qWarning() << "custom top widget sechme " << info.scheme << "has been resigtered!";
        return false;
    }

    WorkspaceHelper::instance()->registerTopWidgetCreator(info.scheme, [=]() {
        CustomTopWidgetInterface *interface { new CustomTopWidgetInterface };
        interface->setKeepShow(info.keepShow);
        interface->registeCreateTopWidgetCallback(info.createTopWidgetCb);
        interface->registeCreateTopWidgetCallback(info.showTopWidgetCb);
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

void WorkspaceUnicastReceiver::invokeSetFileViewFilterData(const quint64 windowID, const QUrl &url, const QVariant &data)
{
    WorkspaceHelper::instance()->setFilterData(windowID, url, data);
}

void WorkspaceUnicastReceiver::invokeSetFileViewFilterCallback(const quint64 windowID, const QUrl &url, const DSB_FM_NAMESPACE::Workspace::FileViewFilterCallback callback)
{
    WorkspaceHelper::instance()->setFilterCallback(windowID, url, callback);
}

void WorkspaceUnicastReceiver::invokeSetWorkspaceMenuScene(const QString &scheme, const QString &scene)
{
    WorkspaceHelper::instance()->setWorkspaceMenuScene(scheme, scene);
}

void WorkspaceUnicastReceiver::invokeSetDefaultViewMode(const QString &scheme, const dfmbase::Global::ViewMode mode)
{
    WorkspaceHelper::instance()->setDefaultViewMode(scheme, mode);
}

DFMBASE_NAMESPACE::Global::ViewMode WorkspaceUnicastReceiver::invokeGetDefaultViewMode(const QString &scheme)
{
    return WorkspaceHelper::instance()->findViewMode(scheme);
}

WorkspaceUnicastReceiver::WorkspaceUnicastReceiver(QObject *parent)
    : QObject(parent)
{
}
DPWORKSPACE_END_NAMESPACE
