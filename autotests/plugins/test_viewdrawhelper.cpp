// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_viewdrawhelper.cpp
 * @brief Unit tests for ViewDrawHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/viewdrawhelper.h"

#include <QTest>

using namespace dfmplugin_workspace;

class ViewDrawHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ViewDrawHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ViewDrawHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ViewDrawHelperTest, renderDragPixmap)
{
    // Test method: QPixmap renderDragPixmap((dfmbase::Global::ViewMode mode, QModelIndexList indexes))
    auto result = obj->renderDragPixmap({}, QModelIndexList());
    EXPECT_TRUE(result.isNull());

}
