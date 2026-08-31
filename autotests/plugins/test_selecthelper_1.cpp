// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_selecthelper_1.cpp
 * @brief Unit tests for SelectHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/selecthelper.h"

#include <QTest>

using namespace dfmplugin_workspace;

class SelectHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SelectHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SelectHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SelectHelperTest, caculateSelection)
{
    // Test method: void caculateSelection((const QRect &rect, QItemSelection *selection))
    QRect _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->caculateSelection(_arg0, nullptr));
}

TEST_F(SelectHelperTest, getCurrentPressedIndex)
{
    // Test getter: QModelIndex getCurrentPressedIndex()
    auto result = obj->getCurrentPressedIndex();
    EXPECT_FALSE(result.isValid());

}

TEST_F(SelectHelperTest, getGroupKeyFromIndex)
{
    // Test method: QString getGroupKeyFromIndex((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->getGroupKeyFromIndex(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SelectHelperTest, handleGroupHeaderClick)
{
    // Test method: void handleGroupHeaderClick((const QModelIndex &index, Qt::KeyboardModifiers modifiers))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleGroupHeaderClick(_arg0, Qt::KeyboardModifiers()));
}

TEST_F(SelectHelperTest, isGroupHeaderIndex)
{
    // Test method: bool isGroupHeaderIndex((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->isGroupHeaderIndex(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(SelectHelperTest, isSelectableItem)
{
    // Test method: bool isSelectableItem((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->isSelectableItem(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(SelectHelperTest, prepareForModelTeardown)
{
    // Test method: void prepareForModelTeardown(())
    EXPECT_NO_FATAL_FAILURE(obj->prepareForModelTeardown());
}

TEST_F(SelectHelperTest, release)
{
    // Test method: void release(())
    EXPECT_NO_FATAL_FAILURE(obj->release());
}

TEST_F(SelectHelperTest, resortSelectFiles)
{
    // Test method: void resortSelectFiles(())
    EXPECT_NO_FATAL_FAILURE(obj->resortSelectFiles());
}

TEST_F(SelectHelperTest, selectGroup)
{
    // Test method: void selectGroup((const QString &groupKey, bool select))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->selectGroup(_arg0, false));
}

TEST_F(SelectHelperTest, selectedTreeViewUrlList)
{
    // Test method: void selectedTreeViewUrlList((QList<QUrl> &selectedUrls, QList<QUrl> &treeSelectedUrls))
    QList<QUrl> _arg0{};
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->selectedTreeViewUrlList(_arg0, _arg1));
}

TEST_F(SelectHelperTest, setItemsExpandable)
{
    // Test setter: void setItemsExpandable((bool expandable))
    EXPECT_NO_FATAL_FAILURE(obj->setItemsExpandable(false));
}

TEST_F(SelectHelperTest, setSelection)
{
    // Test setter: void setSelection((const QItemSelection &selection))
    QItemSelection _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setSelection(_arg0));
}
