// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "upgradelocker.h"

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logToolUpgrade)
using namespace dfm_upgrade;

UpgradeLocker::UpgradeLocker()
{
    sharedMemory.setKey("dde-file-manager-upgrade-locker");
}

UpgradeLocker::~UpgradeLocker()
{
}

bool UpgradeLocker::isLock()
{
    //释放因为异常退出而没有正常释放的共享内存
    sharedMemory.attach();
    sharedMemory.detach();

    // 创建1字节大小的共享内存段。
    sharedMemory.create(1);
    // 将当前进程附加到共享内存。
    if (sharedMemory.attach()) {
        // attach成功表明已经存在运行的进程。
        qCInfo(logToolUpgrade) << "There is already existed shared memory";
        return true;
    } else {
        // 不成功表明没有进程运行。
        return false;
    }
}
