/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
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
#include "trasheventcaller.h"

#include "services/filemanager/sidebar/sidebar_defines.h"
#include "dfm-base/dfm_event_defines.h"

#include <dfm-framework/framework.h>
#include <QUrl>

DPTRASH_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

static DPF_NAMESPACE::EventDispatcherManager *dispatcher()
{
    return &dpfInstance.eventDispatcher();
}

void TrashEventCaller::sendOpenWindow(const QUrl &url)
{
    dispatcher()->publish(GlobalEventType::kOpenNewWindow, url);
}

void TrashEventCaller::sendOpenTab(quint64 windowId, const QUrl &url)
{
    dispatcher()->publish(GlobalEventType::kOpenNewTab, windowId, url);
}

void TrashEventCaller::sendOpenFiles(const quint64 windowID, const QList<QUrl> &urls)
{
    dispatcher()->publish(GlobalEventType::kOpenFiles, windowID, urls);
}

void TrashEventCaller::sendEmptyTrash(const quint64 windowID, const QList<QUrl> &urls)
{
    dispatcher()->publish(GlobalEventType::kCleanTrash, windowID, urls);
}
