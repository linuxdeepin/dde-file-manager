// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "controllers/dfmftpcrumbcontroller.h"

DFM_USE_NAMESPACE;

namespace  {
    class DFMFtpCrumbControllerTest : public testing::Test
    {
    public:
        virtual void SetUp() override
        {
        }

        virtual void TearDown() override
        {
        }

        DFMFtpCrumbController controller;
    };
}

TEST_F(DFMFtpCrumbControllerTest, supportedUrl)
{
    DUrl url;
    url.setScheme(FTP_SCHEME);
    EXPECT_TRUE(controller.supportedUrl(url));
    url.setScheme(FILE_SCHEME);
    EXPECT_FALSE(controller.supportedUrl(url));
}
