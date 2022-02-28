#ifndef WORKSPACEUNICASTRECEIVER_H
#define WORKSPACEUNICASTRECEIVER_H

#include "dfmplugin_workspace_global.h"

#include "services/filemanager/workspace/workspace_defines.h"

#include <QObject>

DPWORKSPACE_BEGIN_NAMESPACE
class WorkspaceUnicastReceiver : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(WorkspaceUnicastReceiver)

public:
    static WorkspaceUnicastReceiver *instance();
    void connectService();

public slots:
    void invokeAddScheme(const QString &scheme);
    bool invokeTabAddable(const quint64 windowID);
    bool invokeAddCustomTopWidget(const DSB_FM_NAMESPACE::Workspace::CustomTopWidgetInfo &info);
    bool invokeGetCustomTopWidgetVisible(const quint64 windowID, const QString &scheme);
    void invokeSetFileViewFilterData(const quint64 windowID, const QUrl &url, const QVariant &data);
    void invokeSetFileViewFilterCallback(const quint64 windowID, const QUrl &url,
                                         const DSB_FM_NAMESPACE::Workspace::FileViewFilterCallback callback);
    void invokeSetWorkspaceMenuScene(const QString &scheme, const QString &scene);

private:
    explicit WorkspaceUnicastReceiver(QObject *parent = nullptr);
};
DPWORKSPACE_END_NAMESPACE
#endif   // WORKSPACEUNICASTRECEIVER_H
