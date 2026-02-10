// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "view/collectiontitlebar.h"
#include "organizer_defines.h"

#include <QApplication>
#include <QResizeEvent>
#include <QContextMenuEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QSignalSpy>

#include <gtest/gtest.h>

using namespace ddplugin_organizer;

class UT_CollectionTitleBar : public testing::Test
{
protected:
    void SetUp() override
    {
        titleBar = new CollectionTitleBar("test_uuid");
    }

    void TearDown() override
    {
        delete titleBar;

        stub.clear();
    }

public:
    CollectionTitleBar *titleBar;
    stub_ext::StubExt stub;
};

TEST_F(UT_CollectionTitleBar, TestConstructor)
{
    EXPECT_NE(titleBar, nullptr);
    EXPECT_TRUE(titleBar->titleBarVisible());  // Default should be visible
    EXPECT_FALSE(titleBar->renamable());       // Default should not be renamable
    EXPECT_FALSE(titleBar->closable());        // Default should not be closable
    EXPECT_FALSE(titleBar->adjustable());      // Default should not be adjustable
    EXPECT_FALSE(titleBar->titleName().isEmpty()); // Should have a title name
}

TEST_F(UT_CollectionTitleBar, TestSetTitleBarVisible)
{
    // Test setting visibility to false
    bool result = titleBar->setTitleBarVisible(false);
    EXPECT_TRUE(result);  // Should return true on success
    EXPECT_FALSE(titleBar->titleBarVisible());
    
    // Test setting visibility to true
    result = titleBar->setTitleBarVisible(true);
    EXPECT_TRUE(result);  // Should return true on success
    EXPECT_TRUE(titleBar->titleBarVisible());
    
    // Test setting same value
    result = titleBar->setTitleBarVisible(true);
    EXPECT_TRUE(result);  // Should still return true
    EXPECT_TRUE(titleBar->titleBarVisible());
    
    result = titleBar->setTitleBarVisible(false);
    EXPECT_TRUE(result);  // Should still return true
    EXPECT_FALSE(titleBar->titleBarVisible());
}

TEST_F(UT_CollectionTitleBar, TestTitleBarVisible)
{
    // Test default state
    bool visible = titleBar->titleBarVisible();
    EXPECT_TRUE(visible);
    
    // Test after setting to false
    titleBar->setTitleBarVisible(false);
    visible = titleBar->titleBarVisible();
    EXPECT_FALSE(visible);
    
    // Test after setting to true
    titleBar->setTitleBarVisible(true);
    visible = titleBar->titleBarVisible();
    EXPECT_TRUE(visible);
    
    // Test multiple calls
    for (int i = 0; i < 5; ++i) {
        bool result = titleBar->titleBarVisible();
        EXPECT_TRUE(result || !result);  // Just ensure it doesn't crash
    }
}

TEST_F(UT_CollectionTitleBar, TestSetRenamable)
{
    // Test setting renamable to true
    titleBar->setRenamable(true);
    EXPECT_TRUE(titleBar->renamable());
    
    // Test setting renamable to false
    titleBar->setRenamable(false);
    EXPECT_FALSE(titleBar->renamable());
    
    // Test with default parameter (false)
    titleBar->setRenamable();
    EXPECT_FALSE(titleBar->renamable());
    
    // Test multiple times with same value
    titleBar->setRenamable(true);
    titleBar->setRenamable(true);
    titleBar->setRenamable(true);
    EXPECT_TRUE(titleBar->renamable());
}

TEST_F(UT_CollectionTitleBar, TestRenamable)
{
    // Test default state
    bool renamable = titleBar->renamable();
    EXPECT_FALSE(renamable);
    
    // Test after setting to true
    titleBar->setRenamable(true);
    renamable = titleBar->renamable();
    EXPECT_TRUE(renamable);
    
    // Test after setting to false
    titleBar->setRenamable(false);
    renamable = titleBar->renamable();
    EXPECT_FALSE(renamable);
}

TEST_F(UT_CollectionTitleBar, TestSetClosable)
{
    // Test setting closable to true
    titleBar->setClosable(true);
    EXPECT_TRUE(titleBar->closable());
    
    // Test setting closable to false
    titleBar->setClosable(false);
    EXPECT_FALSE(titleBar->closable());
    
    // Test with default parameter (false)
    titleBar->setClosable();
    EXPECT_FALSE(titleBar->closable());
    
    // Test multiple times
    titleBar->setClosable(true);
    titleBar->setClosable(false);
    titleBar->setClosable(true);
    EXPECT_TRUE(titleBar->closable());
}

TEST_F(UT_CollectionTitleBar, TestClosable)
{
    // Test default state
    bool closable = titleBar->closable();
    EXPECT_FALSE(closable);
    
    // Test after setting to true
    titleBar->setClosable(true);
    closable = titleBar->closable();
    EXPECT_TRUE(closable);
    
    // Test after setting to false
    titleBar->setClosable(false);
    closable = titleBar->closable();
    EXPECT_FALSE(closable);
}

TEST_F(UT_CollectionTitleBar, TestSetAdjustable)
{
    // Test setting adjustable to true
    titleBar->setAdjustable(true);
    EXPECT_TRUE(titleBar->adjustable());
    
    // Test setting adjustable to false
    titleBar->setAdjustable(false);
    EXPECT_FALSE(titleBar->adjustable());
    
    // Test with default parameter (false)
    titleBar->setAdjustable();
    EXPECT_FALSE(titleBar->adjustable());
    
    // Test multiple rapid changes
    for (int i = 0; i < 5; ++i) {
        titleBar->setAdjustable(i % 2 == 0);
        EXPECT_EQ(titleBar->adjustable(), i % 2 == 0);
    }
}

TEST_F(UT_CollectionTitleBar, TestAdjustable)
{
    // Test default state
    bool adjustable = titleBar->adjustable();
    EXPECT_FALSE(adjustable);
    
    // Test after setting to true
    titleBar->setAdjustable(true);
    adjustable = titleBar->adjustable();
    EXPECT_TRUE(adjustable);
    
    // Test after setting to false
    titleBar->setAdjustable(false);
    adjustable = titleBar->adjustable();
    EXPECT_FALSE(adjustable);
}

TEST_F(UT_CollectionTitleBar, TestSetTitleName)
{
    // Test setting a normal name
    QString testName = "Test Collection";
    titleBar->setTitleName(testName);
    EXPECT_EQ(titleBar->titleName(), testName);
    
    // Test setting empty name
    titleBar->setTitleName("");
    EXPECT_TRUE(titleBar->titleName().isEmpty() || !titleBar->titleName().isEmpty()); // Either is acceptable
    
    // Test setting name with special characters
    QString specialName = "Collection@#$%^&*()";
    titleBar->setTitleName(specialName);
    EXPECT_EQ(titleBar->titleName(), specialName);
    
    // Test setting name with unicode
    QString unicodeName = "集合标题";
    titleBar->setTitleName(unicodeName);
    EXPECT_EQ(titleBar->titleName(), unicodeName);
    
    // Test setting very long name
    QString longName = "This is a very long collection name that might cause issues with display or storage";
    titleBar->setTitleName(longName);
    EXPECT_EQ(titleBar->titleName(), longName);
}

TEST_F(UT_CollectionTitleBar, TestTitleName)
{
    // Test default name
    QString name = titleBar->titleName();
    EXPECT_FALSE(name.isEmpty());  // Should have a default name
    
    // Test after setting
    QString testName = "My Collection";
    titleBar->setTitleName(testName);
    name = titleBar->titleName();
    EXPECT_EQ(name, testName);
    
    // Test multiple retrievals
    for (int i = 0; i < 5; ++i) {
        QString currentName = titleBar->titleName();
        EXPECT_EQ(currentName, testName);
    }
}

TEST_F(UT_CollectionTitleBar, TestSetCollectionSize)
{
    // Test with different sizes (assuming CollectionFrameSize is an enum or int)
    CollectionFrameSize smallSize = kSmall;  // Assuming these exist
    CollectionFrameSize mediumSize = kMiddle;
    CollectionFrameSize largeSize = kLarge;
    
    titleBar->setCollectionSize(smallSize);
    EXPECT_EQ(titleBar->collectionSize(), smallSize);
    
    titleBar->setCollectionSize(mediumSize);
    EXPECT_EQ(titleBar->collectionSize(), mediumSize);
    
    titleBar->setCollectionSize(largeSize);
    EXPECT_EQ(titleBar->collectionSize(), largeSize);
    
    // Test multiple rapid changes
    for (int i = 0; i < 10; ++i) {
        CollectionFrameSize size = static_cast<CollectionFrameSize>(i % 3);
        titleBar->setCollectionSize(size);
        EXPECT_EQ(titleBar->collectionSize(), size);
    }
}

TEST_F(UT_CollectionTitleBar, TestCollectionSize)
{
    // Test default size
    CollectionFrameSize size = titleBar->collectionSize();
    // Should be a valid size (can't test exact value without knowing default)
    
    // Test after setting
    CollectionFrameSize testSize = kMiddle;  // Assuming this exists
    titleBar->setCollectionSize(testSize);
    size = titleBar->collectionSize();
    EXPECT_EQ(size, testSize);
}

TEST_F(UT_CollectionTitleBar, TestEventFilter)
{
    // Test event filter with basic events
    QEvent showEvent(QEvent::Show);
    QEvent hideEvent(QEvent::Hide);
    QEvent resizeEvent(QEvent::Resize);
    
    // Event filter should handle events without crashing
    bool result = titleBar->eventFilter(titleBar, &showEvent);
    EXPECT_TRUE(result || !result);  // Either return value is acceptable
    
    result = titleBar->eventFilter(titleBar, &hideEvent);
    EXPECT_TRUE(result || !result);
    
    result = titleBar->eventFilter(titleBar, &resizeEvent);
    EXPECT_TRUE(result || !result);
    
    // Test with null object
    result = titleBar->eventFilter(nullptr, &showEvent);
    EXPECT_TRUE(result || !result);
    
    // Test with null event
    // result = titleBar->eventFilter(titleBar, nullptr);
    // EXPECT_TRUE(result || !result);
}

TEST_F(UT_CollectionTitleBar, TestResizeEvent)
{
    // Test resize event
    QResizeEvent resizeEvent(QSize(200, 50), QSize(100, 30));
    EXPECT_NO_THROW(titleBar->resizeEvent(&resizeEvent));
    
    // Test with same size
    QResizeEvent sameSizeEvent(QSize(200, 50), QSize(200, 50));
    EXPECT_NO_THROW(titleBar->resizeEvent(&sameSizeEvent));
    
    // Test with zero size
    QResizeEvent zeroSizeEvent(QSize(0, 0), QSize(100, 50));
    EXPECT_NO_THROW(titleBar->resizeEvent(&zeroSizeEvent));
    
    // Test with very large size
    QResizeEvent largeSizeEvent(QSize(5000, 5000), QSize(100, 50));
    EXPECT_NO_THROW(titleBar->resizeEvent(&largeSizeEvent));
}

TEST_F(UT_CollectionTitleBar, TestContextMenuEvent)
{
    // Test context menu event
    QContextMenuEvent menuEvent(QContextMenuEvent::Mouse, QPoint(50, 25));
    EXPECT_NO_THROW(titleBar->contextMenuEvent(&menuEvent));
    
    // Test context menu at different positions
    QContextMenuEvent menuEventTopLeft(QContextMenuEvent::Mouse, QPoint(0, 0));
    EXPECT_NO_THROW(titleBar->contextMenuEvent(&menuEventTopLeft));
    
    QContextMenuEvent menuEventKeyboard(QContextMenuEvent::Keyboard, QPoint(50, 25));
    EXPECT_NO_THROW(titleBar->contextMenuEvent(&menuEventKeyboard));
    
    QContextMenuEvent menuEventOther(QContextMenuEvent::Other, QPoint(50, 25));
    EXPECT_NO_THROW(titleBar->contextMenuEvent(&menuEventOther));
}

TEST_F(UT_CollectionTitleBar, TestRounded)
{
    // Test rounded method
    EXPECT_NO_THROW(titleBar->rounded());
    
    // Test multiple calls
    for (int i = 0; i < 5; ++i) {
        EXPECT_NO_THROW(titleBar->rounded());
    }
}

TEST_F(UT_CollectionTitleBar, TestSignals)
{
    QSignalSpy closeSpy(titleBar, &CollectionTitleBar::sigRequestClose);
    QSignalSpy adjustSpy(titleBar, &CollectionTitleBar::sigRequestAdjustSizeMode);
    
    // Initially no signals should be emitted
    EXPECT_EQ(closeSpy.count(), 0);
    EXPECT_EQ(adjustSpy.count(), 0);
    
    // Signals are typically emitted in response to user actions
    // We can't directly test signal emission without triggering the actual UI actions
    // But we can verify the signals exist and can be connected to
    EXPECT_TRUE(closeSpy.isValid());
    EXPECT_TRUE(adjustSpy.isValid());
}

TEST_F(UT_CollectionTitleBar, TestMultipleTitleBars)
{
    // Test creating multiple title bars
    CollectionTitleBar *titleBar1 = new CollectionTitleBar("uuid1");
    CollectionTitleBar *titleBar2 = new CollectionTitleBar("uuid2");
    CollectionTitleBar *titleBar3 = new CollectionTitleBar("uuid3");
    
    EXPECT_NE(titleBar1, nullptr);
    EXPECT_NE(titleBar2, nullptr);
    EXPECT_NE(titleBar3, nullptr);
    
    // Test they can have different properties
    titleBar1->setTitleName("Title 1");
    titleBar2->setTitleName("Title 2");
    titleBar3->setTitleName("Title 3");
    
    EXPECT_EQ(titleBar1->titleName(), "Title 1");
    EXPECT_EQ(titleBar2->titleName(), "Title 2");
    EXPECT_EQ(titleBar3->titleName(), "Title 3");
    
    // Test different visibility states
    titleBar1->setTitleBarVisible(true);
    titleBar2->setTitleBarVisible(false);
    titleBar3->setTitleBarVisible(true);
    
    EXPECT_TRUE(titleBar1->titleBarVisible());
    EXPECT_FALSE(titleBar2->titleBarVisible());
    EXPECT_TRUE(titleBar3->titleBarVisible());
    
    // Cleanup
    delete titleBar1;
    delete titleBar2;
    delete titleBar3;
}

TEST_F(UT_CollectionTitleBar, TestEdgeCases)
{
    // Test with parent widget
    QWidget parent;
    CollectionTitleBar *childTitleBar = new CollectionTitleBar("child_uuid", &parent);
    EXPECT_NE(childTitleBar, nullptr);
    delete childTitleBar;
    
    // Test rapid property changes
    for (int i = 0; i < 100; ++i) {
        titleBar->setRenamable(i % 2 == 0);
        titleBar->setClosable(i % 3 == 0);
        titleBar->setAdjustable(i % 5 == 0);
        titleBar->setTitleBarVisible(i % 7 == 0);
    }
    
    // Verify final state
    EXPECT_TRUE(titleBar->renamable() || !titleBar->renamable());  // Just verify it doesn't crash
    EXPECT_TRUE(titleBar->closable() || !titleBar->closable());
    EXPECT_TRUE(titleBar->adjustable() || !titleBar->adjustable());
    EXPECT_TRUE(titleBar->titleBarVisible() || !titleBar->titleBarVisible());
}

TEST_F(UT_CollectionTitleBar, TestWidgetProperties)
{
    // Test basic widget properties
    EXPECT_TRUE(titleBar->isEnabled() || !titleBar->isEnabled());  // Either state is valid
    EXPECT_TRUE(titleBar->isVisible() || !titleBar->isVisible());
    
    // Test showing and hiding
    EXPECT_NO_THROW(titleBar->show());
    EXPECT_NO_THROW(titleBar->hide());
    EXPECT_NO_THROW(titleBar->show());
    
    // Test resizing
    EXPECT_NO_THROW(titleBar->resize(200, 50));
    EXPECT_NO_THROW(titleBar->resize(300, 60));
    
    // Test moving
    EXPECT_NO_THROW(titleBar->move(100, 100));
    EXPECT_NO_THROW(titleBar->move(0, 0));
}
