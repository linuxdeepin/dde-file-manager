// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "windowframe_p.h"

#include "desktoputils/widgetutil.h"
#include "desktoputils/ddpugin_eventinterface_helper.h"

#include <QWindow>

DDPCORE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

#define CanvasCoreSubscribe(topic, func) \
    dpfSignalDispatcher->subscribe("ddplugin_core", QT_STRINGIFY2(topic), this, func);

#define CanvasCoreUnsubscribe(topic, func) \
    dpfSignalDispatcher->unsubscribe("ddplugin_core", QT_STRINGIFY2(topic), this, func);

WindowFramePrivate::WindowFramePrivate(WindowFrame *parent)
    : QObject(parent), q(parent)
{
}

void WindowFramePrivate::updateProperty(BaseWindowPointer win, ScreenPointer screen, bool primary)
{
    if (win && screen) {
        win->setProperty(DesktopFrameProperty::kPropScreenName, screen->name());
        win->setProperty(DesktopFrameProperty::kPropIsPrimary, primary);
        win->setProperty(DesktopFrameProperty::kPropScreenGeometry, screen->geometry());
        win->setProperty(DesktopFrameProperty::kPropScreenAvailableGeometry, screen->availableGeometry());
        win->setProperty(DesktopFrameProperty::kPropScreenHandleGeometry, screen->handleGeometry());
        win->setProperty(DesktopFrameProperty::kPropWidgetName, "root");
        win->setProperty(DesktopFrameProperty::kPropWidgetLevel, "0.0");
    }
}

BaseWindowPointer WindowFramePrivate::createWindow(ScreenPointer sp)
{
    BaseWindowPointer win(new BaseWindow);
    win->init();
    win->setGeometry(sp->geometry());   // 经过缩放的区域
    fmDebug() << "screen name" << sp->name() << "geometry" << sp->geometry() << win.get();

    ddplugin_desktop_util::setDesktopWindow(win.get());
    // the Desktop Window is opaque though it has been setted Qt::WA_TranslucentBackground
    // uing setOpacity to set opacity for Desktop Window to be transparent.
    auto handle = win->windowHandle();
    handle->setOpacity(0.99);
    fmInfo() << "set desktop flag for window" << sp->name() << win->winId() << "handle" << handle;

    traceWindow(handle);

    return win;
}

void WindowFramePrivate::traceWindow(QWindow *win) const
{
    if (!win)
        return;

    connect(win, &QWindow::xChanged, this, &WindowFramePrivate::xChanged);
    connect(win, &QWindow::yChanged, this, &WindowFramePrivate::yChanged);
    connect(win, &QWindow::widthChanged, this, &WindowFramePrivate::widthChanged);
    connect(win, &QWindow::heightChanged, this, &WindowFramePrivate::heightChanged);
}

void WindowFramePrivate::xChanged(int arg) const
{
    fmInfo() << "root window" << sender() << "x change to" << arg;
}

void WindowFramePrivate::yChanged(int arg) const
{
    fmInfo() << "root window" << sender() << "y change to" << arg;
}

void WindowFramePrivate::widthChanged(int arg) const
{
    fmInfo() << "root window" << sender() << "width change to" << arg;
}

void WindowFramePrivate::heightChanged(int arg) const
{
    fmInfo() << "root window" << sender() << "height change to" << arg;
}

WindowFrame::WindowFrame(QObject *parent)
    : AbstractDesktopFrame(parent), d(new WindowFramePrivate(this))
{
}

WindowFrame::~WindowFrame()
{
    CanvasCoreUnsubscribe(signal_ScreenProxy_ScreenChanged, &WindowFrame::buildBaseWindow);
    CanvasCoreUnsubscribe(signal_ScreenProxy_DisplayModeChanged, &WindowFrame::buildBaseWindow);
    CanvasCoreUnsubscribe(signal_ScreenProxy_ScreenGeometryChanged, &WindowFrame::onGeometryChanged);
    CanvasCoreUnsubscribe(signal_ScreenProxy_ScreenAvailableGeometryChanged, &WindowFrame::onAvailableGeometryChanged);
}

bool WindowFrame::init()
{
    CanvasCoreSubscribe(signal_ScreenProxy_ScreenChanged, &WindowFrame::buildBaseWindow);
    CanvasCoreSubscribe(signal_ScreenProxy_DisplayModeChanged, &WindowFrame::buildBaseWindow);
    CanvasCoreSubscribe(signal_ScreenProxy_ScreenGeometryChanged, &WindowFrame::onGeometryChanged);
    CanvasCoreSubscribe(signal_ScreenProxy_ScreenAvailableGeometryChanged, &WindowFrame::onAvailableGeometryChanged);

    return true;
}

QList<QWidget *> WindowFrame::rootWindows() const
{
    QList<QWidget *> ret;
    QList<QString> appended;
    auto scs = ddplugin_desktop_util::screenProxyLogicScreens();

    QReadLocker lk(&d->locker);
    for (ScreenPointer s : scs) {
        if (appended.contains(s->name())) {
            fmCritical() << "Duplicate name screen" << s->name()
                         << s->geometry() << "appended" << appended;
            continue;
        }
        if (auto win = d->windows.value(s->name())) {
            ret << win.get();
            appended << s->name();
        }
    }

    return ret;
}

void WindowFrame::layoutChildren()
{
    for (const BaseWindowPointer &root : d->windows.values()) {
        QList<QWidget *> subWidgets;
        const QString screen = d->windows.key(root);
        // find subwidgets
        for (QObject *obj : root->children()) {
            if (QWidget *wid = qobject_cast<QWidget *>(obj)) {
                auto var = wid->property(DesktopFrameProperty::kPropWidgetLevel);
                if (var.isValid()) {
                    subWidgets.append(wid);
                    fmDebug() << screen << "subwidget" << wid->property(DesktopFrameProperty::kPropWidgetName).toString() << "level" << var.toDouble();
                } else {
                    fmWarning() << screen << "subwidget" << wid << "no WidgetLevel property ";
                }
            }
        }

        // sort by level
        std::sort(subWidgets.begin(), subWidgets.end(), [](const QWidget *before, const QWidget *after) -> bool {
            QVariant var1 = before->property(DesktopFrameProperty::kPropWidgetLevel);
            QVariant var2 = after->property(DesktopFrameProperty::kPropWidgetLevel);
            return var1.toDouble() < var2.toDouble();
        });

        // must stack from top to bottom
        for (int i = subWidgets.size() - 1; i > 0; --i) {
            QWidget *top = subWidgets.at(i);
            QWidget *bottom = subWidgets.at(i - 1);
            bottom->stackUnder(top);
        }
    }
}

QStringList WindowFrame::bindedScreens()
{
    return d->windows.keys();
}

void WindowFrame::buildBaseWindow()
{
    // tell other module that the base windows will be rebuild.
    emit windowAboutToBeBuilded();

    DisplayMode mode = ddplugin_desktop_util::screenProxyLastChangedMode();
    auto screens = ddplugin_desktop_util::screenProxyLogicScreens();
    fmInfo() << "screen mode:" << mode << "screen count:" << screens.size();

    QWriteLocker lk(&d->locker);
    // 实际是单屏
    if ((DisplayMode::kShowonly == mode) || (DisplayMode::kDuplicate == mode)   // 仅显示和复制
        || (screens.count() == 1)) {   // 单屏模式

        ScreenPointer primary = ddplugin_desktop_util::screenProxyPrimaryScreen();
        if (primary == nullptr) {
            fmCritical() << "get primary screen failed return";
            //清空并通知重建
            d->windows.clear();
            lk.unlock();
            emit windowBuilded();
            return;
        }

        BaseWindowPointer winPtr = d->windows.value(primary->name());
        d->windows.clear();
        if (!winPtr.isNull()) {
            if (winPtr->geometry() != primary->geometry())
                winPtr->setGeometry(primary->geometry());
        } else {
            winPtr = d->createWindow(primary);
        }
        fmInfo() << "primary frame" << primary->name() << primary->geometry();
        d->updateProperty(winPtr, primary, true);
        d->windows.insert(primary->name(), winPtr);

        //! 必须先隐藏，否则后面调用show时无法带动子窗口显示
        winPtr->hide();
    } else {
        //多屏
        for (auto screenName : d->windows.keys()) {
            // 删除实际不存在的数据
            if (!ddplugin_desktop_util::screenProxyScreen(screenName)) {
                fmInfo() << "screen:" << screenName << "is invalid, delete frame.";
                d->windows.remove(screenName);
            }
        }
        auto primary = ddplugin_desktop_util::screenProxyPrimaryScreen();
        fmInfo() << "primary screen:" << primary->name();
        for (ScreenPointer s : screens) {
            BaseWindowPointer winPtr = d->windows.value(s->name());
            if (!winPtr.isNull()) {
                if (winPtr->geometry() != s->geometry())
                    winPtr->setGeometry(s->geometry());
                fmInfo() << "update frame" << s->name() << "win" << winPtr->geometry() << "screen" << s->geometry();
            } else {
                // 添加缺少的数据
                winPtr = d->createWindow(s);
                d->windows.insert(s->name(), winPtr);
                fmInfo() << "screen:" << s->name() << s->geometry() << " added, create frame." << winPtr->geometry();
            }

            d->updateProperty(winPtr, s, (s == primary));
            //! 必须先隐藏，否则后面调用show时无法带动子窗口显示
            winPtr->hide();
        }
    }

    lk.unlock();

    emit windowBuilded();

    layoutChildren();

    for (auto win : d->windows) {
        // the root windows must be hide before show
        Q_ASSERT(win->isVisible() == false);
        win->show();
    }

    emit windowShowed();
}

void WindowFrame::onGeometryChanged()
{
    bool changed = false;
    auto primary = ddplugin_desktop_util::screenProxyPrimaryScreen();
    for (ScreenPointer sp : ddplugin_desktop_util::screenProxyLogicScreens()) {
        auto win = d->windows.value(sp->name());
        fmDebug() << "screen geometry change:" << sp.get() << win.get();
        if (win.get() != nullptr) {
            // Don't continue! dde-shell change the window geometry,
            // always send geometryChanged in the scene
            if (win->geometry() == sp->geometry())
                fmDebug() << "Window geometry is equal to logic geomertry" << win->geometry() << sp->geometry();

            fmInfo() << "root geometry change from" << win->geometry() << "to" << sp->geometry()
                     << "screen name" << sp->name();
            win->setGeometry(sp->geometry());
            d->updateProperty(win, sp, (sp == primary));
            changed = true;
        } else {
            fmWarning() << "no window for" << sp->name();
        }
    }

    if (changed)
        emit geometryChanged();
}

void WindowFrame::onAvailableGeometryChanged()
{
    bool changed = false;
    auto primary = ddplugin_desktop_util::screenProxyPrimaryScreen();
    for (ScreenPointer sp : ddplugin_desktop_util::screenProxyLogicScreens()) {
        auto win = d->windows.value(sp->name());
        fmDebug() << "screen available geometry change:" << sp.get() << win.get();
        if (win.get() != nullptr) {
            if (win->property(DesktopFrameProperty::kPropScreenAvailableGeometry).toRect() == sp->availableGeometry())
                continue;
            d->updateProperty(win, sp, (sp == primary));
            changed = true;
        } else {
            fmWarning() << "no window for" << sp->name();
        }
    }

    if (changed)
        emit availableGeometryChanged();
}
