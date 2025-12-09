// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "upgradeinterface.h"
#include "core/upgradelocker.h"
#include "core/upgradefactory.h"
#include "dialog/processdialog.h"
#include "units/unitlist.h"
#include "utils/crashhandle.h"

#include "builtininterface.h"

#include "stubext.h"

#include <gtest/gtest.h>

using namespace testing;
using namespace dfm_upgrade;

class DoUpgradeTest : public Test
{
public:
    void SetUp() override {
        system("touch /var/tmp/dfm-upgraded.lock");
        stub.set_lamda(&upgradeConfigDir, [](){
            return QString("/var/tmp");
        });

        stub.set_lamda(&createUnits, [](){
            return QList<QSharedPointer<UpgradeUnit>>();
        });

        stub.set_lamda(&CrashHandle::isCrashed, [this](){
            return crashed;
        });

        stub.set_lamda(&CrashHandle::clearCrash, [this](){
            cleared = true;
        });

        stub.set_lamda(&CrashHandle::regSignal, [this](){
            reged = true;
        });

        stub.set_lamda(&ProcessDialog::restart, [this](){
            restarted = true;
        });

    }
    void TearDown() override {
        system("rm /var/tmp/dfm-upgraded.lock");
        stub.clear();
    }

    stub_ext::StubExt stub;
    bool crashed = false;
    bool cleared = false;
    bool reged = false;
    bool restarted = false;
};


TEST_F(DoUpgradeTest, empty_args)
{
    EXPECT_EQ(-1, dfm_tools_upgrade_doUpgrade({}));
    EXPECT_TRUE(QFile::exists("/var/tmp/dfm-upgraded.lock"));
    EXPECT_FALSE(cleared);
    EXPECT_FALSE(restarted);
    EXPECT_TRUE(reged);
}

TEST_F(DoUpgradeTest, locked)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&UpgradeLocker::isLock, [](){
        return true;
    });

    QMap<QString, QString> args;
    args.insert(kArgDesktop, "6.0.0");
    EXPECT_EQ(-1, dfm_tools_upgrade_doUpgrade(args));
    EXPECT_TRUE(QFile::exists("/var/tmp/dfm-upgraded.lock"));
    EXPECT_FALSE(cleared);
    EXPECT_FALSE(restarted);
    EXPECT_TRUE(reged);
}

TEST_F(DoUpgradeTest, noneed)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&UpgradeLocker::isLock, [](){
        return false;
    });

    stub.set_lamda(&isNeedUpgrade, [](){
        return false;
    });

    QMap<QString, QString> args;
    args.insert(kArgDesktop, "6.0.0");
    EXPECT_EQ(-1, dfm_tools_upgrade_doUpgrade(args));
    EXPECT_TRUE(QFile::exists("/var/tmp/dfm-upgraded.lock"));
    EXPECT_FALSE(cleared);
    EXPECT_FALSE(restarted);
    EXPECT_TRUE(reged);
}


TEST_F(DoUpgradeTest, reject)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&UpgradeLocker::isLock, [](){
        return false;
    });

    stub.set_lamda(&ProcessDialog::execDialog, [](){
        return false;
    });

    QMap<QString, QString> args;
    args.insert(kArgDesktop, "6.0.0");
    EXPECT_EQ(-1, dfm_tools_upgrade_doUpgrade(args));
    EXPECT_TRUE(QFile::exists("/var/tmp/dfm-upgraded.lock"));
    EXPECT_FALSE(cleared);
    EXPECT_FALSE(restarted);
    EXPECT_TRUE(reged);
}

TEST_F(DoUpgradeTest, accept)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&UpgradeLocker::isLock, [](){
        return false;
    });

    stub.set_lamda(&ProcessDialog::execDialog, [](){
        return true;
    });

    QMap<QString, QString> args;
    args.insert(kArgDesktop, "6.0.0");
    EXPECT_EQ(0, dfm_tools_upgrade_doUpgrade(args));
    EXPECT_FALSE(QFile::exists("/var/tmp/dfm-upgraded.lock"));
    EXPECT_TRUE(cleared);
    EXPECT_TRUE(restarted);
    EXPECT_TRUE(reged);
}

TEST_F(DoUpgradeTest, with_crash)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&UpgradeLocker::isLock, [](){
        return false;
    });

    stub.set_lamda(&ProcessDialog::execDialog, [](){
        return false;
    });

    crashed = true;

    QMap<QString, QString> args;
    args.insert(kArgDesktop, "6.0.0");
    EXPECT_EQ(0, dfm_tools_upgrade_doUpgrade(args));
    EXPECT_FALSE(QFile::exists("/var/tmp/dfm-upgraded.lock"));
    EXPECT_TRUE(cleared);
    EXPECT_FALSE(restarted);
    EXPECT_FALSE(reged);
}

// 测试 dfm_tools_upgrade_doRestart 函数

TEST(RestartTest, empty_args)
{
    EXPECT_EQ(-1, dfm_tools_upgrade_doRestart({}));
}

TEST(RestartTest, desktop_restart_reject)
{
    stub_ext::StubExt stub;
    
    bool initialized = false;
    stub.set_lamda(&ProcessDialog::initialize, [&initialized](ProcessDialog *, bool desktop){
        initialized = true;
        EXPECT_TRUE(desktop); // 应该是桌面模式
    });
    
    bool execDialogCalled = false;
    stub.set_lamda(&ProcessDialog::execDialog, [&execDialogCalled](){
        execDialogCalled = true;
        return false; // 用户拒绝
    });
    
    bool restartCalled = false;
    stub.set_lamda(&ProcessDialog::restart, [&restartCalled](){
        restartCalled = true;
    });

    QMap<QString, QString> args;
    args.insert(kArgDesktop, "6.0.0");
    EXPECT_EQ(-1, dfm_tools_upgrade_doRestart(args));
    EXPECT_TRUE(initialized);
    EXPECT_TRUE(execDialogCalled);
    EXPECT_FALSE(restartCalled);
}

TEST(RestartTest, desktop_restart_accept)
{
    stub_ext::StubExt stub;
    
    bool initialized = false;
    stub.set_lamda(&ProcessDialog::initialize, [&initialized](ProcessDialog *, bool desktop){
        initialized = true;
        EXPECT_TRUE(desktop); // 应该是桌面模式
    });
    
    bool execDialogCalled = false;
    stub.set_lamda(&ProcessDialog::execDialog, [&execDialogCalled](){
        execDialogCalled = true;
        return true; // 用户接受
    });
    
    bool restartCalled = false;
    stub.set_lamda(&ProcessDialog::restart, [&restartCalled](){
        restartCalled = true;
    });

    QMap<QString, QString> args;
    args.insert(kArgDesktop, "6.0.0");
    EXPECT_EQ(0, dfm_tools_upgrade_doRestart(args));
    EXPECT_TRUE(initialized);
    EXPECT_TRUE(execDialogCalled);
    EXPECT_TRUE(restartCalled);
}

TEST(RestartTest, filemanager_restart_accept)
{
    stub_ext::StubExt stub;
    
    bool initialized = false;
    stub.set_lamda(&ProcessDialog::initialize, [&initialized](ProcessDialog *, bool desktop){
        initialized = true;
        EXPECT_FALSE(desktop); // 应该是文件管理器模式
    });
    
    bool execDialogCalled = false;
    stub.set_lamda(&ProcessDialog::execDialog, [&execDialogCalled](){
        execDialogCalled = true;
        return true; // 用户接受
    });
    
    bool restartCalled = false;
    stub.set_lamda(&ProcessDialog::restart, [&restartCalled](){
        restartCalled = true;
    });

    QMap<QString, QString> args;
    args.insert(kArgFileManger, "6.0.0");
    EXPECT_EQ(0, dfm_tools_upgrade_doRestart(args));
    EXPECT_TRUE(initialized);
    EXPECT_TRUE(execDialogCalled);
    EXPECT_TRUE(restartCalled);
}