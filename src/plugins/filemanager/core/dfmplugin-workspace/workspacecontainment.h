// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORKSPACECONTAINMENT_H
#define WORKSPACECONTAINMENT_H

#include <dfm-gui/containment.h>

#include <QObject>

DFMGUI_BEGIN_NAMESPACE
class AppletItem;
DFMGUI_END_NAMESPACE

namespace dfmplugin_workspace {
class WorkspacePage;
class WorkspaceContainment : public dfmgui::Containment
{
    Q_OBJECT
public:
    explicit WorkspaceContainment(QObject *parent = nullptr);

    // slots
    Q_SLOT void onCurrentUrlChanged(const QUrl &url);
    Q_SLOT dfmgui::Applet *createPageItem();

    // signals
    Q_SIGNAL void pushNewPage(DFMGUI_NAMESPACE::AppletItem *);

private:
    QMap<int, WorkspacePage *> pageMap {};
    WorkspacePage *currentPage { nullptr };
    quint8 currentPageIndex { 0 };
};
}   // namespace dfmplugin_workspace

#endif   // WORKSPACECONTAINMENT_H
