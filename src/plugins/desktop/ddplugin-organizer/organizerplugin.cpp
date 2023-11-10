// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "organizerplugin.h"
#include "framemanager.h"

using namespace ddplugin_organizer;

void OrganizerPlugin::initialize()
{

}

bool OrganizerPlugin::start()
{
    instance = new FrameManager();   
    bindEvent();

    return instance->initialize();
}

void OrganizerPlugin::stop()
{
    delete instance;
    instance = nullptr;
}

void OrganizerPlugin::bindEvent()
{
    dpfSlotChannel->connect("ddplugin_organizer", "slot_Organizer_Enabled", instance, &FrameManager::organizerEnabled);
}
