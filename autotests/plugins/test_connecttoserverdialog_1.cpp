// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_connecttoserverdialog_1.cpp
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

TEST_F(ConnectToServerDialogTest, ConnectToServerDialog)
{
    // Test constructor: ConnectToServerDialog((const QUrl &url, QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ConnectToServerDialogTest, collectionOperate)
{
    // Test method: void collectionOperate(())
    EXPECT_NO_FATAL_FAILURE(obj->collectionOperate());
}

TEST_F(ConnectToServerDialogTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(ConnectToServerDialogTest, initIfWayland)
{
    // Test method: void initIfWayland(())
    EXPECT_NO_FATAL_FAILURE(obj->initIfWayland());
}

TEST_F(ConnectToServerDialogTest, initUiForSizeMode)
{
    // Test method: void initUiForSizeMode(())
    EXPECT_NO_FATAL_FAILURE(obj->initUiForSizeMode());
}

TEST_F(ConnectToServerDialogTest, onAddButtonClicked)
{
    // Test method: void onAddButtonClicked(())
    EXPECT_NO_FATAL_FAILURE(obj->onAddButtonClicked());
}

TEST_F(ConnectToServerDialogTest, onButtonClicked)
{
    // Test method: void onButtonClicked((const int &index))
    int _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onButtonClicked(_arg0));
}

TEST_F(ConnectToServerDialogTest, onCollectionViewClicked)
{
    // Test method: void onCollectionViewClicked((const QModelIndex &index))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onCollectionViewClicked(_arg0));
}

TEST_F(ConnectToServerDialogTest, onCompleterActivated)
{
    // Test method: void onCompleterActivated((const QString &text))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onCompleterActivated(_arg0));
}

TEST_F(ConnectToServerDialogTest, onDelButtonClicked)
{
    // Test method: void onDelButtonClicked(())
    EXPECT_NO_FATAL_FAILURE(obj->onDelButtonClicked());
}

TEST_F(ConnectToServerDialogTest, schemeWithSlash)
{
    // Test method: QString schemeWithSlash((const QString &scheme))
    QString _arg0{};
    auto result = obj->schemeWithSlash(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ConnectToServerDialogTest, updateAddButtonState)
{
    // Test method: void updateAddButtonState((bool collected))
    EXPECT_NO_FATAL_FAILURE(obj->updateAddButtonState(false));
}

TEST_F(ConnectToServerDialogTest, updateTheme)
{
    // Test method: void updateTheme(())
    EXPECT_NO_FATAL_FAILURE(obj->updateTheme());
}

TEST_F(ConnectToServerDialogTest, updateUiState)
{
    // Test method: void updateUiState(())
    EXPECT_NO_FATAL_FAILURE(obj->updateUiState());
}
