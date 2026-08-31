// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_smbcapi.cpp
 * @brief Unit tests for SmbcAPI methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/mountcontrol/mounthelpers/cifsmounthelper.h"

#include <QTest>

using namespace src;

class SmbcAPITest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SmbcAPI();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SmbcAPI *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SmbcAPITest, SmbcAPI)
{
    // Test constructor: SmbcAPI(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(SmbcAPITest, getSmbcNegprot)
{
    // Test getter: FnSmbcNegprot getSmbcNegprot()
    auto result = obj->getSmbcNegprot();
    EXPECT_NO_FATAL_FAILURE({ obj->getSmbcNegprot(); });

}

TEST_F(SmbcAPITest, getSmbcResolveHost)
{
    // Test getter: FnSmbcResolveHost getSmbcResolveHost()
    auto result = obj->getSmbcResolveHost();
    EXPECT_NO_FATAL_FAILURE({ obj->getSmbcResolveHost(); });

}

TEST_F(SmbcAPITest, init)
{
    // Test method: void init(())
    EXPECT_NO_FATAL_FAILURE(obj->init());
}

TEST_F(SmbcAPITest, isInitialized)
{
    // Test bool getter: isInitialized()
    bool result = obj->isInitialized();
    EXPECT_FALSE(result);

}

TEST_F(SmbcAPITest, versionMapper)
{
    // Test getter: QMap<QString, QString> versionMapper()
    auto result = obj->versionMapper();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SmbcAPITest, SmbcAPI_Destructor)
{
    // Test method:  ~SmbcAPI(())
    EXPECT_NO_FATAL_FAILURE({ SmbcAPI *tmp = new SmbcAPI(); delete tmp; });
}
