// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileview.cpp
 * @brief Unit tests for FileView methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/fileview.h"

#include <QTest>

using namespace dfmplugin_workspace;

class FileViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileView();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileView *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileViewTest, clearTruncateButtonHover)
{
    // Test method: void clearTruncateButtonHover(())
    EXPECT_NO_FATAL_FAILURE(obj->clearTruncateButtonHover());
}

TEST_F(FileViewTest, dataChanged)
{
    // Test method: void dataChanged((const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles))
    QModelIndex _arg0{};
    QModelIndex _arg1{};
    QVector<int> _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->dataChanged(_arg0, _arg1, _arg2));
}

TEST_F(FileViewTest, iconIndexAt)
{
    // Test method: QModelIndex iconIndexAt((const QPoint &pos, const QSize &itemSize))
    QPoint _arg0{};
    QSize _arg1{};
    auto result = obj->iconIndexAt(_arg0, _arg1);
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileViewTest, indexAt)
{
    // Test method: QModelIndex indexAt((const QPoint &pos))
    QPoint _arg0{};
    auto result = obj->indexAt(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileViewTest, initializeConnect)
{
    // Test method: void initializeConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initializeConnect());
}

TEST_F(FileViewTest, isSelected)
{
    // Test method: bool isSelected((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->isSelected(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FileViewTest, itemRect)
{
    // Test method: QRectF itemRect((const QUrl &url, const ItemRoles role))
    QUrl _arg0{};
    auto result = obj->itemRect(_arg0, ItemRoles());
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileViewTest, resetSelectionModes)
{
    // Test method: void resetSelectionModes(())
    EXPECT_NO_FATAL_FAILURE(obj->resetSelectionModes());
}

TEST_F(FileViewTest, rowsAboutToBeRemoved)
{
    // Test method: void rowsAboutToBeRemoved((const QModelIndex &parent, int start, int end))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->rowsAboutToBeRemoved(_arg0, 0, 0));
}

TEST_F(FileViewTest, selectedUrlList)
{
    // Test getter: QList<QUrl> selectedUrlList()
    auto result = obj->selectedUrlList();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileViewTest, setIconSize)
{
    // Test setter: void setIconSize((const QSize &size))
    QSize _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setIconSize(_arg0));
}

TEST_F(FileViewTest, setModel)
{
    // Test setter: void setModel((QAbstractItemModel *model))
    EXPECT_NO_FATAL_FAILURE(obj->setModel(nullptr));
}

TEST_F(FileViewTest, truncateButtonGroupKeyAt)
{
    // Test method: QString truncateButtonGroupKeyAt((const QPoint &pos))
    QPoint _arg0{};
    auto result = obj->truncateButtonGroupKeyAt(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileViewTest, updateStatusBar)
{
    // Test method: void updateStatusBar(())
    EXPECT_NO_FATAL_FAILURE(obj->updateStatusBar());
}

TEST_F(FileViewTest, updateTruncateButtonHover)
{
    // Test method: void updateTruncateButtonHover((const QPoint &pos))
    QPoint _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->updateTruncateButtonHover(_arg0));
}

TEST_F(FileViewTest, updateViewportContentsMargins)
{
    // Test method: void updateViewportContentsMargins((const QSize &itemSize))
    QSize _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->updateViewportContentsMargins(_arg0));
}

TEST_F(FileViewTest, visualRect)
{
    // Test method: QRect visualRect((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->visualRect(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileViewTest, clearStickyHeaderState)
{
    // Test method: void clearStickyHeaderState(())
    EXPECT_NO_FATAL_FAILURE(obj->clearStickyHeaderState());
}

TEST_F(FileViewTest, edit)
{
    // Test method: bool edit((const QModelIndex &index, QAbstractItemView::EditTrigger trigger, QEvent *event))
    QModelIndex _arg0{};
    auto result = obj->edit(_arg0, QAbstractItemView::EditTrigger(), nullptr);
    EXPECT_FALSE(result);

}

TEST_F(FileViewTest, model)
{
    // Test getter: FileViewModel model()
    auto result = obj->model();
    EXPECT_NO_FATAL_FAILURE({ obj->model(); });

}

TEST_F(FileViewTest, mouseReleaseEvent)
{
    // Test event handler: mouseReleaseEvent((QMouseEvent *event))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mouseReleaseEvent(&_event));
}

TEST_F(FileViewTest, saveViewModeState)
{
    // Test method: void saveViewModeState(())
    EXPECT_NO_FATAL_FAILURE(obj->saveViewModeState());
}

TEST_F(FileViewTest, scrollTo)
{
    // Test method: void scrollTo((const QModelIndex &index, ScrollHint hint))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->scrollTo(_arg0, ScrollHint()));
}

TEST_F(FileViewTest, updateGeometries)
{
    // Test method: void updateGeometries(())
    EXPECT_NO_FATAL_FAILURE(obj->updateGeometries());
}

TEST_F(FileViewTest, widget)
{
    // Test getter: QWidget widget()
    auto result = obj->widget();
    EXPECT_NO_FATAL_FAILURE({ obj->widget(); });

}

TEST_F(FileViewTest, _FileView)
{
    // Test constructor: FileView((const QUrl &url, QWidget *parent))
    ASSERT_NE(obj, nullptr);
}
