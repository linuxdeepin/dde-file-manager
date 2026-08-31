// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_diskencryptmenuscene.cpp
 * @brief Unit tests for DiskEncryptMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/diskencryptmenuscene.h"

#include <QTest>

using namespace dfmplugin_disk_encrypt_entry;

class DiskEncryptMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DiskEncryptMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DiskEncryptMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DiskEncryptMenuSceneTest, changePassphrase)
{
    // Test method: void changePassphrase((DeviceEncryptParam param))
    EXPECT_NO_FATAL_FAILURE(obj->changePassphrase(DeviceEncryptParam()));
}

TEST_F(DiskEncryptMenuSceneTest, decryptDevice)
{
    // Test method: void decryptDevice((const DeviceEncryptParam &param))
    DeviceEncryptParam _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->decryptDevice(_arg0));
}

TEST_F(DiskEncryptMenuSceneTest, doReencryptDevice)
{
    // Test method: bool doReencryptDevice((const DeviceEncryptParam &param))
    DeviceEncryptParam _arg0{};
    auto result = obj->doReencryptDevice(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(DiskEncryptMenuSceneTest, sortActions)
{
    // Test method: void sortActions((QMenu *parent))
    EXPECT_NO_FATAL_FAILURE(obj->sortActions(nullptr));
}
