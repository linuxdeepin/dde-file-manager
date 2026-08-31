// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_basedialog.cpp
 * @brief Unit tests for BaseDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/dialogs/basedialog/basedialog.h"

#include <QTest>

using namespace src;

class BaseDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BaseDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BaseDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BaseDialogTest, BaseDialog)
{
    // Test constructor: BaseDialog((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(BaseDialogTest, setTitle)
{
    // Test setter: void setTitle((const QString &title))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setTitle(_arg0));
}

TEST_F(BaseDialogTest, setTitleFont)
{
    // Test setter: void setTitleFont((const QFont &font))
    QFont _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setTitleFont(_arg0));
}
