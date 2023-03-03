// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/framemanager_p.h"
#include "config/configpresenter.h"

#include "stubext.h"

#include <gtest/gtest.h>

using namespace testing;
using namespace ddplugin_organizer;

static bool initializeEnable(){
    return true;
}

static bool turned = false;
static void calledTurnOn(){
    turned = true;
}

TEST(FrameManager, initialize_enable)
{
    FrameManager fm;
    Stub stub;
    turned = false;
    stub.set(&ConfigPresenter::isEnable, initializeEnable);
    stub.set(&FrameManager::turnOn, calledTurnOn);

    fm.initialize();
    EXPECT_TRUE(turned);
}

static bool initializeDisbale(){
    return false;
}

TEST(FrameManager, initialize_disbale)
{
    FrameManager fm;
    Stub stub;
    turned = false;
    stub.set(&ConfigPresenter::isEnable, initializeDisbale);
    stub.set(&FrameManager::turnOn, calledTurnOn);

    fm.initialize();
    EXPECT_FALSE(turned);
}

bool builded = false;
static void onBuild(){
    builded = true;;
}

TEST(FrameManager, turnon_true)
{
    FrameManager fm;
    Stub stub;
    builded = false;
    stub.set(&FrameManager::onBuild, onBuild);

    fm.turnOn(true);
    EXPECT_TRUE(builded);
}

TEST(FrameManager, turnon_off)
{
    FrameManager fm;
    Stub stub;
    builded = false;
    stub.set(&FrameManager::onBuild, onBuild);

    fm.turnOn(false);
    EXPECT_FALSE(builded);
}

TEST(FrameManagerPrivate, showOptionWindow)
{
    FrameManagerPrivate obj(nullptr);
    ASSERT_EQ(obj.options, nullptr);

    stub_ext::StubExt stub;
    bool show = false;
    stub.set_lamda(&QWidget::show, [&show](){
        show = true;
    });

    obj.showOptionWindow();

    ASSERT_NE(obj.options, nullptr);
    EXPECT_TRUE(show);

    bool ac = false;
    stub.set_lamda(&QWidget::activateWindow, [&ac](){
        ac = true;
    });

    show = false;
    obj.showOptionWindow();
    auto old = obj.options;

    EXPECT_EQ(obj.options, old);
    EXPECT_TRUE(ac);
    EXPECT_FALSE(show);
}

