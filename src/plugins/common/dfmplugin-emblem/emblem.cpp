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
#include "emblem.h"
#include "events/emblemeventrecevier.h"
#include "events/emblemeventsequence.h"
#include "utils/emblemhelper.h"
#include "utils/emblemmanager.h"

#include "dfm-base/dfm_global_defines.h"
#include "services/filemanager/workspace/workspace_defines.h"

#include <QRectF>

Q_DECLARE_METATYPE(QRectF *)

DPF_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DPEMBLEM_USE_NAMESPACE

void Emblem::initialize()
{
}

bool Emblem::start()
{
    EmblemEventRecevier::instance()->initializeConnections();

    followPaintEvent();

    return true;
}

void Emblem::followPaintEvent()
{
    EmblemEventSequence::sequence()->follow(Workspace::EventType::kPaintListItem, EmblemManager::instance(), &EmblemManager::paintEmblems);
    EmblemEventSequence::sequence()->follow(Workspace::EventType::kPaintIconItem, EmblemManager::instance(), &EmblemManager::paintEmblems);
}
