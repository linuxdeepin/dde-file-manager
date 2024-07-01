// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORKSPACEPAGE_H
#define WORKSPACEPAGE_H

#include <dfm-gui/containment.h>

#include <QObject>

DFMGUI_BEGIN_NAMESPACE
class AppletItem;
DFMGUI_END_NAMESPACE

namespace dfmplugin_workspace {
class WorkspacePage : public dfmgui::Containment
{
    Q_OBJECT
public:
    explicit WorkspacePage(QObject *parent = nullptr);

    // signals
    Q_SIGNAL void setViewItem(dfmgui::AppletItem *);

    Q_INVOKABLE quint64 getWinId();

    void initPage();

private:
    void createViewItem();

    QMap<int, dfmgui::Applet *> viewContainmentsMap {};
};
}   // namespace dfmplugin_workspace

#endif   // WORKSPACEPAGE_H
