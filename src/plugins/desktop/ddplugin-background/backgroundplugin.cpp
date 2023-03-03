// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "backgroundplugin.h"

#include <dfm-framework/listener/listener.h>

DDP_BACKGROUND_USE_NAMESPACE
DPF_USE_NAMESPACE

void BackgroundPlugin::initialize()
{
}

bool BackgroundPlugin::start()
{
    backgroundManager = new BackgroundManager;
    backgroundManager->init();

    return true;
}

void BackgroundPlugin::stop()
{
    delete backgroundManager;
    backgroundManager = nullptr;
}
