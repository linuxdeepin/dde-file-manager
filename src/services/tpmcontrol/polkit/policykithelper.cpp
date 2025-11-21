// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "policykithelper.h"

SERVICETPMCONTROL_USE_NAMESPACE

PolicyKitHelper *PolicyKitHelper::instance()
{
    static PolicyKitHelper instance;
    return &instance;
}

bool PolicyKitHelper::checkAuthorization(const QString &actionId, const QString &appBusName)
{
    using namespace PolkitQt1;

    if (appBusName.isEmpty()) {
        fmWarning() << "PolicyKit check failed: empty bus name";
        return false;
    }

    Authority::Result result = Authority::instance()->checkAuthorizationSync(
            actionId,
            SystemBusNameSubject(appBusName),
            Authority::AllowUserInteraction);

    if (result == Authority::Yes) {
        fmDebug() << "PolicyKit authorization granted for action:" << actionId;
        return true;
    } else {
        fmWarning() << "PolicyKit authorization denied for action:" << actionId;
        return false;
    }
}

PolicyKitHelper::PolicyKitHelper()
{
}

PolicyKitHelper::~PolicyKitHelper()
{
}
