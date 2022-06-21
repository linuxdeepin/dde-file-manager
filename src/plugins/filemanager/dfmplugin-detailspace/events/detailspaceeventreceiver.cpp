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
#include "detailspaceeventreceiver.h"
#include "utils/detailspacehelper.h"
#include "services/filemanager/detailspace/detailspace_defines.h"

#include <dfm-framework/event/event.h>

#include <functional>

DPDETAILSPACE_USE_NAMESPACE
DSB_FM_USE_NAMESPACE

DetailSpaceEventReceiver *DetailSpaceEventReceiver::instance()
{
    static DetailSpaceEventReceiver receiver;
    return &receiver;
}

void DetailSpaceEventReceiver::connectService()
{
    dpfSignalDispatcher->subscribe(DSB_FM_NAMESPACE::DetailEventType::kShowDetailView,
                                   this, &DetailSpaceEventReceiver::handleTileBarShowDetailView);
    dpfSignalDispatcher->subscribe(DSB_FM_NAMESPACE::DetailEventType::kSetDetailViewSelectFileUrl,
                                   this, &DetailSpaceEventReceiver::setSelect);
}

void DetailSpaceEventReceiver::handleTileBarShowDetailView(quint64 windowId, bool checked)
{
    DetailSpaceHelper::showDetailView(windowId, checked);
}

void DetailSpaceEventReceiver::setSelect(quint64 windowId, const QUrl &url)
{
    DetailSpaceHelper::setDetailViewSelectFileUrl(windowId, url);
}

DetailSpaceEventReceiver::DetailSpaceEventReceiver(QObject *parent)
    : QObject(parent)
{
}
