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

/*!
 * \brief 加载缓存配置的窗口设置，重载此函数自定义加载流程
 */
void Panel::loadState()
{
    if (!window()) {
        return;
    }

    const QVariantMap &state = Application::appObtuselySetting()->value("WindowManager", "WindowState").toMap();

    int width = state.value("width").toInt();
    int height = state.value("height").toInt();
    NetWmStates windowState = static_cast<NetWmStates>(state.value("state").toInt());

    // fix bug 30932,获取全屏属性，必须是width全屏和height全屏属性都满足，才判断是全屏
    if ((WindowManager::instance()->windowIdList().isEmpty())
        && ((windowState & kNetWmStateMaximizedHorz) != 0 && (windowState & kNetWmStateMaximizedVert) != 0)) {
        // make window to be maximized.
        // the following calling is copyed from QWidget::showMaximized()
        window()->setWindowStates((window()->windowStates() & ~(Qt::WindowMinimized | Qt::WindowFullScreen))
                                  | Qt::WindowMaximized);
    } else {
        window()->resize(width, height);
    }
}

/*!
 * \brief 保存窗口设置到缓存，重载此函数自定义加载流程
 */
void Panel::saveState()
{
    if (!window()) {
        return;
    }

    NetWmStates states { 0 };
    if (WindowUtils::isWayLand()) {
        if (window()->windowStates().testFlag(Qt::WindowMaximized)) {
            states = static_cast<NetWmState>(kNetWmStateMaximizedHorz | kNetWmStateMaximizedVert);
        }
    } else {
        /// The power by dxcb platform plugin
        states = static_cast<NetWmStates>(window()->property("_d_netWmStates").toInt());
    }

    QVariantMap state;
    // fix bug 30932,获取全屏属性，必须是width全屏和height全屏属性都满足，才判断是全屏
    if ((states & kNetWmStateMaximizedHorz) == 0 || (states & kNetWmStateMaximizedVert) == 0) {
        state["width"] = window()->size().width();
        state["height"] = window()->size().height();
    } else {
        const QVariantMap &state1 = Application::appObtuselySetting()->value("WindowManager", "WindowState").toMap();
        state["width"] = state1.value("width").toInt();
        state["height"] = state1.value("height").toInt();
        state["state"] = static_cast<int>(states);
    }
    Application::appObtuselySetting()->setValue("WindowManager", "WindowState", state);
}

DFMGUI_END_NAMESPACE
