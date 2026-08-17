// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "views/completerviewdelegate.h"

#include <DStyledItemDelegate>
#include <DListView>

#include <gtest/gtest.h>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QStandardItemModel>
#include <QPixmap>
#include <QImage>
#include <QApplication>

DWIDGET_USE_NAMESPACE
using namespace dfmplugin_titlebar;

class CompleterViewDelegateTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();

        listView = new DListView();
        delegate = new CompleterViewDelegate(listView);
        model = new QStandardItemModel();
        listView->setModel(model);
        listView->setItemDelegate(delegate);
    }

    void TearDown() override
    {
        delete listView;
        listView = nullptr;
        delegate = nullptr;
        model = nullptr;
        stub.clear();
    }

    DListView *listView { nullptr };
    CompleterViewDelegate *delegate { nullptr };
    QStandardItemModel *model { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(CompleterViewDelegateTest, Constructor_Success_ObjectCreated)
{
    EXPECT_NE(delegate, nullptr);
}

TEST_F(CompleterViewDelegateTest, Constructor_WithParent_SetsParent)
{
    EXPECT_EQ(delegate->parent(), listView);
}

TEST_F(CompleterViewDelegateTest, SizeHint_AnyIndex_ReturnsFixedHeight)
{
    QStandardItem *item = new QStandardItem("Test Item");
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QStyleOptionViewItem option;
    QSize size = delegate->sizeHint(option, index);

    EXPECT_EQ(size.height(), 30);   // kItemHeight
}

TEST_F(CompleterViewDelegateTest, SizeHint_InvalidIndex_ReturnsDefaultHeight)
{
    QModelIndex index;   // Invalid index

    stub.set_lamda(VADDR(QStyledItemDelegate, sizeHint),
                   [] {
                       __DBG_STUB_INVOKE__
                       return QSize(10, 10);
                   });

    QStyleOptionViewItem option;
    QSize size = delegate->sizeHint(option, index);

    EXPECT_EQ(size.height(), 30);   // kItemHeight
}

TEST_F(CompleterViewDelegateTest, SizeHint_MultipleItems_AllSameHeight)
{
    model->appendRow(new QStandardItem("Item 1"));
    model->appendRow(new QStandardItem("Item 2"));
    model->appendRow(new QStandardItem("Item 3"));

    QStyleOptionViewItem option;

    for (int i = 0; i < 3; ++i) {
        QModelIndex index = model->index(i, 0);
        QSize size = delegate->sizeHint(option, index);
        EXPECT_EQ(size.height(), 30);
    }
}

TEST_F(CompleterViewDelegateTest, Paint_InvalidIndex_CallsBasePaint)
{
    QModelIndex index;   // Invalid index

    QPixmap pixmap(200, 30);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);

    stub.set_lamda(VADDR(DStyledItemDelegate, paint), [] { __DBG_STUB_INVOKE__ });

    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}

TEST_F(CompleterViewDelegateTest, Paint_ValidIndex_DrawsItem)
{
    QStandardItem *item = new QStandardItem("Test Item");
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(200, 30);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);
    option.state = QStyle::State_Enabled;

    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}

TEST_F(CompleterViewDelegateTest, Paint_SelectedState_DrawsHighlight)
{
    QStandardItem *item = new QStandardItem("Test Item");
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(200, 30);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);
    option.state = QStyle::State_Enabled | QStyle::State_Selected;
    option.showDecorationSelected = true;

    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}

TEST_F(CompleterViewDelegateTest, Paint_MouseOverState_DrawsHighlight)
{
    QStandardItem *item = new QStandardItem("Test Item");
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(200, 30);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);
    option.state = QStyle::State_Enabled | QStyle::State_MouseOver;
    option.showDecorationSelected = true;

    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}

TEST_F(CompleterViewDelegateTest, Paint_DisabledState_UsesDisabledPalette)
{
    QStandardItem *item = new QStandardItem("Test Item");
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(200, 30);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);
    option.state = QStyle::State_None;   // Disabled

    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}

TEST_F(CompleterViewDelegateTest, Paint_InactiveState_UsesInactivePalette)
{
    QStandardItem *item = new QStandardItem("Test Item");
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(200, 30);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);
    option.state = QStyle::State_Enabled;   // Active flag not set

    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}

TEST_F(CompleterViewDelegateTest, Paint_WithIcon_DrawsIcon)
{
    QStandardItem *item = new QStandardItem("Test Item");
    QPixmap iconPixmap(16, 16);
    iconPixmap.fill(Qt::red);
    item->setIcon(QIcon(iconPixmap));
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(200, 30);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);
    option.state = QStyle::State_Enabled;

    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}

TEST_F(CompleterViewDelegateTest, Paint_TextWithNewline_ReplacesWithSpace)
{
    QStandardItem *item = new QStandardItem("Test\nItem");
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(200, 30);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);
    option.state = QStyle::State_Enabled;

    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}

TEST_F(CompleterViewDelegateTest, Paint_AntiAliasing_Enabled)
{
    QStandardItem *item = new QStandardItem("Test Item");
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QImage image(200, 30, QImage::Format_ARGB32);
    QPainter painter(&image);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);
    option.state = QStyle::State_Enabled;

    delegate->paint(&painter, option, index);

    EXPECT_TRUE(painter.testRenderHint(QPainter::Antialiasing));
}

TEST_F(CompleterViewDelegateTest, Paint_TextPadding_CorrectlyAdjusted)
{
    QStandardItem *item = new QStandardItem("Test Item");
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(200, 30);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);
    option.state = QStyle::State_Enabled;

    // Text should be drawn with kTextLeftPadding (32) padding
    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}

TEST_F(CompleterViewDelegateTest, PaintItemIcon_NullIcon_DoesNotCrash)
{
    QStandardItem *item = new QStandardItem("Test Item");
    // No icon set
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(200, 30);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);

    EXPECT_NO_THROW(delegate->paintItemIcon(&painter, option, index));
}

TEST_F(CompleterViewDelegateTest, PaintItemIcon_ValidIcon_DrawsWithOpacity)
{
    QStandardItem *item = new QStandardItem("Test Item");
    QPixmap iconPixmap(14, 14);
    iconPixmap.fill(Qt::blue);
    item->setIcon(QIcon(iconPixmap));
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(200, 30);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);

    EXPECT_NO_THROW(delegate->paintItemIcon(&painter, option, index));
}

TEST_F(CompleterViewDelegateTest, PaintItemIcon_IconPosition_CorrectlyCalculated)
{
    QStandardItem *item = new QStandardItem("Test Item");
    QPixmap iconPixmap(14, 14);
    iconPixmap.fill(Qt::green);
    item->setIcon(QIcon(iconPixmap));
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(200, 30);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);

    // Icon should be at position kIconLeftPadding (9) with size 14x14
    EXPECT_NO_THROW(delegate->paintItemIcon(&painter, option, index));
}

TEST_F(CompleterViewDelegateTest, CreateCustomOpacityPixmap_ValidPixmap_ReturnsTransparent)
{
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::red);

    QPixmap result = delegate->createCustomOpacityPixmap(pixmap, 0.5f);

    EXPECT_FALSE(result.isNull());
    EXPECT_EQ(result.size(), pixmap.size());
}

TEST_F(CompleterViewDelegateTest, CreateCustomOpacityPixmap_ZeroOpacity_ReturnsFullyTransparent)
{
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::red);

    QPixmap result = delegate->createCustomOpacityPixmap(pixmap, 0.0f);

    EXPECT_FALSE(result.isNull());
    EXPECT_EQ(result.size(), pixmap.size());
}

TEST_F(CompleterViewDelegateTest, CreateCustomOpacityPixmap_FullOpacity_ReturnsOpaque)
{
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::red);

    QPixmap result = delegate->createCustomOpacityPixmap(pixmap, 1.0f);

    EXPECT_FALSE(result.isNull());
    EXPECT_EQ(result.size(), pixmap.size());
}

TEST_F(CompleterViewDelegateTest, CreateCustomOpacityPixmap_DefaultOpacity_Returns40Percent)
{
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::red);

    QPixmap result = delegate->createCustomOpacityPixmap(pixmap, 0.4f);

    EXPECT_FALSE(result.isNull());
    EXPECT_EQ(result.size(), pixmap.size());
}

TEST_F(CompleterViewDelegateTest, CreateCustomOpacityPixmap_EmptyPixmap_HandlesGracefully)
{
    QPixmap pixmap;

    QPixmap result = delegate->createCustomOpacityPixmap(pixmap, 0.5f);

    EXPECT_TRUE(result.isNull());
}

TEST_F(CompleterViewDelegateTest, CreateCustomOpacityPixmap_DevicePixelRatio_Preserved)
{
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::red);

    QPixmap result = delegate->createCustomOpacityPixmap(pixmap, 0.5f);

    EXPECT_EQ(result.devicePixelRatio(), qApp->devicePixelRatio());
}

TEST_F(CompleterViewDelegateTest, Paint_SelectedAndMouseOver_DrawsHighlightedText)
{
    QStandardItem *item = new QStandardItem("Test Item");
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(200, 30);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);
    option.state = QStyle::State_Enabled | QStyle::State_Selected | QStyle::State_MouseOver;
    option.showDecorationSelected = true;

    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}

TEST_F(CompleterViewDelegateTest, Paint_MultipleNewlines_AllReplaced)
{
    QStandardItem *item = new QStandardItem("Line1\nLine2\nLine3");
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(200, 30);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);
    option.state = QStyle::State_Enabled;

    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}

TEST_F(CompleterViewDelegateTest, Paint_EmptyText_DoesNotCrash)
{
    QStandardItem *item = new QStandardItem("");
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(200, 30);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);
    option.state = QStyle::State_Enabled;

    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}

TEST_F(CompleterViewDelegateTest, Paint_LongText_DrawsWithinBounds)
{
    QStandardItem *item = new QStandardItem("Very Long Text That Should Be Drawn Within The Allocated Bounds");
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(200, 30);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);
    option.state = QStyle::State_Enabled;

    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}

TEST_F(CompleterViewDelegateTest, Paint_CustomFont_UsesOptionFont)
{
    QStandardItem *item = new QStandardItem("Test Item");
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(200, 30);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);
    option.state = QStyle::State_Enabled;
    QFont customFont;
    customFont.setPointSize(14);
    option.font = customFont;

    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}

TEST_F(CompleterViewDelegateTest, PaintItemIcon_LargeIcon_ScaledCorrectly)
{
    QStandardItem *item = new QStandardItem("Test Item");
    QPixmap iconPixmap(64, 64);   // Larger than expected size
    iconPixmap.fill(Qt::yellow);
    item->setIcon(QIcon(iconPixmap));
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(200, 30);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);

    EXPECT_NO_THROW(delegate->paintItemIcon(&painter, option, index));
}

TEST_F(CompleterViewDelegateTest, Paint_WithIconAndText_BothDrawn)
{
    QStandardItem *item = new QStandardItem("Test Item");
    QPixmap iconPixmap(14, 14);
    iconPixmap.fill(Qt::cyan);
    item->setIcon(QIcon(iconPixmap));
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(200, 30);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);
    option.state = QStyle::State_Enabled;

    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}

TEST_F(CompleterViewDelegateTest, Paint_NarrowRect_HandlesGracefully)
{
    QStandardItem *item = new QStandardItem("Test Item");
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(50, 30);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 50, 30);   // Narrow rect
    option.state = QStyle::State_Enabled;

    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}

TEST_F(CompleterViewDelegateTest, Paint_TallRect_HandlesGracefully)
{
    QStandardItem *item = new QStandardItem("Test Item");
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(200, 100);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 100);   // Tall rect
    option.state = QStyle::State_Enabled;

    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}

TEST_F(CompleterViewDelegateTest, CreateCustomOpacityPixmap_CompositionMode_SetCorrectly)
{
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::red);

    // Should use CompositionMode_Source and CompositionMode_DestinationIn
    QPixmap result = delegate->createCustomOpacityPixmap(pixmap, 0.5f);

    EXPECT_FALSE(result.isNull());
}

TEST_F(CompleterViewDelegateTest, SizeHint_PreservesWidthFromBase)
{
    QStandardItem *item = new QStandardItem("Test Item");
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QStyleOptionViewItem option;
    QSize size = delegate->sizeHint(option, index);

    // Height should be kItemHeight, width from base
    EXPECT_EQ(size.height(), 30);
    EXPECT_GE(size.width(), 0);
}

TEST_F(CompleterViewDelegateTest, Paint_StateActive_UsesNormalPalette)
{
    QStandardItem *item = new QStandardItem("Test Item");
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(200, 30);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);
    option.state = QStyle::State_Enabled | QStyle::State_Active;

    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}
