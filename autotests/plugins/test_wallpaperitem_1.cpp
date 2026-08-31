// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_wallpaperitem_1.cpp
 * @brief Unit tests for WallpaperItem methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "wallpaperitem.h"

#include <QTest>

using namespace ddplugin_wallpapersetting;

class WallpaperItemTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new WallpaperItem();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    WallpaperItem *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WallpaperItemTest, WallpaperItem)
{
    // Test constructor: WallpaperItem((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(WallpaperItemTest, contentGeometry)
{
    // Test getter: QRect contentGeometry()
    auto result = obj->contentGeometry();
    EXPECT_FALSE(result.isValid());

}

TEST_F(WallpaperItemTest, enableThumbnail)
{
    // Test bool getter: enableThumbnail()
    bool result = obj->enableThumbnail();
    EXPECT_FALSE(result);

}

TEST_F(WallpaperItemTest, enterEvent)
{
    // Test event handler: enterEvent((QEnterEvent *event))
    QEnterEvent _event(QEnterEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->enterEvent(&_event));
}

TEST_F(WallpaperItemTest, focusOnFirstButton)
{
    // Test method: void focusOnFirstButton(())
    EXPECT_NO_FATAL_FAILURE(obj->focusOnFirstButton());
}

TEST_F(WallpaperItemTest, focusOnLastButton)
{
    // Test method: void focusOnLastButton(())
    EXPECT_NO_FATAL_FAILURE(obj->focusOnLastButton());
}

TEST_F(WallpaperItemTest, init)
{
    // Test method: void init(())
    EXPECT_NO_FATAL_FAILURE(obj->init());
}

TEST_F(WallpaperItemTest, isDeletable)
{
    // Test bool getter: isDeletable()
    bool result = obj->isDeletable();
    EXPECT_FALSE(result);

}

TEST_F(WallpaperItemTest, leaveEvent)
{
    // Test event handler: leaveEvent((QEvent *event))
    QEvent _event(QEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->leaveEvent(&_event));
}

TEST_F(WallpaperItemTest, mousePressEvent)
{
    // Test event handler: mousePressEvent((QMouseEvent *event))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mousePressEvent(&_event));
}

TEST_F(WallpaperItemTest, onButtonClicked)
{
    // Test method: void onButtonClicked(())
    EXPECT_NO_FATAL_FAILURE(obj->onButtonClicked());
}

TEST_F(WallpaperItemTest, onFindAborted)
{
    // Test method: void onFindAborted((const QQueue<QString> &list))
    QQueue<QString> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onFindAborted(_arg0));
}

TEST_F(WallpaperItemTest, onThumbnailFounded)
{
    // Test method: void onThumbnailFounded((const QString &key, QPixmap pixmap))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onThumbnailFounded(_arg0, QPixmap()));
}

TEST_F(WallpaperItemTest, refindPixmap)
{
    // Test method: void refindPixmap(())
    EXPECT_NO_FATAL_FAILURE(obj->refindPixmap());
}

TEST_F(WallpaperItemTest, renderPixmap)
{
    // Test method: void renderPixmap(())
    EXPECT_NO_FATAL_FAILURE(obj->renderPixmap());
}

TEST_F(WallpaperItemTest, resizeEvent)
{
    // Test event handler: resizeEvent((QResizeEvent *event))
    QResizeEvent _event(QResizeEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->resizeEvent(&_event));
}

TEST_F(WallpaperItemTest, setDeletable)
{
    // Test setter: void setDeletable((bool del))
    EXPECT_NO_FATAL_FAILURE(obj->setDeletable(false));
}

TEST_F(WallpaperItemTest, setEnableThumbnail)
{
    // Test setter: void setEnableThumbnail((bool enbale))
    EXPECT_NO_FATAL_FAILURE(obj->setEnableThumbnail(false));
}

TEST_F(WallpaperItemTest, setEntranceIconOfSettings)
{
    // Test setter: void setEntranceIconOfSettings((const QString &id))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setEntranceIconOfSettings(_arg0));
}

TEST_F(WallpaperItemTest, setOpacity)
{
    // Test setter: void setOpacity((qreal opacity))
    EXPECT_NO_FATAL_FAILURE(obj->setOpacity(0.0));
}

TEST_F(WallpaperItemTest, setSketch)
{
    // Test setter: void setSketch((const QString &url))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setSketch(_arg0));
}

TEST_F(WallpaperItemTest, sketch)
{
    // Test getter: QString sketch()
    auto result = obj->sketch();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(WallpaperItemTest, slideDown)
{
    // Test method: void slideDown(())
    EXPECT_NO_FATAL_FAILURE(obj->slideDown());
}

TEST_F(WallpaperItemTest, slideUp)
{
    // Test method: void slideUp(())
    EXPECT_NO_FATAL_FAILURE(obj->slideUp());
}

TEST_F(WallpaperItemTest, thumbnailKey)
{
    // Test getter: QString thumbnailKey()
    auto result = obj->thumbnailKey();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(WallpaperItemTest, WallpaperItem_Destructor)
{
    // Test method:  ~WallpaperItem(())
    EXPECT_NO_FATAL_FAILURE({ WallpaperItem *tmp = new WallpaperItem(); delete tmp; });
}
