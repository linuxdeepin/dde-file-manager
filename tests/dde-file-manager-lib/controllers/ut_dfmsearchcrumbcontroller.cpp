// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
