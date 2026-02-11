// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "opticalsignalmanager.h"

using namespace dfmplugin_optical;

OpticalSignalManager *OpticalSignalManager::instance()
{
    static OpticalSignalManager ins;
    return &ins;
}

OpticalSignalManager::OpticalSignalManager(QObject *parent)
    : QObject(parent)
{
}
