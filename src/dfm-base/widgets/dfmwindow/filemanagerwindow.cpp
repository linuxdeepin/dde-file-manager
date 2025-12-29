// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/filemanagerwindow_p.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/utils/windowutils.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <DPlatformTheme>
#include <DSizeMode>
#include <DIconTheme>

#include <QUrl>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QHideEvent>
#include <QApplication>
#include <QScreen>
#include <QWindow>

using namespace GlobalDConfDefines::ConfigPath;
using namespace GlobalDConfDefines::AnimationConfig;
using namespace GlobalDConfDefines::BaseConfig;

namespace dfmbase {

enum NetWmState {
    kNetWmStateAbove = 0x1,
    kNetWmStateBelow = 0x2,
    kNetWmStateFullScreen = 0x4,
    kNetWmStateMaximizedHorz = 0x8,
    kNetWmStateMaximizedVert = 0x10,
    kNetWmStateModal = 0x20,
    kNetWmStateStaysOnTop = 0x40,
    kNetWmStateDemandsAttention = 0x80
};
Q_DECLARE_FLAGS(NetWmStates, NetWmState)

/*!
 * \class FileManagerWindowPrivate
 * \brief
 */

FileManagerWindowPrivate::FileManagerWindowPrivate(const QUrl &url, FileManagerWindow *qq)
    : QObject(nullptr),
      q(qq),
      currentUrl(url)
{
    q->setWindowTitle(currentUrl.fileName());
    q->setAttribute(Qt::WA_TranslucentBackground);
    q->setAutoFillBackground(false);
}

bool FileManagerWindowPrivate::processKeyPressEvent(QKeyEvent *event)
{
    switch (event->modifiers()) {
    case Qt::NoModifier: {
        switch (event->key()) {
        case Qt::Key_F5:
            emit q->reqRefresh();
            return true;
        }
        break;
    }
    case Qt::ControlModifier: {
        switch (event->key()) {
        case Qt::Key_Tab:
            emit q->reqActivateNextTab();
            return true;
        case Qt::Key_Backtab:
            emit q->reqActivatePreviousTab();
            return true;
        case Qt::Key_F:
            emit q->reqSearchCtrlF();
            return true;
        case Qt::Key_L:
            emit q->reqSearchCtrlL();
            return true;
        case Qt::Key_Left:
            emit q->reqBack();
            return true;
        case Qt::Key_Right:
            emit q->reqForward();
            return true;
        case Qt::Key_W:
            emit q->reqCloseCurrentTab();
            return true;
        case Qt::Key_T:
            emit q->reqCreateTab();
            return true;
        case Qt::Key_N:
            emit q->reqCreateWindow();
            return true;
        case Qt::Key_1:
        case Qt::Key_2:
        case Qt::Key_3:
        case Qt::Key_4:
        case Qt::Key_5:
        case Qt::Key_6:
        case Qt::Key_7:
        case Qt::Key_8:
        case Qt::Key_9:
            emit q->reqTriggerActionByIndex(event->key() - Qt::Key_1);
            return true;
        }
        break;
    }
    case Qt::AltModifier:
    case Qt::AltModifier | Qt::KeypadModifier:
        if (event->key() >= Qt::Key_1 && event->key() <= Qt::Key_8) {
            emit q->reqActivateTabByIndex(event->key() - Qt::Key_1);
            return true;
        }

        switch (event->key()) {
        case Qt::Key_Left:
            emit q->reqBack();
            return true;
        case Qt::Key_Right:
            emit q->reqForward();
            return true;
        }
        break;
    case Qt::ControlModifier | Qt::ShiftModifier:
        if (event->key() == Qt::Key_Question) {
            emit q->reqShowHotkeyHelp();
            return true;
        } else if (event->key() == Qt::Key_Backtab) {
            emit q->reqActivatePreviousTab();
            return true;
        }
        break;
    }
    return false;
}

int FileManagerWindowPrivate::loadSidebarState() const
{
    const QVariantMap &state = Application::appObtuselySetting()->value("WindowManager", "SplitterState").toMap();
    auto pos = state.value("sidebar", kDefaultLeftWidth).toInt();
    if (pos >= kMinimumLeftWidth && pos <= kMaximumLeftWidth)
        return pos;
    return kDefaultLeftWidth;
}

int FileManagerWindowPrivate::splitterPosition() const
{
    if (!splitter || splitter->sizes().isEmpty())
        return kDefaultLeftWidth;
    return splitter->sizes().at(0);
}

void FileManagerWindowPrivate::setSplitterPosition(int pos)
{
    if (splitter)
        splitter->setSizes({ pos, splitter->width() - pos - splitter->handleWidth() });
}

void FileManagerWindowPrivate::resetTitleBarSize()
{
    if (iconLabel) {
        QSize size(DSizeModeHelper::element(24, 32), DSizeModeHelper::element(24, 32));
        iconLabel->setIconSize(size);
    }
    if (expandButton) {
        QSize size(16, 16);
        expandButton->setIconSize(size);
        expandButton->setFixedSize(30, 30);
    }
    if (iconArea) {
        QSize size(DSizeModeHelper::element(66, 95), DSizeModeHelper::element(40, 50));
        iconArea->setFixedSize(size);
    }
}

void FileManagerWindowPrivate::resetSideBarSize()
{
    if (sideBar) {
        sideBar->setMaximumWidth(kMaximumLeftWidth);
        sideBar->setMinimumWidth(kMinimumLeftWidth);
        if (splitter)
            lastSidebarExpandedPostion = splitter->sizes().at(0);
    }
}

void FileManagerWindowPrivate::animateSplitter(bool expanded)
{
    if (!sideBar || !sidebarSep)
        return;

    if (!isAnimationEnabled()) {
        if (expanded)
            showSideBar();
        else
            hideSideBar();
        return;
    }

    bool lastAnimationStopped = setupAnimation(expanded);
    handleWindowResize(expanded);

    int start = expanded ? 1 : splitter->sizes().at(0);
    int end = expanded ? lastSidebarExpandedPostion : 1;

    if (!expanded && !lastAnimationStopped)
        lastSidebarExpandedPostion = splitter->sizes().at(0);

    configureAnimation(start, end);
    connectAnimationSignals();

    Q_EMIT q->aboutToPlaySplitterAnimation(start, end);
    curSplitterAnimation->start();
}

bool FileManagerWindowPrivate::isAnimationEnabled() const
{
    return DConfigManager::instance()->value(kAnimationDConfName, kAnimationSidebarEnable, true).toBool();
}

bool FileManagerWindowPrivate::isDetailViewAnimationEnabled() const
{
    return DConfigManager::instance()->value(kAnimationDConfName, kAnimationDetailviewEnable, true).toBool();
}

bool FileManagerWindowPrivate::setupAnimation(bool expanded)
{
    sideBar->setVisible(true);
    sideBar->setMinimumWidth(1);

    bool lastAnimationStopped = false;
    if (curSplitterAnimation && curSplitterAnimation->state() == QAbstractAnimation::Running) {
        lastAnimationStopped = true;
        curSplitterAnimation->stop();
        delete curSplitterAnimation;
        curSplitterAnimation = nullptr;
    }

    return lastAnimationStopped;
}

void FileManagerWindowPrivate::handleWindowResize(bool expanded)
{
    if (expanded) {
        int currentWindowWidth = q->width();
        int requiredWidth = kMinimumRightWidth + lastSidebarExpandedPostion + splitter->handleWidth();
        if (currentWindowWidth < requiredWidth) {
            int duration = DConfigManager::instance()->value(kAnimationDConfName, kAnimationSidebarDuration, 366).toInt();
            auto curve = static_cast<QEasingCurve::Type>(DConfigManager::instance()->value(kAnimationDConfName, kAnimationSidebarCurve, 22).toInt());

            auto *windowAnimation = new QPropertyAnimation(q, "geometry");
            windowAnimation->setDuration(duration);
            windowAnimation->setStartValue(q->geometry());
            windowAnimation->setEndValue(QRect(q->x(), q->y(), requiredWidth, q->height()));
            windowAnimation->setEasingCurve(curve);
            windowAnimation->start(QAbstractAnimation::DeleteWhenStopped);
        }
        sidebarSep->setVisible(true);
        sideBarAutoVisible = true;
    }
}

void FileManagerWindowPrivate::configureAnimation(int start, int end)
{
    int duration = DConfigManager::instance()->value(kAnimationDConfName, kAnimationSidebarDuration, 366).toInt();
    auto curve = static_cast<QEasingCurve::Type>(DConfigManager::instance()->value(kAnimationDConfName, kAnimationSidebarCurve, 22).toInt());

    curSplitterAnimation = new QPropertyAnimation(splitter, "splitPosition");
    curSplitterAnimation->setEasingCurve(curve);
    curSplitterAnimation->setDuration(duration);
    curSplitterAnimation->setStartValue(start);
    curSplitterAnimation->setEndValue(end);
}

void FileManagerWindowPrivate::connectAnimationSignals()
{
    connect(curSplitterAnimation, &QPropertyAnimation::finished, this, [this]() {
        bool expanded = curSplitterAnimation->endValue().toInt() > 1;
        if (expanded)
            resetSideBarSize();

        sideBar->setVisible(expanded);
        sidebarSep->setVisible(expanded);

        // 动画完成后更新位置
        updateSideBarSeparatorPosition();

        delete curSplitterAnimation;
        curSplitterAnimation = nullptr;
    });

    connect(curSplitterAnimation, &QPropertyAnimation::valueChanged, q, [this](const QVariant &value) {
        Q_UNUSED(value);
        emit q->windowSplitterWidthChanged(value.toInt());
        // 动画过程中实时更新分割线位置
        updateSideBarSeparatorPosition();
    });
}

void FileManagerWindowPrivate::loadWindowState()
{
    const QVariantMap &state = Application::appObtuselySetting()->value("WindowManager", "WindowState").toMap();

    int width = state.value("width").toInt();
    int height = state.value("height").toInt();
    NetWmStates windowState = static_cast<NetWmStates>(state.value("state").toInt());

    // fix bug 30932,获取全屏属性，必须是width全屏和height全屏属性都满足，才判断是全屏
    if ((FMWindowsIns.windowIdList().isEmpty()) && ((windowState & kNetWmStateMaximizedHorz) != 0 && (windowState & kNetWmStateMaximizedVert) != 0)) {
        // make window to be maximized.
        // the following calling is copyed from QWidget::showMaximized()
        q->setWindowState((q->windowState() & ~(Qt::WindowMinimized | Qt::WindowFullScreen))
                          | Qt::WindowMaximized);
    } else {
        q->resize(width, height);
    }
}

void FileManagerWindowPrivate::saveWindowState()
{
    NetWmStates states { 0 };
    if (WindowUtils::isWayLand()) {
        if (q->isMaximized())
            states = static_cast<NetWmState>(kNetWmStateMaximizedHorz | kNetWmStateMaximizedVert);
    } else {
        /// The power by dxcb platform plugin
        states = static_cast<NetWmStates>(q->window()->windowHandle()->property("_d_netWmStates").toInt());
    }

    QVariantMap state;
    // fix bug 30932,获取全屏属性，必须是width全屏和height全屏属性都满足，才判断是全屏
    if ((states & kNetWmStateMaximizedHorz) == 0 || (states & kNetWmStateMaximizedVert) == 0) {
        if (!splitter || splitter->sizes().isEmpty())
            return;
        int sideBarWidth = splitter->sizes().at(0);
        int minWidth = std::max(sideBarWidth, kMinimumLeftWidth) + kMinimumRightWidth + splitter->handleWidth();
        state["width"] = std::max(q->size().width(), minWidth);
        state["height"] = q->size().height();
    } else {
        const QVariantMap &state1 = Application::appObtuselySetting()->value("WindowManager", "WindowState").toMap();
        state["width"] = state1.value("width").toInt();
        state["height"] = state1.value("height").toInt();
        state["state"] = static_cast<int>(states);
    }
    Application::appObtuselySetting()->setValue("WindowManager", "WindowState", state);
}

void FileManagerWindowPrivate::saveSidebarState()
{
    int width = splitterPosition();
    if (width >= kMinimumLeftWidth && width <= kMaximumLeftWidth) {
        QVariantMap state;
        state["sidebar"] = width;
        Application::appObtuselySetting()->setValue("WindowManager", "SplitterState", state);
    }
}

int FileManagerWindowPrivate::loadDetailSpaceState() const
{
    const QVariantMap &state = Application::appObtuselySetting()->value("WindowManager", "SplitterState").toMap();
    int width = state.value("detailspace", kDefaultDetailWidth).toInt();
    return qBound(kMinimumDetailWidth, width, kMaximumDetailWidth);
}

void FileManagerWindowPrivate::loadDetailSpaceVisibility()
{
    // Check if there are other windows already open
    auto windowList = FMWindowsIns.windowIdList();

    if (windowList.count() <= 1) {
        // First window: load from DConfig
        detailSpaceVisible = DConfigManager::instance()->value(kViewDConfName, kDisplayPreviewVisibleKey, false).toBool();
        qCDebug(logDFMBase) << "First window: loaded detailSpace visibility from DConfig:" << detailSpaceVisible;
    } else {
        // Subsequent windows: inherit from the last activated window
        quint64 lastWinId = FMWindowsIns.lastActivedWindowId();
        auto lastWindow = FMWindowsIns.findWindowById(lastWinId);
        if (lastWindow) {
            detailSpaceVisible = lastWindow->isDetailSpaceVisible();
            qCDebug(logDFMBase) << "New window: inherited detailSpace visibility from window" << lastWinId << ":" << detailSpaceVisible;
        } else {
            // Fallback: load from DConfig if last window not found
            detailSpaceVisible = DConfigManager::instance()->value(kViewDConfName, kDisplayPreviewVisibleKey, false).toBool();
            qCWarning(logDFMBase) << "Cannot find last activated window" << lastWinId << ", falling back to DConfig:" << detailSpaceVisible;
        }
    }
}

void FileManagerWindowPrivate::saveDetailSpaceState()
{
    if (!detailSpace)
        return;

    int width = lastDetailSpaceWidth;
    if (width >= kMinimumDetailWidth && width <= kMaximumDetailWidth) {
        QVariantMap state = Application::appObtuselySetting()->value("WindowManager", "SplitterState").toMap();
        state["detailspace"] = width;
        Application::appObtuselySetting()->setValue("WindowManager", "SplitterState", state);
    }

    // Save detailSpace visibility flag to DConfig when the window is closed
    DConfigManager::instance()->setValue(kViewDConfName, kDisplayPreviewVisibleKey, detailSpaceVisible);
}

void FileManagerWindowPrivate::updateRightAreaMinWidth()
{
    if (!rightArea || !detailSplitter)
        return;

    int requiredMinWidth = kMinimumWorkspaceWidth;   // 260px for workspace

    // If sidebar is visible, calculate dynamic minimum width
    if (sideBar && sideBar->isVisible()) {
        // If detailspace is visible, add its current width
        if (detailSpace && detailSpace->isVisible()) {
            int detailWidth = detailSplitterPosition();
            if (detailWidth > 0) {
                requiredMinWidth += detailWidth + detailSplitter->handleWidth();
            }
        }
    } else {
        // Sidebar is hidden, rightArea takes full window width
        // Use window minimum width to ensure titlebar displays correctly
        requiredMinWidth = kMinimumRightWidth;
    }

    rightArea->setMinimumWidth(requiredMinWidth);
}

int FileManagerWindowPrivate::detailSplitterPosition() const
{
    if (!detailSplitter || detailSplitter->sizes().size() < 2)
        return kDefaultDetailWidth;
    return detailSplitter->sizes().at(1);
}

void FileManagerWindowPrivate::setDetailSplitterPosition(int detailWidth)
{
    if (!detailSplitter)
        return;

    int clampedWidth = qBound(kMinimumDetailWidth, detailWidth, kMaximumDetailWidth);
    int workspaceWidth = detailSplitter->width() - clampedWidth - detailSplitter->handleWidth();
    detailSplitter->setSizes({ workspaceWidth, clampedWidth });
}

void FileManagerWindowPrivate::initDetailSplitter()
{
    if (detailSplitter)
        return;

    detailSplitter = new Splitter(Qt::Horizontal, rightArea);
    detailSplitter->setChildrenCollapsible(false);
    detailSplitter->setHandleWidth(0);   // Same as sidebar splitter

    // Connect splitter moved signal
    QObject::connect(detailSplitter, &QSplitter::splitterMoved,
                     this, &FileManagerWindowPrivate::handleDetailSplitterMoved);
}

void FileManagerWindowPrivate::handleDetailSplitterMoved(int pos, int index)
{
    Q_UNUSED(index);

    if (!detailSplitter || !detailSpace)
        return;

    int detailWidth = detailSplitter->width() - pos - detailSplitter->handleWidth();

    // Clamp to maximum width
    if (detailWidth > kMaximumDetailWidth) {
        setDetailSplitterPosition(kMaximumDetailWidth);
        detailWidth = kMaximumDetailWidth;
    }

    // Clamp to minimum width
    if (detailWidth < kMinimumDetailWidth) {
        setDetailSplitterPosition(kMinimumDetailWidth);
        detailWidth = kMinimumDetailWidth;
    }

    // Remember the width
    lastDetailSpaceWidth = detailWidth;
}

void FileManagerWindowPrivate::animateDetailSplitter(bool show)
{
    if (!detailSplitter || !detailSpace)
        return;

    if (!isDetailViewAnimationEnabled()) {
        if (show) {
            detailSpace->setVisible(true);
            setDetailSplitterPosition(lastDetailSpaceWidth);
        } else {
            detailSpace->setVisible(false);
        }
        updateRightAreaMinWidth();   // Update after visibility change
        // Signal emission moved to showDetailSpace/hideDetailSpace
        return;
    }

    // Setup: set visible and allow shrinking
    detailSpace->setVisible(true);
    detailSpace->setMinimumWidth(1);

    // Stop any running animation
    bool lastAnimationStopped = false;
    if (curDetailSplitterAnimation && curDetailSplitterAnimation->state() == QAbstractAnimation::Running) {
        lastAnimationStopped = true;
        curDetailSplitterAnimation->stop();
        delete curDetailSplitterAnimation;
        curDetailSplitterAnimation = nullptr;
    }

    // Calculate workspace width for animation
    // splitPosition controls the FIRST widget (workspace), not the second (detailSpace)
    int totalWidth = detailSplitter->width() - detailSplitter->handleWidth();
    int currentWorkspaceWidth = detailSplitter->sizes().at(0);

    // Save current detailSpace width when hiding
    if (!show && !lastAnimationStopped)
        lastDetailSpaceWidth = detailSplitter->sizes().at(1);

    // Calculate animation start and end for WORKSPACE width
    int start = show ? (totalWidth - 1) : currentWorkspaceWidth;
    int end = show ? (totalWidth - lastDetailSpaceWidth) : (totalWidth - 1);

    // Configure animation using detailSplitter's splitPosition property
    int duration = DConfigManager::instance()->value(kAnimationDConfName, kAnimationDetailviewDuration, 366).toInt();
    auto curve = static_cast<QEasingCurve::Type>(DConfigManager::instance()->value(kAnimationDConfName, kAnimationDetailviewCurve, 22).toInt());

    curDetailSplitterAnimation = new QPropertyAnimation(detailSplitter, "splitPosition");
    curDetailSplitterAnimation->setEasingCurve(curve);
    curDetailSplitterAnimation->setDuration(duration);
    curDetailSplitterAnimation->setStartValue(start);
    curDetailSplitterAnimation->setEndValue(end);

    // Connect animation signals
    connect(curDetailSplitterAnimation, &QPropertyAnimation::finished, this, [this, show]() {
        if (show) {
            detailSpace->setMinimumWidth(kMinimumDetailWidth);
            detailSpace->setMaximumWidth(kMaximumDetailWidth);
        } else {
            detailSpace->setVisible(false);
            detailSpace->setMinimumWidth(kMinimumDetailWidth);
        }

        updateRightAreaMinWidth();   // Update after animation completes
        // Signal emission moved to showDetailSpace/hideDetailSpace

        delete curDetailSplitterAnimation;
        curDetailSplitterAnimation = nullptr;
    });

    curDetailSplitterAnimation->start();
}

void FileManagerWindowPrivate::updateSideBarSeparatorStyle()
{
    if (!sidebarSep)
        return;

    // 设置颜色 - 仅在主题变化或初始化时需要
    QColor sepColor(0, 0, 0);
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
        sepColor.setAlphaF(0.05);   // 浅色主题 5% 透明度
    } else {
        sepColor.setAlphaF(0.5);   // 深色主题 50% 透明度
    }

    // 使用setBrush而不是setColor以确保透明度正确应用
    QPalette palette;
    palette.setBrush(QPalette::Window, QBrush(sepColor));
    sidebarSep->setPalette(palette);

    // 确保分隔线能够正确显示透明效果
    sidebarSep->setAutoFillBackground(true);

    // 确保没有边框影响显示
    if (auto sidebarFrame = qobject_cast<QFrame *>(sidebarSep))
        sidebarFrame->setFrameShape(QFrame::NoFrame);
}

void FileManagerWindowPrivate::updateSideBarSeparatorPosition()
{
    if (!sidebarSep || !sideBar || !splitter)
        return;

    // 确保分割线显示在侧边栏右侧边缘
    sidebarSep->setParent(q);
    sidebarSep->move(sideBar->x() + sideBar->width() - 1, sideBar->y());
    sidebarSep->setFixedHeight(sideBar->height());
    sidebarSep->raise();

    // 根据侧边栏实际可见性设置分割线可见性
    sidebarSep->setVisible(sideBar->isVisible() && sideBar->width() > 0 && sideBar->height() > 0);
}

void FileManagerWindowPrivate::updateSideBarState()
{
    int totalWidth = q->width();
    int sideBarWidth = splitter->sizes().at(0);
    int workspaceWidth = totalWidth - sideBarWidth - splitter->handleWidth();
    sideBarShrinking = workspaceWidth <= kMinimumRightWidth;

    if (!sideBarShrinking && sideBarWidth >= kMinimumLeftWidth)
        lastSidebarExpandedPostion = sideBarWidth;
}

void FileManagerWindowPrivate::updateSideBarVisibility()
{
    int totalWidth = q->width();

    // Calculate actual minimum right area width based on current state
    int actualMinRightWidth = kMinimumRightWidth;

    // If detailspace is visible, use dynamic calculation
    if (detailSpace && detailSpace->isVisible()) {
        actualMinRightWidth = kMinimumWorkspaceWidth;   // 260px for workspace
        int detailWidth = detailSplitterPosition();
        if (detailWidth > 0) {
            actualMinRightWidth += detailWidth + detailSplitter->handleWidth();
        }
    }

    bool haveSpaceShowSidebar = totalWidth >= (actualMinRightWidth + kMinimumLeftWidth + splitter->handleWidth());

    if (haveSpaceShowSidebar && !sideBarAutoVisible) {
        showSideBar();
    } else if (!haveSpaceShowSidebar && sideBarAutoVisible) {
        hideSideBar();
    } else if (sideBarShrinking && sideBarAutoVisible) {
        int newSideBarWidth = totalWidth - actualMinRightWidth - splitter->handleWidth();
        if (newSideBarWidth <= kMinimumLeftWidth) {
            hideSideBar();
        }
    }
}

void FileManagerWindowPrivate::updateSidebarSeparator()
{
    updateSideBarSeparatorStyle();
    updateSideBarSeparatorPosition();
}

void FileManagerWindowPrivate::showSideBar()
{
    if (sideBar && sideBar->isVisible())
        return;

    sideBar->setVisible(true);
    sidebarSep->setVisible(true);
    expandButton->setProperty("expand", true);
    sideBarAutoVisible = true;
    emit q->windowSplitterWidthChanged(lastSidebarExpandedPostion);

    updateSidebarSeparator();
}

void FileManagerWindowPrivate::hideSideBar()
{
    if (sideBar && !sideBar->isVisible())
        return;

    sideBar->setVisible(false);
    sidebarSep->setVisible(false);
    expandButton->setProperty("expand", false);
    sideBarAutoVisible = false;
    emit q->windowSplitterWidthChanged(0);
}

void FileManagerWindowPrivate::setupSidebarSepTracking()
{
    if (!splitter || !sidebarSep || !sideBar)
        return;

    // 安装事件过滤器
    sideBar->installEventFilter(q);

    // 连接分割器的splitterMoved信号到更新分割线位置的槽
    QObject::connect(splitter, &QSplitter::splitterMoved,
                     q, [this](int pos, int index) {
                         Q_UNUSED(pos);
                         Q_UNUSED(index);
                         updateSideBarSeparatorPosition();
                     });
}

void FileManagerWindowPrivate::installDetailSplitterHandleEventFilter()
{
    if (!detailSplitter)
        return;

    // 获取 splitter 的 handle（分隔条）
    QSplitterHandle *handle = detailSplitter->handle(1);
    if (handle) {
        handle->installEventFilter(q);
    }
}

void FileManagerWindowPrivate::resetDetailDragState()
{
    detailDragTracking = false;
    detailDragAtMinimum = false;
    detailDragHidden = false;
    detailDragContinued = false;
    detailDragMinimumPosX = 0;
}

bool FileManagerWindowPrivate::handleWorkspaceKeyEvent(QObject *watched, QEvent *event)
{
    if (!workspace || watched != workspace)
        return false;

    if (event->type() != QEvent::KeyPress)
        return false;

    return processKeyPressEvent(static_cast<QKeyEvent *>(event));
}

bool FileManagerWindowPrivate::handleSideBarEvent(QObject *watched, QEvent *event)
{
    if (!sideBar || watched != sideBar)
        return false;

    switch (event->type()) {
    case QEvent::Show:
    case QEvent::Resize:
        updateSideBarSeparatorPosition();
        break;
    default:
        break;
    }

    return false;
}

bool FileManagerWindowPrivate::handleDetailSplitterHandleEvent(QObject *watched, QEvent *event)
{
    if (!detailSplitter || !detailSpace)
        return false;

    QSplitterHandle *handle = detailSplitter->handle(1);
    if (!handle || watched != handle)
        return false;

    switch (event->type()) {
    case QEvent::MouseButtonPress: {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            detailDragTracking = true;

            // 检查当前是否已经在最小宽度
            int currentWidth = detailSplitterPosition();
            if (currentWidth <= kMinimumDetailWidth && detailSpace->isVisible()) {
                detailDragAtMinimum = true;
                detailDragMinimumPosX = qRound(mouseEvent->globalPosition().x());   // 记录起始点
                detailDragHidden = false;
            } else {
                detailDragAtMinimum = false;
                detailDragHidden = false;
            }
        }
        break;
    }

    case QEvent::MouseMove: {
        if (!detailDragTracking)
            break;

        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        int currentMouseX = qRound(mouseEvent->globalPosition().x());

        // === 状态1: 检查是否刚达到最小宽度 ===
        int currentWidth = detailSplitterPosition();

        if (!detailDragAtMinimum && currentWidth <= kMinimumDetailWidth) {
            // 刚到达最小宽度，进入弹性拖拽模式
            detailDragAtMinimum = true;
            detailDragMinimumPosX = currentMouseX;   // 记录起始点
            break;
        }

        // === 状态2: 弹性拖拽模式 ===
        if (detailDragAtMinimum) {
            // 计算从起始点的偏移量
            int offsetFromMinimum = currentMouseX - detailDragMinimumPosX;

            if (offsetFromMinimum >= kDetailDragThreshold) {
                // 向右拖拽超过 140px → 直接隐藏

                // 在隐藏之前先设置光标，因为 hideDetailSpace 会触发 handle 的 leaveEvent
                // 导致 restoreOverrideCursor 被调用，我们需要在其之后保持拖拽光标
                // 使用 changeOverrideCursor 而不是 set/restore 来避免栈问题
                QGuiApplication::changeOverrideCursor(Qt::SizeHorCursor);

                QVariantHash options;
                options[DetailSpaceOptions::kAnimated] = false;   // 不使用动画
                options[DetailSpaceOptions::kUserAction] = true;   // 拖拽是用户操作
                q->hideDetailSpace(options);

                // hideDetailSpace 后 handle 的 leaveEvent 会 restoreOverrideCursor
                // 此时光标栈可能为空，需要重新设置
                QGuiApplication::setOverrideCursor(Qt::SizeHorCursor);

                // 进入"已隐藏"状态，安装全局事件过滤器
                detailDragHidden = true;
                detailDragAtMinimum = false;

                // 安装应用级别的事件过滤器来监听全局鼠标移动
                qApp->installEventFilter(q);

            } else if (offsetFromMinimum < 0) {
                // 向左拖拽超过起始点，退出弹性模式，恢复正常拖拽
                detailDragAtMinimum = false;
            }
        }
        break;
    }

    case QEvent::MouseButtonRelease: {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            // 重置所有状态（仅当未进入全局拖拽模式时）
            // 如果已安装全局事件过滤器，由 handleGlobalDragEvent 处理释放
            if (!detailDragHidden && !detailDragContinued) {
                resetDetailDragState();
            }
        }
        break;
    }

    default:
        break;
    }

    return false;
}

bool FileManagerWindowPrivate::handleGlobalDragEvent(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched)

    // 只在全局拖拽模式下处理（detailspace已隐藏或继续拖拽模式）
    if (!detailDragHidden && !detailDragContinued)
        return false;

    // 必须处于拖拽跟踪状态
    if (!detailDragTracking)
        return false;

    if (event->type() == QEvent::MouseMove) {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);

        // 检查鼠标是否在当前窗口内
        QPoint globalPos = mouseEvent->globalPosition().toPoint();
        QRect windowRect(q->mapToGlobal(QPoint(0, 0)), q->size());

        if (!windowRect.contains(globalPos))
            return false;

        int currentMouseX = globalPos.x();
        int windowRightEdgeX = q->mapToGlobal(QPoint(q->width(), 0)).x();
        int distanceFromRight = windowRightEdgeX - currentMouseX;

        if (detailDragHidden) {
            // 状态A: detailspace 已隐藏，检测是否需要显示
            if (distanceFromRight > kDetailDragThreshold) {
                // 光标距离右边界 > 140px → 显示 detailspace
                QVariantHash options;
                options[DetailSpaceOptions::kAnimated] = false;
                options[DetailSpaceOptions::kUserAction] = true;
                q->showDetailSpace(options);

                // 进入继续拖拽模式
                detailDragHidden = false;
                detailDragContinued = true;

                // 根据当前鼠标位置设置 detailspace 宽度
                int detailWidth = qBound(kMinimumDetailWidth, distanceFromRight, kMaximumDetailWidth);
                setDetailSplitterPosition(detailWidth);
                lastDetailSpaceWidth = detailWidth;
            }
        } else if (detailDragContinued) {
            // 状态B: 继续拖拽模式，可调整宽度或再次隐藏
            if (distanceFromRight <= kDetailDragThreshold) {
                // 向右拖拽到阈值以内 → 再次隐藏
                QVariantHash options;
                options[DetailSpaceOptions::kAnimated] = false;
                options[DetailSpaceOptions::kUserAction] = true;
                q->hideDetailSpace(options);

                detailDragHidden = true;
                detailDragContinued = false;
            } else {
                // 正常拖拽调整宽度
                int detailWidth = qBound(kMinimumDetailWidth, distanceFromRight, kMaximumDetailWidth);
                setDetailSplitterPosition(detailWidth);
                lastDetailSpaceWidth = detailWidth;
            }
        }
    } else if (event->type() == QEvent::MouseButtonRelease) {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            // 鼠标释放，恢复光标样式
            QGuiApplication::restoreOverrideCursor();

            // 重置所有状态并移除全局事件过滤器
            resetDetailDragState();
            qApp->removeEventFilter(q);
        }
    }

    return false;
}

/*!
 * \class FileManagerWindow
 * \brief
 */

FileManagerWindow::FileManagerWindow(const QUrl &url, QWidget *parent)
    : DMainWindow(parent),
      d(new FileManagerWindowPrivate(url, this))
{
    auto hideTitlebar = [this]() {
        // hide titlebar
        titlebar()->setHidden(true);
        titlebar()->setFixedHeight(0);
        setTitlebarShadowEnabled(false);
    };
    hideTitlebar();

    // size
    resize(d->kDefaultWindowWidth, d->kDefaultWindowHeight);
    setMinimumSize(d->kMinimumWindowWidth, d->kMinimumWindowHeight);

    d->centralView = new QFrame(this);
    d->centralView->setObjectName("CentralView");

    d->midLayout = new QHBoxLayout;
    d->midLayout->setContentsMargins(0, 0, 0, 0);
    d->midLayout->setSpacing(0);

    d->rightLayout = new QVBoxLayout;
    d->rightLayout->setContentsMargins(0, 0, 0, 0);
    d->rightLayout->setSpacing(0);

    d->rightBottomLayout = new QHBoxLayout;
    d->rightBottomLayout->setContentsMargins(0, 0, 0, 0);
    d->rightBottomLayout->setSpacing(0);

    // icon label
    d->iconLabel = new DIconButton(this);
    d->iconLabel->setWindowFlags(Qt::WindowTransparentForInput);
    d->iconLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    d->iconLabel->setFocusPolicy(Qt::NoFocus);
    d->iconLabel->setFlat(true);

    // expand button
    d->expandButton = new CustomDIconButton(this);
    d->expandButton->setProperty("expand", true);

    // connections
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [hideTitlebar, this]() {
        hideTitlebar();
        d->resetTitleBarSize();
    });

    connect(DGuiApplicationHelper::instance()->systemTheme(), &DPlatformTheme::iconThemeNameChanged, this, [this]() {
        d->iconLabel->update();
    });

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, [this]() {
                d->updateSidebarSeparator();
            });

    connect(d->expandButton, &DIconButton::clicked, this, [this]() {
        bool isExpand = d->expandButton->property("expand").toBool();
        d->expandButton->setProperty("expand", !isExpand);
        d->animateSplitter(!isExpand);
    });
}

FileManagerWindow::~FileManagerWindow()
{
    auto menu = titlebar()->menu();
    if (menu) {
        delete menu;
        menu = nullptr;
    }
}

void FileManagerWindow::cd(const QUrl &url)
{
    d->currentUrl = url;
    if (d->titleBar)
        d->titleBar->setCurrentUrl(url);
    if (d->sideBar)
        d->sideBar->setCurrentUrl(url);
    if (d->workspace)
        d->workspace->setCurrentUrl(url);
    if (d->detailSpace)
        d->detailSpace->setCurrentUrl(url);
    emit currentUrlChanged(url);
}

bool FileManagerWindow::saveClosedSate() const
{
    return true;
}

QUrl FileManagerWindow::currentUrl() const
{
    return d->currentUrl;
}

void FileManagerWindow::moveCenter()
{
    QScreen *cursorScreen = WindowUtils::cursorScreen();
    if (!cursorScreen)
        return;
    int x = (cursorScreen->availableGeometry().width() - width()) / 2;
    int y = (cursorScreen->availableGeometry().height() - height()) / 2;
    move(QPoint(x, y) + cursorScreen->geometry().topLeft());
}

void FileManagerWindow::installTitleBar(AbstractFrame *w)
{
    Q_ASSERT_X(w, "FileManagerWindow", "Null TitleBar");
    std::call_once(d->titleBarFlag, [this, w]() {
        d->titleBar = w;
        initializeUi();

        emit this->titleBarInstallFinished();
    });
}

void FileManagerWindow::installSideBar(AbstractFrame *w)
{
    Q_ASSERT_X(w, "FileManagerWindow", "Null setSideBar");
    std::call_once(d->sideBarFlag, [this, w]() {
        d->sideBar = w;

        initializeUi();
        updateUi();   // setSizes is only valid when the splitter is non-empty
        emit this->sideBarInstallFinished();
    });
}

void FileManagerWindow::installWorkSpace(AbstractFrame *w)
{
    Q_ASSERT_X(w, "FileManagerWindow", "Null Workspace");
    std::call_once(d->workspaceFlag, [this, w]() {
        d->workspace = w;
        d->workspace->installEventFilter(this);

        initializeUi();
        updateUi();   // setSizes is only valid when the splitter is non-empty
        emit this->workspaceInstallFinished();
    });
}

/*!
 * \brief NOTE: shouldn't call it if detail button not clicked in titlebar
 * \param w
 */
void FileManagerWindow::installDetailView(AbstractFrame *w)
{
    d->detailSpace = w;
    if (d->detailSpace && d->detailSplitter) {
        // Add detailSpace as second widget in splitter
        d->detailSplitter->addWidget(d->detailSpace);
        d->detailSpace->setVisible(false);

        // Set stretch factor for detailSpace: 0 to keep fixed size
        d->detailSplitter->setStretchFactor(1, 0);   // detailSpace index=1

        // Set initial splitter position with saved width
        // At this point, detailSplitter already has valid geometry from initializeUi
        d->setDetailSplitterPosition(d->lastDetailSpaceWidth);

        // 安装 handle 事件过滤器
        d->installDetailSplitterHandleEventFilter();
    }

    emit this->detailViewInstallFinished();
}

AbstractFrame *FileManagerWindow::titleBar() const
{
    return d->titleBar;
}

AbstractFrame *FileManagerWindow::sideBar() const
{
    return d->sideBar;
}

AbstractFrame *FileManagerWindow::workSpace() const
{
    return d->workspace;
}

AbstractFrame *FileManagerWindow::detailView() const
{
    return d->detailSpace;
}

void FileManagerWindow::setDetailViewWidth(int width)
{
    d->lastDetailSpaceWidth = qBound(d->kMinimumDetailWidth, width, d->kMaximumDetailWidth);
    if (d->detailSplitter && d->detailSpace && d->detailSpace->isVisible()) {
        d->setDetailSplitterPosition(d->lastDetailSpaceWidth);
    }
}

int FileManagerWindow::detailViewWidth() const
{
    if (d->detailSplitter && d->detailSpace && d->detailSpace->isVisible()) {
        return d->detailSplitterPosition();
    }
    return d->lastDetailSpaceWidth;
}

void FileManagerWindow::showDetailSpace(const QVariantHash &options)
{
    bool animated = options.value(DetailSpaceOptions::kAnimated, true).toBool();
    bool userAction = options.value(DetailSpaceOptions::kUserAction, true).toBool();

    // Only update flag for user-initiated actions
    if (userAction) {
        d->detailSpaceVisible = true;
    }

    if (animated) {
        d->animateDetailSplitter(true);
    } else {
        if (d->detailSpace) {
            d->detailSpace->setVisible(true);
            d->setDetailSplitterPosition(d->lastDetailSpaceWidth);
        }
        d->updateRightAreaMinWidth();
    }

    // Emit signal only for user-initiated actions (unified handling for both animated and non-animated)
    if (userAction) {
        emit detailSpaceVisibilityChanged(true);
    }
}

void FileManagerWindow::hideDetailSpace(const QVariantHash &options)
{
    bool animated = options.value(DetailSpaceOptions::kAnimated, true).toBool();
    bool userAction = options.value(DetailSpaceOptions::kUserAction, true).toBool();

    // Only update flag for user-initiated actions
    if (userAction) {
        d->detailSpaceVisible = false;
    }

    if (animated) {
        d->animateDetailSplitter(false);
    } else {
        if (d->detailSpace)
            d->detailSpace->setVisible(false);
        d->updateRightAreaMinWidth();
    }

    // Emit signal only for user-initiated actions (unified handling for both animated and non-animated)
    if (userAction) {
        emit detailSpaceVisibilityChanged(false);
    }
}

bool FileManagerWindow::isDetailSpaceVisible() const
{
    return d->detailSpaceVisible;
}

void FileManagerWindow::loadState()
{
    d->loadWindowState();
}

void FileManagerWindow::saveState()
{
    d->saveSidebarState();
    d->saveDetailSpaceState();
    d->saveWindowState();
}

void FileManagerWindow::closeEvent(QCloseEvent *event)
{
    // NOTE(zhangs): bug 59239
    emit aboutToClose();
    DMainWindow::closeEvent(event);
}

void FileManagerWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (qRound(event->position().y()) <= d->titleBar->height()) {
        if (isMaximized()) {
            showNormal();
        } else {
            showMaximized();
        }
    } else {
        DMainWindow::mouseDoubleClickEvent(event);
    }
}

void FileManagerWindow::moveEvent(QMoveEvent *event)
{
    DMainWindow::moveEvent(event);

    emit positionChanged(event->pos());
}

void FileManagerWindow::keyPressEvent(QKeyEvent *event)
{
    if (!d->processKeyPressEvent(event))
        return DMainWindow::keyPressEvent(event);
}

bool FileManagerWindow::eventFilter(QObject *watched, QEvent *event)
{
    // 工作区键盘事件
    if (d->handleWorkspaceKeyEvent(watched, event))
        return true;

    // 侧边栏事件
    if (d->handleSideBarEvent(watched, event))
        return true;

    // DetailSplitter handle 拖拽事件
    if (d->handleDetailSplitterHandleEvent(watched, event))
        return true;

    // 全局拖拽事件（用于隐藏后的反向显示和继续拖拽）
    if (d->handleGlobalDragEvent(watched, event))
        return true;

    return false;
}

void FileManagerWindow::initializeUi()
{
    if (d->sideBar && d->titleBar && d->workspace) {
        // left area
        {
            d->sideBar->setContentsMargins(0, 0, 0, 0);
            d->resetSideBarSize();
            d->sidebarSep = new DVerticalLine(this);
            d->sidebarSep->setContentsMargins(0, 0, 0, 0);
            d->sidebarSep->setVisible(true);
            d->sidebarSep->setAutoFillBackground(true);
            d->sidebarSep->setFixedWidth(1);
        }

        // icon area
        {
            d->iconArea = new QWidget(this);
            QHBoxLayout *leftAreaLayout = new QHBoxLayout(d->iconArea);
            d->iconArea->setWindowFlag(Qt::WindowTransparentForInput);
            d->iconArea->move(0, 0);
            leftAreaLayout->setContentsMargins(0, 0, 0, 0);
            leftAreaLayout->addSpacing(10);
            if (d->iconLabel) {
                auto icon = QIcon::fromTheme("dde-file-manager", QIcon::fromTheme("system-file-manager"));
                d->iconLabel->setIcon(icon);
                leftAreaLayout->addWidget(d->iconLabel, 0, Qt::AlignLeading | Qt::AlignVCenter);
            }
            if (d->expandButton) {
                leftAreaLayout->addSpacing(10);
                d->expandButton->setIcon(DDciIcon::fromTheme("sidebar-switch-symbolic"));
                leftAreaLayout->addWidget(d->expandButton, 0, Qt::AlignLeading | Qt::AlignVCenter);
            }
            d->resetTitleBarSize();
            d->iconArea->show();
        }

        // right area
        {
            d->rightArea = new QFrame(this);
            d->rightArea->setAutoFillBackground(true);
            d->rightArea->setMinimumWidth(d->kMinimumRightWidth);
            // NOTE(zccrs): 保证窗口宽度改变时只会调整right view的宽度，侧边栏保持不变
            //              QSplitter是使用QLayout的策略对widgets进行布局，所以此处
            //              设置size policy可以生效
            QSizePolicy sp = d->rightArea->sizePolicy();
            sp.setHorizontalStretch(1);
            d->rightArea->setSizePolicy(sp);

            d->rightLayout->addWidget(d->titleBar);
            d->rightLayout->addLayout(d->rightBottomLayout, 1);

            // Initialize detailSplitter structure (even without detailSpace)
            d->initDetailSplitter();
            d->detailSplitter->addWidget(d->workspace);

            // Set workspace minimum width to ensure proper resize priority
            d->workspace->setMinimumWidth(d->kMinimumWorkspaceWidth);

            // Set stretch factor: workspace=1 (absorbs size changes), detailSpace=0 (keeps fixed)
            d->detailSplitter->setStretchFactor(0, 1);   // workspace index=0

            d->rightBottomLayout->addWidget(d->detailSplitter, 1);

            d->rightArea->setLayout(d->rightLayout);
        }

        // splitter
        {
            d->splitter = new Splitter(Qt::Orientation::Horizontal, this);
            d->splitter->setChildrenCollapsible(false);
            d->splitter->setHandleWidth(0);
            d->splitter->addWidget(d->sideBar);
            d->splitter->addWidget(d->rightArea);
        }

        // central
        {
            QVBoxLayout *mainLayout = new QVBoxLayout;
            QWidget *midWidget = new QWidget;
            midWidget->setLayout(d->midLayout);
            mainLayout->addWidget(midWidget);
            mainLayout->setSpacing(0);
            mainLayout->setContentsMargins(0, 0, 0, 0);
            d->midLayout->insertWidget(0, d->splitter);
            d->centralView->setLayout(mainLayout);
            setCentralWidget(d->centralView);
        }

        // cd
        cd(d->currentUrl);

        // 在splitter初始化后设置分割线跟踪
        d->updateSidebarSeparator();
        d->setupSidebarSepTracking();
    }
}

void FileManagerWindow::updateUi()
{
    if (!d->sideBar || !d->workspace)
        return;

    int splitterPos = d->loadSidebarState();
    d->lastSidebarExpandedPostion = splitterPos;
    d->setSplitterPosition(splitterPos);

    // Load detailSplitter initial size and visibility flag
    d->lastDetailSpaceWidth = d->loadDetailSpaceState();
    d->loadDetailSpaceVisibility();
}

void FileManagerWindow::resizeEvent(QResizeEvent *event)
{
    DMainWindow::resizeEvent(event);

    if (!d->sideBar || !d->titleBar || !d->workspace || !d->splitter)
        return;

    // Update rightArea minimum width to enforce resize priority:
    // workspace shrinks first, then sidebar, then detailspace
    d->updateRightAreaMinWidth();

    d->updateSideBarState();
    d->updateSideBarVisibility();
}

bool FileManagerWindow::event(QEvent *event)
{
    if (event->type() == QEvent::WindowActivate)
        Q_EMIT windowActived();

    return DMainWindow::event(event);
}

}   // namespace dfmbase
