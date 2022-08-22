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

