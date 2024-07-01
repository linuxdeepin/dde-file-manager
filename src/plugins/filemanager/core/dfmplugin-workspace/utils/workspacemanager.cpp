// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "workspacemanager.h"
#include "fileviewcontainment.h"
#include "workspacecontainment.h"
#include "dfmplugin_workspace_global.h"

#include <dfm-gui/windowmanager.h>
#include <dfm-gui/appletfactory.h>
#include <dfm-gui/containment.h>

using namespace dfmgui;
using namespace dfmplugin_workspace;

WorkspaceManager *WorkspaceManager::instance()
{
    static WorkspaceManager ins;
    return &ins;
}

Applet *WorkspaceManager::createViewApplet(const QUrl &url, Containment *parent, QString *errorString)
{
    Q_UNUSED(url)

    auto view = new FileViewContainment(parent);
    return view;
}

Applet *WorkspaceManager::createWorkspaceApplet(const QString &id, dfmgui::Containment *parent, QString *errorString)
{
    if (kWorkspaceUri == id) {
        Q_ASSERT_X(parent && parent->flags().testFlag(dfmgui::Applet::kPanel),
                   "Create workspace applet", "Parent must based on panel");

        auto containment = new WorkspaceContainment(parent);
        return containment;
    }
    return nullptr;
}

void WorkspaceManager::registerViewItem(const QString &scheme)
{
    QString errStr {};
    bool ret = ViewAppletFactory::instance()->regCreator("dfmplugin-workspace", "qml/FileView.qml",
                                                         scheme, &WorkspaceManager::createViewApplet, &errStr);

    if (!ret && !errStr.isEmpty()) {
        qWarning() << "register view item creator error. Error msg: " << errStr;
    }
}

void WorkspaceManager::registerWorkspaceCreator()
{
    // register qml component
    QString errorString;
    bool regSuccess = dfmgui::AppletFactory::instance()->regCreator(
            kWorkspaceUri, &createWorkspaceApplet, &errorString);
    if (!regSuccess) {
        fmWarning() << QString("Register applet %1 failed.").arg(kWorkspaceUri) << errorString;
    }
}

void WorkspaceManager::onAddNewTab(quint64 windowId, const QUrl &url)
{
    auto workspace = findWorkspace(windowId);
    if (!workspace)
        return;

    workspace->createPageItem();
    workspace->setCurrentUrl(url);
}

WorkspaceManager::WorkspaceManager(QObject *parent)
    : QObject(parent)
{
}

// QTC_TEMP
WorkspaceContainment *WorkspaceManager::findWorkspace(quint64 winId)
{
    if (workspaceMap.contains(winId))
        return workspaceMap.value(winId);

    return nullptr;
}
