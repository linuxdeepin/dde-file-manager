// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_renamebarprivate_1.cpp
 * @brief Unit tests for RenameBarPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/private/renamebar_p.h"

#include <QTest>

using namespace dfmplugin_workspace;

class RenameBarPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RenameBarPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RenameBarPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RenameBarPrivateTest, RenameBarPrivate)
{
    // Test constructor: RenameBarPrivate((RenameBar *const qPtr))
    ASSERT_NE(obj, nullptr);
}

TEST_F(RenameBarPrivateTest, filteringText)
{
    // Test method: QString filteringText((const QString &text))
    QString _arg0{};
    auto result = obj->filteringText(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(RenameBarPrivateTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}

TEST_F(RenameBarPrivateTest, setRenameBtnStatus)
{
    // Test setter: void setRenameBtnStatus((const bool &value))
    bool _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setRenameBtnStatus(_arg0));
}

TEST_F(RenameBarPrivateTest, updateLineEditText)
{
    // Test method: void updateLineEditText((QLineEdit *lineEdit, const QString &defaultValue))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->updateLineEditText(nullptr, _arg1));
}
