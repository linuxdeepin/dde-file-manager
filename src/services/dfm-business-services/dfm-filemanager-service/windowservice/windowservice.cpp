/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
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
#include "windowservice.h"
#include "browseview.h"

#include "dfm-base/base/urlroute.h"

#include <QEvent>
#include <QDebug>
#include <QToolButton>
#include <QCoreApplication>

DSB_FM_BEGIN_NAMESPACE

WindowService::WindowService()
{

}

WindowService::~WindowService()
{
    for (auto val : windowList) {
        // 框架退出非程序退出，依然会存在QWidget相关操作，
        // 如果强制使用delete，那么将导致Qt机制的崩溃
        if (val) val->deleteLater();
        windowList.removeOne(val);
    }
}

bool WindowService::removeSideBarItem(int windowIndex, SideBarItem *item)
{
    if (windowIndex < 0)
        return false;

    if (windowIndex >= windowList.size())
        return false;

    return windowList[windowIndex]->sidebar()->removeItem(item);
}

bool WindowService::insertSideBarItem(int windowIndex, int row, SideBarItem *item)
{
    if (windowIndex < 0)
        return false;

    if (windowIndex >= windowList.size())
        return false;

    return  windowList[windowIndex]->sidebar()->insertItem(row, item);
}

bool WindowService::addSideBarItem(int windowIndex, SideBarItem *item)
{
    if (windowIndex < 0)
        return false;

    if (windowIndex >= windowList.size())
        return false;

    if (-1 != windowList[windowIndex]->sidebar()->addItem(item))
        return true;
    else
        return false;
}

BrowseWindow *WindowService::newWindow()
{
    auto window = new BrowseWindow();
    windowList.append(window);
    return window;
}

bool WindowService::setWindowRootUrl(BrowseWindow *newWindow, const QUrl &url, QString *errorString)
{
    if (!url.isValid()) {
        if (errorString) {
            * errorString = QObject::tr("can't new window use not valid url");
            qWarning() << Q_FUNC_INFO << "can't new window use not valid url";
        }
        return false;
    }

    if (url.isEmpty())
    {
        if (errorString) {
            * errorString = QObject::tr("can't new window use empty url");
            qWarning() << Q_FUNC_INFO << "can't new window use empty url";
        }
        return false;
    }

    if (!UrlRoute::hasScheme(url.scheme())) {
        if (errorString) {
            * errorString = QObject::tr("No related scheme is registered "
                                        "in the route form %0").arg(url.scheme());

            qWarning() << Q_FUNC_INFO
                       << QString("No related scheme is registered "
                                  "in the route form %0").arg(url.scheme());
        }
        return false;
    }


    if (newWindow) {
        if (!newWindow->viewIsAdded(url.scheme())) {
            QString errorString;
            BrowseView* view = BrowseWidgetFactory::create<BrowseView>(url);
            if (!view) {
                if (errorString.isEmpty()) errorString = "Unknown error";
                qWarning() << Q_FUNC_INFO << errorString;
            } else {
                newWindow->addView(url.scheme(), view);
            }
        }
        newWindow->setRootUrl(url);
    }

    return true;
}

DSB_FM_END_NAMESPACE
