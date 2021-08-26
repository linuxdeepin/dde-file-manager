/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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
#include "dfmwindowservice.h"
#include "dfmbrowseview.h"

#include "dfm-base/base/dfmurlroute.h"
#include "dfm-base/event/dfmglobaleventdispatcher.h"
#include "dfm-base/event/protocol/dfmwindowevent.h"

#include <QEvent>
#include <QDebug>
#include <QToolButton>

DFMWindowService::~DFMWindowService()
{
    for (auto val: m_windowlist) {

        QObject::connect(val,&QObject::destroyed,[=](QObject *obj){
            qInfo() << "destoryed" << obj;
        });

        if (val) delete val;

        m_windowlist.removeOne(val);
    }
}

DFMBrowseWindow *DFMWindowService::newWindow()
{
    auto window = new DFMBrowseWindow();
    m_windowlist.append(window);
    return window;
}

bool DFMWindowService::setWindowRootUrl(DFMBrowseWindow *newWindow, const QUrl &url, QString *errorString)
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

    if (!DFMUrlRoute::hasScheme(url.scheme())) {
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
            DFMBrowseView* view = DFMBrowseWidgetFactory::instance().create<DFMBrowseView>(url);
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

void DFMWindowService::windowEventFilter(QObject *obj, QEvent *event)
{
    auto window = qobject_cast<DFMBrowseWindow*>(obj);
    if (window && m_windowlist.contains(window)
            && event->type() == QEvent::Close)
    {
        windowColseEvent(window, event);
    }
}

void DFMWindowService::windowColseEvent(DFMBrowseWindow *obj, QEvent *event)
{
    Q_UNUSED(event);
    if (m_windowlist.indexOf(obj)){
        m_windowlist.removeOne(obj);
    }
}
