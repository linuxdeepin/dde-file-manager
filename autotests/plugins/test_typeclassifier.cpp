// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_typeclassifier.cpp
 * @brief Unit tests for TypeClassifier methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "mode/normalized/type/typeclassifier.h"

#include <QTest>

using namespace ddplugin_organizer;

class TypeClassifierTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TypeClassifier();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TypeClassifier *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TypeClassifierTest, TypeClassifier)
{
    // Test constructor: TypeClassifier((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TypeClassifierTest, acceptRename)
{
    // Test method: bool acceptRename((const QUrl &oldUrl, const QUrl &newUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->acceptRename(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(TypeClassifierTest, remove)
{
    // Test method: QString remove((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->remove(_arg0);
    EXPECT_TRUE(result.isEmpty());

}
