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
#include "interface/canvasinterface.h"
#include "interface/canvasinterface_p.h"

#include <gtest/gtest.h>

using namespace testing;
using namespace ddplugin_organizer;

class CanvasInterfaceTest : public Test
{
public:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

    stub_ext::StubExt stub;
};

TEST_F(CanvasInterfaceTest, initialize) {
    CanvasInterface interface;

    EXPECT_EQ(interface.d->fileInfoModel, nullptr);
    EXPECT_EQ(interface.d->canvaModel, nullptr);
    EXPECT_EQ(interface.d->canvasView, nullptr);
    EXPECT_EQ(interface.d->canvasGrid, nullptr);
    EXPECT_EQ(interface.d->canvasManager, nullptr);

    interface.initialize();
    EXPECT_NE(interface.d->fileInfoModel, nullptr);
    EXPECT_NE(interface.d->canvaModel, nullptr);
    EXPECT_NE(interface.d->canvasView, nullptr);
    EXPECT_NE(interface.d->canvasGrid, nullptr);
    EXPECT_NE(interface.d->canvasManager, nullptr);
}

TEST_F(CanvasInterfaceTest, canvasManager) {
    CanvasInterface interface;
    interface.initialize();
    CanvasManagerShell *managerShell = interface.canvasManager();

    EXPECT_EQ(managerShell, interface.d->canvasManager);
}
