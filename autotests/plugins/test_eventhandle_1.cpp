// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_eventhandle_1.cpp
 * @brief Unit tests for EventHandle methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "core.h"

#include <QTest>

using namespace ddplugin_core;

class EventHandleTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new EventHandle();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    EventHandle *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(EventHandleTest, EventHandle)
{
    // Test constructor: EventHandle((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(EventHandleTest, EventHandle_EventHan)
{
    // Test constructor: EventHandle((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(EventHandleTest, desktopFrame)
{
    // Test getter: AbstractDesktopFrame desktopFrame()
    auto result = obj->desktopFrame();
    EXPECT_NO_FATAL_FAILURE({ obj->desktopFrame(); });

}

TEST_F(EventHandleTest, displayMode)
{
    // Test getter: int displayMode()
    auto result = obj->displayMode();
    EXPECT_EQ(result, 0);

}

TEST_F(EventHandleTest, hookCanvasRequest)
{
    // Test method: bool hookCanvasRequest((const QString &screen))
    QString _arg0{};
    auto result = obj->hookCanvasRequest(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(EventHandleTest, init)
{
    // Test bool getter: init()
    bool result = obj->init();
    EXPECT_FALSE(result);

}

TEST_F(EventHandleTest, init_init)
{
    // Test bool getter: init()
    bool result = obj->init();
    EXPECT_FALSE(result);

}

TEST_F(EventHandleTest, lastChangedMode)
{
    // Test getter: int lastChangedMode()
    auto result = obj->lastChangedMode();
    EXPECT_EQ(result, 0);

}

TEST_F(EventHandleTest, layoutWidget)
{
    // Test method: void layoutWidget(())
    EXPECT_NO_FATAL_FAILURE(obj->layoutWidget());
}

TEST_F(EventHandleTest, logicScreens)
{
    // Test getter: QList<ScreenPointer> logicScreens()
    auto result = obj->logicScreens();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(EventHandleTest, onChanged)
{
    // Test method: void onChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onChanged());
}

TEST_F(EventHandleTest, onQuit)
{
    // Test method: void onQuit(())
    EXPECT_NO_FATAL_FAILURE(obj->onQuit());
}

TEST_F(EventHandleTest, publishAvailableGeometryChanged)
{
    // Test method: void publishAvailableGeometryChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->publishAvailableGeometryChanged());
}

TEST_F(EventHandleTest, publishDisplayModeChanged)
{
    // Test method: void publishDisplayModeChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->publishDisplayModeChanged());
}

TEST_F(EventHandleTest, publishGeometryChanged)
{
    // Test method: void publishGeometryChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->publishGeometryChanged());
}

TEST_F(EventHandleTest, publishScreenAvailableGeometryChanged)
{
    // Test method: void publishScreenAvailableGeometryChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->publishScreenAvailableGeometryChanged());
}

TEST_F(EventHandleTest, publishScreenChanged)
{
    // Test method: void publishScreenChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->publishScreenChanged());
}

TEST_F(EventHandleTest, publishScreenGeometryChanged)
{
    // Test method: void publishScreenGeometryChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->publishScreenGeometryChanged());
}

TEST_F(EventHandleTest, publishWindowAboutToBeBuilded)
{
    // Test method: void publishWindowAboutToBeBuilded(())
    EXPECT_NO_FATAL_FAILURE(obj->publishWindowAboutToBeBuilded());
}

TEST_F(EventHandleTest, publishWindowBuilded)
{
    // Test method: void publishWindowBuilded(())
    EXPECT_NO_FATAL_FAILURE(obj->publishWindowBuilded());
}

TEST_F(EventHandleTest, publishWindowShowed)
{
    // Test method: void publishWindowShowed(())
    EXPECT_NO_FATAL_FAILURE(obj->publishWindowShowed());
}

TEST_F(EventHandleTest, rootWindows)
{
    // Test getter: QList<QWidget *> rootWindows()
    auto result = obj->rootWindows();
    // Pointer return type
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(EventHandleTest, screenProxyInstance)
{
    // Test getter: AbstractScreenProxy screenProxyInstance()
    auto result = obj->screenProxyInstance();
    EXPECT_NO_FATAL_FAILURE({ obj->screenProxyInstance(); });

}

TEST_F(EventHandleTest, screensInUse)
{
    // Test method: bool screensInUse((QStringList *out))
    auto result = obj->screensInUse(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(EventHandleTest, EventHandle_Destructor)
{
    // Test method:  ~EventHandle(())
    EXPECT_NO_FATAL_FAILURE({ EventHandle *tmp = new EventHandle(); delete tmp; });
}

TEST_F(EventHandleTest, EventHandle_Destructor_xEventHa)
{
    // Test method:  ~EventHandle(())
    EXPECT_NO_FATAL_FAILURE({ EventHandle *tmp = new EventHandle(); delete tmp; });
}
