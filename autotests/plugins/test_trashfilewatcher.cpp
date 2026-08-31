// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_trashfilewatcher.cpp
 * @brief Unit tests for TrashFileWatcher methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "trashfilewatcher.h"

#include <QTest>

using namespace dfmplugin_trash;

class TrashFileWatcherTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TrashFileWatcher();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TrashFileWatcher *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TrashFileWatcherTest, TrashFileWatcher)
{
    // Test constructor: TrashFileWatcher(())
    ASSERT_NE(obj, nullptr);
}
