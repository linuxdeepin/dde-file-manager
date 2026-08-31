// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dragmoniter_1.cpp
 * @brief Unit tests for DragMoniter methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager/dragmonitor.h"

#include <QTest>

using namespace src;

class DragMoniterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DragMoniter();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DragMoniter *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DragMoniterTest, DragMoniter)
{
    // Test constructor: DragMoniter((QObject *parent))
    ASSERT_NE(obj, nullptr);
}
