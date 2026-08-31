// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_textindexadaptor.cpp
 * @brief Unit tests for TextIndexAdaptor methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "config/dbus/TextIndexAdaptor.h"

#include <QTest>

using namespace src;

class TextIndexAdaptorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TextIndexAdaptor();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TextIndexAdaptor *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TextIndexAdaptorTest, RemoveIndexTask)
{
    // Test method: bool RemoveIndexTask((const QStringList &paths))
    QStringList _arg0{};
    auto result = obj->RemoveIndexTask(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(TextIndexAdaptorTest, TextIndexAdaptor)
{
    // Test constructor: TextIndexAdaptor((QObject *parent))
    ASSERT_NE(obj, nullptr);
}
