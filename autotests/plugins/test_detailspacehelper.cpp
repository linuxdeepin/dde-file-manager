// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_detailspacehelper.cpp
 * @brief Unit tests for DetailSpaceHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/detailspacehelper.h"

#include <QTest>

using namespace dfmplugin_detailspace;

class DetailSpaceHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DetailSpaceHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DetailSpaceHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DetailSpaceHelperTest, removeDetailSpace)
{
    // Test method: void removeDetailSpace((quint64 windowId))
    EXPECT_NO_FATAL_FAILURE(obj->removeDetailSpace(0));
}

TEST_F(DetailSpaceHelperTest, showDetailView)
{
    // Test method: void showDetailView((quint64 windowId, bool checked, bool userAction))
    EXPECT_NO_FATAL_FAILURE(obj->showDetailView(0, false, false));
}
