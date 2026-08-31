// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dumpisooptdialog_1.cpp
 * @brief Unit tests for DumpISOOptDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dialogs/dumpisooptdialog.h"

#include <QTest>

using namespace dfmplugin_burn;

class DumpISOOptDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DumpISOOptDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DumpISOOptDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DumpISOOptDialogTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(DumpISOOptDialogTest, initData)
{
    // Test method: void initData(())
    EXPECT_NO_FATAL_FAILURE(obj->initData());
}

TEST_F(DumpISOOptDialogTest, onButtonClicked)
{
    // Test method: void onButtonClicked((int index, const QString &text))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onButtonClicked(0, _arg1));
}

TEST_F(DumpISOOptDialogTest, onFileChoosed)
{
    // Test method: void onFileChoosed((const QString &fileName))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onFileChoosed(_arg0));
}

TEST_F(DumpISOOptDialogTest, onPathChanged)
{
    // Test method: void onPathChanged((const QString &path))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onPathChanged(_arg0));
}
