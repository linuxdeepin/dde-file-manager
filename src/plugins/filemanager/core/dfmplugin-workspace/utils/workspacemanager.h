// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORKSPACEMANAGER_H
#define WORKSPACEMANAGER_H

#include <dfm-gui/applet.h>

#include <QObject>

namespace dfmplugin_workspace {

inline constexpr char kViewItemApplet[] { "org.dfm.workspace.view" };
inline constexpr char kAppletUrl[] { "org.dfm.workspace" };

class WorkspaceContainment;
class WorkspaceManager : public QObject
{
    Q_OBJECT
public:
    static WorkspaceManager *instance();

    static dfmgui::Applet *createViewApplet(const QUrl &url, dfmgui::Containment *parent, QString *errorString);
    static dfmgui::Applet *createWorkspaceApplet(const QString &id, dfmgui::Containment *parent, QString *errorString);

    void registerViewItem(const QString &scheme);
    void registerWorkspaceCreator();

    Q_SLOT void onAddNewTab(quint64 windowId, const QUrl &url);

private:
    explicit WorkspaceManager(QObject *parent = nullptr);
    WorkspaceContainment *findWorkspace(quint64 winId);

    inline QString viewAppletWithScheme(const QString &scheme) { return QString(kViewItemApplet).append(".") + scheme; };

    QMap<quint64, WorkspaceContainment *> workspaceMap {};
};

}   // namespace dfmplugin_workspace

#endif   // WORKSPACEMANAGER_H
