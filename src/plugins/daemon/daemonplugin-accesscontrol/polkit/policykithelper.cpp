/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "policykithelper.h"

DAEMONPAC_USE_NAMESPACE

PolicyKitHelper *PolicyKitHelper::instance()
{
    static PolicyKitHelper instance;
    return &instance;
}

bool PolicyKitHelper::checkAuthorization(const QString &actionId, const QString &appBusName)
{
    using namespace PolkitQt1;

    if(appBusName.isEmpty()) {
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
