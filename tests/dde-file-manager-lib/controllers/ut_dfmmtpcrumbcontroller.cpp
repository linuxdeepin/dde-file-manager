// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "controllers/dfmmtpcrumbcontroller.h"

using DFM_NAMESPACE::DFMMtpCrumbController;

namespace  {
    class DFMMtpCrumbControllerTest : public testing::Test
    {
    public:
        virtual void SetUp() override
        {
        }

        virtual void TearDown() override
        {
        }

        DFMMtpCrumbController controller;
    };
}

TEST_F(DFMMtpCrumbControllerTest, supportedUrl)
{
    DUrl url;
    url.setScheme(MTP_SCHEME);
    EXPECT_TRUE(controller.supportedUrl(url));
    url.setScheme(FILE_SCHEME);
    EXPECT_FALSE(controller.supportedUrl(url));
}
