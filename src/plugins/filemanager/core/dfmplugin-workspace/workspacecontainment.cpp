// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "workspacecontainment.h"
#include "workspacepage.h"

#include <dfm-gui/applet.h>
#include <dfm-gui/appletitem.h>
#include <dfm-gui/appletfactory.h>
#include <dfm-gui/windowmanager.h>

#include <dfm-framework/dpf.h>
#include <dfm-base/dfm_event_defines.h>

using namespace dfmgui;
using namespace dfmplugin_workspace;

WorkspaceContainment::WorkspaceContainment(QObject *parent)
    : Containment(parent)
{
    connect(this, &Applet::currentUrlChanged, this, &WorkspaceContainment::onCurrentUrlChanged);
}

void WorkspaceContainment::onCurrentUrlChanged(const QUrl &url)
{
    if (pageMap.isEmpty()) {
        createPageItem();
        currentPage = pageMap.first();
        currentPage->setCurrentUrl(url);
        currentPage->initPage();
        return;
    }

    if (!currentPage)
        return;

    currentPage->setCurrentUrl(url);
}

Applet *WorkspaceContainment::createPageItem()
{
    QUrl componentUrl = AppletFactory::instance()->pluginComponent("dfmplugin-workspace", "qml/WorkspacePage.qml");
    if (!componentUrl.isValid())
        return nullptr;

    auto pageStack = new WorkspacePage(this);

    pageStack->setComponentUrl(componentUrl);
    this->appendApplet(pageStack);

    pageMap.insert(pageMap.count(), pageStack);

    AppletItem *item = AppletItem::itemForApplet(pageStack);
    Q_EMIT pushNewPage(item);
    return pageStack;
}
