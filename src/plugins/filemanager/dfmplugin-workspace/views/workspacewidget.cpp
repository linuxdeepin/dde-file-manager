// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "workspacewidget.h"
#include "workspacepage.h"
#include "fileview.h"
#include "events/workspaceeventcaller.h"
#include "utils/workspacehelper.h"
#include "utils/customtopwidgetinterface.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/interfaces/abstractbaseview.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-framework/event/event.h>

#include <DIconButton>
#include <DHorizontalLine>
#include <DGuiApplicationHelper>
#include <dtkwidget_global.h>

#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
#endif

#include <QVBoxLayout>
#include <QStackedLayout>
#include <QKeyEvent>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_workspace;
using namespace GlobalDConfDefines::ConfigPath;
using namespace GlobalDConfDefines::AnimationConfig;

WorkspaceWidget::WorkspaceWidget(QFrame *parent)
    : AbstractFrame(parent)
{
    initializeUi();
}

Global::ViewMode WorkspaceWidget::currentViewMode() const
{
    auto view = currentView();
    if (!view)
        return Global::ViewMode::kNoneMode;

    auto fileView = dynamic_cast<FileView *>(view);
    if (fileView)
        return fileView->currentViewMode();

    return Global::ViewMode::kNoneMode;
}

void WorkspaceWidget::setCurrentUrl(const QUrl &url)
{
    if (currentPageId.isEmpty()) {
        qDebug() << "currentPageId is empty";
        return;
    }

    if (!pages[currentPageId]) {
        qDebug() << "current page is not initialized" << currentPageId;
        return;
    }

    pages[currentPageId]->setUrl(url);
}

QUrl WorkspaceWidget::currentUrl() const
{
    if (currentPageId.isEmpty()) {
        qDebug() << "currentPageId is empty";
        return {};
    }

    if (!pages[currentPageId]) {
        qDebug() << "current page is not initialized" << currentPageId;
        return {};
    }

    return pages[currentPageId]->currentUrl();
}

AbstractBaseView *WorkspaceWidget::currentView() const
{
    if (!pages.contains(currentPageId) || !pages[currentPageId]) {
        qDebug() << "can not find current page" << currentPageId;
        return nullptr;
    }

    return pages[currentPageId]->currentViewPtr();
}

void WorkspaceWidget::setCustomTopWidgetVisible(const QString &scheme, bool visible)
{
    if (currentPageId.isEmpty()) {
        qDebug() << "Cannot find current page, currentPageId is empty";
        return;
    }

    if (!pages[currentPageId]) {
        qDebug() << "Cannot find current page, currentPageId is empty";
        return;
    }

    pages[currentPageId]->setCustomTopWidgetVisible(scheme, visible);
}

bool WorkspaceWidget::getCustomTopWidgetVisible(const QString &scheme)
{
    if (currentPageId.isEmpty()) {
        qDebug() << "Cannot find current page, currentPageId is empty";
        return false;
    }

    if (!pages[currentPageId]) {
        qDebug() << "Cannot find current page, currentPageId is empty";
        return false;
    }

    return pages[currentPageId]->getCustomTopWidgetVisible(scheme);
}

QRectF WorkspaceWidget::viewVisibleGeometry()
{
    FileView *view = dynamic_cast<FileView *>(currentView());
    if (view) {
        QRectF localRect = view->geometry();
        QPoint topLeft(static_cast<int>(localRect.x()), static_cast<int>(localRect.y()));

        QRectF globalRect(view->viewport()->mapToGlobal(topLeft), QSizeF(localRect.width(), localRect.height()));

        return globalRect;
    }

    return {};
}

QRectF WorkspaceWidget::itemRect(const QUrl &url, const Global::ItemRoles role)
{
    FileView *view = dynamic_cast<FileView *>(currentView());
    if (view) {
        QRectF localRect = view->itemRect(url, role);
        QPoint topLeft(static_cast<int>(localRect.x()), static_cast<int>(localRect.y()));

        QRectF globalRect(view->viewport()->mapToGlobal(topLeft), QSizeF(localRect.width(), localRect.height()));

        return globalRect;
    }

    return {};
}

void WorkspaceWidget::createNewPage(const QString &uniqueId)
{
    if (pages.contains(uniqueId)) {
        qDebug() << "pages already contains" << uniqueId;
        return;
    }

    auto page = new WorkspacePage(this);
    pages[uniqueId] = page;
    viewStackLayout->addWidget(page);
    viewStackLayout->setCurrentWidget(page);
    currentPageId = uniqueId;
}

void WorkspaceWidget::removePage(const QString &removedId, const QString &nextId)
{
    if (!pages.contains(removedId) || !pages.contains(nextId)) {
        qDebug() << "pages does not contain" << removedId << nextId;
        return;
    }

    if (currentPageId == removedId) {
        currentPageId = nextId;
        viewStackLayout->setCurrentWidget(pages[currentPageId]);
    }

    auto page = pages[removedId];
    pages.remove(removedId);
    if (page) {
        viewStackLayout->removeWidget(page);
        page->deleteLater();
    }
}

void WorkspaceWidget::setCurrentPage(const QString &uniqueId)
{
    if (pages.contains(uniqueId)) {
        currentPageId = uniqueId;
        viewStackLayout->setCurrentWidget(pages[uniqueId]);
    }
}

void WorkspaceWidget::onRefreshCurrentView()
{
    if (auto view = currentView())
        view->refresh();
}

void WorkspaceWidget::handleViewStateChanged()
{
    if (currentPageId.isEmpty()) {
        qDebug() << "Cannot find current page, currentPageId is empty";
        return;
    }

    if (auto page = pages[currentPageId])
        page->viewStateChanged();
}

void WorkspaceWidget::handleAboutToPlaySplitterAnim(int startValue, int endValue)
{
    if (auto view = dynamic_cast<FileView *>(currentView())) {
        int deltaWidth = startValue - endValue;
        view->aboutToChangeWidth(deltaWidth);
    }
}

void WorkspaceWidget::showEvent(QShowEvent *event)
{
    AbstractFrame::showEvent(event);

    setFocus();
}

void WorkspaceWidget::focusInEvent(QFocusEvent *event)
{
    FileView *view = dynamic_cast<FileView *>(currentView());
    if (view && !view->hasFocus())
        view->setFocus();

    AbstractFrame::focusInEvent(event);
}

// NOTE(zhangs): please ref to: DFileManagerWindow::initRightView (old filemanager)
void WorkspaceWidget::initializeUi()
{
    initViewLayout();
}

void WorkspaceWidget::initViewLayout()
{
    viewStackLayout = new QStackedLayout;
    viewStackLayout->setSpacing(0);
    viewStackLayout->setContentsMargins(0, 0, 0, 0);

    widgetLayout = new QHBoxLayout;
    widgetLayout->addLayout(viewStackLayout, 1);
    widgetLayout->setSpacing(0);
    widgetLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(widgetLayout);
}

void WorkspaceWidget::onCreateNewWindow()
{
    auto fileView = currentView();
    if (!fileView) {
        fmWarning() << "Cannot find view";
        return;
    }

    QList<QUrl> urlList;
    for (const auto &url : fileView->selectedUrlList()) {
        const auto &info = InfoFactory::create<FileInfo>(url);
        if (info && info->canAttributes(CanableInfoType::kCanFetch))
            urlList << url;
    }

    WorkspaceEventCaller::sendOpenWindow(urlList);
}
