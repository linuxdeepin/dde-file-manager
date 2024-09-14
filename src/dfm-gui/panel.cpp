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

ShortcutMapPtr PanelPrivate::shortcutPtr = PanelPrivate::windowShortcut();

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

        // 事件过滤设置
        window->installEventFilter(q);
    }

    ContainmentPrivate::setRootObject(item);
}

/*!
 * \brief 从配置中恢复窗口的大小和状态信息
 */
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

/*!
 * \brief 保存当前窗口的大小和状态信息到配置文件
 */
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

/*!
 * \brief 退出时会保存当前侧边栏状态
 */
void PanelPrivate::saveSidebarState() const
{
    QVariantMap state;
    state[kSidebar] = leftWidth;
    state[kManualHide] = manualHideSidebar;
    state[kAutoHide] = autoHideSidebar;
    Application::appObtuselySetting()->setValue(kWindowManager, kSplitterState, state);
}

/*!
 * \brief 处理快捷键事件 \a keyEvent
 * \return 是否有响应此事件，不继续向下传递
 */
bool PanelPrivate::handleKeyPressed(QKeyEvent *keyEvent)
{
    if (!keyEvent)
        return false;

    const int type = shortcutPtr->detectShortcut(keyEvent);
    if (ShortcutMap::kType_Unknown == type)
        return false;

    Q_Q(Panel);
    const QKeyCombination combin = keyEvent->keyCombination();
    Q_EMIT q->shortcutTriggered(static_cast<QuickUtils::ShortcutType>(type), combin);

    return true;
}

/*!
 * \return 创建当前窗口使用的默认快捷键映射
 */
QSharedPointer<ShortcutMap> PanelPrivate::windowShortcut()
{
    const QList<ShortcutMap::KeyBinding> defaultWindowShortcut {
        { QuickUtils::Refresh, Qt::Key_F5 },
        { QuickUtils::ActivateNextTab, Qt::CTRL | Qt::Key_Tab },
        { QuickUtils::ActivatePreviousTab, Qt::CTRL | Qt::SHIFT | Qt::Key_Backtab },
        { QuickUtils::SearchCtrlF, Qt::CTRL | Qt::Key_F },
        { QuickUtils::SearchCtrlL, Qt::CTRL | Qt::Key_L },
        { QuickUtils::Back, Qt::CTRL | Qt::Key_Left },
        { QuickUtils::BackAlias, Qt::ALT | Qt::Key_Left },
        { QuickUtils::Forward, Qt::CTRL | Qt::Key_Right },
        { QuickUtils::ForwardAlias, Qt::ALT | Qt::Key_Right },
        { QuickUtils::CloseCurrentTab, Qt::CTRL | Qt::Key_W },
        { QuickUtils::CreateTab, Qt::CTRL | Qt::Key_T },
        { QuickUtils::CreateWindow, Qt::CTRL | Qt::Key_N },
        { QuickUtils::TriggerActionByIndex, QKeyCombination::fromCombined(Qt::CTRL | ShortcutMap::kKey_NumRange) },
        { QuickUtils::ActivateTabByIndex, QKeyCombination::fromCombined(Qt::ALT | ShortcutMap::kKey_NumRange) },
        { QuickUtils::ShowHotkeyHelp, Qt::CTRL | Qt::SHIFT | Qt::Key_Question }
    };

    ShortcutMapPtr ptr = ShortcutMapPtr::create();
    ptr->addShortcutList(defaultWindowShortcut);
    return ptr;
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

/*!
 * \return 若 Window QML组件已创建，返回对应的 winId ，否则返回 0
 */
quint64 Panel::windId() const
{
    Q_D(const Panel);
    return d->window ? d->window->winId() : 0;
}

/*!
 * \return 返回当前侧边栏的展开状态
 */
bool Panel::showSidebar() const
{
    return d_func()->showSidebar;
}

/*!
 * \brief 更新侧边栏展开状态为 \a b ，将抛出变更信号 showSidebarChanged()
 */
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

/*!
 * \brief 截取部分事件处理，当前用于快捷键处理和双击切换窗口状态
 * \return 是否过滤当前事件
 */
bool Panel::eventFilter(QObject *object, QEvent *event)
{
    Q_D(Panel);
    if (d->window == object) {
        switch (event->type()) {
        case QEvent::KeyPress:
            return d->handleKeyPressed(dynamic_cast<QKeyEvent *>(event));
        default:
            break;
        }
    }
    return false;
}

/*!
 * \brief 将标题栏 \a titlebar 注册到当前窗口，通过 titleBar() 取得
 */
void Panel::installTitleBar(AppletItem *titlebar)
{
    Q_ASSERT(titlebar);
    d_func()->titlebar = titlebar;
    Q_EMIT titleBarInstallFinished();
}

/*!
 * \brief 将侧边栏 \a sidebar 注册到当前窗口，通过 sideBar() 取得
 */
void Panel::installSideBar(AppletItem *sidebar)
{
    Q_ASSERT(sidebar);
    d_func()->sidebar = sidebar;
    Q_EMIT sideBarInstallFinished();
}

/*!
 * \brief 将工作区 \a workspace 注册到当前窗口，通过 workSpace() 取得
 */
void Panel::installWorkSpace(AppletItem *workspace)
{
    Q_ASSERT(workspace);
    d_func()->workspace = workspace;
    Q_EMIT workspaceInstallFinished();
}

/*!
 * \brief 将详细信息 \a detailview 注册到当前窗口，通过 detailView() 取得
 */
void Panel::installDetailView(AppletItem *detailview)
{
    Q_ASSERT(detailview);
    d_func()->detailview = detailview;
    Q_EMIT detailViewInstallFinished();
}

/*!
 * \return 返回注册的侧边栏，未注册时为 nullptr
 */
AppletItem *Panel::titleBar() const
{
    return d_func()->titlebar;
}

/*!
 * \return 返回注册的标题栏，未注册时为 nullptr
 */
AppletItem *Panel::sideBar() const
{
    return d_func()->sidebar;
}

/*!
 * \return 返回注册的工作区，未注册时为 nullptr
 */
AppletItem *Panel::workSpace() const
{
    return d_func()->workspace;
}

/*!
 * \return 返回注册的详细信息，未注册时为 nullptr
 */
AppletItem *Panel::detailView() const
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
