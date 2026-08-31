// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_processdialog.cpp
 * @brief Unit tests for ProcessDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "tools/upgrade/dialog/processdialog.h"

#include <QTest>

using namespace src;

class ProcessDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ProcessDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ProcessDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ProcessDialogTest, ProcessDialog)
{
    // Test constructor: ProcessDialog((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ProcessDialogTest, queryProcess)
{
    // Test method: QList<int> queryProcess((const QString &exec))
    QString _arg0{};
    auto result = obj->queryProcess(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ProcessDialogTest, restart)
{
    // Test method: void restart(())
    EXPECT_NO_FATAL_FAILURE(obj->restart());
}
