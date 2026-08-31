// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_openwithdialog_1.cpp
 * @brief Unit tests for OpenWithDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "openwith/openwithdialog.h"

#include <QTest>

using namespace dfmplugin_utils;

class OpenWithDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OpenWithDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OpenWithDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpenWithDialogTest, checkItem)
{
    // Test method: void checkItem((OpenWithDialogListItem *item))
    EXPECT_NO_FATAL_FAILURE(obj->checkItem(nullptr));
}

TEST_F(OpenWithDialogTest, createItem)
{
    // Test method: OpenWithDialogListItem createItem((const QString &iconName, const QString &name, const QString &filePath))
    QString _arg0{};
    QString _arg1{};
    QString _arg2{};
    auto result = obj->createItem(_arg0, _arg1, _arg2);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OpenWithDialogTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(OpenWithDialogTest, initUiForSizeMode)
{
    // Test method: void initUiForSizeMode(())
    EXPECT_NO_FATAL_FAILURE(obj->initUiForSizeMode());
}

TEST_F(OpenWithDialogTest, showEvent)
{
    // Test event handler: showEvent((QShowEvent *event))
    QShowEvent _event(QShowEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->showEvent(&_event));
}
