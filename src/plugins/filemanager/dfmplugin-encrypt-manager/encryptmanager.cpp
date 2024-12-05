// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "encryptmanager.h"
#include "events/eventreceiver.h"

DPENCRYPTMANAGER_USE_NAMESPACE


void EncryptManager::initialize()
{
    EventReceiver::instance();
}

bool EncryptManager::start()
{
    return true;
}
