// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_cryfsversioninfo.cpp
 * @brief Unit tests for CryfsVersionInfo methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/fileencrypthandle_p.h"

#include <QTest>

using namespace dfmplugin_vault;

class CryfsVersionInfoTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CryfsVersionInfo();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CryfsVersionInfo *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CryfsVersionInfoTest, CryfsVersionInfo)
{
    // Test constructor: CryfsVersionInfo(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(CryfsVersionInfoTest, isOlderThan)
{
    // Test bool getter: isOlderThan()
    bool result = obj->isOlderThan();
    EXPECT_FALSE(result);

}

TEST_F(CryfsVersionInfoTest, isVaild)
{
    // Test bool getter: isVaild()
    bool result = obj->isVaild();
    EXPECT_FALSE(result);

}
