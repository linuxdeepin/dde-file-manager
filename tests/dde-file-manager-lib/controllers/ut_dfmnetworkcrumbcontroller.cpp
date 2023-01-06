// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "controllers/dfmnetworkcrumbcontroller.h"

using DFM_NAMESPACE::DFMNetworkCrumbController;
using DFM_NAMESPACE::CrumbData;

namespace  {
    class DFMNetworkCrumbControllerTest : public testing::Test
    {
    public:
        virtual void SetUp() override
        {
        }

        virtual void TearDown() override
        {
        }

        DFMNetworkCrumbController controller;
    };
}

TEST_F(DFMNetworkCrumbControllerTest, supportedUrl)
{
    DUrl url;
    url.setScheme(NETWORK_SCHEME);
    EXPECT_TRUE(controller.supportedUrl(url));
    url.setScheme(FILE_SCHEME);
    EXPECT_FALSE(controller.supportedUrl(url));
}

TEST_F(DFMNetworkCrumbControllerTest, seprateUrl){
    DUrl url;
    url.setScheme(NETWORK_SCHEME);
    QList<CrumbData> list = controller.seprateUrl(url);
    EXPECT_TRUE(list.size() > 0);
}
