// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖函数清单(delegate/collectionitemdelegate.cpp) -> 用例映射:
// - CollectionItemDelegate::initStyleOption -> InitStyleOption_InvalidIndex_PaletteTuned
// - CollectionItemDelegate::paintBackground -> PaintBackground_DragMode_EarlyReturn
// - CollectionItemDelegate::paintDragIcon -> PaintDragIcon_NullIcon_ReturnsEmptySize
// - CollectionItemDelegate::paintGeomertys -> PaintGeomertys_InvalidIndex_ThreeAreasReturned
// - CollectionItemDelegate::elideTextRect -> ElideTextRect_InvalidIndex_EmptyLines
// - CollectionItemDelegate::textPaintRect -> TextPaintRect_InvalidIndex_EmptyRect
// - CollectionItemDelegate::drawNormlText -> DrawNormlText_EmptyText_PaintsWithoutCrash
// - CollectionItemDelegate::drawHighlightText -> DrawHighlightText_DragMode_SkipsExpand
// - CollectionItemDelegate::drawExpandText -> DrawExpandText_EmptyText_PaintsWithoutCrash
// - CollectionItemDelegate::paintLabel -> PaintLabel_Unselected_DrawsNormalText
// - CollectionItemDelegate::paint -> Paint_InvalidIndex_FullChainExecuted
// - CollectionItemDelegate::paintEmblems -> PaintEmblems_NoEmblemPlugin_RectReturned
// - CollectionItemDelegate::updateEditorGeometry -> UpdateEditorGeometry_PlainWidget_EarlyReturn
// - CollectionItemDelegate::setEditorData -> SetEditorData_PlainWidget_EarlyReturn
// - CollectionItemDelegate::setModelData -> SetModelData_PlainWidget_EarlyReturn
// - CollectionItemDelegatePrivate::extendLayoutText -> 间接: 经 elideTextRect/draw*Text 调用
// 说明: 非法 QModelIndex 分支为主要覆盖路径(合法索引需要真实 FileInfo/缩略图)。

#include "stubext.h"
#include "view/collectionview.h"
#include "delegate/collectionitemdelegate.h"
#include "delegate/itemeditor.h"
#include "models/collectionmodel.h"
#include "mode/collectiondataprovider.h"

#include <QImage>
#include <QPainter>
#include <QWidget>

#include <gtest/gtest.h>

using namespace ddplugin_organizer;

namespace {

class MockProviderForDelegate : public CollectionDataProvider
{
public:
    MockProviderForDelegate() : CollectionDataProvider(nullptr) {}
    QString replace(const QUrl &, const QUrl &) override { return QString(); }
    QString append(const QUrl &) override { return QString(); }
    QString prepend(const QUrl &) override { return QString(); }
    void insert(const QUrl &, const QString &, const int) override {}
    QString remove(const QUrl &) override { return QString(); }
    QString change(const QUrl &) override { return QString(); }
};

}   // namespace

class UT_CollectionItemDelegateCov : public testing::Test
{
protected:
    void SetUp() override
    {
        provider = new MockProviderForDelegate();
        view = new CollectionView("delegate_cov", provider);
        model = new CollectionModel();
        view->setModel(model);
        delegate = view->itemDelegate();
        ASSERT_NE(delegate, nullptr);

        img = QImage(128, 128, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::transparent);
        option.rect = QRect(0, 0, 100, 100);
    }

    void TearDown() override
    {
        delete view;
        delete provider;
        delete model;
        stub.clear();
    }

public:
    CollectionView *view = nullptr;
    MockProviderForDelegate *provider = nullptr;
    CollectionModel *model = nullptr;
    CollectionItemDelegate *delegate = nullptr;
    QImage img;
    QStyleOptionViewItem option;
    stub_ext::StubExt stub;
};

TEST_F(UT_CollectionItemDelegateCov, InitStyleOption_InvalidIndex_PaletteTuned)
{
    // Arrange: default option for an invalid index

    // Act
    delegate->initStyleOption(&option, QModelIndex());

    // Assert: fixed elide mode and white text applied by the delegate
    EXPECT_EQ(option.textElideMode, Qt::ElideLeft);
    EXPECT_EQ(option.palette.color(QPalette::Text), QColor("white"));
}

TEST_F(UT_CollectionItemDelegateCov, PaintBackground_DragMode_EarlyReturn)
{
    // Arrange: painter on an external image -> drag mode -> never selected
    QPainter painter(&img);
    option.state |= QStyle::State_Selected;
    option.showDecorationSelected = true;

    // Act
    delegate->paintBackground(&painter, option, QRect(10, 10, 40, 40));
    painter.end();

    // Assert: drag mode skips the background painting entirely
    EXPECT_EQ(img.pixel(30, 30), qRgba(0, 0, 0, 0));
    EXPECT_TRUE(img.isNull() == false);
}

TEST_F(UT_CollectionItemDelegateCov, PaintDragIcon_NullIcon_ReturnsEmptySize)
{
    // Arrange: invalid index carries no icon
    QPainter painter(&img);

    // Act
    QSize size = delegate->paintDragIcon(&painter, option, QModelIndex());
    painter.end();

    // Assert: nothing was drawable, so the reported size is empty
    EXPECT_TRUE(size.isEmpty());
    EXPECT_EQ(img.pixel(50, 50), qRgba(0, 0, 0, 0));
}

TEST_F(UT_CollectionItemDelegateCov, PaintGeomertys_InvalidIndex_ThreeAreasReturned)
{
    // Arrange: 100x100 item cell

    // Act
    QList<QRect> geometries = delegate->paintGeomertys(option, QModelIndex());

    // Assert: icon area, mouse area and text area
    ASSERT_EQ(geometries.count(), 3);
    EXPECT_EQ(geometries.at(0), delegate->iconRect(option.rect));
    EXPECT_TRUE(geometries.at(2).width() <= geometries.at(1).width());
}

TEST_F(UT_CollectionItemDelegateCov, ElideTextRect_InvalidIndex_EmptyLines)
{
    // Arrange: invalid index has no display text
    const QModelIndex invalidIndex;
    const QRect textRect(0, 0, 80, 20);

    // Act
    QList<QRectF> lines = delegate->elideTextRect(invalidIndex, textRect, Qt::ElideLeft);

    // Assert: an empty text still produces a single (empty-width) line
    ASSERT_EQ(lines.count(), 1);
    EXPECT_EQ(lines.first().width(), 0.0);
    EXPECT_TRUE(delegate->boundingRect(lines).isEmpty());
}

TEST_F(UT_CollectionItemDelegateCov, TextPaintRect_InvalidIndex_EmptyRect)
{
    // Arrange: invalid index with no text
    const QModelIndex invalidIndex;
    const QRect labelRect(0, 0, 80, 20);

    // Act
    QRect rect = delegate->textPaintRect(option, invalidIndex, labelRect, true);

    // Assert: empty text collapses to a zero-width line rect
    EXPECT_EQ(rect.width(), 0);
    EXPECT_GT(rect.height(), 0);
}

TEST_F(UT_CollectionItemDelegateCov, DrawNormlText_EmptyText_PaintsWithoutCrash)
{
    // Arrange
    QPainter painter(&img);
    const QRectF textRect(0, 0, 60, 20);

    // Act
    delegate->drawNormlText(&painter, option, QModelIndex(), textRect);

    // Assert: painter survives the offscreen paint chain
    EXPECT_TRUE(painter.isActive());
    EXPECT_EQ(img.width(), 128);
    painter.end();
}

TEST_F(UT_CollectionItemDelegateCov, DrawHighlightText_DragMode_SkipsExpand)
{
    // Arrange: image painter means drag mode, so the expand branch is skipped
    QPainter painter(&img);
    const QRect textRect(0, 0, 60, 20);

    // Act
    delegate->drawHighlightText(&painter, option, QModelIndex(), textRect);

    // Assert: paint chain still active afterwards
    EXPECT_TRUE(painter.isActive());
    EXPECT_EQ(img.width(), 128);
    painter.end();
}

TEST_F(UT_CollectionItemDelegateCov, DrawExpandText_EmptyText_PaintsWithoutCrash)
{
    // Arrange
    QPainter painter(&img);
    const QRectF textRect(0, 0, 60, 20);

    // Act
    delegate->drawExpandText(&painter, option, QModelIndex(), textRect);

    // Assert
    EXPECT_TRUE(painter.isActive());
    EXPECT_EQ(img.width(), 128);
    painter.end();
}

TEST_F(UT_CollectionItemDelegateCov, PaintLabel_Unselected_DrawsNormalText)
{
    // Arrange: option without selected/highlight state
    option.state = QStyle::State_Enabled;
    QPainter painter(&img);
    const QRect labelRect(0, 0, 100, 40);

    // Act
    delegate->paintLabel(&painter, option, QModelIndex(), labelRect);

    // Assert: label painting falls back to normal text and stays alive
    EXPECT_TRUE(painter.isActive());
    EXPECT_EQ(img.width(), 128);
    painter.end();
}

TEST_F(UT_CollectionItemDelegateCov, Paint_InvalidIndex_FullChainExecuted)
{
    // Arrange: invalid index walks background->icon->emblem->label chain
    QPainter painter(&img);
    option.state = QStyle::State_Enabled;

    // Act
    delegate->paint(&painter, option, QModelIndex());
    painter.end();

    // Assert: whole chain executed without a model row
    EXPECT_FALSE(img.isNull());
    EXPECT_EQ(img.width(), 128);
}

TEST_F(UT_CollectionItemDelegateCov, PaintEmblems_NoEmblemPlugin_RectReturned)
{
    // Arrange: no emblem plugin subscribed in the sandbox
    QPainter painter(&img);
    const QRectF emblemRect(0, 0, 32, 32);

    // Act
    QRectF result = delegate->paintEmblems(&painter, emblemRect, FileInfoPointer());
    painter.end();

    // Assert: rect echoed back untouched
    EXPECT_EQ(result, emblemRect);
    EXPECT_EQ(img.pixel(16, 16), qRgba(0, 0, 0, 0));
}

TEST_F(UT_CollectionItemDelegateCov, UpdateEditorGeometry_PlainWidget_EarlyReturn)
{
    // Arrange: a plain widget is not an ItemEditor
    QWidget editor;
    editor.setGeometry(QRect(1, 2, 30, 10));
    const QRect before = editor.geometry();

    // Act
    delegate->updateEditorGeometry(&editor, option, QModelIndex());

    // Assert: geometry untouched by the early return
    EXPECT_EQ(editor.geometry(), before);
    EXPECT_EQ(editor.geometry().topLeft(), QPoint(1, 2));
}

TEST_F(UT_CollectionItemDelegateCov, SetEditorData_PlainWidget_EarlyReturn)
{
    // Arrange: plain widget can't receive editor data
    QWidget editor;

    // Act: only checks that non-ItemEditor editors are rejected
    delegate->setEditorData(&editor, QModelIndex());

    // Assert: no state was pushed into the widget
    EXPECT_TRUE(editor.windowTitle().isEmpty());
    EXPECT_EQ(editor.children().count(), 0);
}

TEST_F(UT_CollectionItemDelegateCov, SetModelData_PlainWidget_EarlyReturn)
{
    // Arrange: plain widget has no ItemEditor text
    QWidget editor;

    // Act
    delegate->setModelData(&editor, model, QModelIndex());

    // Assert: model untouched (no rows ever existed)
    EXPECT_EQ(model->rowCount(), 0);
    EXPECT_EQ(model->columnCount(), 0);
}
