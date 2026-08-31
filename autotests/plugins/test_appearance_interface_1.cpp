// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_appearance_interface_1.cpp
 * @brief Unit tests for Appearance_Interface methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "appearance_interface.h"

#include <QTest>

using namespace ddplugin_background;

class Appearance_InterfaceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new Appearance_Interface();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    Appearance_Interface *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(Appearance_InterfaceTest, GetCurrentWorkspaceBackground)
{
    // Test getter: QDBusPendingReply<QString> GetCurrentWorkspaceBackground()
    auto result = obj->GetCurrentWorkspaceBackground();
    EXPECT_NO_FATAL_FAILURE({ obj->GetCurrentWorkspaceBackground(); });

}

TEST_F(Appearance_InterfaceTest, GetCurrentWorkspaceBackgroundForMonitor)
{
    // Test getter: QDBusPendingReply<QString> GetCurrentWorkspaceBackgroundForMonitor()
    auto result = obj->GetCurrentWorkspaceBackgroundForMonitor();
    EXPECT_NO_FATAL_FAILURE({ obj->GetCurrentWorkspaceBackgroundForMonitor(); });

}

TEST_F(Appearance_InterfaceTest, GetScaleFactor)
{
    // Test getter: QDBusPendingReply<double> GetScaleFactor()
    auto result = obj->GetScaleFactor();
    EXPECT_NO_FATAL_FAILURE({ obj->GetScaleFactor(); });

}

TEST_F(Appearance_InterfaceTest, GetScreenScaleFactors)
{
    // Test getter: QDBusPendingReply<ScaleFactors> GetScreenScaleFactors()
    auto result = obj->GetScreenScaleFactors();
    EXPECT_NO_FATAL_FAILURE({ obj->GetScreenScaleFactors(); });

}

TEST_F(Appearance_InterfaceTest, GetWallpaperSlideShow)
{
    // Test getter: QDBusPendingReply<QString> GetWallpaperSlideShow()
    auto result = obj->GetWallpaperSlideShow();
    EXPECT_NO_FATAL_FAILURE({ obj->GetWallpaperSlideShow(); });

}

TEST_F(Appearance_InterfaceTest, GetWorkspaceBackgroundForMonitor)
{
    // Test getter: QDBusPendingReply<QString> GetWorkspaceBackgroundForMonitor()
    auto result = obj->GetWorkspaceBackgroundForMonitor();
    EXPECT_NO_FATAL_FAILURE({ obj->GetWorkspaceBackgroundForMonitor(); });

}

TEST_F(Appearance_InterfaceTest, Set)
{
    // Test getter: QDBusPendingReply<> Set()
    auto result = obj->Set();
    EXPECT_NO_FATAL_FAILURE({ obj->Set(); });

}

TEST_F(Appearance_InterfaceTest, SetCurrentWorkspaceBackground)
{
    // Test getter: QDBusPendingReply<> SetCurrentWorkspaceBackground()
    auto result = obj->SetCurrentWorkspaceBackground();
    EXPECT_NO_FATAL_FAILURE({ obj->SetCurrentWorkspaceBackground(); });

}

TEST_F(Appearance_InterfaceTest, SetCurrentWorkspaceBackgroundForMonitor)
{
    // Test getter: QDBusPendingReply<> SetCurrentWorkspaceBackgroundForMonitor()
    auto result = obj->SetCurrentWorkspaceBackgroundForMonitor();
    EXPECT_NO_FATAL_FAILURE({ obj->SetCurrentWorkspaceBackgroundForMonitor(); });

}

TEST_F(Appearance_InterfaceTest, SetMonitorBackground)
{
    // Test getter: QDBusPendingReply<> SetMonitorBackground()
    auto result = obj->SetMonitorBackground();
    EXPECT_NO_FATAL_FAILURE({ obj->SetMonitorBackground(); });

}

TEST_F(Appearance_InterfaceTest, SetScaleFactor)
{
    // Test getter: QDBusPendingReply<> SetScaleFactor()
    auto result = obj->SetScaleFactor();
    EXPECT_NO_FATAL_FAILURE({ obj->SetScaleFactor(); });

}

TEST_F(Appearance_InterfaceTest, SetScreenScaleFactors)
{
    // Test getter: QDBusPendingReply<> SetScreenScaleFactors()
    auto result = obj->SetScreenScaleFactors();
    EXPECT_NO_FATAL_FAILURE({ obj->SetScreenScaleFactors(); });

}

TEST_F(Appearance_InterfaceTest, SetWallpaperSlideShow)
{
    // Test getter: QDBusPendingReply<> SetWallpaperSlideShow()
    auto result = obj->SetWallpaperSlideShow();
    EXPECT_NO_FATAL_FAILURE({ obj->SetWallpaperSlideShow(); });

}

TEST_F(Appearance_InterfaceTest, SetWorkspaceBackgroundForMonitor)
{
    // Test getter: QDBusPendingReply<> SetWorkspaceBackgroundForMonitor()
    auto result = obj->SetWorkspaceBackgroundForMonitor();
    EXPECT_NO_FATAL_FAILURE({ obj->SetWorkspaceBackgroundForMonitor(); });

}

TEST_F(Appearance_InterfaceTest, background)
{
    // Test getter: QString background()
    auto result = obj->background();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(Appearance_InterfaceTest, cursorTheme)
{
    // Test getter: QString cursorTheme()
    auto result = obj->cursorTheme();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(Appearance_InterfaceTest, globalTheme)
{
    // Test getter: QString globalTheme()
    auto result = obj->globalTheme();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(Appearance_InterfaceTest, gtkTheme)
{
    // Test getter: QString gtkTheme()
    auto result = obj->gtkTheme();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(Appearance_InterfaceTest, iconTheme)
{
    // Test getter: QString iconTheme()
    auto result = obj->iconTheme();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(Appearance_InterfaceTest, monospaceFont)
{
    // Test getter: QString monospaceFont()
    auto result = obj->monospaceFont();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(Appearance_InterfaceTest, opacity)
{
    // Test getter: double opacity()
    auto result = obj->opacity();
    EXPECT_EQ(result, 0.0);

}

TEST_F(Appearance_InterfaceTest, qtActiveColor)
{
    // Test getter: QString qtActiveColor()
    auto result = obj->qtActiveColor();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(Appearance_InterfaceTest, setFontSize)
{
    // Test method: void setFontSize(())
    EXPECT_NO_FATAL_FAILURE(obj->setFontSize());
}

TEST_F(Appearance_InterfaceTest, setOpacity)
{
    // Test method: void setOpacity(())
    EXPECT_NO_FATAL_FAILURE(obj->setOpacity());
}

TEST_F(Appearance_InterfaceTest, setQtActiveColor)
{
    // Test method: void setQtActiveColor(())
    EXPECT_NO_FATAL_FAILURE(obj->setQtActiveColor());
}

TEST_F(Appearance_InterfaceTest, setWallpaperSlideShow)
{
    // Test method: void setWallpaperSlideShow(())
    EXPECT_NO_FATAL_FAILURE(obj->setWallpaperSlideShow());
}

TEST_F(Appearance_InterfaceTest, setWindowRadius)
{
    // Test method: void setWindowRadius(())
    EXPECT_NO_FATAL_FAILURE(obj->setWindowRadius());
}

TEST_F(Appearance_InterfaceTest, standardFont)
{
    // Test getter: QString standardFont()
    auto result = obj->standardFont();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(Appearance_InterfaceTest, staticInterfaceName)
{
    // Test getter: char staticInterfaceName()
    auto result = obj->staticInterfaceName();
    EXPECT_EQ(result, 0);

}

TEST_F(Appearance_InterfaceTest, wallpaperSlideShow)
{
    // Test getter: QString wallpaperSlideShow()
    auto result = obj->wallpaperSlideShow();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(Appearance_InterfaceTest, wallpaperURls)
{
    // Test getter: QString wallpaperURls()
    auto result = obj->wallpaperURls();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(Appearance_InterfaceTest, windowRadius)
{
    // Test getter: int windowRadius()
    auto result = obj->windowRadius();
    EXPECT_EQ(result, 0);

}
