// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_folderlistwidgetprivate.cpp
 * @brief Unit tests for FolderListWidgetPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/folderlistwidget.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class FolderListWidgetPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FolderListWidgetPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FolderListWidgetPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FolderListWidgetPrivateTest, FolderListWidgetPrivate)
{
    // Test constructor: FolderListWidgetPrivate((FolderListWidget *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FolderListWidgetPrivateTest, clicked)
{
    // Test method: void clicked((const QModelIndex &index))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->clicked(_arg0));
}

TEST_F(FolderListWidgetPrivateTest, findAndSelectMatch)
{
    // Test method: bool findAndSelectMatch((const QString &text, int startRow))
    QString _arg0{};
    auto result = obj->findAndSelectMatch(_arg0, 0);
    EXPECT_FALSE(result);

}

TEST_F(FolderListWidgetPrivateTest, getStartIndexFromHover)
{
    // Test method: QModelIndex getStartIndexFromHover((bool isUp))
    auto result = obj->getStartIndexFromHover(false);
    EXPECT_FALSE(result.isValid());

}

TEST_F(FolderListWidgetPrivateTest, handleKeyInput)
{
    // Test method: void handleKeyInput((const QString &pressedText))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleKeyInput(_arg0));
}

TEST_F(FolderListWidgetPrivateTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(FolderListWidgetPrivateTest, matchText)
{
    // Test method: bool matchText((const QString &source, const QString &input))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->matchText(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(FolderListWidgetPrivateTest, returnPressed)
{
    // Test method: void returnPressed(())
    EXPECT_NO_FATAL_FAILURE(obj->returnPressed());
}

TEST_F(FolderListWidgetPrivateTest, selectDown)
{
    // Test method: void selectDown(())
    EXPECT_NO_FATAL_FAILURE(obj->selectDown());
}

TEST_F(FolderListWidgetPrivateTest, selectUp)
{
    // Test method: void selectUp(())
    EXPECT_NO_FATAL_FAILURE(obj->selectUp());
}

TEST_F(FolderListWidgetPrivateTest, FolderListWidgetPrivate_Destructor)
{
    // Test method:  ~FolderListWidgetPrivate(())
    EXPECT_NO_FATAL_FAILURE({ FolderListWidgetPrivate *tmp = new FolderListWidgetPrivate(); delete tmp; });
}
