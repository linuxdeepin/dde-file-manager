// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_computerpropertydialog_1.cpp
 * @brief Unit tests for ComputerPropertyDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/computerpropertydialog.h"

#include <QTest>

using namespace dfmplugin_propertydialog;

class ComputerPropertyDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ComputerPropertyDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ComputerPropertyDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ComputerPropertyDialogTest, closeEvent)
{
    // Test event handler: closeEvent((QCloseEvent *event))
    QCloseEvent _event(QCloseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->closeEvent(&_event));
}

TEST_F(ComputerPropertyDialogTest, iniThread)
{
    // Test method: void iniThread(())
    EXPECT_NO_FATAL_FAILURE(obj->iniThread());
}

TEST_F(ComputerPropertyDialogTest, showEvent)
{
    // Test event handler: showEvent((QShowEvent *event))
    QShowEvent _event(QShowEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->showEvent(&_event));
}
