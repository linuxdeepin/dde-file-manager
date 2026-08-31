// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_multifilepermissionwidget.cpp
 * @brief Unit tests for MultiFilePermissionWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/multifilepermissionwidget.h"

#include <QTest>

using namespace dfmplugin_propertydialog;

class MultiFilePermissionWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MultiFilePermissionWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MultiFilePermissionWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MultiFilePermissionWidgetTest, canChmodByFile)
{
    // Test method: bool canChmodByFile((const FileInfoPointer &info))
    FileInfoPointer _arg0{};
    auto result = obj->canChmodByFile(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(MultiFilePermissionWidgetTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}
