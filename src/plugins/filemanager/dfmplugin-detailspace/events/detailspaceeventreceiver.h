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
#ifndef DETAILSPACEEVENTRECEIVER_H
#define DETAILSPACEEVENTRECEIVER_H

#include "dfmplugin_detailspace_global.h"

#include "services/filemanager/titlebar/titlebar_defines.h"

#include <QObject>

DPDETAILSPACE_BEGIN_NAMESPACE

class DetailSpaceEventReceiver final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DetailSpaceEventReceiver)

public:
    static DetailSpaceEventReceiver *instance();

public slots:
    void handleTileBarShowDetailView(quint64 windowId, bool checked);

private:
    explicit DetailSpaceEventReceiver(QObject *parent = nullptr);
};

DPDETAILSPACE_END_NAMESPACE

#endif   // DETAILSPACEEVENTRECEIVER_H
