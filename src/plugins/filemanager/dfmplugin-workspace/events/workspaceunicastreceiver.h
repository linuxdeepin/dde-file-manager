#ifndef WORKSPACEUNICASTRECEIVER_H
#define WORKSPACEUNICASTRECEIVER_H

#include "dfmplugin_workspace_global.h"

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

private:
    explicit WorkspaceUnicastReceiver(QObject *parent = nullptr);
};
DPWORKSPACE_END_NAMESPACE
#endif   // WORKSPACEUNICASTRECEIVER_H
