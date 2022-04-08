/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
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
#include "emblemeventrecevier.h"
#include "utils/emblemmanager.h"
#include "services/common/emblem/emblem_defines.h"

#include "dfm-base/dfm_global_defines.h"
#include <dfm-framework/framework.h>

DFMGLOBAL_USE_NAMESPACE
DPEMBLEM_USE_NAMESPACE

EmblemEventRecevier::EmblemEventRecevier(QObject *parent)
    : QObject(parent)
{
}

EmblemEventRecevier *EmblemEventRecevier::instance()
{
    static EmblemEventRecevier ins;
    return &ins;
}

void EmblemEventRecevier::handlePaintEmblems(QPainter *painter, const QRectF &paintArea, const QUrl &url)
{
    int role = kItemIconRole;
    QRectF rect = paintArea;
    EmblemManager::instance()->paintEmblems(role, url, painter, &rect);
}

void EmblemEventRecevier::initializeConnections() const
{
    DSC_USE_NAMESPACE

    dpfInstance.eventDispatcher().subscribe(Emblem::EventType::kPaintEmblems,
                                            EmblemEventRecevier::instance(),
                                            &EmblemEventRecevier::handlePaintEmblems);
}
