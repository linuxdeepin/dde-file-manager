// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "backgroundplugin.h"

#include <dfm-framework/listener/listener.h>

DPF_USE_NAMESPACE

namespace ddplugin_background {
DFM_LOG_REGISTER_CATEGORY(DDP_BACKGROUND_NAMESPACE)

void BackgroundPlugin::initialize()
{
}

bool BackgroundPlugin::start()
{
    backgroundManager = new BackgroundManager;
    backgroundManager->init();

    bindEvent();

    return true;
}

void BackgroundPlugin::stop()
{
    delete backgroundManager;
    backgroundManager = nullptr;
}

void BackgroundPlugin::bindEvent()
{
    dpfSlotChannel->connect("ddplugin_background", "slot_FetchUseColorBackground", backgroundManager, &BackgroundManager::useColorBackground);
}

}   // namespace ddplugin_background
