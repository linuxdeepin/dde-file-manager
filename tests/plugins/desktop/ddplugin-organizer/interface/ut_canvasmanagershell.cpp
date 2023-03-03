// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "interface/canvasmanagershell.h"

#include <dfm-framework/event/eventchannel.h>

#include <gtest/gtest.h>

using namespace testing;
using namespace ddplugin_organizer;
using namespace dpf;

class CanvasManagerShellTest : public Test
{
public:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

    stub_ext::StubExt stub;
};

TEST_F(CanvasManagerShellTest, initialize)
{
    CanvasManagerShell sh;
    bool resutl = sh.initialize();

    EXPECT_TRUE(resutl);
}

TEST_F(CanvasManagerShellTest, iconLevel)
{
    CanvasManagerShell sh;
    bool isCall = false;

    stub.set_lamda((QVariant(EventChannelManager::*)(const QString &, const QString &))ADDR(EventChannelManager, push), [&]() {
        isCall = true;
        return QVariant(1);
    });

    int level = sh.iconLevel();

    EXPECT_TRUE(isCall);
    EXPECT_EQ(level, 1);
}

TEST_F(CanvasManagerShellTest, setIconLevel)
{

    typedef QVariant (EventChannelManager::*PushLevel)(const QString &, const QString &, int);
    auto pushLevel = static_cast<PushLevel>(&EventChannelManager::push);

    bool isCall = false;
    stub.set_lamda(pushLevel, [&]() {
        isCall = true;
        return QVariant();
    });

    CanvasManagerShell sh;
    sh.setIconLevel(3);

    EXPECT_TRUE(isCall);
}
