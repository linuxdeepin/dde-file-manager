// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_npdevicealiasmanager.cpp
 * @brief Unit tests for NPDeviceAliasManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/base/device/devicealiasmanager.h"

#include <QTest>

using namespace src;

class NPDeviceAliasManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new NPDeviceAliasManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    NPDeviceAliasManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(NPDeviceAliasManagerTest, getAlias)
{
    // Test method: QString getAlias((const QUrl &protocolUrl))
    QUrl _arg0{};
    auto result = obj->getAlias(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(NPDeviceAliasManagerTest, removeAlias)
{
    // Test method: void removeAlias((const QUrl &protocolUrl))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removeAlias(_arg0));
}
