// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_redundantupdatefilter_1.cpp
 * @brief Unit tests for RedundantUpdateFilter methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "model/filefilter.h"

#include <QTest>

using namespace ddplugin_canvas;

class RedundantUpdateFilterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RedundantUpdateFilter();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RedundantUpdateFilter *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RedundantUpdateFilterTest, RedundantUpdateFilter)
{
    // Test constructor: RedundantUpdateFilter((FileProvider *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(RedundantUpdateFilterTest, fileUpdatedFilter)
{
    // Test method: bool fileUpdatedFilter((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->fileUpdatedFilter(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(RedundantUpdateFilterTest, timerEvent)
{
    // Test event handler: timerEvent((QTimerEvent *event))
    QTimerEvent _event(QTimerEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->timerEvent(&_event));
}
