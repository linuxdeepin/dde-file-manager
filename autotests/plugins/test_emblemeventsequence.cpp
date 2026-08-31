// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_emblemeventsequence.cpp
 * @brief Unit tests for EmblemEventSequence methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/emblemeventsequence.h"

#include <QTest>

using namespace dfmplugin_emblem;

class EmblemEventSequenceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new EmblemEventSequence();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    EmblemEventSequence *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(EmblemEventSequenceTest, EmblemEventSequence)
{
    // Test constructor: EmblemEventSequence((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(EmblemEventSequenceTest, doFetchCustomEmblems)
{
    // Test method: bool doFetchCustomEmblems((const QUrl &url, QList<QIcon> *emblems))
    QUrl _arg0{};
    auto result = obj->doFetchCustomEmblems(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(EmblemEventSequenceTest, doFetchExtendEmblems)
{
    // Test method: bool doFetchExtendEmblems((const QUrl &url, QList<QIcon> *emblems))
    QUrl _arg0{};
    auto result = obj->doFetchExtendEmblems(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(EmblemEventSequenceTest, instance)
{
    // Test getter: DPEMBLEM_USE_NAMESPACE instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
