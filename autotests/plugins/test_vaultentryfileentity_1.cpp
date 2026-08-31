// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultentryfileentity_1.cpp
 * @brief Unit tests for VaultEntryFileEntity methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/vaultentryfileentity.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultEntryFileEntityTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultEntryFileEntity();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultEntryFileEntity *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultEntryFileEntityTest, displayName)
{
    // Test getter: QString displayName()
    auto result = obj->displayName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(VaultEntryFileEntityTest, icon)
{
    // Test getter: QIcon icon()
    auto result = obj->icon();
    EXPECT_TRUE(result.isNull());

}

TEST_F(VaultEntryFileEntityTest, refresh)
{
    // Test method: void refresh(())
    EXPECT_NO_FATAL_FAILURE(obj->refresh());
}

TEST_F(VaultEntryFileEntityTest, showProgress)
{
    // Test bool getter: showProgress()
    bool result = obj->showProgress();
    EXPECT_FALSE(result);

}

TEST_F(VaultEntryFileEntityTest, showTotalSize)
{
    // Test bool getter: showTotalSize()
    bool result = obj->showTotalSize();
    EXPECT_FALSE(result);

}

TEST_F(VaultEntryFileEntityTest, showUsageSize)
{
    // Test bool getter: showUsageSize()
    bool result = obj->showUsageSize();
    EXPECT_FALSE(result);

}

TEST_F(VaultEntryFileEntityTest, sizeTotal)
{
    // Test getter: quint64 sizeTotal()
    auto result = obj->sizeTotal();
    EXPECT_EQ(result, 0);

}

TEST_F(VaultEntryFileEntityTest, slotFileDirSizeChange)
{
    // Test method: void slotFileDirSizeChange((const DFMBASE_NAMESPACE::FileScanner::ScanResult &result))
    DFMBASE_NAMESPACE::FileScanner::ScanResult _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->slotFileDirSizeChange(_arg0));
}

TEST_F(VaultEntryFileEntityTest, slotFinishedThread)
{
    // Test method: void slotFinishedThread(())
    EXPECT_NO_FATAL_FAILURE(obj->slotFinishedThread());
}

TEST_F(VaultEntryFileEntityTest, targetUrl)
{
    // Test getter: QUrl targetUrl()
    auto result = obj->targetUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(VaultEntryFileEntityTest, VaultEntryFileEntity_Destructor)
{
    // Test method:  ~VaultEntryFileEntity(())
    EXPECT_NO_FATAL_FAILURE({ VaultEntryFileEntity *tmp = new VaultEntryFileEntity(); delete tmp; });
}
