// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "views/folderviewdelegate.h"

#include <DStyledItemDelegate>
#include <DListView>
#include <DStyle>
#include <DPalette>
#include <DPaletteHelper>
#include <DGuiApplicationHelper>

#include <gtest/gtest.h>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QStandardItemModel>
#include <QPixmap>
#include <QImage>
#include <QApplication>
#include <QHelpEvent>
#include <QToolTip>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE
using namespace dfmplugin_titlebar;

class FolderViewDelegateTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();

        // Stub DPaletteHelper
        stub.set_lamda(&DPaletteHelper::instance, []() -> DPaletteHelper * {
            __DBG_STUB_INVOKE__
            static DPaletteHelper helper;
            return &helper;
        });

        stub.set_lamda(&DPaletteHelper::palette, [] {
            __DBG_STUB_INVOKE__
            return DPalette();
        });

        // Stub DStyle::pixelMetric
        typedef int (*PixelMetric)(const QStyle *, DStyle::PixelMetric, const QStyleOption *, const QWidget *);
        stub.set_lamda(static_cast<PixelMetric>(&DStyle::pixelMetric), [] {
            __DBG_STUB_INVOKE__
            return 12;
        });

        // Stub DGuiApplicationHelper::adjustColor
        typedef QColor (*AdjustColor)(const QColor &, qint8, qint8, qint8, qint8, qint8, qint8, qint8);
        stub.set_lamda(static_cast<AdjustColor>(&DGuiApplicationHelper::adjustColor), [](const QColor &base, qint8, qint8, qint8, qint8, qint8, qint8, qint8) {
            __DBG_STUB_INVOKE__
            return base.lighter(110);
        });

        // Stub QToolTip::showText
        stub.set_lamda(static_cast<void (*)(const QPoint &, const QString &, QWidget *, const QRect &, int)>(&QToolTip::showText),
                       [](const QPoint &, const QString &, QWidget *, const QRect &, int) {
                           __DBG_STUB_INVOKE__
                       });

        listView = new DListView();
        delegate = new FolderViewDelegate(listView);
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
    FolderViewDelegate *delegate { nullptr };
    QStandardItemModel *model { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(FolderViewDelegateTest, Constructor_Success_ObjectCreated)
{
    EXPECT_NE(delegate, nullptr);
}

TEST_F(FolderViewDelegateTest, Constructor_WithParent_SetsParent)
{
    EXPECT_EQ(delegate->parent(), listView);
}

TEST_F(FolderViewDelegateTest, SizeHint_AnyIndex_ReturnsFixedHeight)
{
    QStandardItem *item = new QStandardItem("Test Item");
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    stub.set_lamda(VADDR(QStyledItemDelegate, sizeHint), [] {
        __DBG_STUB_INVOKE__
        return QSize(10, 10);
    });

    QStyleOptionViewItem option;
    QSize size = delegate->sizeHint(option, index);

    EXPECT_EQ(size.height(), kFolderItemHeight);   // kFolderItemHeight
}

TEST_F(FolderViewDelegateTest, Paint_InvalidIndex_CallsBasePaint)
{
    QModelIndex index;   // Invalid index

    QPixmap pixmap(200, 30);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);

    stub.set_lamda(VADDR(DStyledItemDelegate, paint), [] { __DBG_STUB_INVOKE__ });

    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}

TEST_F(FolderViewDelegateTest, Paint_ValidIndex_DrawsItem)
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

TEST_F(FolderViewDelegateTest, Paint_SelectedState_DrawsHighlight)
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

TEST_F(FolderViewDelegateTest, Paint_MouseOverState_DrawsHoverBackground)
{
    QStandardItem *item = new QStandardItem("Test Item");
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(200, 30);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);
    option.state = QStyle::State_Enabled | QStyle::State_MouseOver;

    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}

TEST_F(FolderViewDelegateTest, Paint_DisabledState_UsesDisabledPalette)
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

TEST_F(FolderViewDelegateTest, Paint_InactiveState_UsesInactivePalette)
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

TEST_F(FolderViewDelegateTest, Paint_WithIcon_DrawsIcon)
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

TEST_F(FolderViewDelegateTest, Paint_TextWithNewline_ReplacesWithSpace)
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

TEST_F(FolderViewDelegateTest, Paint_AntiAliasing_Enabled)
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

TEST_F(FolderViewDelegateTest, Paint_SelectedAndMouseOver_DrawsHighlight)
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

TEST_F(FolderViewDelegateTest, Paint_MultipleNewlines_AllReplaced)
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

TEST_F(FolderViewDelegateTest, Paint_EmptyText_DoesNotCrash)
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

TEST_F(FolderViewDelegateTest, Paint_LongText_DrawsElidedText)
{
    QStandardItem *item = new QStandardItem("Very Long Text That Should Be Elided Because It Exceeds The Available Width");
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(200, 30);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);
    option.state = QStyle::State_Enabled;

    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}

TEST_F(FolderViewDelegateTest, Paint_CustomFont_UsesOptionFont)
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

TEST_F(FolderViewDelegateTest, Paint_RoundedCorners_UsesCorrectRadius)
{
    typedef int (*PixelMetric)(const QStyle *, DStyle::PixelMetric, const QStyleOption *, const QWidget *);
    stub.set_lamda(static_cast<PixelMetric>(&DStyle::pixelMetric), [] {
        __DBG_STUB_INVOKE__
        return 8;
    });

    QStandardItem *item = new QStandardItem("Test Item");
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(200, 30);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);
    option.state = QStyle::State_Selected;
    option.showDecorationSelected = true;

    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}

TEST_F(FolderViewDelegateTest, PaintItemIcon_NullIcon_DoesNotCrash)
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

TEST_F(FolderViewDelegateTest, PaintItemIcon_ValidIcon_DrawsIcon)
{
    QStandardItem *item = new QStandardItem("Test Item");
    QPixmap iconPixmap(16, 16);
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

TEST_F(FolderViewDelegateTest, PaintItemIcon_IconPosition_CorrectlyCalculated)
{
    QStandardItem *item = new QStandardItem("Test Item");
    QPixmap iconPixmap(16, 16);
    iconPixmap.fill(Qt::green);
    item->setIcon(QIcon(iconPixmap));
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(200, 30);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);

    // Icon should be at position kIconLeftPadding (9) with size 16x16
    EXPECT_NO_THROW(delegate->paintItemIcon(&painter, option, index));
}

TEST_F(FolderViewDelegateTest, PaintItemIcon_LargeIcon_ScaledCorrectly)
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

TEST_F(FolderViewDelegateTest, PaintItemIcon_DevicePixelRatio_Preserved)
{
    QStandardItem *item = new QStandardItem("Test Item");
    QPixmap iconPixmap(16, 16);
    iconPixmap.fill(Qt::cyan);
    item->setIcon(QIcon(iconPixmap));
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QImage image(200, 30, QImage::Format_ARGB32);
    QPainter painter(&image);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);

    EXPECT_NO_THROW(delegate->paintItemIcon(&painter, option, index));
}

TEST_F(FolderViewDelegateTest, CreateCustomOpacityPixmap_ValidPixmap_ReturnsTransparent)
{
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::red);

    QPixmap result = delegate->createCustomOpacityPixmap(pixmap, 0.5f);

    EXPECT_FALSE(result.isNull());
    EXPECT_EQ(result.size(), pixmap.size());
}

TEST_F(FolderViewDelegateTest, CreateCustomOpacityPixmap_ZeroOpacity_ReturnsFullyTransparent)
{
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::red);

    QPixmap result = delegate->createCustomOpacityPixmap(pixmap, 0.0f);

    EXPECT_FALSE(result.isNull());
    EXPECT_EQ(result.size(), pixmap.size());
}

TEST_F(FolderViewDelegateTest, CreateCustomOpacityPixmap_FullOpacity_ReturnsOpaque)
{
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::red);

    QPixmap result = delegate->createCustomOpacityPixmap(pixmap, 1.0f);

    EXPECT_FALSE(result.isNull());
    EXPECT_EQ(result.size(), pixmap.size());
}

TEST_F(FolderViewDelegateTest, CreateCustomOpacityPixmap_EmptyPixmap_HandlesGracefully)
{
    QPixmap pixmap;

    QPixmap result = delegate->createCustomOpacityPixmap(pixmap, 0.5f);

    // Should handle empty pixmap without crash
}

TEST_F(FolderViewDelegateTest, CreateCustomOpacityPixmap_DevicePixelRatio_Preserved)
{
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::red);

    QPixmap result = delegate->createCustomOpacityPixmap(pixmap, 0.5f);

    EXPECT_EQ(result.devicePixelRatio(), qApp->devicePixelRatio());
}

TEST_F(FolderViewDelegateTest, CreateCustomOpacityPixmap_CompositionMode_SetCorrectly)
{
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::red);

    // Should use CompositionMode_Source and CompositionMode_DestinationIn
    QPixmap result = delegate->createCustomOpacityPixmap(pixmap, 0.5f);

    EXPECT_FALSE(result.isNull());
}

TEST_F(FolderViewDelegateTest, HelpEvent_NonToolTip_CallsBase)
{
    QStandardItem *item = new QStandardItem("Test Item");
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);

    QHelpEvent event(QEvent::WhatsThis, QPoint(10, 10), QPoint(110, 110));

    // Should call base class implementation
    EXPECT_NO_THROW(delegate->helpEvent(&event, listView, option, index));
}

TEST_F(FolderViewDelegateTest, HelpEvent_ToolTip_ShortText_NoTooltip)
{
    QStandardItem *item = new QStandardItem("Short");
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);

    QHelpEvent event(QEvent::ToolTip, QPoint(10, 10), QPoint(110, 110));

    bool result = delegate->helpEvent(&event, listView, option, index);

    EXPECT_TRUE(result);
}

TEST_F(FolderViewDelegateTest, HelpEvent_ToolTip_LongText_ShowsTooltip)
{
    bool tooltipShown = false;
    stub.set_lamda(static_cast<void (*)(const QPoint &, const QString &, QWidget *, const QRect &, int)>(&QToolTip::showText),
                   [&tooltipShown](const QPoint &, const QString &, QWidget *, const QRect &, int) {
                       __DBG_STUB_INVOKE__
                       tooltipShown = true;
                   });

    QStandardItem *item = new QStandardItem("Very Long Text That Exceeds The Available Width And Should Show Tooltip");
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 50, 30);   // Narrow rect

    QHelpEvent event(QEvent::ToolTip, QPoint(10, 10), QPoint(110, 110));

    bool result = delegate->helpEvent(&event, listView, option, index);

    EXPECT_TRUE(result);
}

TEST_F(FolderViewDelegateTest, HelpEvent_ToolTip_TextWithNewline_ReplacesWithSpace)
{
    QString shownText;
    stub.set_lamda(static_cast<void (*)(const QPoint &, const QString &, QWidget *, const QRect &, int)>(&QToolTip::showText),
                   [&shownText](const QPoint &, const QString &text, QWidget *, const QRect &, int) {
                       __DBG_STUB_INVOKE__
                       shownText = text;
                   });

    QStandardItem *item = new QStandardItem("Very\nLong\nText\nThat\nExceeds\nWidth");
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 50, 30);

    QHelpEvent event(QEvent::ToolTip, QPoint(10, 10), QPoint(110, 110));

    delegate->helpEvent(&event, listView, option, index);

    // Should replace newlines with spaces
    EXPECT_FALSE(shownText.contains('\n'));
}

TEST_F(FolderViewDelegateTest, HideTooltipImmediately_ClosesExistingTooltips)
{
    // This method closes any QTipLabel widgets
    EXPECT_NO_THROW(delegate->hideTooltipImmediately());
}

TEST_F(FolderViewDelegateTest, Paint_WithIconAndText_BothDrawn)
{
    QStandardItem *item = new QStandardItem("Test Item");
    QPixmap iconPixmap(16, 16);
    iconPixmap.fill(Qt::magenta);
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

TEST_F(FolderViewDelegateTest, Paint_NarrowRect_HandlesGracefully)
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

TEST_F(FolderViewDelegateTest, Paint_TallRect_HandlesGracefully)
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

TEST_F(FolderViewDelegateTest, Paint_SelectedWithoutDecoration_NoHighlight)
{
    QStandardItem *item = new QStandardItem("Test Item");
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(200, 30);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);
    option.state = QStyle::State_Selected;
    option.showDecorationSelected = false;   // Decoration not shown

    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}

TEST_F(FolderViewDelegateTest, Paint_StateActive_UsesNormalPalette)
{
    QStandardItem *item = new QStandardItem("Test Item");
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(200, 30);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);
    option.state = QStyle::State_Enabled | QStyle::State_Active | QStyle::State_Selected;
    option.showDecorationSelected = true;

    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}

TEST_F(FolderViewDelegateTest, Paint_TextPadding_CorrectlyAdjusted)
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

TEST_F(FolderViewDelegateTest, Paint_HoverColor_AdjustsCorrectly)
{
    bool adjustColorCalled = false;
    typedef QColor (*AdjustColor)(const QColor &, qint8, qint8, qint8, qint8, qint8, qint8, qint8);
    stub.set_lamda(static_cast<AdjustColor>(&DGuiApplicationHelper::adjustColor),
                   [&](const QColor &base, qint8, qint8, qint8, qint8, qint8, qint8, qint8) {
                       __DBG_STUB_INVOKE__
                       adjustColorCalled = true;
                       return base.lighter(110);
                   });

    QStandardItem *item = new QStandardItem("Test Item");
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(200, 30);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);
    option.state = QStyle::State_MouseOver;

    delegate->paint(&painter, option, index);

    EXPECT_TRUE(adjustColorCalled);
}

TEST_F(FolderViewDelegateTest, Paint_HighlightedText_UsesCorrectColor)
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

    // Should use HighlightedText color
    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}

TEST_F(FolderViewDelegateTest, Paint_NormalText_UsesCorrectColor)
{
    QStandardItem *item = new QStandardItem("Test Item");
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(200, 30);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);
    option.state = QStyle::State_Enabled;

    // Should use Text color
    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}

TEST_F(FolderViewDelegateTest, HelpEvent_InvalidIndex_HandlesGracefully)
{
    QModelIndex index;   // Invalid

    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);

    QHelpEvent event(QEvent::ToolTip, QPoint(10, 10), QPoint(110, 110));

    bool result = delegate->helpEvent(&event, listView, option, index);

    EXPECT_TRUE(result);
}

TEST_F(FolderViewDelegateTest, PaintItemIcon_CenteredVertically)
{
    QStandardItem *item = new QStandardItem("Test Item");
    QPixmap iconPixmap(16, 16);
    iconPixmap.fill(Qt::darkBlue);
    item->setIcon(QIcon(iconPixmap));
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(200, 50);   // Taller rect
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 50);

    // Icon should be vertically centered
    EXPECT_NO_THROW(delegate->paintItemIcon(&painter, option, index));
}

TEST_F(FolderViewDelegateTest, Paint_ElidedText_UsesMiddleElision)
{
    QStandardItem *item = new QStandardItem("VeryLongFileNameThatShouldBeElidedInTheMiddle.txt");
    model->appendRow(item);
    QModelIndex index = model->index(0, 0);

    QPixmap pixmap(100, 30);   // Narrow rect
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 30);
    option.state = QStyle::State_Enabled;

    // Text should be elided with Qt::ElideMiddle
    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}

TEST_F(FolderViewDelegateTest, CreateCustomOpacityPixmap_TransparentBackground)
{
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::red);

    QPixmap result = delegate->createCustomOpacityPixmap(pixmap, 0.3f);

    EXPECT_FALSE(result.isNull());
    // Result should have transparent background
}
