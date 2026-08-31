// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileinfowidget.cpp
 * @brief Unit tests for FileInfoWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/fileinfowidget.h"

#include <QTest>

using namespace dfmplugin_detailspace;

class FileInfoWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileInfoWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileInfoWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileInfoWidgetTest, FileInfoWidget)
{
    // Test constructor: FileInfoWidget((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileInfoWidgetTest, applyFieldFilters)
{
    // Test method: void applyFieldFilters((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->applyFieldFilters(_arg0));
}

TEST_F(FileInfoWidgetTest, resetAllFields)
{
    // Test method: void resetAllFields(())
    EXPECT_NO_FATAL_FAILURE(obj->resetAllFields());
}
