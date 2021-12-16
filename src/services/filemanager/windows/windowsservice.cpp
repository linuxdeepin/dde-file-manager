/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "windowsservice.h"
#include "private/windowsservice_p.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/utils/finallyutil.h"

#include <QDebug>
#include <QEvent>
#include <QApplication>
#include <QX11Info>
#include <QScreen>

DSB_FM_BEGIN_NAMESPACE

using dfmbase::Application;
using dfmbase::FileManagerWindow;

/*!
 * \class WindowServicePrivate
 * \brief
 */

WindowsServicePrivate::WindowsServicePrivate(WindowsService *serv)
    : QObject(nullptr), service(serv)
{
}

FileManagerWindow *WindowsServicePrivate::activeExistsWindowByUrl(const QUrl &url)
{
    int count = windows.count();

    for (int i = 0; i != count; ++i) {
        quint64 key = windows.keys().at(i);
        auto window = windows.value(key);
        if (window && window->currentUrl() == url) {
            qInfo() << "Find url: " << url << " window: " << window;
            if (window->isMinimized())
                window->setWindowState(window->windowState() & ~Qt::WindowMinimized);
            qApp->setActiveWindow(window);
            return window;
        }
    }

    return nullptr;
}

void WindowsServicePrivate::moveWindowToScreenCenter(FileManagerWindow *window)
{
    QPoint pos = QCursor::pos();
    QRect currentScreenGeometry;

    for (QScreen *screen : qApp->screens()) {
        if (screen->geometry().contains(pos)) {
            currentScreenGeometry = screen->geometry();
        }
    }

    if (currentScreenGeometry.isEmpty()) {
        currentScreenGeometry = qApp->primaryScreen()->geometry();
    }

    window->moveCenter(currentScreenGeometry.center());
}

/*!
 * \class WindowService
 * \brief
 */

WindowsService::WindowsService(QObject *parent)
    : dpf::PluginService(parent),
      dpf::AutoServiceRegister<WindowsService>(),
      d(new WindowsServicePrivate(this))
{
}

WindowsService::~WindowsService()
{
    for (auto val : d->windows.values()) {
        // 框架退出非程序退出，依然会存在QWidget相关操作，
        // 如果强制使用delete，那么将导致Qt机制的与懒汉单例冲突崩溃
        if (val)
            val->deleteLater();
    }
    d->windows.clear();
}

WindowsService::FMWindow *WindowsService::showWindow(const QUrl &url, bool isNewWindow, QString *errorString)
{
    QString error;
    dfmbase::FinallyUtil finally([&]() { if (errorString) *errorString = error; });

    if (url.isEmpty()) {
        error = "Can't new window use empty url";
        return nullptr;
    }

    if (!url.isValid()) {
        error = "Can't new window use not valid ur";
        return nullptr;
    }

    if (!dfmbase::UrlRoute::hasScheme(url.scheme())) {
        error = QString("No related scheme is registered "
                        "in the route form %0")
                        .arg(url.scheme());
        return nullptr;
    }

    // TODO(zhangs): isInitAppOver isAppQuiting, ref: WindowManager::showNewWindow

    // Directly active window if the window exists
    if (!isNewWindow) {
        auto window = d->activeExistsWindowByUrl(url);
        if (window)
            return window;
        else
            qWarning() << "Cannot find a exists window by url: " << url;
    }

    QX11Info::setAppTime(QX11Info::appUserTime());
    auto window = new FMWindow(url.isEmpty() ? Application::instance()->appUrlAttribute(Application::AA_UrlOfNewWindow) : url);
    // TODO(zhangs): loadWindowState
    // TODO(zhangs): aboutToClose
    // TODO(zhangs): requestToSelectUrls
    qInfo() << "New window created: " << window->winId() << url;
    d->windows.insert(window->internalWinId(), window);

    if (d->windows.size() == 1)
        d->moveWindowToScreenCenter(window);

    finally.dismiss();
    window->show();
    qApp->setActiveWindow(window);
    emit windowOpended(window->internalWinId());
    return window;
}

quint64 WindowsService::findWindowId(const QWidget *window)
{
    int count = d->windows.size();
    for (int i = 0; i != count; ++i) {
        quint64 key = d->windows.keys().at(i);
        if (d->windows.value(key) == window->topLevelWidget())
            return key;
    }

    const QWidget *newWindow = window;

    while (newWindow) {
        if (newWindow->inherits("FileManagerWindow"))
            return newWindow->winId();

        newWindow = newWindow->parentWidget();
    }

    return window->window()->internalWinId();
}

WindowsService::FMWindow *WindowsService::findWindowById(quint64 winId)
{
    if (winId <= 0)
        return nullptr;

    if (d->windows.contains(winId))
        return d->windows.value(winId);

    qWarning() << "The `d->windows` cannot find winId: " << winId;
    for (QWidget *top : qApp->topLevelWidgets()) {
        if (top->internalWinId() == winId)
            return qobject_cast<WindowsService::FMWindow *>(top);
    }

    return nullptr;
}

QList<quint64> WindowsService::windowIdList()
{
    return d->windows.keys();
}

DSB_FM_END_NAMESPACE
