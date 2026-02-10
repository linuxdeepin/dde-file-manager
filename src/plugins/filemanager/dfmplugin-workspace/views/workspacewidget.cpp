// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
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
    fmInfo() << "WorkspaceWidget setCurrentUrl called with url:" << url;
    if (currentPageId.isEmpty()) {
        fmWarning() << "setCurrentUrl: currentPageId is empty";
        return;
    }

    if (!pages[currentPageId]) {
        fmWarning() << "setCurrentUrl: current page is not initialized, currentPageId:" << currentPageId;
        return;
    }

    pages[currentPageId]->setUrl(url);
    fmDebug() << "setCurrentUrl: URL set for page:" << currentPageId;
}

QUrl WorkspaceWidget::currentUrl() const
{
    if (currentPageId.isEmpty()) {
        fmWarning() << "currentUrl: currentPageId is empty";
        return {};
    }

    if (!pages[currentPageId]) {
        fmWarning() << "currentUrl: current page is not initialized, currentPageId:" << currentPageId;
        return {};
    }

    return pages[currentPageId]->currentUrl();
}

AbstractBaseView *WorkspaceWidget::currentView() const
{
    if (!pages.contains(currentPageId) || !pages[currentPageId]) {
        fmWarning() << "currentView: cannot find current page, currentPageId:" << currentPageId;
        return nullptr;
    }

    return pages[currentPageId]->currentViewPtr();
}

void WorkspaceWidget::setCustomTopWidgetVisible(const QString &scheme, bool visible)
{
    fmDebug() << "setCustomTopWidgetVisible called for scheme:" << scheme << ", visible:" << visible;
    if (currentPageId.isEmpty()) {
        fmWarning() << "setCustomTopWidgetVisible: Cannot find current page, currentPageId is empty";
        return;
    }

    if (!pages[currentPageId]) {
        fmWarning() << "setCustomTopWidgetVisible: Cannot find current page, currentPageId is empty";
        return;
    }

    pages[currentPageId]->setCustomTopWidgetVisible(scheme, visible);
    fmDebug() << "setCustomTopWidgetVisible: visibility set for scheme:" << scheme;
}

bool WorkspaceWidget::getCustomTopWidgetVisible(const QString &scheme)
{
    fmDebug() << "getCustomTopWidgetVisible called for scheme:" << scheme;
    if (currentPageId.isEmpty()) {
        fmWarning() << "getCustomTopWidgetVisible: Cannot find current page, currentPageId is empty";
        return false;
    }

    if (!pages[currentPageId]) {
        fmWarning() << "getCustomTopWidgetVisible: Cannot find current page, currentPageId is empty";
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
    fmInfo() << "createNewPage called with uniqueId:" << uniqueId;
    if (pages.contains(uniqueId)) {
        fmWarning() << "createNewPage: pages already contains" << uniqueId;
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
    fmInfo() << "removePage called, removedId:" << removedId << ", nextId:" << nextId;
    if (!pages.contains(removedId) || !pages.contains(nextId)) {
        fmWarning() << "removePage: pages does not contain" << removedId << "or" << nextId;
        return;
    }

    if (currentPageId == removedId) {
        currentPageId = nextId;
        viewStackLayout->setCurrentWidget(pages[currentPageId]);
        fmDebug() << "removePage: current page changed to:" << nextId;
    }

    auto page = pages[removedId];
    pages.remove(removedId);
    if (page) {
        viewStackLayout->removeWidget(page);
        page->deleteLater();
        fmDebug() << "removePage: page removed and scheduled for deletion:" << removedId;
    }
}

void WorkspaceWidget::setCurrentPage(const QString &uniqueId)
{
    fmDebug() << "setCurrentPage called with uniqueId:" << uniqueId;
    if (pages.contains(uniqueId)) {
        currentPageId = uniqueId;
        viewStackLayout->setCurrentWidget(pages[uniqueId]);
        fmDebug() << "setCurrentPage: current page set to:" << uniqueId;
    } else {
        fmWarning() << "setCurrentPage: page not found:" << uniqueId;
    }
}

void WorkspaceWidget::onRefreshCurrentView()
{
    fmDebug() << "onRefreshCurrentView called";
    if (auto view = currentView()) {
        view->refresh();
        fmDebug() << "onRefreshCurrentView: view refreshed";
    } else {
        fmWarning() << "onRefreshCurrentView: no current view to refresh";
    }
}

void WorkspaceWidget::handleViewStateChanged()
{
    fmDebug() << "handleViewStateChanged called";
    if (currentPageId.isEmpty()) {
        fmWarning() << "handleViewStateChanged: Cannot find current page, currentPageId is empty";
        return;
    }

    if (auto page = pages[currentPageId]) {
        page->viewStateChanged();
        fmDebug() << "handleViewStateChanged: view state changed for page:" << currentPageId;
    } else {
        fmWarning() << "handleViewStateChanged: current page is null";
    }
}

void WorkspaceWidget::handleAboutToPlaySplitterAnim(int startValue, int endValue)
{
    fmDebug() << "handleAboutToPlaySplitterAnim called, startValue:" << startValue << ", endValue:" << endValue;
    if (auto view = dynamic_cast<FileView *>(currentView())) {
        int deltaWidth = startValue - endValue;
        view->aboutToChangeWidth(deltaWidth);
        fmDebug() << "handleAboutToPlaySplitterAnim: width change applied, delta:" << deltaWidth;
    } else {
        fmWarning() << "handleAboutToPlaySplitterAnim: current view is not FileView";
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
    fmDebug() << "Initializing WorkspaceWidget UI";
    initViewLayout();
}

void WorkspaceWidget::initViewLayout()
{
    fmDebug() << "initViewLayout called";
    viewStackLayout = new QStackedLayout;
    viewStackLayout->setSpacing(0);
    viewStackLayout->setContentsMargins(0, 0, 0, 0);

    widgetLayout = new QHBoxLayout;
    widgetLayout->addLayout(viewStackLayout, 1);
    widgetLayout->setSpacing(0);
    widgetLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(widgetLayout);
    fmDebug() << "initViewLayout: layout initialized";
}

void WorkspaceWidget::onCreateNewWindow()
{
    fmInfo() << "onCreateNewWindow called";
    auto fileView = currentView();
    if (!fileView) {
        fmWarning() << "onCreateNewWindow: Cannot find view";
        return;
    }

    QList<QUrl> urlList;
    for (const auto &url : fileView->selectedUrlList()) {
        const auto &info = InfoFactory::create<FileInfo>(url);
        if (info && info->canAttributes(CanableInfoType::kCanFetch))
            urlList << url;
    }

    fmDebug() << "onCreateNewWindow: selected URLs count:" << urlList.count();

    if (urlList.count() > DFMGLOBAL_NAMESPACE::kOpenNewWindowMaxCount) {
        fmWarning() << "onCreateNewWindow: Too much windows to open is not supported! Count:" << urlList.count();
        return;
    }

    WorkspaceEventCaller::sendOpenWindow(urlList);
    fmDebug() << "onCreateNewWindow: open window event sent for" << urlList.count() << "URLs";
}
