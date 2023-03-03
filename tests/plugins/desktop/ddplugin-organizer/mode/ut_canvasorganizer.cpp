// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "mode/canvasorganizer.h"
#include "interface/canvasmanagershell.h"

#include <gtest/gtest.h>

using namespace testing;
using namespace ddplugin_organizer;

class CanvasOrganizerTest : public Test
{
public:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

    stub_ext::StubExt stub;
};

TEST_F(CanvasOrganizerTest, setCanvasManagerShell) {

    CanvasOrganizer *organizer = OrganizerCreator::createOrganizer(OrganizerMode::kNormalized);
    EXPECT_EQ(organizer->canvasManagerShell, nullptr);

    CanvasManagerShell *sh = nullptr;
    organizer->setCanvasManagerShell(sh);
    EXPECT_EQ(organizer->canvasManagerShell, nullptr);

    sh = new CanvasManagerShell;
    organizer->setCanvasManagerShell(sh);
    EXPECT_EQ(organizer->canvasManagerShell, sh);

    organizer->setCanvasManagerShell(nullptr);
    EXPECT_EQ(organizer->canvasManagerShell, nullptr);

    delete sh;
    sh = nullptr;
}

