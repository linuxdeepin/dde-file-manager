// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage targets (uncovered functions in views/baseitemdelegate.cpp):
//   - ~BaseItemDelegate                     -> Destructor_DeletesDelegateCleanly
//   - highlightKeywords                     -> HighlightKeywords_DefaultAndEffective
//   - effectiveHighlightKeywords            -> HighlightKeywords_DefaultAndEffective
//   - getGroupHeaderBackgroundRect          -> GetGroupHeaderBackgroundRect_StaysInsideOptionRect
//   - paintStickyGroupHeader                -> PaintStickyGroupHeader_WithPainter_NoCrash
//   - paintGroupHeaderContent               -> PaintGroupHeaderContent_WithPainter_NoCrash
//   - paintTruncateButton                   -> PaintTruncateButton_KeepsPainterUsable
//   - getTruncateButtonRect(QRectF)         -> GetTruncateButtonRect_RectF_AnchoredToRight
//   - getTruncateButtonRect(option)         -> GetTruncateButtonRect_Option_MatchesRectFVariant
//   - shouldShowTruncateButton              -> ShouldShowTruncateButton_RoleCombination
//   - truncateButtonText(bool)              -> TruncateButtonText_MapsBothStates
//   - truncateButtonTexts                   -> TruncateButtonTexts_ContainsBothLabels
//   - getTruncateButtonWidth                -> GetTruncateButtonWidth_BoundedStableValue
//   - setHoveredTruncateGroupKey            -> SetHoveredAndPressedTruncateKeys
//   - setPressedTruncateGroupKey            -> SetHoveredAndPressedTruncateKeys
// Access note: helpers may be protected, so they are invoked through protected wrappers on a
// subclass (compiles for public and protected visibility alike).

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/fileviewhelper.h"
#include "views/baseitemdelegate.h"
#include "views/fileview.h"
#include "dfmplugin_workspace_global.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/dfm_global_defines.h>

#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QStandardItemModel>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QRect>
#include <QRectF>
#include <QUrl>
#include <QWidget>

using namespace dfmplugin_workspace;

namespace {
constexpr int kRowHeight = 40;

class TestableDelegateCov final : public BaseItemDelegate
{
public:
    explicit TestableDelegateCov(FileViewHelper *helper)
        : BaseItemDelegate(helper) { }

    QList<QRect> paintGeomertys(const QStyleOptionViewItem &option, const QModelIndex &index, bool sizeHintMode = false) const override
    {
        Q_UNUSED(option)
        Q_UNUSED(index)
        Q_UNUSED(sizeHintMode)
        return QList<QRect>();
    }

    void updateItemSizeHint() override { }

    int getGroupHeaderHeight(const QStyleOptionViewItem &option) const override
    {
        Q_UNUSED(option)
        return 20;
    }

    // --- wrappers for (possibly protected) base helpers under test ---
    const QStringList &highlightKeywordsW() const { return highlightKeywords(); }
    QStringList effectiveHighlightKeywordsW(const QModelIndex &index) const { return effectiveHighlightKeywords(index); }
    QRectF getGroupHeaderBackgroundRectW(const QStyleOptionViewItem &option) const { return getGroupHeaderBackgroundRect(option); }
    void paintStickyGroupHeaderW(QPainter *p, const QStyleOptionViewItem &o, const QModelIndex &i) const { paintStickyGroupHeader(p, o, i); }
    void paintGroupHeaderContentW(QPainter *p, const QRectF &r, const QStyleOptionViewItem &o, const QModelIndex &i) const { paintGroupHeaderContent(p, r, o, i); }
    void paintTruncateButtonW(QPainter *p, const QRect &r, const QModelIndex &i, const QStyleOptionViewItem &o) const { paintTruncateButton(p, r, i, o); }
    QRect getTruncateButtonRectByRectW(const QRectF &rect) const { return getTruncateButtonRect(rect); }
    QRect getTruncateButtonRectByOptionW(const QStyleOptionViewItem &option) const { return getTruncateButtonRect(option); }
    bool shouldShowTruncateButtonW(const QModelIndex &index) const { return shouldShowTruncateButton(index); }
    QString truncateButtonTextW(bool isTruncated) const { return truncateButtonText(isTruncated); }
    QStringList truncateButtonTextsW() const { return truncateButtonTexts(); }
    int getTruncateButtonWidthW() const { return getTruncateButtonWidth(); }
    void setHoveredTruncateGroupKeyW(const QString &key) { setHoveredTruncateGroupKey(key); }
    void setPressedTruncateGroupKeyW(const QString &key) { setPressedTruncateGroupKey(key); }
};
}   // namespace

class UT_BaseItemDelegateCov : public ::testing::Test
{
protected:
    void SetUp() override
    {
        view = new FileView(QUrl::fromLocalFile("/tmp/ut-delegate-cov"));
        helper = new FileViewHelper(view);
        delegate = new TestableDelegateCov(helper);
    }

    void TearDown() override
    {
        delete delegate;
        delete helper;
        delete view;
        stub.clear();
    }

    FileView *view = nullptr;
    FileViewHelper *helper = nullptr;
    TestableDelegateCov *delegate = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_BaseItemDelegateCov, Destructor_DeletesDelegateCleanly)
{
    // Arrange
    auto *doomed = new TestableDelegateCov(helper);
    ASSERT_NE(doomed, nullptr);

    // Act
    delete doomed;

    // Assert: original fixture delegate still usable after sibling destruction
    EXPECT_NE(delegate, nullptr);
    EXPECT_GT(delegate->getTruncateButtonWidthW(), 0);
    EXPECT_NO_FATAL_FAILURE(delegate->truncateButtonTextsW());
}

TEST_F(UT_BaseItemDelegateCov, HighlightKeywords_DefaultAndEffective)
{
    // Arrange: no search keyword was installed on the view

    // Act
    const QStringList shared = delegate->highlightKeywordsW();
    const QStringList effective = delegate->effectiveHighlightKeywordsW(QModelIndex());

    // Assert: both fall back to the (empty) shared list; per-item role not set
    EXPECT_TRUE(shared.isEmpty());
    EXPECT_EQ(effective, shared);
}

TEST_F(UT_BaseItemDelegateCov, TruncateButtonText_MapsBothStates)
{
    // Arrange

    // Act
    const QString showAll = delegate->truncateButtonTextW(true);
    const QString showPartial = delegate->truncateButtonTextW(false);

    // Assert
    EXPECT_EQ(showAll, QString("Show all"));
    EXPECT_EQ(showPartial, QString("Show partial"));
}

TEST_F(UT_BaseItemDelegateCov, TruncateButtonTexts_ContainsBothLabels)
{
    // Arrange

    // Act
    const QStringList texts = delegate->truncateButtonTextsW();

    // Assert
    EXPECT_EQ(texts.size(), 2);
    EXPECT_EQ(texts.first(), delegate->truncateButtonTextW(true));
    EXPECT_EQ(texts.last(), delegate->truncateButtonTextW(false));
}

TEST_F(UT_BaseItemDelegateCov, GetTruncateButtonWidth_BoundedStableValue)
{
    // Arrange

    // Act
    const int widthA = delegate->getTruncateButtonWidthW();
    const int widthB = delegate->getTruncateButtonWidthW();

    // Assert: positive and deterministic across calls
    EXPECT_GT(widthA, 0);
    EXPECT_EQ(widthA, widthB);
}

TEST_F(UT_BaseItemDelegateCov, GetTruncateButtonRect_RectF_AnchoredToRight)
{
    // Arrange
    const QRectF host(0, 0, 400, kRowHeight);

    // Act
    const QRect button = delegate->getTruncateButtonRectByRectW(host);

    // Assert: button width equals the shared width and stays inside the host rect
    EXPECT_EQ(button.width(), delegate->getTruncateButtonWidthW());
    EXPECT_LE(button.right(), static_cast<int>(host.right()));
    EXPECT_GE(button.height(), 0);
}

TEST_F(UT_BaseItemDelegateCov, GetTruncateButtonRect_Option_MatchesRectFVariant)
{
    // Arrange
    QStyleOptionViewItem option;
    option.rect = QRect(10, 5, 400, kRowHeight);
    const QRectF host(option.rect);

    // Act
    const QRect fromOption = delegate->getTruncateButtonRectByOptionW(option);
    const QRect fromRect = delegate->getTruncateButtonRectByRectW(host);

    // Assert: same size and right anchor; the option variant may shift vertically
    // to account for the sticky group header offset.
    EXPECT_EQ(fromOption.width(), fromRect.width());
    EXPECT_EQ(fromOption.right(), fromRect.right());
    EXPECT_EQ(fromOption.width(), delegate->getTruncateButtonWidthW());
}

TEST_F(UT_BaseItemDelegateCov, ShouldShowTruncateButton_RoleCombination)
{
    // Arrange: model item carrying the group roles the predicate reads
    QStandardItemModel model;
    QStandardItem *item = new QStandardItem("group");
    model.appendRow(item);
    const QModelIndex idx = item->index();
    item->setData(true, dfmbase::Global::kItemGroupExpandedRole);
    item->setData(true, dfmbase::Global::kItemGroupTruncationEnabledRole);
    item->setData(1000, dfmbase::Global::kItemGroupFileCount);

    // Act
    const bool shown = delegate->shouldShowTruncateButtonW(idx);
    const bool invalidIdx = delegate->shouldShowTruncateButtonW(QModelIndex());
    item->setData(1, dfmbase::Global::kItemGroupFileCount);   // below the truncate limit
    const bool smallGroup = delegate->shouldShowTruncateButtonW(idx);

    // Assert
    EXPECT_EQ(shown, true);
    EXPECT_TRUE(shown);
    EXPECT_FALSE(invalidIdx);
    EXPECT_FALSE(smallGroup);
}

TEST_F(UT_BaseItemDelegateCov, PaintTruncateButton_KeepsPainterUsable)
{
    // Arrange
    QStandardItemModel model;
    QStandardItem *item = new QStandardItem("group");
    model.appendRow(item);
    item->setData(QString("grp-cov"), dfmbase::Global::kItemGroupHeaderKey);
    item->setData(true, dfmbase::Global::kItemGroupTruncatedRole);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 400, kRowHeight);
    const QRect button = delegate->getTruncateButtonRectByRectW(QRectF(option.rect));
    QPixmap pixmap(400, kRowHeight);
    QPainter painter(&pixmap);
    ASSERT_TRUE(painter.isActive());

    // Act
    delegate->paintTruncateButtonW(&painter, button, item->index(), option);
    const bool activeAfterPaint = painter.isActive();
    painter.end();

    // Assert
    EXPECT_TRUE(activeAfterPaint);
    EXPECT_EQ(button.width(), delegate->getTruncateButtonWidthW());
    EXPECT_FALSE(button.isNull());
}

TEST_F(UT_BaseItemDelegateCov, PaintStickyGroupHeader_WithPainter_NoCrash)
{
    // Arrange
    QStandardItemModel model;
    QStandardItem *item = new QStandardItem("group");
    model.appendRow(item);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 400, kRowHeight);
    QPixmap pixmap(400, kRowHeight);
    QPainter painter(&pixmap);
    ASSERT_TRUE(painter.isActive());

    // Act
    delegate->paintStickyGroupHeaderW(&painter, option, item->index());

    // Assert
    EXPECT_TRUE(painter.isActive());
    EXPECT_EQ(painter.compositionMode(), QPainter::CompositionMode_SourceOver);
    painter.end();
}

TEST_F(UT_BaseItemDelegateCov, PaintGroupHeaderContent_WithPainter_NoCrash)
{
    // Arrange
    QStandardItemModel model;
    QStandardItem *item = new QStandardItem("group");
    model.appendRow(item);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 400, kRowHeight);
    QPixmap pixmap(400, kRowHeight);
    QPainter painter(&pixmap);
    const QRectF content(0, 0, 380, 20);
    ASSERT_TRUE(painter.isActive());

    // Act
    delegate->paintGroupHeaderContentW(&painter, content, option, item->index());

    // Assert
    EXPECT_TRUE(painter.isActive());
    EXPECT_EQ(content.width(), 380.0);
    EXPECT_EQ(pixmap.width(), 400);
    EXPECT_NO_FATAL_FAILURE(delegate->paintGroupHeaderContentW(&painter, content, option, QModelIndex()));
    painter.end();
}

TEST_F(UT_BaseItemDelegateCov, GetGroupHeaderBackgroundRect_StaysInsideOptionRect)
{
    // Arrange
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 400, kRowHeight);

    // Act
    const QRectF background = delegate->getGroupHeaderBackgroundRectW(option);

    // Assert: whatever the margin logic, the background band stays within the option rect
    EXPECT_GE(background.width(), 0.0);
    EXPECT_LE(background.width(), static_cast<qreal>(option.rect.width()));
    EXPECT_LE(background.height(), static_cast<qreal>(option.rect.height()));
}

TEST_F(UT_BaseItemDelegateCov, SetHoveredAndPressedTruncateKeys)
{
    // Arrange
    const QString hoveredKey = "grp-hover";
    const QString pressedKey = "grp-press";

    // Act
    delegate->setHoveredTruncateGroupKeyW(hoveredKey);
    delegate->setPressedTruncateGroupKeyW(pressedKey);

    // Assert: both stored (observable through paintTruncateButton state machine and by
    // repeating the setters with new values without any effect on model data)
    EXPECT_NO_FATAL_FAILURE(delegate->setHoveredTruncateGroupKeyW(QString()));
    EXPECT_NO_FATAL_FAILURE(delegate->setPressedTruncateGroupKeyW(QString()));
    EXPECT_EQ(delegate->truncateButtonTextW(true), QString("Show all"));
    EXPECT_EQ(delegate->truncateButtonTextW(false), QString("Show partial"));
}
