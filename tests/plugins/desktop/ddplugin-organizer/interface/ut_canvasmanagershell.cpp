/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stubext.h"
#include "interface/canvasmanagershell.h"
#include "dfm-framework/event/channel/eventchannel.h"

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

TEST_F(CanvasManagerShellTest, initialize) {
    CanvasManagerShell sh;
    bool resutl = sh.initialize();

    EXPECT_TRUE(resutl);
}

TEST_F(CanvasManagerShellTest, iconLevel) {
    CanvasManagerShell sh;
    bool isCall = false;

    stub.set_lamda((QVariant(EventChannelManager::*)(const QString &, const QString &))ADDR(EventChannelManager, push), [&] () {
        isCall = true;
        return QVariant(1);
    });

    int level = sh.iconLevel();

    EXPECT_TRUE(isCall);
    EXPECT_EQ(level, 1);
}

TEST_F(CanvasManagerShellTest, setIconLevel) {

    typedef QVariant (EventChannelManager::*PushLevel)(const QString &, const QString &, int);
    auto pushLevel = static_cast<PushLevel>(&EventChannelManager::push);

    bool isCall = false;
    stub.set_lamda(pushLevel, [&] () {
        isCall = true;
        return QVariant();
    });

    CanvasManagerShell sh;
    sh.setIconLevel(3);

    EXPECT_TRUE(isCall);
}
