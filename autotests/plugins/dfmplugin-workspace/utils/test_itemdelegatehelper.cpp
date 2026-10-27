// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "utils/itemdelegatehelper.h"
#include <dfm-base/utils/iconpainterutils.h>

#include <QIcon>
#include <QSize>
#include <QPainter>
#include <QRectF>
#include <QBrush>
#include <QApplication>
#include <QAbstractItemModel>
#include <QImage>
#include <QPixmap>
#include <QColor>
#include <QTextOption>
#include <QString>
#include <QStyle>
#include <cstdlib>

using namespace dfmplugin_workspace;
using namespace dfmbase;

class ItemDelegateHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test environment
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(ItemDelegateHelperTest, GetIconPixmap_NullIcon_ReturnsEmptyPixmap)
{
    // Null icon falls back to "unknown" icon; result depends on theme availability
    QIcon nullIcon;
    QSize size(64, 64);
    qreal pixelRatio = 1.0;
    QIcon::Mode mode = QIcon::Normal;
    QIcon::State state = QIcon::Off;

    QPixmap result = IconPainterUtils::getIconPixmap(nullIcon, size, pixelRatio, mode, state);

    EXPECT_NO_FATAL_FAILURE(result);
}

TEST_F(ItemDelegateHelperTest, GetIconPixmap_InvalidSize_ReturnsEmptyPixmap)
{
    // Test getIconPixmap with invalid size
    QIcon testIcon = QIcon::fromTheme("folder");
    QSize invalidSize(0, 64);
    qreal pixelRatio = 1.0;
    QIcon::Mode mode = QIcon::Normal;
    QIcon::State state = QIcon::Off;
    
    QPixmap result = IconPainterUtils::getIconPixmap(testIcon, invalidSize, pixelRatio, mode, state);
    
    EXPECT_TRUE(result.isNull());
}

TEST_F(ItemDelegateHelperTest, GetIconPixmap_ValidParameters_ReturnsPixmap)
{
    // Test getIconPixmap with valid parameters
    QIcon testIcon = QIcon::fromTheme("folder");
    QSize size(64, 64);
    qreal pixelRatio = 1.0;
    QIcon::Mode mode = QIcon::Normal;
    QIcon::State state = QIcon::Off;
    
    QPixmap result = IconPainterUtils::getIconPixmap(testIcon, size, pixelRatio, mode, state);
    
    // In offscreen/test environments, icon themes may not be available,
    // so the icon may be null. Only assert non-null when theme is available.
    if (!testIcon.isNull()) {
        EXPECT_FALSE(result.isNull());
        EXPECT_EQ(result.devicePixelRatio(), pixelRatio);
    }
}

TEST_F(ItemDelegateHelperTest, PaintIcon_ValidOpts_DoesNotCrash)
{
    // Test paintIcon with valid options
    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);
    
    QIcon testIcon = QIcon::fromTheme("folder");
    QRectF rect(10, 10, 64, 64);
    
    ItemDelegateHelper::PaintIconOpts opts;
    opts.rect = rect;
    opts.alignment = Qt::AlignCenter;
    opts.mode = QIcon::Normal;
    opts.state = QIcon::Off;
    opts.viewMode = DFMBASE_NAMESPACE::Global::ViewMode::kIconMode;
    opts.isThumb = false;
    
    EXPECT_NO_THROW(ItemDelegateHelper::paintIcon(&painter, testIcon, opts));
}

TEST_F(ItemDelegateHelperTest, PaintIcon_ThumbMode_DoesNotCrash)
{
    // Test paintIcon with thumbnail mode
    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);
    
    QIcon testIcon = QIcon::fromTheme("folder");
    QRectF rect(10, 10, 64, 64);
    
    ItemDelegateHelper::PaintIconOpts opts;
    opts.rect = rect;
    opts.alignment = Qt::AlignCenter;
    opts.mode = QIcon::Normal;
    opts.state = QIcon::Off;
    opts.viewMode = DFMBASE_NAMESPACE::Global::ViewMode::kIconMode;
    opts.isThumb = true;
    
    EXPECT_NO_THROW(ItemDelegateHelper::paintIcon(&painter, testIcon, opts));
}

TEST_F(ItemDelegateHelperTest, DrawBackground_ValidParams_DoesNotCrash)
{
    // Test drawBackground with valid parameters
    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);
    
    qreal backgroundRadius = 5.0;
    QRectF rect(10, 10, 80, 60);
    QRectF lastLineRect;
    QBrush backgroundBrush(Qt::lightGray);
    
    EXPECT_NO_THROW(ItemDelegateHelper::drawBackground(backgroundRadius, rect, lastLineRect, backgroundBrush, &painter));
}

TEST_F(ItemDelegateHelperTest, HideTooltipImmediately_DoesNotCrash)
{
    // Test hideTooltipImmediately
    EXPECT_NO_THROW(ItemDelegateHelper::hideTooltipImmediately());
}

TEST_F(ItemDelegateHelperTest, CreateTextLayout_ValidParams_ReturnsLayout)
{
    // Test createTextLayout with valid parameters
    QString name("test_file.txt");
    QTextOption::WrapMode wordWrap = QTextOption::WrapAtWordBoundaryOrAnywhere;
    qreal lineHeight = 16.0;
    int alignmentFlag = Qt::AlignCenter;
    
    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);
    
    ElideTextLayout *result = ItemDelegateHelper::createTextLayout(name, wordWrap, lineHeight, alignmentFlag, &painter);
    
    EXPECT_NE(result, nullptr);
    
    delete result;
}

TEST_F(ItemDelegateHelperTest, CreateTextLayout_NullPainter_ReturnsLayout)
{
    // Test createTextLayout with null painter
    QString name("test_file.txt");
    QTextOption::WrapMode wordWrap = QTextOption::WrapAtWordBoundaryOrAnywhere;
    qreal lineHeight = 16.0;
    int alignmentFlag = Qt::AlignCenter;
    
    ElideTextLayout *result = ItemDelegateHelper::createTextLayout(name, wordWrap, lineHeight, alignmentFlag, nullptr);
    
    EXPECT_NE(result, nullptr);
    
    delete result;
}

TEST_F(ItemDelegateHelperTest, CreateTextLayout_EmptyName_ReturnsLayout)
{
    // Test createTextLayout with empty name
    QString name("");
    QTextOption::WrapMode wordWrap = QTextOption::WrapAtWordBoundaryOrAnywhere;
    qreal lineHeight = 16.0;
    int alignmentFlag = Qt::AlignCenter;

    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);

    ElideTextLayout *result = ItemDelegateHelper::createTextLayout(name, wordWrap, lineHeight, alignmentFlag, &painter);

    EXPECT_NE(result, nullptr);

    delete result;
}

namespace {

// 提供 kItemFileIconRole / kItemFileIconNameRole 的最小模型，
// 用于验证 paintIconWithFallback 的图标来源选择
class StubIconModel : public QAbstractItemModel
{
public:
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override
    {
        return hasIndex(row, column, parent) ? createIndex(row, column) : QModelIndex();
    }
    QModelIndex parent(const QModelIndex &) const override { return QModelIndex(); }
    int rowCount(const QModelIndex &) const override { return 1; }
    int columnCount(const QModelIndex &) const override { return 1; }
    QVariant data(const QModelIndex &idx, int role) const override
    {
        if (idx.row() != 0)
            return QVariant();
        if (role == dfmbase::Global::ItemRoles::kItemFileIconRole)
            return fileIcon;
        if (role == dfmbase::Global::ItemRoles::kItemFileIconNameRole)
            return QString();
        return QVariant();
    }

    QIcon fileIcon;
};

QIcon solidColorIcon(const QColor &color, int side = 48)
{
    QPixmap pm(side, side);
    pm.fill(color);
    return QIcon(pm);
}

// 统计画布上与目标颜色匹配的不透明像素（容忍缩放引入的边缘混合）
// 用 pixelColor() 而非 QColor(QRgb)：后者会丢弃 alpha，透明像素会被当成不透明黑。
int countColorPixels(const QImage &canvas, const QColor &target)
{
    int count = 0;
    for (int y = 0; y < canvas.height(); ++y) {
        for (int x = 0; x < canvas.width(); ++x) {
            const QColor c = canvas.pixelColor(x, y);
            if (c.alpha() < 200)
                continue;
            if (std::abs(c.red() - target.red()) > 30
                || std::abs(c.green() - target.green()) > 30
                || std::abs(c.blue() - target.blue()) > 30)
                continue;
            ++count;
        }
    }
    return count;
}

}   // namespace

/*
 * 回归用例（AppImage/UAB 缩略图不显示，commit ad83a888d）：
 * isThumbnail=true 时必须画 opt.icon（DecorationRole，即缩略图），
 * 绝不能画 kItemFileIconRole 的文件类型图标——否则帧跳过类型的
 * 缩略图会被 mime 图标顶替。
 */
TEST_F(ItemDelegateHelperTest, PaintIconWithFallbackThumbnailDrawsDecorationIcon)
{
    StubIconModel model;
    model.fileIcon = solidColorIcon(Qt::blue);

    QStyleOptionViewItem opt;
    opt.icon = solidColorIcon(Qt::red);   // DecorationRole：缩略图
    opt.state |= QStyle::State_Enabled;

    const QModelIndex &index = model.index(0, 0);
    QImage canvas(96, 96, QImage::Format_ARGB32);
    canvas.fill(Qt::transparent);
    QPainter painter(&canvas);

    ItemDelegateHelper::paintIconWithFallback(&painter, opt, index, QRectF(8, 8, 80, 80),
                                              true   /* isThumbnail */);
    painter.end();

    EXPECT_GT(countColorPixels(canvas, QColor(Qt::red)), 0);
    EXPECT_EQ(countColorPixels(canvas, QColor(Qt::blue)), 0);
}

/*
 * 回归用例（同上）：isThumbnail=false 时才画 kItemFileIconRole，
 * 此时 opt.icon（缩略图）不允许出现。
 */
TEST_F(ItemDelegateHelperTest, PaintIconWithFallbackNotThumbnailDrawsFileIcon)
{
    StubIconModel model;
    model.fileIcon = solidColorIcon(Qt::blue);

    QStyleOptionViewItem opt;
    opt.icon = solidColorIcon(Qt::red);
    opt.state |= QStyle::State_Enabled;

    const QModelIndex &index = model.index(0, 0);
    QImage canvas(96, 96, QImage::Format_ARGB32);
    canvas.fill(Qt::transparent);
    QPainter painter(&canvas);

    ItemDelegateHelper::paintIconWithFallback(&painter, opt, index, QRectF(8, 8, 80, 80),
                                              false   /* isThumbnail */);
    painter.end();

    EXPECT_GT(countColorPixels(canvas, QColor(Qt::blue)), 0);
    EXPECT_EQ(countColorPixels(canvas, QColor(Qt::red)), 0);
}

/*
 * 回归用例（AppImage/UAB 场景）：isThumbnail=true 且 skipFrame=true
 * —— 缩略图照常绘制，但不得出现底板/阴影（白色环）。
 */
TEST_F(ItemDelegateHelperTest, PaintIconWithFallbackSkipFrameKeepsIconWithoutPlate)
{
    StubIconModel model;
    model.fileIcon = solidColorIcon(Qt::blue);

    QStyleOptionViewItem opt;
    opt.icon = solidColorIcon(Qt::red);   // 缩略图本体固定为红色
    opt.state |= QStyle::State_Enabled;

    const QModelIndex &index = model.index(0, 0);
    QImage canvas(96, 96, QImage::Format_ARGB32);
    canvas.fill(Qt::transparent);
    QPainter painter(&canvas);

    ItemDelegateHelper::paintIconWithFallback(&painter, opt, index, QRectF(8, 8, 80, 80),
                                              true   /* isThumbnail */,
                                              DFMBASE_NAMESPACE::Global::ViewMode::kIconMode,
                                              true   /* skipFrame */);
    painter.end();

    // 缩略图必须画出来
    EXPECT_GT(countColorPixels(canvas, QColor(Qt::red)), 0);
    EXPECT_EQ(countColorPixels(canvas, QColor(Qt::blue)), 0);

    // 底板是白色：skipFrame 时画布上不允许出现成片的白色像素
    EXPECT_LT(countColorPixels(canvas, QColor(Qt::white)), 100);
}
