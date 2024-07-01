// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "workspacepage.h"
#include "fileviewcontainment.h"

#include <dfm-gui/panel.h>
#include <dfm-gui/appletitem.h>
#include <dfm-gui/appletfactory.h>

using namespace dfmgui;
using namespace dfmplugin_workspace;

WorkspacePage::WorkspacePage(QObject *parent)
    : Containment(parent)
{
}

quint64 WorkspacePage::getWinId()
{
    return this->panel()->windId();
}

void WorkspacePage::initPage()
{
    createViewItem();
}

void WorkspacePage::createViewItem()
{
    QString errStr {};

    auto viewItem = ViewAppletFactory::instance()->create(currentUrl(), this, &errStr);
    connect(this, &WorkspacePage::currentUrlChanged, viewItem, [viewItem](const QUrl &url) {
        viewItem->setCurrentUrl(url);
    });

    this->appendApplet(viewItem);

    AppletItem *item = AppletItem::itemForApplet(viewItem);
    Q_EMIT setViewItem(item);

    viewItem->setCurrentUrl(currentUrl());

    viewContainmentsMap.insert(0, viewItem);
}
