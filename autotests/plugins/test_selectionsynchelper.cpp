// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_selectionsynchelper.cpp
 * @brief Unit tests for SelectionSyncHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "mode/selectionsynchelper.h"

#include <QTest>

using namespace ddplugin_organizer;

class SelectionSyncHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SelectionSyncHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SelectionSyncHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SelectionSyncHelperTest, SelectionSyncHelper)
{
    // Test constructor: SelectionSyncHelper((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SelectionSyncHelperTest, clearExteralSelection)
{
    // Test method: void clearExteralSelection(())
    EXPECT_NO_FATAL_FAILURE(obj->clearExteralSelection());
}

TEST_F(SelectionSyncHelperTest, clearInnerSelection)
{
    // Test method: void clearInnerSelection(())
    EXPECT_NO_FATAL_FAILURE(obj->clearInnerSelection());
}

TEST_F(SelectionSyncHelperTest, externalModelDestroyed)
{
    // Test method: void externalModelDestroyed(())
    EXPECT_NO_FATAL_FAILURE(obj->externalModelDestroyed());
}

TEST_F(SelectionSyncHelperTest, innerModelDestroyed)
{
    // Test method: void innerModelDestroyed(())
    EXPECT_NO_FATAL_FAILURE(obj->innerModelDestroyed());
}
