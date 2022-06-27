#ifndef WORKSPACEUNICASTRECEIVER_H
#define WORKSPACEUNICASTRECEIVER_H

#include "dfmplugin_workspace_global.h"

#include "dfm-base/dfm_global_defines.h"

#include "services/filemanager/workspace/workspace_defines.h"

#include <QObject>

namespace dfmplugin_workspace {
class WorkspaceUnicastReceiver : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(WorkspaceUnicastReceiver)

public:
    static WorkspaceUnicastReceiver *instance();
    void connectService();

public slots:
    void invokeAddScheme(const QString &scheme);
    bool invokeSchemeViewIsFileView(const QString &scheme);
    bool invokeTabAddable(const quint64 windowID);
    bool invokeAddCustomTopWidget(const DSB_FM_NAMESPACE::Workspace::CustomTopWidgetInfo &info);
    bool invokeGetCustomTopWidgetVisible(const quint64 windowID, const QString &scheme);
    void invokeSetFileViewFilterData(const quint64 windowID, const QUrl &url, const QVariant &data);
    void invokeSetFileViewFilterCallback(const quint64 windowID, const QUrl &url,
                                         const DSB_FM_NAMESPACE::Workspace::FileViewFilterCallback callback);
    void invokeSetWorkspaceMenuScene(const QString &scheme, const QString &scene);
    QString invokeFindMenuScene(const QString &scheme);
    void invokeSetDefaultViewMode(const QString &scheme, const DFMBASE_NAMESPACE::Global::ViewMode mode);
    DFMBASE_NAMESPACE::Global::ViewMode invokeGetDefaultViewMode(const QString &scheme);
    DFMBASE_NAMESPACE::Global::ViewMode invokeCurrentViewMode(const quint64 windowID);
    bool invokeRegisterFileViewRoutePrehanlder(const QString &scheme, const DSB_FM_NAMESPACE::Workspace::FileViewRoutePrehaldler &prehandler);
    QList<QUrl> invokeSelectedUrls(const quint64 windowID);

    QRectF invokeGetViewVisibleGeometry(const quint64 windowID);
    QRectF invokeGetItemRect(const quint64 windowID, const QUrl &url, const DFMGLOBAL_NAMESPACE::ItemRoles role);
    int invokeGetViewFilter(const quint64 windowID);

private:
    explicit WorkspaceUnicastReceiver(QObject *parent = nullptr);
};
}
#endif   // WORKSPACEUNICASTRECEIVER_H
