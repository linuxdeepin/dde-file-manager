#include "workspaceunicastreceiver.h"
#include "views/workspacewidget.h"
#include "views/fileview.h"
#include "utils/workspacehelper.h"
#include "utils/customtopwidgetinterface.h"
#include "services/filemanager/dfm_filemanager_service_global.h"

#include <dfm-framework/framework.h>

DFMGLOBAL_USE_NAMESPACE
DPWORKSPACE_USE_NAMESPACE

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
    dpfInstance.eventUnicast().connect(topic("WorkspaceService::schemeViewIsFileView"), this, &WorkspaceUnicastReceiver::invokeSchemeViewIsFileView);
    dpfInstance.eventUnicast().connect(topic("WorkspaceService::tabAddable"), this, &WorkspaceUnicastReceiver::invokeTabAddable);
    dpfInstance.eventUnicast().connect(topic("WorkspaceService::addCustomTopWidget"), this, &WorkspaceUnicastReceiver::invokeAddCustomTopWidget);
    dpfInstance.eventUnicast().connect(topic("WorkspaceService::getCustomTopWidgetVisible"), this, &WorkspaceUnicastReceiver::invokeGetCustomTopWidgetVisible);
    dpfInstance.eventUnicast().connect(topic("WorkspaceService::setFileViewFilterData"), this, &WorkspaceUnicastReceiver::invokeSetFileViewFilterData);
    dpfInstance.eventUnicast().connect(topic("WorkspaceService::setFileViewFilterCallback"), this, &WorkspaceUnicastReceiver::invokeSetFileViewFilterCallback);
    dpfInstance.eventUnicast().connect(topic("WorkspaceService::setWorkspaceMenuScene"), this, &WorkspaceUnicastReceiver::invokeSetWorkspaceMenuScene);
    dpfInstance.eventUnicast().connect(topic("WorkspaceService::setDefaultViewMode"), this, &WorkspaceUnicastReceiver::invokeSetDefaultViewMode);
    dpfInstance.eventUnicast().connect(topic("WorkspaceService::getDefaultViewMode"), this, &WorkspaceUnicastReceiver::invokeGetDefaultViewMode);
    dpfInstance.eventUnicast().connect(topic("WorkspaceService::currentViewMode"), this, &WorkspaceUnicastReceiver::invokeCurrentViewMode);
    dpfInstance.eventUnicast().connect(topic("WorkspaceService::registerFileViewRoutePrehandle"), this, &WorkspaceUnicastReceiver::invokeRegisterFileViewRoutePrehanlder);
    dpfInstance.eventUnicast().connect(topic("WorkspaceService::selectedUrls"), this, &WorkspaceUnicastReceiver::invokeSelectedUrls);
    dpfInstance.eventUnicast().connect(topic("WorkspaceService::getViewVisibleGeometry"), this, &WorkspaceUnicastReceiver::invokeGetViewVisibleGeometry);
    dpfInstance.eventUnicast().connect(topic("WorkspaceService::getItemRect"), this, &WorkspaceUnicastReceiver::invokeGetItemRect);
    dpfInstance.eventUnicast().connect(topic("WorkspaceService::getViewFilter"), this, &WorkspaceUnicastReceiver::invokeGetViewFilter);
    dpfInstance.eventUnicast().connect(topic("WorkspaceService::findMenuScene"), this, &WorkspaceUnicastReceiver::invokeFindMenuScene);
}

void WorkspaceUnicastReceiver::invokeAddScheme(const QString &scheme)
{
    WorkspaceHelper::instance()->addScheme(scheme);
}

bool WorkspaceUnicastReceiver::invokeSchemeViewIsFileView(const QString &scheme)
{
    return WorkspaceHelper::instance()->schemeViewIsFileView(scheme);
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

QString WorkspaceUnicastReceiver::invokeFindMenuScene(const QString &scheme)
{
    return WorkspaceHelper::instance()->findMenuScene(scheme);
}

void WorkspaceUnicastReceiver::invokeSetDefaultViewMode(const QString &scheme, const dfmbase::Global::ViewMode mode)
{
    WorkspaceHelper::instance()->setDefaultViewMode(scheme, mode);
}

DFMBASE_NAMESPACE::Global::ViewMode WorkspaceUnicastReceiver::invokeGetDefaultViewMode(const QString &scheme)
{
    return WorkspaceHelper::instance()->findViewMode(scheme);
}

ViewMode WorkspaceUnicastReceiver::invokeCurrentViewMode(const quint64 windowID)
{
    WorkspaceWidget *workspaceWidget = WorkspaceHelper::instance()->findWorkspaceByWindowId(windowID);

    if (workspaceWidget)
        return workspaceWidget->currentViewMode();

    return ViewMode::kNoneMode;
}

bool WorkspaceUnicastReceiver::invokeRegisterFileViewRoutePrehanlder(const QString &scheme, const dfm_service_filemanager::Workspace::FileViewRoutePrehaldler &prehandler)
{
    return WorkspaceHelper::instance()->reigsterViewRoutePrehandler(scheme, prehandler);
}

QList<QUrl> WorkspaceUnicastReceiver::invokeSelectedUrls(const quint64 windowID)
{
    WorkspaceWidget *workspaceWidget = WorkspaceHelper::instance()->findWorkspaceByWindowId(windowID);
    if (workspaceWidget) {
        auto view = workspaceWidget->currentViewPtr();
        if (view)
            return view->selectedUrlList();
        else
            return {};
    }

    return {};
}

QRectF WorkspaceUnicastReceiver::invokeGetViewVisibleGeometry(const quint64 windowID)
{
    WorkspaceWidget *workspaceWidget = WorkspaceHelper::instance()->findWorkspaceByWindowId(windowID);
    if (workspaceWidget)
        return workspaceWidget->viewVisibleGeometry();

    return QRectF();
}

QRectF WorkspaceUnicastReceiver::invokeGetItemRect(const quint64 windowID, const QUrl &url, const ItemRoles role)
{
    WorkspaceWidget *workspaceWidget = WorkspaceHelper::instance()->findWorkspaceByWindowId(windowID);
    if (workspaceWidget)
        return workspaceWidget->itemRect(url, role);

    return QRectF();
}

int WorkspaceUnicastReceiver::invokeGetViewFilter(const quint64 windowID)
{
    return WorkspaceHelper::instance()->getViewFilter(windowID);
}

WorkspaceUnicastReceiver::WorkspaceUnicastReceiver(QObject *parent)
    : QObject(parent)
{
}
