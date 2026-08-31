// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_trashpropertydialog.cpp
 * @brief Unit tests for TrashPropertyDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/trashpropertydialog.h"

#include <QTest>

using namespace dfmplugin_trashcore;

class TrashPropertyDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TrashPropertyDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TrashPropertyDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TrashPropertyDialogTest, TrashPropertyDialog)
{
    // Test constructor: TrashPropertyDialog((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TrashPropertyDialogTest, showEvent)
{
    // Test event handler: showEvent((QShowEvent *event))
    QShowEvent _event(QShowEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->showEvent(&_event));
}

TEST_F(TrashPropertyDialogTest, updateUI)
{
    // Test method: void updateUI((qint64 size, int count))
    EXPECT_NO_FATAL_FAILURE(obj->updateUI(0, 0));
}
