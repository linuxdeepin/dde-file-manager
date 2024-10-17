// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "workspacewidget.h"
#include "fileview.h"
#include "enterdiranimationwidget.h"
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

WorkspaceWidget::WorkspaceWidget(QFrame *parent)
    : AbstractFrame(parent)
{
    initializeUi();
}

WorkspaceWidget::ViewPtr WorkspaceWidget::currentViewPtr() const
{
    auto scheme = currentUrl().scheme();
    return views.value(scheme);
}

Global::ViewMode WorkspaceWidget::currentViewMode() const
{
    auto scheme = currentUrl().scheme();
    auto view = views.value(scheme);
    if (!view)
        return Global::ViewMode::kNoneMode;

    auto fileView = dynamic_cast<FileView *>(view);
    if (fileView)
        return fileView->currentViewMode();

    return Global::ViewMode::kNoneMode;
}

void WorkspaceWidget::setCurrentUrl(const QUrl &url)
{
    auto curView = currentViewPtr();
    if (curView) {
        if (UniversalUtils::urlEquals(url, curView->rootUrl())
            && !dpfHookSequence->run("dfmplugin_workspace", "hook_Allow_Repeat_Url", url, curView->rootUrl()))
            return;

        bool animEnable = DConfigManager::instance()->value(kAnimationDConfName, kAnimationEnable, true).toBool();
        if (animEnable) {
            auto contentWidget = curView->contentWidget();
            if (!contentWidget)
                contentWidget = curView->widget();

            if (contentWidget) {
                if (!enterAnim)
                    enterAnim = new EnterDirAnimationWidget(this);

                auto globalPos = contentWidget->mapToGlobal(QPoint(0, 0));
                auto localPos = mapFromGlobal(globalPos);
                enterAnim->move(localPos);
                enterAnim->resetWidgetSize(contentWidget->size());

                QPixmap preDirPix = contentWidget->grab();
                enterAnim->setDisappearPixmap(preDirPix);
                enterAnim->show();
                enterAnim->raise();

                enterAnim->playDisappear();
            }
        }

        FileView *view = qobject_cast<FileView *>(curView->widget());
        if (view)
            view->stopWork();
    }

    auto lastUrl = workspaceUrl;
    workspaceUrl = url;

    // NOTE: In the function `initCustomTopWidgets` the `cd` event may be
    // called causing this function to reentrant!!!
    if (workspaceUrl != url)
        return;

    QString scheme { url.scheme() };

    // do not paly appear animation when create first view.
    canPlayAppearAnimation = !views.isEmpty();

    if (!views.contains(scheme)) {
        QString error;
        ViewPtr fileView = ViewFactory::create<AbstractBaseView>(url, &error);
        if (!fileView) {
            fmWarning() << "Cannot create view for " << url << "Reason: " << error;

            // reset to last view and notify other plugins return to last dir.
            workspaceUrl = lastUrl;
            setCurrentView(workspaceUrl);
            auto windowID = WorkspaceHelper::instance()->windowId(this);
            dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl, windowID, lastUrl);
            return;
        }
#ifdef ENABLE_TESTING
        dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                             qobject_cast<QWidget *>(fileView->widget()), AcName::kAcFileView);
#endif

        views.insert(url.scheme(), fileView);
        viewStackLayout->addWidget(fileView->widget());
    }

    setCurrentView(url);
}

QUrl WorkspaceWidget::currentUrl() const
{
    return workspaceUrl;
}

AbstractBaseView *WorkspaceWidget::currentView()
{
    auto scheme = currentUrl().scheme();
    return views.value(scheme);
}

void WorkspaceWidget::setCustomTopWidgetVisible(const QString &scheme, bool visible)
{
    if (topWidgets.contains(scheme)) {
        topWidgets[scheme]->setVisible(visible);
    } else {
        auto interface = WorkspaceHelper::instance()->createTopWidgetByScheme(scheme);
        if (interface) {
            TopWidgetPtr topWidgetPtr = QSharedPointer<QWidget>(interface->create(this));
            if (topWidgetPtr) {
                widgetLayout->insertWidget(0, topWidgetPtr.get());
                topWidgets.insert(scheme, topWidgetPtr);
                topWidgetPtr->setVisible(visible);
            }
        }
    }
}

bool WorkspaceWidget::getCustomTopWidgetVisible(const QString &scheme)
{
    if (topWidgets.contains(scheme)) {
        return topWidgets[scheme]->isVisible();
    }
    return false;
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

void WorkspaceWidget::onRefreshCurrentView()
{
    if (auto view = currentView())
        view->refresh();
}

void WorkspaceWidget::handleViewStateChanged()
{
    if (!canPlayAppearAnimation)
        return;

    if (!enterAnim)
        return;

    if (!appearAnimDelayTimer) {
        appearAnimDelayTimer = new QTimer(this);
        appearAnimDelayTimer->setInterval(100);
        appearAnimDelayTimer->setSingleShot(true);
        connect(appearAnimDelayTimer, &QTimer::timeout, this, &WorkspaceWidget::onAnimDelayTimeout);
    }

    auto view = views[workspaceUrl.scheme()];
    if (!view)
        return;

    auto contentWidget = view->contentWidget();
    if (!contentWidget)
        contentWidget = view->widget();

    if (!contentWidget) {
        enterAnim->stopAndHide();
        return;
    }

    auto globalPos = contentWidget->mapToGlobal(QPoint(0, 0));
    auto localPos = mapFromGlobal(globalPos);
    enterAnim->move(localPos);
    enterAnim->resetWidgetSize(contentWidget->size());

    appearAnimDelayTimer->start();
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

void WorkspaceWidget::onAnimDelayTimeout()
{
    auto view = views[workspaceUrl.scheme()];
    if (!enterAnim)
        return;

    if (!view || view->viewState() != AbstractBaseView::ViewState::kViewIdle) {
        appearAnimDelayTimer->start();
        return;
    }

    auto contentWidget = view->contentWidget();
    if (!contentWidget)
        contentWidget = view->widget();

    if (!contentWidget) {
        enterAnim->stopAndHide();
        return;
    }

    QPixmap curDirPix = contentWidget->grab();
    if (curDirPix.isNull()) {
        enterAnim->stopAndHide();
        return;
    }

    auto globalPos = contentWidget->mapToGlobal(QPoint(0, 0));
    auto localPos = mapFromGlobal(globalPos);

    enterAnim->resize(contentWidget->size());
    enterAnim->move(localPos);

    enterAnim->setAppearPixmap(curDirPix);
    enterAnim->playAppear();
}

void WorkspaceWidget::initViewLayout()
{
    viewStackLayout = new QStackedLayout;
    viewStackLayout->setSpacing(0);
    viewStackLayout->setContentsMargins(0, 0, 0, 0);

    widgetLayout = new QVBoxLayout;
    widgetLayout->addLayout(viewStackLayout, 1);
    widgetLayout->setSpacing(0);
    widgetLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(widgetLayout);
}

void WorkspaceWidget::onCreateNewWindow()
{
    ViewPtr fileView = views[workspaceUrl.scheme()];
    if (!fileView) {
        fmWarning() << "Cannot find view by url: " << workspaceUrl;
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

void WorkspaceWidget::initCustomTopWidgets(const QUrl &url)
{
    QString scheme { url.scheme() };

    for (auto widget : topWidgets.values()) {
        if (topWidgets.value(scheme) != widget)
            widget->hide();
    }

    auto interface = WorkspaceHelper::instance()->createTopWidgetByUrl(url);
    if (interface == nullptr)
        return;

    if (!interface->parent())
        interface->setParent(this);

    if (topWidgets.contains(scheme)) {
        bool showUrl { interface->isShowFromUrl(topWidgets[scheme].data(), url) };
        fmDebug() << interface->isKeepShow() << showUrl;
        topWidgets[scheme]->setVisible(interface && (showUrl || interface->isKeepShow()));
        fmDebug() << topWidgets[scheme]->contentsMargins();
    } else {
        TopWidgetPtr topWidgetPtr = QSharedPointer<QWidget>(interface->create());
        if (topWidgetPtr) {
            widgetLayout->insertWidget(0, topWidgetPtr.get());
            topWidgets.insert(scheme, topWidgetPtr);
            topWidgetPtr->setVisible(interface->isShowFromUrl(topWidgets[scheme].data(), url) || interface->isKeepShow());
        }
    }
}

void WorkspaceWidget::setCurrentView(const QUrl &url)
{
    auto view = views[url.scheme()];
    if (!view)
        return;

    viewStackLayout->setCurrentWidget(view->widget());

    if (canPlayAppearAnimation && enterAnim)
        enterAnim->raise();

    initCustomTopWidgets(url);

    view->setRootUrl(url);

    if (view->viewState() != AbstractBaseView::ViewState::kViewBusy)
        handleViewStateChanged();
}
