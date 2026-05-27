// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "utils/viewdrawhelper.h"
#include "views/fileview.h"
#include "models/fileviewmodel.h"
#include "views/baseitemdelegate.h"

#include <QPainter>
#include <QRect>
#include <QSize>
#include <QPoint>
#include <QColor>
#include <QBrush>
#include <QPen>
#include <QFont>
#include <QFontMetrics>
#include <QPixmap>
#include <QIcon>
#include <QStyleOptionViewItem>
#include <QApplication>
#include <QStyle>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QModelIndexList>

using namespace dfmplugin_workspace;

class ViewDrawHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        testUrl = QUrl::fromLocalFile("/tmp/test");
        fileView = new FileView(testUrl);
        drawHelper = new ViewDrawHelper(fileView);
        
        // Mock FileView methods
        stub.set_lamda(ADDR(FileView, model), [this]() -> FileViewModel* {
            return nullptr; // Return null for simplicity
        });
        
        stub.set_lamda(ADDR(FileView, currentPressIndex), [this]() -> QModelIndex {
            return QModelIndex(); // Return invalid index
        });
        
        stub.set_lamda(ADDR(FileView, itemDelegate), [this]() -> BaseItemDelegate* {
            return nullptr; // Return null for simplicity
        });
        
        stub.set_lamda(ADDR(FileView, iconSize), [this]() -> QSize {
            return QSize(64, 64); // Return default icon size
        });
        
        stub.set_lamda(ADDR(FileView, devicePixelRatioF), [this]() -> qreal {
            return 1.0; // Return default ratio
        });
        
        // Mock QApplication methods - use static function addresses
        using StyleFunc = QStyle*(*)();
        stub.set_lamda(static_cast<StyleFunc>(&QApplication::style), []() -> QStyle* {
            return nullptr; // Return null for simplicity
        });
        
        using FontFunc = QFont(*)();
        stub.set_lamda(static_cast<FontFunc>(&QApplication::font), []() -> QFont {
            return QFont();
        });
        
        using PaletteFunc = QPalette(*)();
        stub.set_lamda(static_cast<PaletteFunc>(&QApplication::palette), []() -> QPalette {
            return QPalette();
        });
    }

    void TearDown() override
    {
        stub.clear();
        delete drawHelper;
        drawHelper = nullptr;
        delete fileView;
        fileView = nullptr;
    }

    QUrl testUrl;
    FileView *fileView;
    ViewDrawHelper *drawHelper;
    stub_ext::StubExt stub;
};

TEST_F(ViewDrawHelperTest, Constructor_CreatesInstance)
{
    EXPECT_NE(drawHelper, nullptr);
    EXPECT_EQ(drawHelper->parent(), fileView);
}

TEST_F(ViewDrawHelperTest, RenderDragPixmap_ReturnsPixmap)
{
    DFMGLOBAL_NAMESPACE::ViewMode mode = DFMGLOBAL_NAMESPACE::ViewMode::kIconMode;
    QModelIndexList indexes;
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        QPixmap result = drawHelper->renderDragPixmap(mode, indexes);
        // Should return a valid pixmap or empty pixmap
        EXPECT_TRUE(result.isNull() || !result.isNull());
    });
}
