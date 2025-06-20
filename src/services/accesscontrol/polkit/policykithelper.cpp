// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "policykithelper.h"

SERVICEACCESSCONTROL_USE_NAMESPACE

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

    // PolkitUnixProcess表示 UNIX 进程的对象。注意：这个设计的对象现在已知已损坏；确定了一种利用 Linux 内核中启动时间延迟的机制。避免调用 `polkit_subject_equal()` 来比较两个进程。
    Authority::Result result = Authority::instance()->checkAuthorizationSync(actionId, SystemBusNameSubject(appBusName),   /// 第一个参数是需要验证的action，和规则文件写的保持一致
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
