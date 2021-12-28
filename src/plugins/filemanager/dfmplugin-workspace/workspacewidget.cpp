/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "workspacewidget.h"
#include "fileview.h"

#include "services/filemanager/windows/windowsservice.h"

#include "dfm-base/interfaces/abstractbaseview.h"
#include "dfm-base/base/schemefactory.h"

#include <QVBoxLayout>
#include <QStackedLayout>
#include <QKeyEvent>

DSB_FM_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPWORKSPACE_USE_NAMESPACE

WorkspaceWidget::WorkspaceWidget(QFrame *parent)
    : AbstractFrame(parent)
{
    initializeUi();
    initConnect();
}

// NOTE(zhangs): please ref to : DFileManagerWindowPrivate::cdForTab (old filemanager)
void WorkspaceWidget::setCurrentUrl(const QUrl &url)
{
    workspaceUrl = url;
    // NOTE(zhangs): follw is temp code, need tab!
    if (fileView == nullptr) {
        QString error;
        fileView = ViewFactory::create<AbstractBaseView>(url, &error);
        if (!fileView) {
            qWarning() << "Cannot create view for " << url << "Reason: " << error;
            return;
        }
        viewStackLayout->addWidget(dynamic_cast<QWidget *>(fileView.get()));
        return;
    }
    fileView->setRootUrl(url);
}

QUrl WorkspaceWidget::currentUrl() const
{

    return workspaceUrl;
}

void WorkspaceWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->modifiers()) {
    case Qt::ControlModifier:
        switch (event->key()) {
        case Qt::Key_N:
            handleCtrlN();
            return;
        default:
            break;
        }
        break;
    }
    AbstractFrame::keyPressEvent(event);
}

void WorkspaceWidget::showEvent(QShowEvent *event)
{
    AbstractFrame::showEvent(event);

    setFocus();
}

// NOTE(zhangs): please ref to: DFileManagerWindow::initRightView (old filemanager)
void WorkspaceWidget::initializeUi()
{
    initTabBar();
    initViewLayout();
    // TODO(zhangs): initRenameBarState

    widgetLayout = new QVBoxLayout;
    widgetLayout->addLayout(viewStackLayout, 1);
    widgetLayout->setSpacing(0);
    widgetLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(widgetLayout);
}

void WorkspaceWidget::initConnect()
{
}

void WorkspaceWidget::initTabBar()
{
}

void WorkspaceWidget::initViewLayout()
{
    viewStackLayout = new QStackedLayout;
    viewStackLayout->setSpacing(0);
    viewStackLayout->setContentsMargins(0, 0, 0, 0);
}

void WorkspaceWidget::handleCtrlN()
{
    QList<QUrl> urls { fileView->selectedUrlList() };
    auto &ctx = dpfInstance.serviceContext();
    auto windowService = ctx.service<WindowsService>(WindowsService::name());
    if (urls.isEmpty()) {
        windowService->showWindow(QUrl(), true);
    } else {
        for (const QUrl &url : urls) {
            windowService->showWindow(url, true);
        }
    }
}
