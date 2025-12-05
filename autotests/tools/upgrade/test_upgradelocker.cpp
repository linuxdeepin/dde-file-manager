// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/upgradelocker.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QSharedMemory>
#include <QString>

using namespace testing;
using namespace dfm_upgrade;

TEST(UpgradeLocker, constructor)
{
    UpgradeLocker locker;
    EXPECT_EQ(locker.sharedMemory.key(), QString("dde-file-manager-upgrade-locker"));
}

TEST(UpgradeLocker, isLock_attach_success)
{
    UpgradeLocker locker;
    
    stub_ext::StubExt stub;
    int attachCallCount = 0;
    bool detachCalled = false;
    bool createCalled = false;
    
    // 第一次attach调用（清理旧的共享内存）
    // 第二次attach调用（检查是否已有进程）
    stub.set_lamda(static_cast<bool (QSharedMemory::*)(QSharedMemory::AccessMode)>(&QSharedMemory::attach), [&attachCallCount]() {
        attachCallCount++;
        if (attachCallCount == 2) {
            return true; // 第二次调用返回true，表示已有进程在运行
        }
        return false;
    });
    
    stub.set_lamda(static_cast<bool (QSharedMemory::*)()>(&QSharedMemory::detach), [&detachCalled]() {
        detachCalled = true;
        return true;
    });
    
    stub.set_lamda(static_cast<bool (QSharedMemory::*)(qsizetype, QSharedMemory::AccessMode)>(&QSharedMemory::create), [&createCalled]() {
        createCalled = true;
        return true;
    });
    
    // 测试当第二次attach返回true时的情况（表示已经有进程在运行）
    EXPECT_TRUE(locker.isLock());
    EXPECT_EQ(attachCallCount, 2);
    EXPECT_TRUE(detachCalled);
    EXPECT_TRUE(createCalled);
}

TEST(UpgradeLocker, isLock_attach_fail)
{
    UpgradeLocker locker;
    
    stub_ext::StubExt stub;
    int attachCallCount = 0;
    bool detachCalled = false;
    bool createCalled = false;
    
    // 第一次attach调用（清理旧的共享内存）
    // 第二次attach调用（检查是否已有进程）
    stub.set_lamda(static_cast<bool (QSharedMemory::*)(QSharedMemory::AccessMode)>(&QSharedMemory::attach), [&attachCallCount]() {
        attachCallCount++;
        return false; // 总是返回false，表示没有进程在运行
    });
    
    stub.set_lamda(static_cast<bool (QSharedMemory::*)()>(&QSharedMemory::detach), [&detachCalled]() {
        detachCalled = true;
        return true;
    });
    
    stub.set_lamda(static_cast<bool (QSharedMemory::*)(qsizetype, QSharedMemory::AccessMode)>(&QSharedMemory::create), [&createCalled]() {
        createCalled = true;
        return true;
    });
    
    // 测试当第二次attach返回false时的情况（表示没有进程在运行）
    EXPECT_FALSE(locker.isLock());
    EXPECT_EQ(attachCallCount, 2);
    EXPECT_TRUE(detachCalled);
    EXPECT_TRUE(createCalled);
}