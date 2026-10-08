// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "delegate/collectionitemdelegate.h"
#include "delegate/collectionitemdelegate_p.h"
#include "delegate/itemeditor.h"
#include "view/collectionview.h"
#include "models/collectionmodel.h"
#include "mode/custom/customdatahandler.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/iconpainterutils.h>
#include <dfm-base/utils/elidetextlayout.h>

#include <QPainter>
#include <QStyleOptionViewItem>
#include <QApplication>
#include <QScrollBar>
#include <QStandardItemModel>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
using namespace ddplugin_organizer;

class UT_CollectionItemDelegate : public testing::Test
{
protected:
    void SetUp() override
    {
        provider = new CustomDataHandler();
        view = new CollectionView("test_uuid", provider);
        delegate = new CollectionItemDelegate(view);
    }

    void TearDown() override
    {
        delete delegate;
        delegate = nullptr;
        delete view;
        view = nullptr;
        delete provider;
        provider = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    CollectionItemDelegate *delegate = nullptr;
    CollectionView *view = nullptr;
    CollectionDataProvider *provider = nullptr;
};

TEST_F(UT_CollectionItemDelegate, MinimumIconLevel_ReturnsZero)
{
    EXPECT_EQ(CollectionItemDelegate::minimumIconLevel(), 0);
}

TEST_F(UT_CollectionItemDelegate, MaximumIconLevel_ReturnsMaxIndex)
{
    EXPECT_EQ(CollectionItemDelegate::maximumIconLevel(), 4);
}

TEST_F(UT_CollectionItemDelegate, IconSize_ValidLevel_ReturnsCorrectSize)
{
    EXPECT_EQ(delegate->iconSize(0), QSize(32, 32));
    EXPECT_EQ(delegate->iconSize(1), QSize(48, 48));
    EXPECT_EQ(delegate->iconSize(2), QSize(64, 64));
    EXPECT_EQ(delegate->iconSize(3), QSize(96, 96));
    EXPECT_EQ(delegate->iconSize(4), QSize(128, 128));
}

TEST_F(UT_CollectionItemDelegate, IconSize_InvalidLevel_ReturnsEmptySize)
{
    EXPECT_EQ(delegate->iconSize(-1), QSize());
    EXPECT_EQ(delegate->iconSize(5), QSize());
}

TEST_F(UT_CollectionItemDelegate, IconLevel_DefaultLevel_ReturnsOne)
{
    EXPECT_EQ(delegate->iconLevel(), 1);
}

TEST_F(UT_CollectionItemDelegate, SetIconLevel_SameLevel_ReturnsCurrentLevel)
{
    int currentLevel = delegate->iconLevel();
    EXPECT_EQ(delegate->setIconLevel(currentLevel), currentLevel);
}

TEST_F(UT_CollectionItemDelegate, SetIconLevel_ValidLevel_ReturnsNewLevel)
{
    int result = delegate->setIconLevel(2);
    EXPECT_EQ(result, 2);
    EXPECT_EQ(delegate->iconLevel(), 2);
}

TEST_F(UT_CollectionItemDelegate, SetIconLevel_InvalidLevel_ReturnsNegativeOne)
{
    EXPECT_EQ(delegate->setIconLevel(-1), -1);
    EXPECT_EQ(delegate->setIconLevel(10), -1);
}

TEST_F(UT_CollectionItemDelegate, IconSizeLevelDescription_ValidIndex_ReturnsDescription)
{
    EXPECT_FALSE(delegate->iconSizeLevelDescription(0).isEmpty());
    EXPECT_FALSE(delegate->iconSizeLevelDescription(1).isEmpty());
    EXPECT_FALSE(delegate->iconSizeLevelDescription(4).isEmpty());
}

TEST_F(UT_CollectionItemDelegate, IconSizeLevelDescription_InvalidIndex_ReturnsEmpty)
{
    EXPECT_TRUE(delegate->iconSizeLevelDescription(-1).isEmpty());
    EXPECT_TRUE(delegate->iconSizeLevelDescription(5).isEmpty());
}

TEST_F(UT_CollectionItemDelegate, SizeHint_ReturnsCachedHint)
{
    QStyleOptionViewItem option;
    QModelIndex index;
    QSize hint = delegate->sizeHint(option, index);
    EXPECT_FALSE(hint.isValid());
}

TEST_F(UT_CollectionItemDelegate, BoundingRect_EmptyList_ReturnsEmptyRect)
{
    QList<QRectF> rects;
    QRectF result = CollectionItemDelegate::boundingRect(rects);
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_CollectionItemDelegate, BoundingRect_SingleRect_ReturnsSameRect)
{
    QList<QRectF> rects;
    rects << QRectF(10, 20, 100, 50);
    QRectF result = CollectionItemDelegate::boundingRect(rects);
    EXPECT_EQ(result, rects.first());
}

TEST_F(UT_CollectionItemDelegate, BoundingRect_MultipleRects_ReturnsBoundingRect)
{
    QList<QRectF> rects;
    rects << QRectF(10, 20, 50, 50);
    rects << QRectF(100, 100, 50, 50);
    rects << QRectF(5, 150, 200, 30);

    QRectF result = CollectionItemDelegate::boundingRect(rects);

    EXPECT_EQ(result.left(), 5);
    EXPECT_EQ(result.top(), 20);
    EXPECT_EQ(result.right(), 205);
    EXPECT_EQ(result.bottom(), 180);
}

TEST_F(UT_CollectionItemDelegate, IconRect_ReturnsCorrectPosition)
{
    QRect paintRect(0, 0, 100, 150);
    QRect iconR = delegate->iconRect(paintRect);

    EXPECT_EQ(iconR.top(), CollectionItemDelegate::kIconTopSpacing);
    EXPECT_TRUE(iconR.width() > 0);
    EXPECT_TRUE(iconR.height() > 0);
}

TEST_F(UT_CollectionItemDelegate, LabelRect_ReturnsRectBelowIcon)
{
    QRect paintRect(0, 0, 100, 150);
    QRect usedRect(26, 4, 48, 48);

    QRect labelR = CollectionItemDelegate::labelRect(paintRect, usedRect);

    EXPECT_EQ(labelR.top(), usedRect.bottom() + CollectionItemDelegate::kIconBackgroundMargin);
    EXPECT_EQ(labelR.width(), paintRect.width() - 2 * CollectionItemDelegate::kTextPadding);
}

TEST_F(UT_CollectionItemDelegate, MayExpand_NoSelection_ReturnsFalse)
{
    stub.set_lamda(VADDR(CollectionView, selectedIndexes), [] {
        __DBG_STUB_INVOKE__
        return QModelIndexList();
    });

    EXPECT_FALSE(delegate->mayExpand());
}

TEST_F(UT_CollectionItemDelegate, MayExpand_MultipleSelection_ReturnsFalse)
{
    stub.set_lamda(VADDR(CollectionView, selectedIndexes), [] {
        __DBG_STUB_INVOKE__
        QModelIndexList list;
        list << QModelIndex() << QModelIndex();
        return list;
    });

    EXPECT_FALSE(delegate->mayExpand());
}

TEST_F(UT_CollectionItemDelegate, MayExpand_SingleSelection_ReturnsTrue)
{
    stub.set_lamda(VADDR(CollectionView, selectedIndexes), [] {
        __DBG_STUB_INVOKE__
        QModelIndexList list;
        list << QModelIndex();
        return list;
    });

    QModelIndex who;
    EXPECT_TRUE(delegate->mayExpand(&who));
}

TEST_F(UT_CollectionItemDelegate, VisualAlignment_LeftToRight_KeepsAlignment)
{
    Qt::Alignment align = Qt::AlignLeft | Qt::AlignTop;
    Qt::Alignment result = IconPainterUtils::visualAlignment(Qt::LeftToRight, align);
    EXPECT_TRUE(result & Qt::AlignLeft);
}

TEST_F(UT_CollectionItemDelegate, VisualAlignment_RightToLeft_SwapsLeftRight)
{
    Qt::Alignment align = Qt::AlignLeft | Qt::AlignTop;
    Qt::Alignment result = IconPainterUtils::visualAlignment(Qt::RightToLeft, align);
    EXPECT_TRUE(result & Qt::AlignRight);
}

TEST_F(UT_CollectionItemDelegate, VisualAlignment_NoHorizontal_AddsLeft)
{
    Qt::Alignment align = Qt::AlignTop;
    Qt::Alignment result = IconPainterUtils::visualAlignment(Qt::LeftToRight, align);
    EXPECT_TRUE(result & Qt::AlignLeft);
}

TEST_F(UT_CollectionItemDelegate, GetIconPixmap_NullIcon_ReturnsEmptyPixmap)
{
    QIcon nullIcon;
    QPixmap result = IconPainterUtils::getIconPixmap(nullIcon, QSize(48, 48), 1.0);
    EXPECT_TRUE(result.isNull());
}

TEST_F(UT_CollectionItemDelegate, GetIconPixmap_ZeroSize_ReturnsEmptyPixmap)
{
    QIcon icon = QIcon::fromTheme("folder");
    QPixmap result = IconPainterUtils::getIconPixmap(icon, QSize(0, 0), 1.0);
    EXPECT_TRUE(result.isNull());
}

TEST_F(UT_CollectionItemDelegate, GetIconPixmap_NegativeSize_ReturnsEmptyPixmap)
{
    QIcon icon = QIcon::fromTheme("folder");
    QPixmap result = IconPainterUtils::getIconPixmap(icon, QSize(-10, 48), 1.0);
    EXPECT_TRUE(result.isNull());
}

TEST_F(UT_CollectionItemDelegate, CreateEditor_ReturnsItemEditor)
{
    stub.set_lamda(&FileUtils::supportLongName, [] {
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(ADDR(CollectionItemDelegate, isTransparent), [] {
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(&CollectionModel::rootUrl, [] {
        __DBG_STUB_INVOKE__
        return QUrl::fromLocalFile("/home");
    });

    QWidget parent;
    QStyleOptionViewItem option;
    QModelIndex index;

    QWidget *editor = delegate->createEditor(&parent, option, index);

    EXPECT_NE(editor, nullptr);
    EXPECT_NE(qobject_cast<ItemEditor *>(editor), nullptr);

    delete editor;
}

TEST_F(UT_CollectionItemDelegate, IsTransparent_NoCutAction_ReturnsFalse)
{
    stub.set_lamda(ADDR(ClipBoard, clipboardAction), [] {
        __DBG_STUB_INVOKE__
        return ClipBoard::kCopyAction;
    });

    QModelIndex index;
    EXPECT_FALSE(delegate->isTransparent(index));
}

TEST_F(UT_CollectionItemDelegate, IsTransparent_CutActionNoFile_ReturnsFalse)
{
    stub.set_lamda(ADDR(ClipBoard, clipboardAction), [] {
        __DBG_STUB_INVOKE__
        return ClipBoard::kCutAction;
    });
    stub.set_lamda(ADDR(CollectionModel, fileInfo), [] {
        __DBG_STUB_INVOKE__
        return FileInfoPointer(nullptr);
    });

    QModelIndex index;
    EXPECT_FALSE(delegate->isTransparent(index));
}

TEST_F(UT_CollectionItemDelegate, IsThumbnailIcon_NullInfo_ReturnsFalse)
{
    FileInfoPointer nullInfo(nullptr);
    EXPECT_FALSE(IconPainterUtils::isThumbnailIcon(nullInfo));
}

TEST_F(UT_CollectionItemDelegate, UpdateItemSizeHint_UpdatesSizeHint)
{
    QSize oldHint = delegate->sizeHint(QStyleOptionViewItem(), QModelIndex());

    delegate->setIconLevel(3);
    delegate->updateItemSizeHint();

    QSize newHint = delegate->sizeHint(QStyleOptionViewItem(), QModelIndex());
    EXPECT_NE(oldHint, newHint);
}

TEST_F(UT_CollectionItemDelegate, CommitDataAndCloseEditor_NoOpenEditor_DoesNotCrash)
{
    stub.set_lamda(VADDR(CollectionView, currentIndex), [] {
        __DBG_STUB_INVOKE__
        return QModelIndex();
    });
    stub.set_lamda(ADDR(QAbstractItemView, isPersistentEditorOpen), [] {
        __DBG_STUB_INVOKE__
        return false;
    });

    EXPECT_NO_THROW(delegate->commitDataAndCloseEditor());
}

TEST_F(UT_CollectionItemDelegate, RevertAndCloseEditor_NoOpenEditor_DoesNotCrash)
{
    stub.set_lamda(VADDR(CollectionView, currentIndex), [] {
        __DBG_STUB_INVOKE__
        return QModelIndex();
    });
    stub.set_lamda(ADDR(QAbstractItemView, isPersistentEditorOpen), [] {
        __DBG_STUB_INVOKE__
        return false;
    });

    EXPECT_NO_THROW(delegate->revertAndcloseEditor());
}

TEST_F(UT_CollectionItemDelegate, RevertAndCloseEditor_HasOpenEditor_ClosesEditor)
{
    bool closeCalled = false;
    stub.set_lamda(VADDR(CollectionView, currentIndex), [] {
        __DBG_STUB_INVOKE__
        return QModelIndex();
    });
    stub.set_lamda(ADDR(QAbstractItemView, isPersistentEditorOpen), [] {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(ADDR(QAbstractItemView, closePersistentEditor), [&closeCalled] {
        __DBG_STUB_INVOKE__
        closeCalled = true;
    });

    delegate->revertAndcloseEditor();
    EXPECT_TRUE(closeCalled);
}

TEST_F(UT_CollectionItemDelegate, ClipboardDataChanged_NoOpenEditor_UpdatesView)
{
    bool updateCalled = false;
    stub.set_lamda(VADDR(CollectionView, currentIndex), [] {
        __DBG_STUB_INVOKE__
        return QModelIndex();
    });
    stub.set_lamda(ADDR(QAbstractItemView, isPersistentEditorOpen), [] {
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(qOverload<>(&QWidget::update), [&updateCalled] {
        __DBG_STUB_INVOKE__
        updateCalled = true;
    });

    delegate->clipboardDataChanged();
    EXPECT_TRUE(updateCalled);
    stub.clear();
}

class UT_CollectionItemDelegatePrivate : public testing::Test
{
protected:
    void SetUp() override
    {
        provider = new CustomDataHandler();
        view = new CollectionView("test_uuid", provider);
        delegate = new CollectionItemDelegate(view);

        // mock the UI show
        stub.set_lamda(&QWidget::show, [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
        stub.set_lamda(&QWidget::hide, [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
    }

    void TearDown() override
    {
        delete delegate;
        delegate = nullptr;
        delete view;
        view = nullptr;
        delete provider;
        provider = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    CollectionItemDelegate *delegate = nullptr;
    CollectionView *view = nullptr;
    CollectionDataProvider *provider = nullptr;
};

TEST_F(UT_CollectionItemDelegatePrivate, AvailableTextRect_ModifiesTop)
{
    QRect labelRect(10, 50, 80, 100);
    QRect result = delegate->d->availableTextRect(labelRect);

    int expectedTop = labelRect.top() + CollectionItemDelegate::kIconSpacing
            + CollectionItemDelegate::kTextPadding;
    EXPECT_EQ(result.top(), expectedTop);
    EXPECT_EQ(result.left(), labelRect.left());
    EXPECT_EQ(result.width(), labelRect.width());
}

TEST_F(UT_CollectionItemDelegatePrivate, IsHighlight_SelectedWithDecoration_ReturnsTrue)
{
    QStyleOptionViewItem option;
    option.state = QStyle::State_Selected;
    option.showDecorationSelected = true;

    EXPECT_TRUE(delegate->d->isHighlight(option));
}

TEST_F(UT_CollectionItemDelegatePrivate, IsHighlight_NotSelected_ReturnsFalse)
{
    QStyleOptionViewItem option;
    option.state = QStyle::State_None;
    option.showDecorationSelected = true;

    EXPECT_FALSE(delegate->d->isHighlight(option));
}

TEST_F(UT_CollectionItemDelegatePrivate, IsHighlight_NoDecorationSelected_ReturnsFalse)
{
    QStyleOptionViewItem option;
    option.state = QStyle::State_Selected;
    option.showDecorationSelected = false;

    EXPECT_FALSE(delegate->d->isHighlight(option));
}

TEST_F(UT_CollectionItemDelegatePrivate, CreateTextlayout_ReturnsValidLayout)
{
    stub.set_lamda(ADDR(Application, instance), [] {
        __DBG_STUB_INVOKE__
        static Application app;
        return &app;
    });
    stub.set_lamda(&Application::genericAttribute, [] {
        __DBG_STUB_INVOKE__
        return QVariant(true);
    });

    QStandardItemModel model;
    model.appendRow(new QStandardItem("test"));
    QModelIndex index = model.index(0, 0);

    auto layout = delegate->d->createTextlayout(index, nullptr);
    EXPECT_NE(layout, nullptr);
    delete layout;
}

TEST_F(UT_CollectionItemDelegatePrivate, NeedExpend_SmallText_ReturnsFalse)
{
    stub.set_lamda(ADDR(Application, instance), [] {
        __DBG_STUB_INVOKE__
        static Application app;
        return &app;
    });
    stub.set_lamda(&Application::genericAttribute, [] {
        __DBG_STUB_INVOKE__
        return QVariant(true);
    });
    stub.set_lamda(ADDR(CollectionItemDelegate, textPaintRect), [] {
        __DBG_STUB_INVOKE__
        return QRect(0, 0, 80, 20);
    });

    QStyleOptionViewItem option;
    QStandardItemModel model;
    model.appendRow(new QStandardItem("short"));
    QModelIndex index = model.index(0, 0);
    QRect textRect(0, 0, 80, 40);

    bool result = delegate->d->needExpend(option, index, textRect);
    EXPECT_FALSE(result);
}

TEST_F(UT_CollectionItemDelegatePrivate, NeedExpend_LongText_ReturnsTrue)
{
    stub.set_lamda(ADDR(Application, instance), [] {
        __DBG_STUB_INVOKE__
        static Application app;
        return &app;
    });
    stub.set_lamda(&Application::genericAttribute, [] {
        __DBG_STUB_INVOKE__
        return QVariant(true);
    });
    stub.set_lamda(ADDR(CollectionItemDelegate, textPaintRect), [] {
        __DBG_STUB_INVOKE__
        return QRect(0, 0, 80, 100);
    });

    QStyleOptionViewItem option;
    QStandardItemModel model;
    model.appendRow(new QStandardItem("very long text that needs expansion"));
    QModelIndex index = model.index(0, 0);
    QRect textRect(0, 0, 80, 40);

    bool result = delegate->d->needExpend(option, index, textRect);
    EXPECT_TRUE(result);
}

TEST_F(UT_CollectionItemDelegatePrivate, CurrentIconLevel_InitialValue_IsValid)
{
    EXPECT_GE(delegate->d->currentIconLevel, 0);
    EXPECT_LE(delegate->d->currentIconLevel, CollectionItemDelegate::maximumIconLevel());
}

TEST_F(UT_CollectionItemDelegatePrivate, IconLevelDescriptions_HasCorrectCount)
{
    EXPECT_EQ(delegate->d->iconLevelDescriptions.size(),
              CollectionItemDelegatePrivate::kIconSizes.size());
}

// ---------------------------------------------------------------------------
// PMS sev-2 regression tests
// ---------------------------------------------------------------------------
class CollectionItemDelegateTest : public testing::Test
{
protected:
    void SetUp() override
    {
        provider = new CustomDataHandler();
    }

    void TearDown() override
    {
        delete provider;
        provider = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    CollectionDataProvider *provider = nullptr;
};

// PMS:215793 CollectionItemDelegate 析构未释放私有数据：析构函数中补上 delete d，
// 避免桌面 CollectionView 反复创建/销毁时泄漏 CollectionItemDelegatePrivate。
// （该 bug 的操作分组策略部分在 dfmplugin-fileoperations 分片，不在此目录验证。）
TEST_F(CollectionItemDelegateTest, BUG215793_DelegateLifecycle_DestroyReleasesPrivateNoCrash)
{
    for (int i = 0; i < 25; ++i) {
        CollectionView *v = new CollectionView(QString("bug215793_%1").arg(i), provider);
        CollectionItemDelegate *del = new CollectionItemDelegate(v);
        EXPECT_NE(del->d, nullptr);
        delete del;   // 修复前：此处泄漏 CollectionItemDelegatePrivate
        delete v;
    }

    // 重复销毁后堆状态完好，新实例仍可用
    CollectionView *v = new CollectionView("bug215793_final", provider);
    CollectionItemDelegate *del = new CollectionItemDelegate(v);
    EXPECT_NE(del->d, nullptr);
    delete del;
    delete v;
    SUCCEED();
}

// ---------------------------------------------------------------------------
// PMS:302075 图标缩放与性能修复回归（CollectionView/CollectionItemDelegate 右键
// 菜单卡顿）：修复后图标绘制统一走 IconPainterUtils，QIcon::pixmap 请求
// size*dpr 并显式 setDevicePixelRatio，保证高分屏下图标逻辑尺寸正确且等比缩放。
// ---------------------------------------------------------------------------
class IconPainterUtilsTest : public testing::Test
{
protected:
    void SetUp() override { }
    void TearDown() override { stub.clear(); }
    stub_ext::StubExt stub;
};

// 96x96 源图标按逻辑尺寸 48 请求：修复后 px 应为 96 物理像素、dpr=2.0，
// 修复前 icon.pixmap(48) 返回被缩小到 48 的位图再标 dpr=2，逻辑尺寸错误。
TEST_F(IconPainterUtilsTest, BUG302075_GetIconPixmap_MultipliesSizeByDevicePixelRatio)
{
    QPixmap src(96, 96);
    src.fill(Qt::red);
    QIcon icon(src);

    const QPixmap px = IconPainterUtils::getIconPixmap(icon, QSize(48, 48), 2.0);

    EXPECT_FALSE(px.isNull());
    EXPECT_EQ(px.width(), 96);                       // 物理 96 = 48 * 2
    EXPECT_EQ(px.height(), 96);
    EXPECT_NEAR(px.devicePixelRatio(), 2.0, 0.001);   // 逻辑 48x48
}

// 64x64 图标绘制进 50x100 区域：等比缩放到 50x50 且垂直居中，
// 修复前直接 drawPixmap/QIcon::paint 会拉伸变形（x 方向 50、y 方向 100）。
TEST_F(IconPainterUtilsTest, BUG302075_PaintIcon_PreservesAspectRatioAndCentersInRect)
{
    QPixmap src(64, 64);
    src.fill(Qt::blue);
    QIcon icon(src);

    QPixmap device(50, 100);
    device.fill(Qt::white);
    QPainter painter(&device);

    IconPainterUtils::PaintIconOpts opts;
    opts.rect = QRectF(0, 0, 50, 100);
    opts.alignment = Qt::AlignCenter;
    opts.mode = QIcon::Normal;
    opts.state = QIcon::Off;
    opts.isThumb = false;
    opts.viewMode = Global::ViewMode::kNoneMode;

    const std::optional<QRect> painted = IconPainterUtils::paintIcon(&painter, icon, opts);
    painter.end();

    ASSERT_TRUE(painted.has_value());
    // 64x64 等比缩到 50x50 后在 50x100 区域内垂直居中
    EXPECT_EQ(*painted, QRect(0, 25, 50, 50));
}
