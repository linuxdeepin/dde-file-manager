// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "workspacepage.h"
#include "fileview.h"
#include "enterdiranimationwidget.h"
#include "events/workspaceeventcaller.h"
#include "utils/workspacehelper.h"
#include "utils/customtopwidgetinterface.h"

#include <dfm-base/utils/universalutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/interfaces/abstractbaseview.h>
#include <dfm-base/dfm_event_defines.h>

#include <dfm-framework/dpf.h>

#include <QVBoxLayout>
#include <QStackedLayout>

DPWORKSPACE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace GlobalDConfDefines::ConfigPath;
using namespace GlobalDConfDefines::AnimationConfig;

WorkspacePage::WorkspacePage(QWidget *parent)
    : QWidget(parent)
{
    initUI();
}

WorkspacePage::~WorkspacePage()
{
    fmInfo() << "Destroying WorkspacePage";

    // 显式清理 views,确保析构顺序可控
    // 注意: 必须先从 layout 移除,再 delete,避免 Qt 父子关系的二次删除
    for (auto it = views.begin(); it != views.end(); ) {
        QString scheme = it.key();
        ViewPtr view = it.value();

        fmDebug() << "Cleaning up view for scheme:" << scheme;

        // 先从 layout 移除
        if (view && view->widget()) {
            viewStackLayout->removeWidget(view->widget());
        }

        // 删除 view (会触发 FileView::~FileView)
        if (view) {
            delete view;
        }

        it = views.erase(it);
    }

    fmDebug() << "WorkspacePage destruction completed";
}

void WorkspacePage::setUrl(const QUrl &url)
{
    fmInfo() << "WorkspacePage setUrl called with url:" << url;
    auto curView = currentViewPtr();
    if (curView) {
        if (UniversalUtils::urlEquals(url, curView->rootUrl())
            && !dpfHookSequence->run("dfmplugin_workspace", "hook_Allow_Repeat_Url", url, curView->rootUrl())) {
            fmDebug() << "setUrl: url is same as current, skip.";
            return;
        }

        bool animEnable = DConfigManager::instance()->value(kAnimationDConfName, kAnimationEnterEnable, true).toBool();
        if (animEnable) {
            fmDebug() << "setUrl: play disappear animation for current view.";
            playDisappearAnimation(curView);
        }

        FileView *view = qobject_cast<FileView *>(curView->widget());
        if (view) {
            fmDebug() << "setUrl: stopWork for FileView.";
            view->stopWork(url);
        }
    }

    auto lastUrl = currentPageUrl;
    currentPageUrl = url;

    // NOTE: In the function `initCustomTopWidgets` the `cd` event may be
    // called causing this function to reentrant!!!
    if (currentPageUrl != url) {
        fmWarning() << "setUrl: currentPageUrl changed during reentrant, abort.";
        return;
    }

    QString scheme { url.scheme() };

    // do not play appear animation when create first view.
    canPlayAppearAnimation = !views.isEmpty();

    if (!views.contains(scheme)) {
        QString error;
        fmDebug() << "setUrl: create new view for scheme:" << scheme;
        ViewPtr fileView = ViewFactory::create<AbstractBaseView>(url, &error);
        if (!fileView) {
            fmWarning() << "Cannot create view for" << url << "Reason:" << error;

            // reset to last view and notify other plugins return to last dir.
            currentPageUrl = lastUrl;
            setCurrentView(currentPageUrl);
            auto windowID = WorkspaceHelper::instance()->windowId(this);
            dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl, windowID, lastUrl);
            return;
        }

        views.insert(scheme, fileView);
        viewStackLayout->addWidget(fileView->widget());
        fmDebug() << "setUrl: new view created and added to stack for scheme:" << scheme;
    }

    setCurrentView(url);
    fmDebug() << "setUrl: setCurrentView called for url:" << url;
}

QUrl WorkspacePage::currentUrl() const
{
    return currentPageUrl;
}

WorkspacePage::ViewPtr WorkspacePage::currentViewPtr()
{
    if (currentViewScheme.isEmpty()) {
        fmWarning() << "Can not find current view, currentViewScheme is empty";
        return nullptr;
    }

    return views[currentViewScheme];
}

void WorkspacePage::viewStateChanged()
{
    fmDebug() << "viewStateChanged called";
    if (!canPlayAppearAnimation)
        return;

    if (!enterAnim)
        return;

    if (!appearAnimDelayTimer) {
        appearAnimDelayTimer = new QTimer(this);
        appearAnimDelayTimer->setInterval(100);
        appearAnimDelayTimer->setSingleShot(true);
        connect(appearAnimDelayTimer, &QTimer::timeout, this, &WorkspacePage::onAnimDelayTimeout);
        fmDebug() << "viewStateChanged: appearAnimDelayTimer created";
    }

    auto view = views[currentViewScheme];
    if (!view)
        return;

    auto contentWidget = view->contentWidget();
    if (!contentWidget)
        contentWidget = view->widget();

    if (!contentWidget) {
        enterAnim->stopAndHide();
        fmWarning() << "viewStateChanged: contentWidget is null, animation stopped";
        return;
    }

    auto globalPos = contentWidget->mapToGlobal(QPoint(0, 0));
    auto localPos = mapFromGlobal(globalPos);
    enterAnim->move(localPos);
    enterAnim->resetWidgetSize(contentWidget->size());

    appearAnimDelayTimer->start();
    fmDebug() << "viewStateChanged: animation timer started";
}

void WorkspacePage::setCustomTopWidgetVisible(const QString &scheme, bool visible)
{
    fmDebug() << "setCustomTopWidgetVisible called for scheme:" << scheme << ", visible:" << visible;
    if (topWidgets.contains(scheme)) {
        topWidgets[scheme]->setVisible(visible);
        fmDebug() << "setCustomTopWidgetVisible: set visible for existing topWidget";
    } else {
        auto interface = WorkspaceHelper::instance()->createTopWidgetByScheme(scheme);
        if (interface) {
            TopWidgetPtr topWidgetPtr = QSharedPointer<QWidget>(interface->create(this));
            if (topWidgetPtr) {
                bool keepTop = interface->isKeepTop();
                int insertIndex = 0;
                if (keepTop) {
                    ++highPriorityTopWidgetsCount;
                } else {
                    insertIndex = highPriorityTopWidgetsCount;
                }
                topLayout->insertWidget(insertIndex, topWidgetPtr.get());
                topWidgets.insert(scheme, topWidgetPtr);
                topWidgetPtr->setVisible(visible);
                fmDebug() << "setCustomTopWidgetVisible: new topWidget created and set visible";
            }
        }
    }
}

bool WorkspacePage::getCustomTopWidgetVisible(const QString &scheme)
{
    if (topWidgets.contains(scheme)) {
        return topWidgets[scheme]->isVisible();
    }
    return false;
}

void WorkspacePage::onAnimDelayTimeout()
{
    if (!enterAnim)
        return;

    auto view = views[currentViewScheme];
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

void WorkspacePage::initUI()
{
    // 创建顶部容器和布局
    topContainer = new QWidget(this);
    topContainer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    topLayout = new QVBoxLayout(topContainer);
    topLayout->setSpacing(0);
    topLayout->setContentsMargins(0, 0, 0, 0);

    // 创建底部容器和视图布局
    viewContainer = new QWidget(this);
    viewContainer->setMinimumHeight(10);
    viewStackLayout = new QStackedLayout(viewContainer);
    viewStackLayout->setSpacing(0);
    viewStackLayout->setContentsMargins(0, 0, 0, 0);

    // 创建主垂直布局
    widgetLayout = new QVBoxLayout;
    widgetLayout->setSpacing(0);
    widgetLayout->setContentsMargins(0, 0, 0, 0);

    // 添加顶部和底部容器到主布局
    widgetLayout->addWidget(topContainer, 0);    // 顶部容器不拉伸
    widgetLayout->addWidget(viewContainer, 1);   // 底部容器占用剩余空间

    setLayout(widgetLayout);
}

void WorkspacePage::initCustomTopWidgets(const QUrl &url)
{
    QString scheme { url.scheme() };
    fmDebug() << "initCustomTopWidgets called for url:" << url << ", scheme:" << scheme;

    // 隐藏其他scheme的topWidget
    for (auto widget : topWidgets.values()) {
        if (topWidgets.value(scheme) != widget)
            widget->hide();
    }

    auto interface = WorkspaceHelper::instance()->createTopWidgetByUrl(url);
    if (interface == nullptr) {
        fmDebug() << "initCustomTopWidgets: no interface for url:" << url;
        return;
    }

    if (!interface->parent())
        interface->setParent(this);

    if (topWidgets.contains(scheme)) {
        bool showUrl { interface->isShowFromUrl(topWidgets[scheme].data(), url) };
        topWidgets[scheme]->setVisible(interface && (showUrl || interface->isKeepShow()));
        fmDebug() << "initCustomTopWidgets: set visible for existing topWidget, visible:" << (interface && (showUrl || interface->isKeepShow()));
    } else {
        TopWidgetPtr topWidgetPtr = QSharedPointer<QWidget>(interface->create());
        if (topWidgetPtr) {
            // 将topWidget添加到顶部布局中
            bool keepTop = interface->isKeepTop();
            int insertIndex = 0;
            if (keepTop) {
                ++highPriorityTopWidgetsCount;
            } else {
                insertIndex = highPriorityTopWidgetsCount;
            }
            topLayout->insertWidget(insertIndex, topWidgetPtr.get());
            topWidgets.insert(scheme, topWidgetPtr);
            topWidgetPtr->setVisible(interface->isShowFromUrl(topWidgets[scheme].data(), url) || interface->isKeepShow());
            fmDebug() << "initCustomTopWidgets: new topWidget created and set visible:" << (interface->isShowFromUrl(topWidgets[scheme].data(), url) || interface->isKeepShow());
        }
    }
}

void WorkspacePage::setCurrentView(const QUrl &url)
{
    fmDebug() << "setCurrentView called for url:" << url;
    currentViewScheme = url.scheme();
    auto view = views[currentViewScheme];
    if (!view) {
        fmWarning() << "setCurrentView: no view found for scheme:" << currentViewScheme;
        return;
    }

    viewStackLayout->setCurrentWidget(view->widget());
    fmDebug() << "setCurrentView: view set in stack for scheme:" << currentViewScheme;

    if (canPlayAppearAnimation && enterAnim)
        enterAnim->raise();

    initCustomTopWidgets(url);
    fmDebug() << "setCurrentView: initCustomTopWidgets called for url:" << url;

    view->setRootUrl(url);
    fmDebug() << "setCurrentView: setRootUrl called for url:" << url;

    if (view->viewState() != AbstractBaseView::ViewState::kViewBusy)
        viewStateChanged();
}

void WorkspacePage::playDisappearAnimation(ViewPtr view)
{
    fmDebug() << "playDisappearAnimation called";
    if (!view) {
        fmWarning() << "playDisappearAnimation: view is null";
        return;
    }

    auto contentWidget = view->contentWidget();
    if (!contentWidget)
        contentWidget = view->widget();

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
        fmDebug() << "playDisappearAnimation: disappear animation played";
    } else {
        fmWarning() << "playDisappearAnimation: contentWidget is null, cannot play animation";
    }
}
