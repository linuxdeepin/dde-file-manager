// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "emblemeventrecevier.h"
#include "utils/emblemmanager.h"

#include <dfm-base/dfm_global_defines.h>

#include <dfm-framework/event/event.h>

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

bool EmblemEventRecevier::handlePaintEmblems(QPainter *painter, const QRectF &paintArea, const FileInfoPointer &info)
{
    int role = kItemIconRole;
    QRectF rect = paintArea;
    return EmblemManager::instance()->paintEmblems(role, info, painter, &rect);
}

void EmblemEventRecevier::initializeConnections() const
{
    dpfSlotChannel->connect(DPF_MACRO_TO_STR(DPEMBLEM_NAMESPACE), "slot_FileEmblems_Paint",
                            EmblemEventRecevier::instance(),
                            &EmblemEventRecevier::handlePaintEmblems);
}
