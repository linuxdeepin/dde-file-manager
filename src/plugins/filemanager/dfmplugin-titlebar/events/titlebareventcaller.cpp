/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "titlebareventcaller.h"
#include "utils/titlebarhelper.h"
#include "services/filemanager/detailspace/detailspace_defines.h"

#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/base/schemefactory.h"

#include <dfm-framework/framework.h>

DPTITLEBAR_USE_NAMESPACE
DSB_FM_USE_NAMESPACE

void TitleBarEventCaller::sendViewMode(QWidget *sender, DFMBASE_NAMESPACE::Global::ViewMode mode)
{
    quint64 id = TitleBarHelper::windowId(sender);
    Q_ASSERT(id > 0);
    dpfInstance.eventDispatcher().publish(TitleBar::EventType::kSwitchMode, id, int(mode));
}

void TitleBarEventCaller::sendDetailViewState(QWidget *sender, bool checked)
{
    quint64 id = TitleBarHelper::windowId(sender);
    Q_ASSERT(id > 0);
    dpfInstance.eventDispatcher().publish(DSB_FM_NAMESPACE::DetailEventType::kShowDetailView, id, checked);
}

void TitleBarEventCaller::sendCd(QWidget *sender, const QUrl &url)
{
    DFMBASE_USE_NAMESPACE
    quint64 id = TitleBarHelper::windowId(sender);
    Q_ASSERT(id > 0);
    if (!url.isValid()) {
        qWarning() << "Invalid url: " << url;
        return;
    }
    const AbstractFileInfoPointer &info = InfoFactory::create<AbstractFileInfo>(url);
    if (info && info->exists() && info->isFile()) {
        TitleBarEventCaller::sendOpenFile(sender, url);
    } else {
        if (Q_UNLIKELY(TitleBarHelper::handleConnection(sender, url)))
            return;
        dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kChangeCurrentUrl, id, url);
    }
}

void TitleBarEventCaller::sendOpenFile(QWidget *sender, const QUrl &url)
{
    quint64 id = TitleBarHelper::windowId(sender);
    Q_ASSERT(id > 0);
    dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenFiles, id, QList<QUrl>() << url);
}

void TitleBarEventCaller::sendOpenWindow(const QUrl &url)
{
    dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenNewWindow, url);
}

void TitleBarEventCaller::sendOpenTab(quint64 windowId, const QUrl &url)
{
    dpfInstance.eventDispatcher().publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenNewTab, windowId, url);
}

void TitleBarEventCaller::sendSearch(QWidget *sender, const QString &keyword)
{
    quint64 id = TitleBarHelper::windowId(sender);
    Q_ASSERT(id > 0);
    dpfInstance.eventDispatcher().publish(TitleBar::EventType::kDoSearch, id, keyword);
}
