// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "controllers/dfmsftpcrumbcontroller.h"

DFM_USE_NAMESPACE

namespace  {
    class DFMSftpCrumbControllerTest : public testing::Test
    {
    public:
        virtual void SetUp() override
        {
        }

        virtual void TearDown() override
        {
        }

        DFMSftpCrumbController controller;
    };
}

TEST_F(DFMSftpCrumbControllerTest, supportedUrl)
{
    DUrl url;
    url.setScheme(SFTP_SCHEME);
    EXPECT_TRUE(controller.supportedUrl(url));
    url.setScheme(FILE_SCHEME);
    EXPECT_FALSE(controller.supportedUrl(url));
}
