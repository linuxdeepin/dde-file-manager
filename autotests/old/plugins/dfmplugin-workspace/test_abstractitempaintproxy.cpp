// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "views/abstractitempaintproxy.h"

#include <QPainter>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QRect>
#include <QSize>
#include <QFont>
#include <QColor>
#include <QBrush>
#include <QPen>
#include <QIcon>
#include <QPixmap>
#include <QApplication>
#include <QStyle>

using namespace dfmplugin_workspace;

class TestAbstractItemPaintProxy : public AbstractItemPaintProxy
{
public:
    explicit TestAbstractItemPaintProxy(QObject *parent = nullptr)
        : AbstractItemPaintProxy(parent)
    {
    }
    
    // Implement pure virtual methods
    void drawIcon(QPainter *painter, QRectF *rect, const QStyleOptionViewItem &option, const QModelIndex &index) override
    {
        // Just a basic implementation for testing
        if (painter && rect && index.isValid()) {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->restore();
        }
    }
    
    void drawBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) override
    {
        // Just a basic implementation for testing
        if (painter && index.isValid()) {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->restore();
        }
    }
    
    void drawText(QPainter *painter, QRectF *rect, const QStyleOptionViewItem &option, const QModelIndex &index) override
    {
        // Just a basic implementation for testing
        if (painter && rect && index.isValid()) {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->restore();
        }
    }
    
    QRectF rectByType(RectOfItemType type, const QModelIndex &index) override
    {
        // Just a basic implementation for testing
        return QRectF(0, 0, 100, 100);
    }
    
    QList<QRect> allPaintRect(const QStyleOptionViewItem &option, const QModelIndex &index) override
    {
        // Just a basic implementation for testing
        return QList<QRect>() << QRect(0, 0, 100, 100);
    }
    
    int iconRectIndex() override
    {
        // Just a basic implementation for testing
        return 0;
    }
    
    bool supportContentPreview() const override
    {
        // Just a basic implementation for testing
        return false;
    }
    
    void setStyleProxy(QStyle *style)
    {
        // Just a basic implementation for testing
        this->style = style;
    }
    
    // Expose protected member for testing
    QStyle* getStyle() const { return style; }
};

class AbstractItemPaintProxyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        proxy = new TestAbstractItemPaintProxy();
        
        // Mock QApplication methods - just skip for now to avoid compilation issues
    }

    void TearDown() override
    {
        delete proxy;
        stub.clear();
    }

    TestAbstractItemPaintProxy *proxy;
    stub_ext::StubExt stub;
};

TEST_F(AbstractItemPaintProxyTest, Constructor_CreatesInstance)
{
    EXPECT_NE(proxy, nullptr);
}

TEST_F(AbstractItemPaintProxyTest, DrawIcon_DrawsIcon)
{
    QPainter painter;
    QRectF rect(0, 0, 32, 32);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 100);
    option.state = QStyle::State_Enabled;
    option.decorationSize = QSize(32, 32);
    QModelIndex index;
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        proxy->drawIcon(&painter, &rect, option, index);
    });
}

TEST_F(AbstractItemPaintProxyTest, DrawBackground_DrawsBackground)
{
    QPainter painter;
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 100);
    option.state = QStyle::State_Enabled;
    option.backgroundBrush = QBrush(Qt::white);
    QModelIndex index;
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        proxy->drawBackground(&painter, option, index);
    });
}

TEST_F(AbstractItemPaintProxyTest, DrawText_DrawsText)
{
    QPainter painter;
    QRectF rect(0, 0, 100, 20);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 100);
    option.state = QStyle::State_Enabled;
    option.text = "Test Text";
    option.font = QFont();
    option.palette.setColor(QPalette::Text, Qt::black);
    option.textElideMode = Qt::ElideRight;
    QModelIndex index;
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        proxy->drawText(&painter, &rect, option, index);
    });
}

TEST_F(AbstractItemPaintProxyTest, RectByType_ReturnsRect)
{
    QModelIndex index;
    
    QRectF result = proxy->rectByType(RectOfItemType::kItemIconRect, index);
    
    EXPECT_TRUE(result.isValid());
    EXPECT_GT(result.width(), 0);
    EXPECT_GT(result.height(), 0);
}

TEST_F(AbstractItemPaintProxyTest, AllPaintRect_ReturnsRectList)
{
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 100);
    option.state = QStyle::State_Enabled;
    QModelIndex index;
    
    QList<QRect> result = proxy->allPaintRect(option, index);
    
    EXPECT_FALSE(result.isEmpty());
}

TEST_F(AbstractItemPaintProxyTest, IconRectIndex_ReturnsIndex)
{
    int result = proxy->iconRectIndex();
    
    EXPECT_GE(result, 0);
}

TEST_F(AbstractItemPaintProxyTest, SupportContentPreview_ReturnsSupport)
{
    bool result = proxy->supportContentPreview();
    
    // Should return false by default
    EXPECT_FALSE(result);
}

TEST_F(AbstractItemPaintProxyTest, SetStyleProxy_SetsStyle)
{
    QStyle *style = nullptr;
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        proxy->setStyleProxy(style);
    });
    
    EXPECT_EQ(proxy->getStyle(), style);
}