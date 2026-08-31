// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagmanageradaptor.cpp
 * @brief Unit tests for TagManagerAdaptor methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "config/dbus/TagManagerAdaptor.h"

#include <QTest>

using namespace src;

class TagManagerAdaptorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagManagerAdaptor();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagManagerAdaptor *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagManagerAdaptorTest, Delete)
{
    // Test method: bool Delete((int opt, const QVariantMap &value))
    QVariantMap _arg1{};
    auto result = obj->Delete(0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(TagManagerAdaptorTest, TagManagerAdaptor)
{
    // Test constructor: TagManagerAdaptor((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TagManagerAdaptorTest, Update)
{
    // Test method: bool Update((int opt, const QVariantMap &value))
    QVariantMap _arg1{};
    auto result = obj->Update(0, _arg1);
    EXPECT_FALSE(result);

}
