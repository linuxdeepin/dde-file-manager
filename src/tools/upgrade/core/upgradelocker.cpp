/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#include "upgradelocker.h"

#include <QDebug>

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
        qInfo()<<"There is already existed shared memory";
        return true;
    } else {
        // 不成功表明没有进程运行。
        return false;
    }
}
