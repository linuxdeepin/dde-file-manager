// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core.h"

#include "stubext.h"

#include <gtest/gtest.h>

DPF_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DDPCORE_USE_NAMESPACE

#if 0 // unstale
TEST(TestEvent, event_undefined)
{
    // AbstractScreenProxy begin
    EXPECT_EQ(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(signal_ScreenProxy_ScreenChanged))
              , EventTypeScope::kInValid);
    EXPECT_EQ(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(signal_ScreenProxy_DisplayModeChanged))
              , EventTypeScope::kInValid);
    EXPECT_EQ(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(signal_ScreenProxy_ScreenGeometryChanged))
              , EventTypeScope::kInValid);
    EXPECT_EQ(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(signal_ScreenProxy_ScreenAvailableGeometryChanged))
              , EventTypeScope::kInValid);

    EXPECT_EQ(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(slot_ScreenProxy_PrimaryScreen))
              , EventTypeScope::kInValid);
    EXPECT_EQ(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(slot_ScreenProxy_Screens))
              , EventTypeScope::kInValid);
    EXPECT_EQ(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(slot_ScreenProxy_LogicScreens))
              , EventTypeScope::kInValid);
    EXPECT_EQ(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(slot_ScreenProxy_Screen))
              , EventTypeScope::kInValid);
    EXPECT_EQ(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(slot_ScreenProxy_DevicePixelRatio))
              , EventTypeScope::kInValid);
    EXPECT_EQ(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(slot_ScreenProxy_DisplayMode))
              , EventTypeScope::kInValid);
    EXPECT_EQ(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(slot_ScreenProxy_LastChangedMode))
              , EventTypeScope::kInValid);
    EXPECT_EQ(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(slot_ScreenProxy_Reset))
              , EventTypeScope::kInValid);

    // WindowFrame begin
    EXPECT_EQ(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(signal_DesktopFrame_WindowAboutToBeBuilded))
              , EventTypeScope::kInValid);
    EXPECT_EQ(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(signal_DesktopFrame_WindowBuilded))
              , EventTypeScope::kInValid);
    EXPECT_EQ(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(signal_DesktopFrame_WindowShowed))
              , EventTypeScope::kInValid);
    EXPECT_EQ(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(signal_DesktopFrame_GeometryChanged))
              , EventTypeScope::kInValid);
    EXPECT_EQ(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(signal_DesktopFrame_AvailableGeometryChanged))
              , EventTypeScope::kInValid);

    EXPECT_EQ(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(slot_DesktopFrame_RootWindows))
              , EventTypeScope::kInValid);
    EXPECT_EQ(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(slot_DesktopFrame_LayoutWidget))
              , EventTypeScope::kInValid);
}

TEST(TestEvent, event_define)
{
    Core core;
    // AbstractScreenProxy begin
    EXPECT_NE(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(signal_ScreenProxy_ScreenChanged))
              , EventTypeScope::kInValid);
    EXPECT_NE(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(signal_ScreenProxy_DisplayModeChanged))
              , EventTypeScope::kInValid);
    EXPECT_NE(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(signal_ScreenProxy_ScreenGeometryChanged))
              , EventTypeScope::kInValid);
    EXPECT_NE(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(signal_ScreenProxy_ScreenAvailableGeometryChanged))
              , EventTypeScope::kInValid);

    EXPECT_NE(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(slot_ScreenProxy_PrimaryScreen))
              , EventTypeScope::kInValid);
    EXPECT_NE(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(slot_ScreenProxy_Screens))
              , EventTypeScope::kInValid);
    EXPECT_NE(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(slot_ScreenProxy_LogicScreens))
              , EventTypeScope::kInValid);
    EXPECT_NE(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(slot_ScreenProxy_Screen))
              , EventTypeScope::kInValid);
    EXPECT_NE(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(slot_ScreenProxy_DevicePixelRatio))
              , EventTypeScope::kInValid);
    EXPECT_NE(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(slot_ScreenProxy_DisplayMode))
              , EventTypeScope::kInValid);
    EXPECT_NE(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(slot_ScreenProxy_LastChangedMode))
              , EventTypeScope::kInValid);
    EXPECT_NE(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(slot_ScreenProxy_Reset))
              , EventTypeScope::kInValid);

    // WindowFrame begin
    EXPECT_NE(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(signal_DesktopFrame_WindowAboutToBeBuilded))
              , EventTypeScope::kInValid);
    EXPECT_NE(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(signal_DesktopFrame_WindowBuilded))
              , EventTypeScope::kInValid);
    EXPECT_NE(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(signal_DesktopFrame_WindowShowed))
              , EventTypeScope::kInValid);
    EXPECT_NE(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(signal_DesktopFrame_GeometryChanged))
              , EventTypeScope::kInValid);
    EXPECT_NE(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(signal_DesktopFrame_AvailableGeometryChanged))
              , EventTypeScope::kInValid);

    EXPECT_NE(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(slot_DesktopFrame_RootWindows))
              , EventTypeScope::kInValid);
    EXPECT_NE(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE), QT_STRINGIFY2(slot_DesktopFrame_LayoutWidget))
              , EventTypeScope::kInValid);
}

TEST(TestEvent, event_slot_null)
{
   EXPECT_EQ(dpfSlotChannel->channelMap.value(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE),
                                                                      QT_STRINGIFY2(slot_ScreenProxy_PrimaryScreen))),

                                              nullptr);
   EXPECT_EQ(dpfSlotChannel->channelMap.value(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE),
                                                                      QT_STRINGIFY2(slot_ScreenProxy_Screens))),

                                              nullptr);
   EXPECT_EQ(dpfSlotChannel->channelMap.value(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE),
                                                                      QT_STRINGIFY2(slot_ScreenProxy_LogicScreens))),

                                              nullptr);
   EXPECT_EQ(dpfSlotChannel->channelMap.value(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE),
                                                                      QT_STRINGIFY2(slot_ScreenProxy_Screen))),

                                              nullptr);
   EXPECT_EQ(dpfSlotChannel->channelMap.value(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE),
                                                                      QT_STRINGIFY2(slot_ScreenProxy_DevicePixelRatio))),

                                              nullptr);
   EXPECT_EQ(dpfSlotChannel->channelMap.value(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE),
                                                                      QT_STRINGIFY2(slot_ScreenProxy_DisplayMode))),

                                              nullptr);
   EXPECT_EQ(dpfSlotChannel->channelMap.value(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE),
                                                                      QT_STRINGIFY2(slot_ScreenProxy_LastChangedMode))),

                                              nullptr);
   EXPECT_EQ(dpfSlotChannel->channelMap.value(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE),
                                                                      QT_STRINGIFY2(slot_ScreenProxy_Reset))),

                                              nullptr);
   EXPECT_EQ(dpfSlotChannel->channelMap.value(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE),
                                                                      QT_STRINGIFY2(slot_DesktopFrame_RootWindows))),

                                              nullptr);
   EXPECT_EQ(dpfSlotChannel->channelMap.value(EventConverter::convert(QT_STRINGIFY(DDPCORE_NAMESPACE),
                                                                      QT_STRINGIFY2(slot_DesktopFrame_LayoutWidget))),

                                              nullptr);
}
#endif
