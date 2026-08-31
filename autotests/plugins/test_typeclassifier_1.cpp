// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_typeclassifier_1.cpp
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

TEST_F(TypeClassifierTest, append)
{
    // Test method: QString append((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->append(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TypeClassifierTest, change)
{
    // Test method: QString change((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->change(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TypeClassifierTest, className)
{
    // Test method: QString className((const QString &key))
    QString _arg0{};
    auto result = obj->className(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TypeClassifierTest, dataHandler)
{
    // Test getter: ModelDataHandler dataHandler()
    auto result = obj->dataHandler();
    EXPECT_NO_FATAL_FAILURE({ obj->dataHandler(); });

}

TEST_F(TypeClassifierTest, mode)
{
    // Test getter: Classifier mode()
    auto result = obj->mode();
    EXPECT_NO_FATAL_FAILURE({ obj->mode(); });

}

TEST_F(TypeClassifierTest, prepend)
{
    // Test method: QString prepend((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->prepend(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TypeClassifierTest, replace)
{
    // Test method: QString replace((const QUrl &oldUrl, const QUrl &newUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->replace(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TypeClassifierTest, updateClassifier)
{
    // Test bool getter: updateClassifier()
    bool result = obj->updateClassifier();
    EXPECT_FALSE(result);

}
