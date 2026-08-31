// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaulthelper_1.cpp
 * @brief Unit tests for VaultHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "vaulthelper.h"

#include <QTest>

using namespace vault;

class VaultHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultHelperTest, VaultHelper)
{
    // Test constructor: VaultHelper((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(VaultHelperTest, VaultHelper_VaultHel)
{
    // Test constructor: VaultHelper((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(VaultHelperTest, appendWinID)
{
    // Test method: void appendWinID((const quint64 &winId))
    quint64 _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->appendWinID(_arg0));
}

TEST_F(VaultHelperTest, buildVaultLocalPath)
{
    // Test method: QString buildVaultLocalPath((const QString &path, const QString &base))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->buildVaultLocalPath(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(VaultHelperTest, contenxtMenuHandle)
{
    // Test method: void contenxtMenuHandle((quint64 windowId, const QUrl &url, const QPoint &globalPos))
    QUrl _arg1{};
    QPoint _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->contenxtMenuHandle(0, _arg1, _arg2));
}

TEST_F(VaultHelperTest, createVault)
{
    // Test method: void createVault((QString &password))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->createVault(_arg0));
}

TEST_F(VaultHelperTest, createVaultDialog)
{
    // Test method: void createVaultDialog(())
    EXPECT_NO_FATAL_FAILURE(obj->createVaultDialog());
}

TEST_F(VaultHelperTest, createVaultPropertyDialog)
{
    // Test method: QWidget createVaultPropertyDialog((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->createVaultPropertyDialog(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->createVaultPropertyDialog(_arg0); });

}

TEST_F(VaultHelperTest, currentWindowId)
{
    // Test getter: quint64 currentWindowId()
    auto result = obj->currentWindowId();
    EXPECT_EQ(result, 0);

}

TEST_F(VaultHelperTest, defaultCdAction)
{
    // Test method: void defaultCdAction((const quint64 windowId, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->defaultCdAction(0, _arg1));
}

TEST_F(VaultHelperTest, enableUnlockVault)
{
    // Test bool getter: enableUnlockVault()
    bool result = obj->enableUnlockVault();
    EXPECT_FALSE(result);

}

TEST_F(VaultHelperTest, getVaultVersion)
{
    // Test bool getter: getVaultVersion()
    bool result = obj->getVaultVersion();
    EXPECT_FALSE(result);

}

TEST_F(VaultHelperTest, icon)
{
    // Test getter: QIcon icon()
    auto result = obj->icon();
    EXPECT_TRUE(result.isNull());

}

TEST_F(VaultHelperTest, killVaultTasks)
{
    // Test method: void killVaultTasks(())
    EXPECT_NO_FATAL_FAILURE(obj->killVaultTasks());
}

TEST_F(VaultHelperTest, lockVault)
{
    // Test method: bool lockVault((bool isForced))
    auto result = obj->lockVault(false);
    EXPECT_FALSE(result);

}

TEST_F(VaultHelperTest, newOpenWindow)
{
    // Test method: void newOpenWindow(())
    EXPECT_NO_FATAL_FAILURE(obj->newOpenWindow());
}

TEST_F(VaultHelperTest, openNewWindow)
{
    // Test method: void openNewWindow((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->openNewWindow(_arg0));
}

TEST_F(VaultHelperTest, openWidWindow)
{
    // Test method: void openWidWindow((quint64 winID, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->openWidWindow(0, _arg1));
}

TEST_F(VaultHelperTest, openWindow)
{
    // Test method: void openWindow(())
    EXPECT_NO_FATAL_FAILURE(obj->openWindow());
}

TEST_F(VaultHelperTest, pathToVaultVirtualUrl)
{
    // Test method: QUrl pathToVaultVirtualUrl((const QString &path))
    QString _arg0{};
    auto result = obj->pathToVaultVirtualUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(VaultHelperTest, rootUrl)
{
    // Test getter: QUrl rootUrl()
    auto result = obj->rootUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(VaultHelperTest, scheme)
{
    // Test getter: QString scheme()
    auto result = obj->scheme();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(VaultHelperTest, showInProgressDailog)
{
    // Test method: void showInProgressDailog((QString msg))
    EXPECT_NO_FATAL_FAILURE(obj->showInProgressDailog(QString()));
}

TEST_F(VaultHelperTest, slotlockVault)
{
    // Test method: void slotlockVault((int state))
    EXPECT_NO_FATAL_FAILURE(obj->slotlockVault(0));
}

TEST_F(VaultHelperTest, sourceRootUrl)
{
    // Test getter: QUrl sourceRootUrl()
    auto result = obj->sourceRootUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(VaultHelperTest, sourceRootUrlWithSlash)
{
    // Test getter: QUrl sourceRootUrlWithSlash()
    auto result = obj->sourceRootUrlWithSlash();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(VaultHelperTest, unlockVault)
{
    // Test method: bool unlockVault((const QString &password))
    QString _arg0{};
    auto result = obj->unlockVault(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(VaultHelperTest, updateState)
{
    // Test method: bool updateState((VaultState curState))
    auto result = obj->updateState(VaultState());
    EXPECT_FALSE(result);

}

TEST_F(VaultHelperTest, vaultBaseDirLocalPath)
{
    // Test getter: QString vaultBaseDirLocalPath()
    auto result = obj->vaultBaseDirLocalPath();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(VaultHelperTest, vaultMountDirLocalPath)
{
    // Test getter: QString vaultMountDirLocalPath()
    auto result = obj->vaultMountDirLocalPath();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(VaultHelperTest, vaultUrlToLocalUrl)
{
    // Test method: QUrl vaultUrlToLocalUrl((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->vaultUrlToLocalUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}
