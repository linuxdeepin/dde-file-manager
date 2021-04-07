/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
#include <gtest/gtest.h>

#include "controllers/dfmsearchcrumbcontroller.h"
#include "interfaces/dfmcrumbbar.h"

DFM_USE_NAMESPACE

namespace  {
class TestDFMSearchCrumbController : public testing::Test
{
public:
    virtual void SetUp() override
    {
        std::cout << "start TestDFMSearchCrumbController" << std::endl;
        controller.setCrumbBar(new DFMCrumbBar());
    }

    virtual void TearDown() override
    {
        std::cout << "end TestDFMSearchCrumbController"  << std::endl;
    }

public:
    DFMSearchCrumbController controller;
};
}

TEST_F(TestDFMSearchCrumbController, processAction)
{
    controller.processAction(DFMCrumbInterface::EscKeyPressed);
    controller.processAction(DFMCrumbInterface::AddressBarLostFocus);
}

TEST_F(TestDFMSearchCrumbController, crumbUrlChangedBehavior)
{
    controller.crumbUrlChangedBehavior(DUrl("file:///tmp"));
}

TEST_F(TestDFMSearchCrumbController, supportedUrl)
{
    EXPECT_TRUE(controller.supportedUrl(DUrl::fromSearchFile("/tmp")));
}
