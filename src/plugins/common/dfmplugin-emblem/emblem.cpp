// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "emblem.h"
#include "events/emblemeventrecevier.h"

DPEMBLEM_USE_NAMESPACE

void Emblem::initialize()
{
    EmblemEventRecevier::instance()->initializeConnections();
}

bool Emblem::start()
{
    return true;
}
