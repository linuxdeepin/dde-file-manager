// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
