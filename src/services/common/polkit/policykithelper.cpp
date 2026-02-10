// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "policykithelper.h"

#include <QLoggingCategory>

namespace ServiceCommon {

Q_LOGGING_CATEGORY(serviceCommonLog, "org.deepin.dde.filemanager.service.common")

PolicyKitHelper *PolicyKitHelper::instance()
{
    static PolicyKitHelper instance;
    return &instance;
}

bool PolicyKitHelper::checkAuthorization(const QString &actionId, const QString &appBusName)
{
    using namespace PolkitQt1;

    if (appBusName.isEmpty()) {
        qCWarning(serviceCommonLog) << "PolicyKit check failed: empty bus name";
        return false;
    }

    Authority::Result result = Authority::instance()->checkAuthorizationSync(
            actionId,
            SystemBusNameSubject(appBusName),
            Authority::AllowUserInteraction);

    if (result == Authority::Yes) {
        qCDebug(serviceCommonLog) << "PolicyKit authorization granted for action:" << actionId;
        return true;
    } else {
        qCWarning(serviceCommonLog) << "PolicyKit authorization denied for action:" << actionId;
        return false;
    }
}

PolicyKitHelper::PolicyKitHelper()
{
}

PolicyKitHelper::~PolicyKitHelper()
{
}

}  // namespace ServiceCommon
