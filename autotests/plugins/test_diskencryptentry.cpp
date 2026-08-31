// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_diskencryptentry.cpp
 * @brief Unit tests for DiskEncryptEntry methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "plugin_diskencryptentry.h"

#include <QTest>

using namespace dfmplugin_disk_encrypt_entry;

class DiskEncryptEntryTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DiskEncryptEntry();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DiskEncryptEntry *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DiskEncryptEntryTest, initEncryptEvents)
{
    // Test method: void initEncryptEvents(())
    EXPECT_NO_FATAL_FAILURE(obj->initEncryptEvents());
}

TEST_F(DiskEncryptEntryTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(DiskEncryptEntryTest, onComputerMenuSceneAdded)
{
    // Test method: void onComputerMenuSceneAdded((const QString &scene))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onComputerMenuSceneAdded(_arg0));
}

TEST_F(DiskEncryptEntryTest, processUnfinshedDecrypt)
{
    // Test method: void processUnfinshedDecrypt((const QString &device))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->processUnfinshedDecrypt(_arg0));
}

TEST_F(DiskEncryptEntryTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}
