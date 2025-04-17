// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
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

void WorkspacePage::setUrl(const QUrl &url)
{
    auto curView = currentViewPtr();
    if (curView) {
        if (UniversalUtils::urlEquals(url, curView->rootUrl())
            && !dpfHookSequence->run("dfmplugin_workspace", "hook_Allow_Repeat_Url", url, curView->rootUrl()))
            return;

        bool animEnable = DConfigManager::instance()->value(kAnimationDConfName, kAnimationEnterEnable, true).toBool();
        if (animEnable) 
            playDisappearAnimation(curView);

        FileView *view = qobject_cast<FileView *>(curView->widget());
        if (view)
            view->stopWork();
    }

    auto lastUrl = currentPageUrl;
    currentPageUrl = url;

    // NOTE: In the function `initCustomTopWidgets` the `cd` event may be
    // called causing this function to reentrant!!!
    if (currentPageUrl != url)
        return;

    QString scheme { url.scheme() };

    // do not play appear animation when create first view.
    canPlayAppearAnimation = !views.isEmpty();

    if (!views.contains(scheme)) {
        QString error;
        ViewPtr fileView = ViewFactory::create<AbstractBaseView>(url, &error);
        if (!fileView) {
            fmWarning() << "Cannot create view for " << url << "Reason: " << error;

            // reset to last view and notify other plugins return to last dir.
            currentPageUrl = lastUrl;
            setCurrentView(currentPageUrl);
            auto windowID = WorkspaceHelper::instance()->windowId(this);
            dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl, windowID, lastUrl);
            return;
        }

        views.insert(scheme, fileView);
        viewStackLayout->addWidget(fileView->widget());
    }

    setCurrentView(url);
}

QUrl WorkspacePage::currentUrl() const
{
    return currentPageUrl;
}

WorkspacePage::ViewPtr WorkspacePage::currentViewPtr()
{
    if (currentViewScheme.isEmpty()) {
        qDebug() << "Can not find current view, currentViewScheme is empty";
        return nullptr;
    }

    return views[currentViewScheme];
}

void WorkspacePage::viewStateChanged()
{
    if (!canPlayAppearAnimation)
        return;

    if (!enterAnim)
        return;

    if (!appearAnimDelayTimer) {
        appearAnimDelayTimer = new QTimer(this);
        appearAnimDelayTimer->setInterval(100);
        appearAnimDelayTimer->setSingleShot(true);
        connect(appearAnimDelayTimer, &QTimer::timeout, this, &WorkspacePage::onAnimDelayTimeout);
    }

    auto view = views[currentViewScheme];
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

void WorkspacePage::setCustomTopWidgetVisible(const QString &scheme, bool visible)
{
    if (topWidgets.contains(scheme)) {
        topWidgets[scheme]->setVisible(visible);
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

    // 隐藏其他scheme的topWidget
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
        topWidgets[scheme]->setVisible(interface && (showUrl || interface->isKeepShow()));
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
        }
    }
}

void WorkspacePage::setCurrentView(const QUrl &url)
{
    currentViewScheme = url.scheme();
    auto view = views[currentViewScheme];
    if (!view)
        return;

    viewStackLayout->setCurrentWidget(view->widget());

    if (canPlayAppearAnimation && enterAnim)
        enterAnim->raise();

    initCustomTopWidgets(url);

    view->setRootUrl(url);

    if (view->viewState() != AbstractBaseView::ViewState::kViewBusy)
        viewStateChanged();
}

void WorkspacePage::playDisappearAnimation(ViewPtr view)
{
    if (!view)
        return;

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
    }
}
