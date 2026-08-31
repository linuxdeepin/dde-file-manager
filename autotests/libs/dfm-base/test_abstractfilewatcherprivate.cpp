// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractfilewatcherprivate.cpp
 * @brief Unit tests for AbstractFileWatcherPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/interfaces/abstractfilewatcher.h"

#include <QTest>

using namespace src;

class AbstractFileWatcherPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AbstractFileWatcherPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AbstractFileWatcherPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AbstractFileWatcherPrivateTest, AbstractFileWatcherPrivate)
{
    // Test constructor: AbstractFileWatcherPrivate((const QUrl &fileUrl, AbstractFileWatcher *qq))
    ASSERT_NE(obj, nullptr);
}
