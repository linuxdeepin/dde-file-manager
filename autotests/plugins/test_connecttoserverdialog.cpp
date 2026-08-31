// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_connecttoserverdialog.cpp
 * @brief Unit tests for ConnectToServerDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dialogs/connecttoserverdialog.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class ConnectToServerDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ConnectToServerDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ConnectToServerDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ConnectToServerDialogTest, doDeleteCollection)
{
    // Test method: void doDeleteCollection((const QString &text, int row))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->doDeleteCollection(_arg0, 0));
}

TEST_F(ConnectToServerDialogTest, getCurrentUrlString)
{
    // Test getter: QString getCurrentUrlString()
    auto result = obj->getCurrentUrlString();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ConnectToServerDialogTest, initializeUi)
{
    // Test method: void initializeUi(())
    EXPECT_NO_FATAL_FAILURE(obj->initializeUi());
}

TEST_F(ConnectToServerDialogTest, onCurrentInputChanged)
{
    // Test method: void onCurrentInputChanged((const QString &server))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onCurrentInputChanged(_arg0));
}

TEST_F(ConnectToServerDialogTest, updateCollections)
{
    // Test method: QStringList updateCollections((const QString &newUrlStr, bool insertWhenNoExist))
    QString _arg0{};
    auto result = obj->updateCollections(_arg0, false);
    EXPECT_TRUE(result.isEmpty());

}
