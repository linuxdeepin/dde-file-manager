// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "burnsignalmanager.h"

DPBURN_USE_NAMESPACE

BurnSignalManager *BurnSignalManager::instance()
{
    static BurnSignalManager ins;
    return &ins;
}

BurnSignalManager::BurnSignalManager(QObject *parent)
    : QObject(parent)
{
}
