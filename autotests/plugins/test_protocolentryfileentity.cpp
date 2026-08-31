// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_protocolentryfileentity.cpp
 * @brief Unit tests for ProtocolEntryFileEntity methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileentity/protocolentryfileentity.h"

#include <QTest>

using namespace dfmplugin_computer;

class ProtocolEntryFileEntityTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ProtocolEntryFileEntity();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ProtocolEntryFileEntity *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ProtocolEntryFileEntityTest, displayName)
{
    // Test getter: QString displayName()
    auto result = obj->displayName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ProtocolEntryFileEntityTest, exists)
{
    // Test bool getter: exists()
    bool result = obj->exists();
    EXPECT_FALSE(result);

}

TEST_F(ProtocolEntryFileEntityTest, icon)
{
    // Test getter: QIcon icon()
    auto result = obj->icon();
    EXPECT_TRUE(result.isNull());

}

TEST_F(ProtocolEntryFileEntityTest, renamable)
{
    // Test bool getter: renamable()
    bool result = obj->renamable();
    EXPECT_FALSE(result);

}

TEST_F(ProtocolEntryFileEntityTest, sizeTotal)
{
    // Test getter: quint64 sizeTotal()
    auto result = obj->sizeTotal();
    EXPECT_EQ(result, 0);

}

TEST_F(ProtocolEntryFileEntityTest, sizeUsage)
{
    // Test getter: quint64 sizeUsage()
    auto result = obj->sizeUsage();
    EXPECT_EQ(result, 0);

}

TEST_F(ProtocolEntryFileEntityTest, targetUrl)
{
    // Test getter: QUrl targetUrl()
    auto result = obj->targetUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}
