// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "controllers/dfmrecentcrumbcontroller.h"

using DFM_NAMESPACE::DFMRecentCrumbController;
using DFM_NAMESPACE::CrumbData;

namespace  {
    class DFMRecentCrumbControllerTest : public testing::Test
    {
    public:
        virtual void SetUp() override
        {
        }

        virtual void TearDown() override
        {
        }

        DFMRecentCrumbController controller;
    };
}

TEST_F(DFMRecentCrumbControllerTest, supportedUrl)
{
    DUrl url;
    url.setScheme(RECENT_SCHEME);
    EXPECT_TRUE(controller.supportedUrl(url));
    url.setScheme(FILE_SCHEME);
    EXPECT_FALSE(controller.supportedUrl(url));
}

TEST_F(DFMRecentCrumbControllerTest, seprateUrl){
     DUrl url;
     url.setScheme(RECENT_SCHEME);
     QList<CrumbData> list = controller.seprateUrl(url);
     EXPECT_TRUE(!list.empty());
}
