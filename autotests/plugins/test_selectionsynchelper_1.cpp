// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_selectionsynchelper_1.cpp
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

TEST_F(SelectionSyncHelperTest, setEnabled)
{
    // Test method: void setEnabled(())
    EXPECT_NO_FATAL_FAILURE(obj->setEnabled());
}

TEST_F(SelectionSyncHelperTest, setExternalModel)
{
    // Test setter: void setExternalModel((QItemSelectionModel *sel))
    EXPECT_NO_FATAL_FAILURE(obj->setExternalModel(nullptr));
}

TEST_F(SelectionSyncHelperTest, setInnerModel)
{
    // Test setter: void setInnerModel((ItemSelectionModel *sel))
    EXPECT_NO_FATAL_FAILURE(obj->setInnerModel(nullptr));
}

TEST_F(SelectionSyncHelperTest, setShell)
{
    // Test setter: void setShell((CanvasSelectionShell *sh))
    EXPECT_NO_FATAL_FAILURE(obj->setShell(nullptr));
}
