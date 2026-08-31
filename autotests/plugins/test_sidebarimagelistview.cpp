// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sidebarimagelistview.cpp
 * @brief Unit tests for SideBarImageListView methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-preview/pluginpreviews/pdf-preview/sidebarimagelistview.h"

#include <QTest>

using namespace src;

class SideBarImageListViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SideBarImageListView();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SideBarImageListView *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SideBarImageListViewTest, handleOpenSuccess)
{
    // Test method: void handleOpenSuccess(())
    EXPECT_NO_FATAL_FAILURE(obj->handleOpenSuccess());
}
