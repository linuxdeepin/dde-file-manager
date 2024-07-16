// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "policykithelper.h"

SERVICESHARECONTROL_USE_NAMESPACE

PolicyKitHelper *PolicyKitHelper::instance()
{
    static PolicyKitHelper instance;
    return &instance;
}

bool PolicyKitHelper::checkAuthorization(const QString &actionId, const QString &appBusName)
{
    using namespace PolkitQt1;

    if (appBusName.isEmpty())
        return false;

    Authority::Result result;
    result = Authority::instance()->checkAuthorizationSync(actionId, SystemBusNameSubject(appBusName),   /// 第一个参数是需要验证的action，和规则文件写的保持一致
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
