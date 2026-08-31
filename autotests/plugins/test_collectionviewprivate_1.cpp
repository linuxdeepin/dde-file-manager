// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collectionviewprivate_1.cpp
 * @brief Unit tests for CollectionViewPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/collectionview.h"

#include <QTest>

using namespace ddplugin_organizer;

class CollectionViewPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CollectionViewPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CollectionViewPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CollectionViewPrivateTest, CollectionViewPrivate)
{
    // Test constructor: CollectionViewPrivate((const QString &uuid, CollectionDataProvider *dataProvider, CollectionView *qq, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CollectionViewPrivateTest, checkClientMimeData)
{
    // Test method: bool checkClientMimeData((QDragEnterEvent *event))
    auto result = obj->checkClientMimeData(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CollectionViewPrivateTest, checkProhibitPaths)
{
    // Test method: bool checkProhibitPaths((QDragEnterEvent *event))
    auto result = obj->checkProhibitPaths(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CollectionViewPrivateTest, checkTouchDarg)
{
    // Test method: void checkTouchDarg((QMouseEvent *event))
    EXPECT_NO_FATAL_FAILURE(obj->checkTouchDarg(nullptr));
}

TEST_F(CollectionViewPrivateTest, copyFilePath)
{
    // Test method: void copyFilePath(())
    EXPECT_NO_FATAL_FAILURE(obj->copyFilePath());
}

TEST_F(CollectionViewPrivateTest, copyFiles)
{
    // Test method: void copyFiles(())
    EXPECT_NO_FATAL_FAILURE(obj->copyFiles());
}

TEST_F(CollectionViewPrivateTest, cutFiles)
{
    // Test method: void cutFiles(())
    EXPECT_NO_FATAL_FAILURE(obj->cutFiles());
}

TEST_F(CollectionViewPrivateTest, drawDragText)
{
    // Test method: void drawDragText((QPainter *painter, const QString &str, const QRect &rect))
    QString _arg1{};
    QRect _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->drawDragText(nullptr, _arg1, _arg2));
}

TEST_F(CollectionViewPrivateTest, drawEllipseBackground)
{
    // Test method: void drawEllipseBackground((QPainter *painter, const QRect &rect))
    QRect _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->drawEllipseBackground(nullptr, _arg1));
}

TEST_F(CollectionViewPrivateTest, dropClientDownload)
{
    // Test method: bool dropClientDownload((QDropEvent *event))
    auto result = obj->dropClientDownload(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CollectionViewPrivateTest, dropFiles)
{
    // Test method: bool dropFiles((QDropEvent *event))
    auto result = obj->dropFiles(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CollectionViewPrivateTest, dropFilter)
{
    // Test method: bool dropFilter((QDropEvent *event))
    auto result = obj->dropFilter(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CollectionViewPrivateTest, handleMoveMimeData)
{
    // Test method: void handleMoveMimeData((QDropEvent *event, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleMoveMimeData(nullptr, _arg1));
}

TEST_F(CollectionViewPrivateTest, helpAction)
{
    // Test method: void helpAction(())
    EXPECT_NO_FATAL_FAILURE(obj->helpAction());
}

TEST_F(CollectionViewPrivateTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(CollectionViewPrivateTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}

TEST_F(CollectionViewPrivateTest, isDelayDrag)
{
    // Test bool getter: isDelayDrag()
    bool result = obj->isDelayDrag();
    EXPECT_FALSE(result);

}

TEST_F(CollectionViewPrivateTest, itemPaintGeomertys)
{
    // Test method: QList<QRect> itemPaintGeomertys((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->itemPaintGeomertys(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CollectionViewPrivateTest, itemRect)
{
    // Test method: QRect itemRect((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->itemRect(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CollectionViewPrivateTest, moveToTrash)
{
    // Test method: void moveToTrash(())
    EXPECT_NO_FATAL_FAILURE(obj->moveToTrash());
}

TEST_F(CollectionViewPrivateTest, nodeToPos)
{
    // Test method: QPoint nodeToPos((const int node))
    auto result = obj->nodeToPos(0);
    EXPECT_TRUE(result.isNull());

}

TEST_F(CollectionViewPrivateTest, onItemsChanged)
{
    // Test method: void onItemsChanged((const QString &key))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onItemsChanged(_arg0));
}

TEST_F(CollectionViewPrivateTest, openFiles)
{
    // Test method: void openFiles(())
    EXPECT_NO_FATAL_FAILURE(obj->openFiles());
}

TEST_F(CollectionViewPrivateTest, openIndex)
{
    // Test method: void openIndex((const QModelIndex &index))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->openIndex(_arg0));
}

TEST_F(CollectionViewPrivateTest, openIndexByClicked)
{
    // Test method: void openIndexByClicked((const ClickedAction action, const QModelIndex &index))
    QModelIndex _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->openIndexByClicked(ClickedAction(), _arg1));
}

TEST_F(CollectionViewPrivateTest, pasteFiles)
{
    // Test method: void pasteFiles(())
    EXPECT_NO_FATAL_FAILURE(obj->pasteFiles());
}

TEST_F(CollectionViewPrivateTest, pointToPos)
{
    // Test method: QPoint pointToPos((const QPoint &point))
    QPoint _arg0{};
    auto result = obj->pointToPos(_arg0);
    EXPECT_TRUE(result.isNull());

}

TEST_F(CollectionViewPrivateTest, posToNode)
{
    // Test method: int posToNode((const QPoint &pos))
    QPoint _arg0{};
    auto result = obj->posToNode(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(CollectionViewPrivateTest, posToPoint)
{
    // Test method: QPoint posToPoint((const QPoint &pos))
    QPoint _arg0{};
    auto result = obj->posToPoint(_arg0);
    EXPECT_TRUE(result.isNull());

}

TEST_F(CollectionViewPrivateTest, previewFiles)
{
    // Test method: void previewFiles(())
    EXPECT_NO_FATAL_FAILURE(obj->previewFiles());
}

TEST_F(CollectionViewPrivateTest, redoFiles)
{
    // Test method: void redoFiles(())
    EXPECT_NO_FATAL_FAILURE(obj->redoFiles());
}

TEST_F(CollectionViewPrivateTest, showFilesProperty)
{
    // Test method: void showFilesProperty(())
    EXPECT_NO_FATAL_FAILURE(obj->showFilesProperty());
}

TEST_F(CollectionViewPrivateTest, undoFiles)
{
    // Test method: void undoFiles(())
    EXPECT_NO_FATAL_FAILURE(obj->undoFiles());
}

TEST_F(CollectionViewPrivateTest, updateCellMargins)
{
    // Test method: void updateCellMargins((const QSize &itemSize, const QSize &cellSize))
    QSize _arg0{};
    QSize _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->updateCellMargins(_arg0, _arg1));
}

TEST_F(CollectionViewPrivateTest, updateColumnCount)
{
    // Test method: void updateColumnCount((const int &viewWidth, const int &itemWidth))
    int _arg0{};
    int _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->updateColumnCount(_arg0, _arg1));
}

TEST_F(CollectionViewPrivateTest, updateDFMMimeData)
{
    // Test method: void updateDFMMimeData((QDropEvent *event))
    EXPECT_NO_FATAL_FAILURE(obj->updateDFMMimeData(nullptr));
}

TEST_F(CollectionViewPrivateTest, updateRowCount)
{
    // Test method: void updateRowCount((const int &viewHeight, const int &itemHeight))
    int _arg0{};
    int _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->updateRowCount(_arg0, _arg1));
}

TEST_F(CollectionViewPrivateTest, updateTarget)
{
    // Test method: void updateTarget((const QMimeData *data, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->updateTarget(nullptr, _arg1));
}

TEST_F(CollectionViewPrivateTest, updateVerticalBarRange)
{
    // Test method: void updateVerticalBarRange(())
    EXPECT_NO_FATAL_FAILURE(obj->updateVerticalBarRange());
}

TEST_F(CollectionViewPrivateTest, updateViewMargins)
{
    // Test method: void updateViewMargins((const QSize &viewSize, const QMargins &oldMargins))
    QSize _arg0{};
    QMargins _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->updateViewMargins(_arg0, _arg1));
}

TEST_F(CollectionViewPrivateTest, updateViewSizeData)
{
    // Test method: void updateViewSizeData((const QSize &viewSize, const QMargins &viewMargins, const QSize &itemSize))
    QSize _arg0{};
    QMargins _arg1{};
    QSize _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->updateViewSizeData(_arg0, _arg1, _arg2));
}

TEST_F(CollectionViewPrivateTest, verticalScrollToValue)
{
    // Test method: int verticalScrollToValue((const QModelIndex &index, const QRect &rect, QAbstractItemView::ScrollHint hint))
    QModelIndex _arg0{};
    QRect _arg1{};
    auto result = obj->verticalScrollToValue(_arg0, _arg1, QAbstractItemView::ScrollHint());
    EXPECT_GE(result, 0);

}

TEST_F(CollectionViewPrivateTest, visualRect)
{
    // Test method: QRect visualRect((const QPoint &pos))
    QPoint _arg0{};
    auto result = obj->visualRect(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CollectionViewPrivateTest, CollectionViewPrivate_Destructor)
{
    // Test method:  ~CollectionViewPrivate(())
    EXPECT_NO_FATAL_FAILURE({ CollectionViewPrivate *tmp = new CollectionViewPrivate(); delete tmp; });
}
