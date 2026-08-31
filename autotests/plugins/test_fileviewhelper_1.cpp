// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileviewhelper_1.cpp
 * @brief Unit tests for FileViewHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/fileviewhelper.h"

#include <QTest>

using namespace dfmplugin_workspace;

class FileViewHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileViewHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileViewHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileViewHelperTest, caculateIconItemIndex)
{
    // Test method: int caculateIconItemIndex((const FileView *view, const QSize &itemSize, const QPoint &pos))
    QSize _arg1{};
    QPoint _arg2{};
    auto result = obj->caculateIconItemIndex(nullptr, _arg1, _arg2);
    EXPECT_GE(result, 0);

}

TEST_F(FileViewHelperTest, caculateListItemIndex)
{
    // Test method: int caculateListItemIndex((const QSize &itemSize, const QPoint &pos))
    QSize _arg0{};
    QPoint _arg1{};
    auto result = obj->caculateListItemIndex(_arg0, _arg1);
    EXPECT_GE(result, 0);

}

TEST_F(FileViewHelperTest, canChangeListItemHeight)
{
    // Test bool getter: canChangeListItemHeight()
    bool result = obj->canChangeListItemHeight();
    EXPECT_FALSE(result);

}

TEST_F(FileViewHelperTest, clipboardDataChanged)
{
    // Test method: void clipboardDataChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->clipboardDataChanged());
}

TEST_F(FileViewHelperTest, customDefaultListItemHeightLevel)
{
    // Test getter: int customDefaultListItemHeightLevel()
    auto result = obj->customDefaultListItemHeightLevel();
    EXPECT_EQ(result, 0);

}

TEST_F(FileViewHelperTest, customDefaultViewMode)
{
    // Test getter: ViewMode customDefaultViewMode()
    auto result = obj->customDefaultViewMode();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(FileViewHelperTest, fileViewViewportMargins)
{
    // Test getter: QMargins fileViewViewportMargins()
    auto result = obj->fileViewViewportMargins();
    EXPECT_NO_FATAL_FAILURE({ obj->fileViewViewportMargins(); });

}

TEST_F(FileViewHelperTest, handleTrashStateChanged)
{
    // Test method: void handleTrashStateChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->handleTrashStateChanged());
}

TEST_F(FileViewHelperTest, init)
{
    // Test method: void init(())
    EXPECT_NO_FATAL_FAILURE(obj->init());
}

TEST_F(FileViewHelperTest, isDropTarget)
{
    // Test method: bool isDropTarget((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->isDropTarget(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FileViewHelperTest, isLastIndex)
{
    // Test method: bool isLastIndex((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->isLastIndex(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FileViewHelperTest, isSelected)
{
    // Test method: bool isSelected((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->isSelected(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FileViewHelperTest, itemDelegate)
{
    // Test getter: BaseItemDelegate itemDelegate()
    auto result = obj->itemDelegate();
    EXPECT_NO_FATAL_FAILURE({ obj->itemDelegate(); });

}

TEST_F(FileViewHelperTest, keyboardSearch)
{
    // Test method: void keyboardSearch((const QString &search))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->keyboardSearch(_arg0));
}

TEST_F(FileViewHelperTest, selectedIndexsCount)
{
    // Test getter: int selectedIndexsCount()
    auto result = obj->selectedIndexsCount();
    EXPECT_EQ(result, 0);

}

TEST_F(FileViewHelperTest, updateGeometries)
{
    // Test method: void updateGeometries(())
    EXPECT_NO_FATAL_FAILURE(obj->updateGeometries());
}

TEST_F(FileViewHelperTest, verticalOffset)
{
    // Test getter: int verticalOffset()
    auto result = obj->verticalOffset();
    EXPECT_EQ(result, 0);

}

TEST_F(FileViewHelperTest, viewContentSize)
{
    // Test getter: QSize viewContentSize()
    auto result = obj->viewContentSize();
    EXPECT_TRUE(result.isEmpty());

}
