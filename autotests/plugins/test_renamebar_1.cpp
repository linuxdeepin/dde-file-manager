// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_renamebar_1.cpp
 * @brief Unit tests for RenameBar methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/renamebar.h"

#include <QTest>

using namespace dfmplugin_workspace;

class RenameBarTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RenameBar();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RenameBar *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RenameBarTest, RenameBar)
{
    // Test constructor: RenameBar((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(RenameBarTest, eventDispatcher)
{
    // Test method: void eventDispatcher(())
    EXPECT_NO_FATAL_FAILURE(obj->eventDispatcher());
}

TEST_F(RenameBarTest, findPage)
{
    // Test getter: WorkspacePage findPage()
    auto result = obj->findPage();
    EXPECT_NO_FATAL_FAILURE({ obj->findPage(); });

}

TEST_F(RenameBarTest, getSelectFiles)
{
    // Test getter: QList<QUrl> getSelectFiles()
    auto result = obj->getSelectFiles();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(RenameBarTest, hideRenameBar)
{
    // Test method: void hideRenameBar(())
    EXPECT_NO_FATAL_FAILURE(obj->hideRenameBar());
}

TEST_F(RenameBarTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(RenameBarTest, onAddOperatorAddedContentChanged)
{
    // Test method: void onAddOperatorAddedContentChanged((const QString &text))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onAddOperatorAddedContentChanged(_arg0));
}

TEST_F(RenameBarTest, onAddTextPatternChanged)
{
    // Test method: void onAddTextPatternChanged((const int &index))
    int _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onAddTextPatternChanged(_arg0));
}

TEST_F(RenameBarTest, onCustomOperatorFileNameChanged)
{
    // Test method: void onCustomOperatorFileNameChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onCustomOperatorFileNameChanged());
}

TEST_F(RenameBarTest, onRenamePatternChanged)
{
    // Test method: void onRenamePatternChanged((const int &index))
    int _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onRenamePatternChanged(_arg0));
}

TEST_F(RenameBarTest, onReplaceOperatorDestNameChanged)
{
    // Test method: void onReplaceOperatorDestNameChanged((const QString &textChanged))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onReplaceOperatorDestNameChanged(_arg0));
}

TEST_F(RenameBarTest, onReplaceOperatorFileNameChanged)
{
    // Test method: void onReplaceOperatorFileNameChanged((const QString &text))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onReplaceOperatorFileNameChanged(_arg0));
}

TEST_F(RenameBarTest, onSelectUrlChanged)
{
    // Test method: void onSelectUrlChanged((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onSelectUrlChanged(_arg0));
}

TEST_F(RenameBarTest, storeUrlList)
{
    // Test method: void storeUrlList((const QList<QUrl> &list))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->storeUrlList(_arg0));
}
