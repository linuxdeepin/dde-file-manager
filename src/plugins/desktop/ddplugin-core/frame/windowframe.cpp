/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "windowframe_p.h"

#include "desktoputils/widgetutil.h"

#include <services/desktop/frame/frameservice.h>

#include <QWindow>

DDPCORE_USE_NAMESPACE
DSB_D_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

WindowFramePrivate::WindowFramePrivate(WindowFrame *parent)
    : QObject(parent)
    , q(parent)
{

}

void WindowFramePrivate::updateProperty(BaseWindowPointer win, ScreenPointer screen, bool primary)
{
    if (win && screen) {
        win->setProperty(FrameProperty::kPropScreenName, screen->name());
        win->setProperty(FrameProperty::kPropIsPrimary, primary);
        win->setProperty(FrameProperty::kPropScreenGeometry, screen->geometry());
        win->setProperty(FrameProperty::kPropScreenAvailableGeometry, screen->availableGeometry());
        win->setProperty(FrameProperty::kPropScreenHandleGeometry, screen->handleGeometry());
        win->setProperty(FrameProperty::kPropWidgetName, "root");
        win->setProperty(FrameProperty::kPropWidgetLevel, "0.0");
    }
}

BaseWindowPointer WindowFramePrivate::createWindow(ScreenPointer sp)
{
    BaseWindowPointer win(new BaseWindow);
    win->init();
    win->setGeometry(sp->geometry()); // 经过缩放的区域
    qDebug() << "screen name" << sp->name() << "geometry" << sp->geometry() << win.get();

    ddplugin_desktop_util::setDesktopWindow(win.get());
    // the Desktop Window is opaque though it has been setted Qt::WA_TranslucentBackground
    // uing setOpacity to set opacity for Desktop Window to be transparent.
    win->windowHandle()->setOpacity(0.99);
    return win;
}

WindowFrame::WindowFrame(QObject *parent)
    : AbstractDesktopFrame(parent)
    , d(new WindowFramePrivate(this))
{

}

bool WindowFrame::init()
{
    // get the screen service
    auto &ctx = dpfInstance.serviceContext();
    ScreenService *screenService = ctx.service<ScreenService>(ScreenService::name());
    Q_ASSERT_X(screenService, "WindowFrame", "ScreenService not found.");
    d->screen = screenService;

    connect(screenService, &ScreenService::screenChanged, this, &WindowFrame::buildBaseWindow, Qt::DirectConnection);
    connect(screenService, &ScreenService::displayModeChanged, this, &WindowFrame::buildBaseWindow, Qt::DirectConnection);
    connect(screenService, &ScreenService::screenGeometryChanged, this, &WindowFrame::onGeometryChanged, Qt::DirectConnection);
    connect(screenService, &ScreenService::screenAvailableGeometryChanged, this, &WindowFrame::onAvailableGeometryChanged, Qt::DirectConnection);

    return true;
}

QList<QWidget *> WindowFrame::rootWindows() const
{
    QList<QWidget *> ret;
    QReadLocker lk(&d->locker);
    for (ScreenPointer s : d->screen->logicScreens()) {
        if (auto win = d->windows.value(s->name()))
            ret << win.get();
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
                auto var = wid->property(FrameProperty::kPropWidgetLevel);
                if (var.isValid()) {
                    subWidgets.append(wid);
                    qDebug() << screen << "subwidget" << wid->property(FrameProperty::kPropWidgetName).toString() << "level" << var.toDouble();
                } else {
                    qWarning() << screen << "subwidget" << wid << "no WidgetLevel property ";
                }
            }
        }

        // sort by level
        std::sort(subWidgets.begin(), subWidgets.end(), [](const QWidget *before, const QWidget *after)->bool {
            QVariant var1 = before->property(FrameProperty::kPropWidgetLevel);
            QVariant var2 = after->property(FrameProperty::kPropWidgetLevel);
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

void WindowFrame::buildBaseWindow()
{
    if (!d->screen) {
        qWarning() << "no screen service, give up build.";
        return;
    }

    // tell other module that the base windows will be rebuild.
    emit windowAboutToBeBuilded();

    DisplayMode mode = d->screen->lastChangedMode();
    qInfo() << "screen mode:" << mode << "screen count:" << d->screen->screens().size();

    QWriteLocker lk(&d->locker);
    // 实际是单屏
    if ((DisplayMode::Showonly == mode) || (DisplayMode::Duplicate == mode) // 仅显示和复制
            || (d->screen->screens().count() == 1)) {  // 单屏模式

        ScreenPointer primary = d->screen->primaryScreen();
        if (primary == nullptr) {
            qCritical() << "get primary screen failed return";
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

        d->updateProperty(winPtr, primary, true);
        d->windows.insert(primary->name(), winPtr);
    } else {
        //多屏
        auto screes = d->screen->logicScreens();
        for (auto screenName : d->windows.keys()) {
            // 删除实际不存在的数据
            if (!d->screen->screen(screenName)) {
                qInfo() << "screen:" << screenName << "  invalid, delete it.";
                d->windows.remove(screenName);
            }
        }
        auto primary = d->screen->primaryScreen();
        for (ScreenPointer s : screes) {
            BaseWindowPointer winPtr = d->windows.value(s->name());
            if (!winPtr.isNull()) {
                if (winPtr->geometry() != s->geometry())
                    winPtr->setGeometry(s->geometry());
            } else {
                // 添加缺少的数据
                qInfo() << "screen:" << s->name() << "  added, create it.";
                winPtr = d->createWindow(s);
                d->windows.insert(s->name(), winPtr);
            }

            d->updateProperty(winPtr, s, (s == primary));
        }
    }

    lk.unlock();

    emit windowBuilded();

    layoutChildren();

    for (auto win : d->windows)
        win->show();

    emit windowShowed();
}

void WindowFrame::onGeometryChanged()
{
    if (!d->screen) {
        qWarning() << "no screen service";
        return;
    }

    bool changed = false;
    auto primary = d->screen->primaryScreen();
    for (ScreenPointer sp : d->screen->logicScreens()) {
        auto win = d->windows.value(sp->name());
        qDebug() << "screen geometry change:" << sp.get() << win.get();
        if (win.get() != nullptr) {
            if (win->geometry() == sp->geometry())
                continue;

            qInfo() << "root geometry change from" << win->geometry() << "to" << sp->geometry()
                    << "screen name" << sp->name();
            win->setGeometry(sp->geometry());
            d->updateProperty(win, sp, (sp == primary));
            changed = true;
        }
    }

    if (changed)
        emit geometryChanged();
}

void WindowFrame::onAvailableGeometryChanged()
{
    bool changed = false;
    auto primary = d->screen->primaryScreen();
    for (ScreenPointer sp : d->screen->logicScreens()) {
        auto win = d->windows.value(sp->name());
        qDebug() << "screen available geometry change:" << sp.get() << win.get();
        if (win.get() != nullptr) {
            if (win->property(FrameProperty::kPropScreenAvailableGeometry).toRect() == sp->availableGeometry())
                continue;
            d->updateProperty(win, sp, (sp == primary));
            changed = true;
        }
    }

    if (changed)
        emit availableGeometryChanged();
}


