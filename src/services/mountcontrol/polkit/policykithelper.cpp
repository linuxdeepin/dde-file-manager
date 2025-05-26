// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "policykithelper.h"

SERVICEMOUNTCONTROL_USE_NAMESPACE

PolicyKitHelper *PolicyKitHelper::instance()
{
    static PolicyKitHelper instance;
    return &instance;
}

bool PolicyKitHelper::checkAuthorization(const QString &actionId, const QString &appBusName)
{
    using namespace PolkitQt1;

    if (appBusName.isEmpty()) {
        return false;
    }

    Authority::Result result = Authority::instance()->checkAuthorizationSync(
            actionId, 
            SystemBusNameSubject(appBusName),
            Authority::AllowUserInteraction);
    
    if (result == Authority::Yes) {
        return true;
    } else {
        return false;
    }
}

PolicyKitHelper::PolicyKitHelper()
{
}

PolicyKitHelper::~PolicyKitHelper()
{
} 