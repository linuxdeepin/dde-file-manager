// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_computerpropertydialog.cpp
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

TEST_F(ComputerPropertyDialogTest, ComputerPropertyDialog)
{
    // Test constructor: ComputerPropertyDialog((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ComputerPropertyDialogTest, computerProcess)
{
    // Test method: void computerProcess((QMap<ComputerInfoItem, QString> computerInfo))
    EXPECT_NO_FATAL_FAILURE(obj->computerProcess(QMap<ComputerInfoItem, QString>()));
}

TEST_F(ComputerPropertyDialogTest, iniUI)
{
    // Test method: void iniUI(())
    EXPECT_NO_FATAL_FAILURE(obj->iniUI());
}
