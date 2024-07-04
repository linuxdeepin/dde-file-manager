// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-gui/panel.h>
#include <dfm-gui/windowmanager.h>
#include "panel_p.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/utils/windowutils.h>

#include <QQuickWindow>

DFMBASE_USE_NAMESPACE

DFMGUI_BEGIN_NAMESPACE

static constexpr char kWindowManager[] { "WindowManager" };
// Window config
static constexpr char kWindowState[] { "WindowState" };
static constexpr char kWidth[] = { "width" };
static constexpr char kHeight[] = { "height" };
static constexpr char kState[] = { "state" };
// Sidebar config
static constexpr char kSplitterState[] = { "SplitterState" };
static constexpr char kSidebar[] = { "sidebar" };
static constexpr char kManualHide[] = { "manualHide" };
static constexpr char kAutoHide[] = { "autoHide" };

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

PanelPrivate::PanelPrivate(Panel *q)
    : ContainmentPrivate(q)
{
    flag = Applet::kPanel;
}

void PanelPrivate::setRootObject(QObject *item)
{
    Q_ASSERT(nullptr == window);

    window = qobject_cast<QQuickWindow *>(item);
    if (window) {
        Q_Q(Panel);
        QObject::connect(window, &QQuickWindow::closing, q, &Panel::aboutToClose);
    }

    ContainmentPrivate::setRootObject(item);
}

void PanelPrivate::loadWindowState()
{
    const QVariantMap &state = Application::appObtuselySetting()->value(kWindowManager, kWindowState).toMap();

    int width = state.value(kWidth).toInt();
    int height = state.value(kHeight).toInt();
    NetWmStates windowState = static_cast<NetWmStates>(state.value(kState).toInt());

    // fix bug 30932,获取全屏属性，必须是width全屏和height全屏属性都满足，才判断是全屏
    if ((WindowManager::instance()->windowIdList().isEmpty())
        && ((windowState & kNetWmStateMaximizedHorz) != 0 && (windowState & kNetWmStateMaximizedVert) != 0)) {
        // make window to be maximized.
        // the following calling is copyed from QWidget::showMaximized()
        window->setWindowStates((window->windowStates() & ~(Qt::WindowMinimized | Qt::WindowFullScreen))
                                | Qt::WindowMaximized);
    } else {
        window->resize(width, height);
    }
}

void PanelPrivate::saveWindowState() const
{
    NetWmStates states { 0 };
    if (WindowUtils::isWayLand()) {
        if (window->windowStates().testFlag(Qt::WindowMaximized)) {
            states = static_cast<NetWmState>(kNetWmStateMaximizedHorz | kNetWmStateMaximizedVert);
        }
    } else {
        /// The power by dxcb platform plugin
        states = static_cast<NetWmStates>(window->property("_d_netWmStates").toInt());
    }

    QVariantMap state;
    // fix bug 30932,获取全屏属性，必须是width全屏和height全屏属性都满足，才判断是全屏
    if ((states & kNetWmStateMaximizedHorz) == 0 || (states & kNetWmStateMaximizedVert) == 0) {
        state[kWidth] = window->size().width();
        state[kHeight] = window->size().height();
    } else {
        const QVariantMap &state1 = Application::appObtuselySetting()->value(kWindowManager, kWindowState).toMap();
        state[kWidth] = state1.value(kWidth).toInt();
        state[kHeight] = state1.value(kHeight).toInt();
        state[kState] = static_cast<int>(states);
    }
    Application::appObtuselySetting()->setValue(kWindowManager, kWindowState, state);
}

/*!
 * \return 从配置文件中取得侧边栏的宽度，仅在初始化时调用
 */
void PanelPrivate::loadSidebarState()
{
    const QVariantMap &state = Application::appObtuselySetting()->value(kWindowManager, kSplitterState).toMap();
    leftWidth = state.value(kSidebar, PanelPrivate::kDefaultLeftWidth).toInt();
    manualHideSidebar = state.value(kManualHide, false).toBool();
    autoHideSidebar = state.value(kAutoHide, false).toBool();

    Q_Q(Panel);
    // 更新 qml 组件中的设置
    q->setShowSidebar(!(manualHideSidebar || autoHideSidebar));
    Q_EMIT q->sidebarStateChanged(leftWidth, manualHideSidebar, autoHideSidebar);
}

void PanelPrivate::saveSidebarState() const
{
    QVariantMap state;
    state[kSidebar] = leftWidth;
    state[kManualHide] = manualHideSidebar;
    state[kAutoHide] = autoHideSidebar;
    Application::appObtuselySetting()->setValue(kWindowManager, kSplitterState, state);
}

/*!
 * \class Panel
 * \brief 面板/窗体，用于弹出式的 QML 窗体
 * \details 此类主要用于关联 QML 组件，进行 QML / C++ 间通信
 * \note Panel 的 componentUrl 需要指向基于 QQuickWindow 的 QML 组件
 */
Panel::Panel(QObject *parent)
    : Containment(*new PanelPrivate(this), parent)
{
}

/*!
 * \return 返回当前 Panel 关联的 QQuickWindow 窗体指针
 */
QQuickWindow *Panel::window() const
{
    return d_func()->window;
}

quint64 Panel::windId() const
{
    Q_D(const Panel);
    return d->window ? d->window->winId() : 0;
}

bool Panel::showSidebar() const
{
    return d_func()->showSidebar;
}

void Panel::setShowSidebar(bool b)
{
    Q_D(Panel);
    if (d->showSidebar != b) {
        d->showSidebar = b;
        Q_EMIT showSidebarChanged(b);
    }
}

/*!
 * \brief 窗口关闭时会更新侧边栏的宽度和手动/自动隐藏状态，但仅在最后一个窗口关闭时触发保存
 * \sa saveState()
 */
void Panel::setSidebarState(int width, bool manualHide, bool autoHide)
{
    Q_D(Panel);
    d->leftWidth = width;
    d->manualHideSidebar = manualHide;
    d->autoHideSidebar = autoHide;
}

void Panel::installTitleBar(Applet *titlebar)
{
}

void Panel::installSideBar(Applet *sidebar)
{
}

void Panel::installWorkSpace(Applet *workspace)
{
}

void Panel::installDetailView(Applet *detailview)
{
}

Applet *Panel::titleBar() const
{
    return d_func()->titlebar;
}

Applet *Panel::sideBar() const
{
    return d_func()->sidebar;
}

Applet *Panel::workSpace() const
{
    return d_func()->workspace;
}

Applet *Panel::detailView() const
{
    return d_func()->detailview;
}

/*!
 * \brief 加载缓存配置的窗口设置，重载此函数自定义加载流程
 */
void Panel::loadState()
{
    Q_ASSERT_X(window(), "Init window state", "Window must exist when load state");

    Q_D(Panel);
    d->loadWindowState();
    // 设置窗口后再更新侧边栏
    d->loadSidebarState();
}

/*!
 * \brief 保存窗口设置到缓存，重载此函数自定义加载流程
 */
void Panel::saveState()
{
    Q_ASSERT_X(window(), "Init window state", "Window must exist when save state.");

    Q_D(Panel);
    d->saveSidebarState();
    d->saveWindowState();
}

DFMGUI_END_NAMESPACE
