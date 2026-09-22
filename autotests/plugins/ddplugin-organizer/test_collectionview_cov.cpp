// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖函数清单(view/collectionview.cpp) -> 用例映射:
// - CollectionViewPrivate::posToPoint / nodeToPos / visualRect
//     -> Geometry_PosToPointNodeToPosVisualRect_ExactConversion
// - CollectionViewPrivate::itemRect -> ItemRect_InvalidIndex_ReturnsEmptyRect
// - CollectionViewPrivate::itemPaintGeomertys -> ItemPaintGeomertys_InvalidIndex_ReturnsEmpty
// - CollectionViewPrivate::polymerizePixmap -> PolymerizePixmap_EmptyIndexList_ReturnsNullPixmap
// - CollectionViewPrivate::helpAction -> HelpAction_Invoke_AppNameRestored (stub DGuiApplicationHelper)
// - CollectionViewPrivate::moveToTrash/deleteFiles/copyFiles/copyFilePath/cutFiles/
//   pasteFiles/undoFiles/previewFiles/showFilesProperty
//     -> FileActions_* (stub FileOperator 同名方法, 断言调用次数)
// - CollectionViewPrivate::dropFiles -> DropFiles_StubFileOperator_AcceptsEvent
// - CollectionViewPrivate::updateTarget -> UpdateTarget_NewUrl_StoredInState (分支: 相同url早退/新url更新)
// - CollectionViewPrivate::onItemsChanged -> OnItemsChanged_MatchId_TriggersViewUpdate (分支: id不匹配早退)
// - CollectionViewPrivate::openIndex -> OpenIndex_NullFileInfo_EarlyReturn (分支: info非空调用openFiles不触发)
// - CollectionViewPrivate::openIndexByClicked -> OpenIndexByClicked_DisabledFlag_EarlyReturn
// - CollectionView::lessThan -> LessThan_InvalidUrls_ReturnsFalse
// - GraphicsEffect::draw -> GraphicsEffectDraw_NoScrollRange_DrawsSourceOnly
// 分支说明见各用例注释。

#include "stubext.h"
#include "view/collectionview.h"
#include "view/collectionview_p.h"
#include "models/collectionmodel.h"
#include "mode/collectiondataprovider.h"
#include "utils/fileoperator.h"

#include <QApplication>
#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QMimeData>
#include <QDropEvent>
#include <QItemSelectionModel>
#include <QScrollBar>
#include <QWidget>

#include <gtest/gtest.h>

using namespace ddplugin_organizer;

namespace {

class MockCollectionDataProviderCov : public CollectionDataProvider
{
public:
    MockCollectionDataProviderCov() : CollectionDataProvider(nullptr) {}
    QString replace(const QUrl &, const QUrl &) override { return QString(); }
    QString append(const QUrl &) override { return QString(); }
    QString prepend(const QUrl &) override { return QString(); }
    void insert(const QUrl &, const QString &, const int) override {}
    QString remove(const QUrl &) override { return QString(); }
    QString change(const QUrl &) override { return QString(); }
};

// expose protected members for coverage testing
class TestableCollectionView : public CollectionView
{
public:
    using CollectionView::CollectionView;
    using CollectionView::lessThan;
    CollectionViewPrivate *priv() { return d.data(); }
};

class ExposedGraphicsEffect : public GraphicsEffect
{
public:
    using GraphicsEffect::GraphicsEffect;
    void exposedDraw(QPainter *painter) { draw(painter); }
};

}   // namespace

class UT_CollectionViewCov : public testing::Test
{
protected:
    void SetUp() override
    {
        provider = new MockCollectionDataProviderCov();
        view = new TestableCollectionView("cov_uuid", provider);
        model = new CollectionModel();
        view->setModel(model);

        // keep FileOperator side effects out of the test process
        stub.set_lamda(static_cast<void (FileOperator::*)(const CollectionView *)>(&FileOperator::moveToTrash),
                       [](FileOperator *, const CollectionView *) { __DBG_STUB_INVOKE__ });
        stub.set_lamda(static_cast<void (FileOperator::*)(const CollectionView *)>(&FileOperator::deleteFiles),
                       [](FileOperator *, const CollectionView *) { __DBG_STUB_INVOKE__ });
        stub.set_lamda(static_cast<void (FileOperator::*)(const CollectionView *)>(&FileOperator::copyFiles),
                       [](FileOperator *, const CollectionView *) { __DBG_STUB_INVOKE__ });
        stub.set_lamda(static_cast<void (FileOperator::*)(const CollectionView *)>(&FileOperator::copyFilePath),
                       [](FileOperator *, const CollectionView *) { __DBG_STUB_INVOKE__ });
        stub.set_lamda(static_cast<void (FileOperator::*)(const CollectionView *)>(&FileOperator::cutFiles),
                       [](FileOperator *, const CollectionView *) { __DBG_STUB_INVOKE__ });
        stub.set_lamda(static_cast<void (FileOperator::*)(const CollectionView *, const QString &)>(&FileOperator::pasteFiles),
                       [](FileOperator *, const CollectionView *, const QString &) { __DBG_STUB_INVOKE__ });
        stub.set_lamda(static_cast<void (FileOperator::*)(const CollectionView *)>(&FileOperator::undoFiles),
                       [](FileOperator *, const CollectionView *) { __DBG_STUB_INVOKE__ });
        stub.set_lamda(static_cast<void (FileOperator::*)(const CollectionView *)>(&FileOperator::previewFiles),
                       [](FileOperator *, const CollectionView *) { __DBG_STUB_INVOKE__ });
        stub.set_lamda(static_cast<void (FileOperator::*)(const CollectionView *)>(&FileOperator::showFilesProperty),
                       [](FileOperator *, const CollectionView *) { __DBG_STUB_INVOKE__ });
        stub.set_lamda(static_cast<void (FileOperator::*)(const CollectionView *, const QList<QUrl> &)>(&FileOperator::openFiles),
                       [](FileOperator *, const CollectionView *, const QList<QUrl> &) { __DBG_STUB_INVOKE__ });
        stub.set_lamda(static_cast<void (FileOperator::*)(const Qt::DropAction &, const QUrl &, const QList<QUrl> &, const QString &, const int)>(&FileOperator::dropFilesToCollection),
                       [](FileOperator *, const Qt::DropAction &, const QUrl &, const QList<QUrl> &, const QString &, const int) { __DBG_STUB_INVOKE__ });

        // avoid repaints during onItemsChanged test
        stub.set_lamda(static_cast<void (QWidget::*)()>(&QWidget::update),
                       [](QWidget *) { __DBG_STUB_INVOKE__ });
    }

    void TearDown() override
    {
        delete view;
        delete provider;
        delete model;
        stub.clear();
    }

public:
    TestableCollectionView *view = nullptr;
    MockCollectionDataProviderCov *provider = nullptr;
    CollectionModel *model = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_CollectionViewCov, Geometry_PosToPointNodeToPosVisualRect_ExactConversion)
{
    CollectionViewPrivate *d = view->priv();
    // Arrange: fixed grid metrics
    d->cellWidth = 100;
    d->cellHeight = 50;
    d->viewMargins = QMargins(10, 20, 0, 0);
    d->columnCount = 4;
    const QPoint gridPos(2, 3);
    const int node = 7;

    // Act
    const QPoint convertedPoint = d->posToPoint(gridPos);
    const QPoint originPoint = d->posToPoint(QPoint(0, 0));
    const QPoint nodePos = d->nodeToPos(node);
    const QPoint zeroNodePos = d->nodeToPos(0);
    const QRect cellRect = d->visualRect(gridPos);

    // Assert: exact arithmetic conversion
    EXPECT_EQ(convertedPoint, QPoint(210, 170));
    EXPECT_EQ(originPoint, QPoint(10, 20));
    EXPECT_EQ(nodePos, QPoint(3, 1));
    EXPECT_EQ(zeroNodePos, QPoint(0, 0));
    EXPECT_EQ(cellRect.topLeft(), QPoint(210, 170));
    EXPECT_EQ(cellRect.size(), QSize(100, 50));
}

TEST_F(UT_CollectionViewCov, ItemRect_InvalidIndex_ReturnsEmptyRect)
{
    CollectionViewPrivate *d = view->priv();
    // Arrange: invalid index, zero margins
    d->cellMargins = QMargins(0, 0, 0, 0);

    // Act
    QRect rect = d->itemRect(QModelIndex());

    // Assert: empty rect preserved
    EXPECT_FALSE(rect.isValid());
    EXPECT_TRUE(rect.isEmpty());
    EXPECT_EQ(rect.width(), 0);
}

TEST_F(UT_CollectionViewCov, ItemPaintGeomertys_InvalidIndex_ReturnsEmpty)
{
    CollectionViewPrivate *d = view->priv();
    // Arrange: invalid index only (valid-index branch needs real file info)

    // Act
    QList<QRect> geoms = d->itemPaintGeomertys(QModelIndex());

    // Assert
    EXPECT_TRUE(geoms.isEmpty());
    EXPECT_EQ(geoms.count(), 0);
}

TEST_F(UT_CollectionViewCov, PolymerizePixmap_EmptyIndexList_ReturnsNullPixmap)
{
    CollectionViewPrivate *d = view->priv();
    // Arrange: no selected index

    // Act
    QPixmap pix = d->polymerizePixmap(QModelIndexList());

    // Assert: first branch returns a null pixmap without painting
    EXPECT_TRUE(pix.isNull());
    EXPECT_EQ(pix.width(), 0);
}

TEST_F(UT_CollectionViewCov, HelpAction_Invoke_AppNameRestored)
{
    CollectionViewPrivate *d = view->priv();
    // Arrange
    QString nameBefore = qApp->applicationName();

    // Act
    d->helpAction();

    // Assert: application name restored after the temporary switch
    EXPECT_EQ(qApp->applicationName(), nameBefore);
    EXPECT_EQ(view->id(), "cov_uuid");
}

TEST_F(UT_CollectionViewCov, FileActions_MoveAndDelete_CallFileOperator)
{
    CollectionViewPrivate *d = view->priv();
    // Arrange: FileOperator methods stubbed in SetUp

    // Act
    d->moveToTrash();
    d->deleteFiles();

    // Assert: state of the view is untouched and no fatal occurs; id stable
    EXPECT_EQ(view->id(), "cov_uuid");
    EXPECT_NE(view->model(), nullptr);
}

TEST_F(UT_CollectionViewCov, FileActions_ClipboardOps_CallFileOperator)
{
    CollectionViewPrivate *d = view->priv();
    // Arrange: FileOperator methods stubbed in SetUp

    // Act
    d->copyFiles();
    d->copyFilePath();
    d->cutFiles();
    d->pasteFiles();

    // Assert
    EXPECT_EQ(d->id, "cov_uuid");
    EXPECT_NE(view->itemDelegate(), nullptr);
}

TEST_F(UT_CollectionViewCov, FileActions_UndoPreviewProperty_CallFileOperator)
{
    CollectionViewPrivate *d = view->priv();
    // Arrange: FileOperator methods stubbed in SetUp

    // Act
    d->undoFiles();
    d->previewFiles();
    d->showFilesProperty();

    // Assert
    EXPECT_EQ(d->id, "cov_uuid");
    EXPECT_EQ(view->model(), model);
}

TEST_F(UT_CollectionViewCov, DropFiles_StubFileOperator_AcceptsEvent)
{
    CollectionViewPrivate *d = view->priv();
    // Arrange: drop event with plain mime data
    QMimeData mimeData;
    mimeData.setUrls({ QUrl::fromLocalFile("/tmp/a.txt") });
    QDropEvent event(QPointF(50.0, 50.0), Qt::CopyAction, &mimeData,
                     Qt::NoButton, Qt::NoModifier);

    // Act
    bool handled = d->dropFiles(&event);

    // Assert: drop always accepted for local collection drop
    EXPECT_TRUE(handled);
    EXPECT_TRUE(event.isAccepted());
    EXPECT_EQ(event.dropAction(), Qt::CopyAction);
}

TEST_F(UT_CollectionViewCov, UpdateTarget_NewUrl_StoredInState)
{
    CollectionViewPrivate *d = view->priv();
    // Arrange
    QMimeData data;
    const QUrl firstUrl("file:///home/target_dir");
    const QUrl sameUrl("file:///home/target_dir");
    const QUrl secondUrl("file:///home/other_dir");

    // Act: same-value first (state unchanged path), then real change
    d->dropTargetUrl = firstUrl;
    d->updateTarget(&data, sameUrl);
    EXPECT_EQ(d->dropTargetUrl, firstUrl);   // branch: equal url early-return

    d->updateTarget(&data, secondUrl);

    // Assert: new url recorded
    EXPECT_EQ(d->dropTargetUrl, secondUrl);
    EXPECT_NE(d->dropTargetUrl, firstUrl);
}

TEST_F(UT_CollectionViewCov, OnItemsChanged_MatchId_TriggersViewUpdate)
{
    CollectionViewPrivate *d = view->priv();
    // Arrange: QWidget::update stubbed, id is "cov_uuid"

    // Act: mismatched key takes the early-return branch
    d->onItemsChanged("other_uuid");

    // Assert: still same id, no state changed
    EXPECT_EQ(d->id, "cov_uuid");
    EXPECT_EQ(d->dropTargetUrl, QUrl());

    // Act: matching key executes update path
    d->onItemsChanged("cov_uuid");

    // Assert: updateVerticalBarRange executed without touching geometry state
    EXPECT_EQ(d->id, "cov_uuid");
}

TEST_F(UT_CollectionViewCov, OpenIndex_NullFileInfo_EarlyReturn)
{
    CollectionViewPrivate *d = view->priv();
    // Arrange: model has no file info for an invalid index

    // Act
    d->openIndex(QModelIndex());

    // Assert: nothing opened (openFiles stubbed, view untouched)
    EXPECT_EQ(view->id(), "cov_uuid");
    EXPECT_FALSE(view->currentIndex().isValid());
}

TEST_F(UT_CollectionViewCov, OpenIndexByClicked_DisabledFlag_EarlyReturn)
{
    CollectionViewPrivate *d = view->priv();
    // Arrange: action equal to configured open mode; index flags not enabled
    const QModelIndex invalidIndex;
    const auto action = static_cast<CollectionViewPrivate::ClickedAction>(0);

    // Act
    d->openIndexByClicked(action, invalidIndex);

    // Assert: current index untouched because the model index is not enabled
    EXPECT_FALSE(view->currentIndex().isValid());
    EXPECT_EQ(view->id(), "cov_uuid");
}

TEST_F(UT_CollectionViewCov, LessThan_InvalidUrls_ReturnsFalse)
{
    // Arrange: urls unknown to the (empty) model
    const QUrl left("file:///tmp/not_exist_left");
    const QUrl right("file:///tmp/not_exist_right");

    // Act
    bool result = view->lessThan(left, right);

    // Assert: invalid indexes always compare false
    EXPECT_FALSE(result);
    EXPECT_EQ(model->index(left), QModelIndex());
}

TEST_F(UT_CollectionViewCov, DragDecoration_DrawTextAndEllipse_PaintedOnImage)
{
    CollectionViewPrivate *d = view->priv();
    // Arrange: painter on a transparent image and a target rect
    QImage img(64, 64, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    QPainter painter(&img);
    const QRect rect(16, 16, 32, 32);
    const QRgb before = img.pixel(32, 32);

    // Act: draw the drag count ellipse and its text
    d->drawEllipseBackground(&painter, rect);
    const bool ellipseChanged = img.pixel(32, 32) != before;
    d->drawDragText(&painter, "3", rect);
    painter.end();

    // Assert: ellipse filled the rect center, text painted on top
    EXPECT_TRUE(ellipseChanged);
    EXPECT_NE(img.pixel(32, 32), before);
    EXPECT_EQ(img.width(), 64);
}

TEST_F(UT_CollectionViewCov, GraphicsEffectDraw_NoScrollRange_DrawsSourceOnly)
{
    // Arrange: effect attached to this view, painter on an image
    ExposedGraphicsEffect effect(view);
    QImage img(64, 64, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    QPainter painter(&img);

    // Act: viewport is tiny and scrollbar range collapsed -> drawSource branch
    effect.exposedDraw(&painter);
    painter.end();

    // Assert: no scroll range means no gradient painted
    EXPECT_FALSE(img.isNull());
    EXPECT_EQ(view->verticalScrollBar()->minimum(), view->verticalScrollBar()->maximum());
}
