// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "controllers/dfmmdcrumbcontrooler.h"

using DFM_NAMESPACE::DFMMDCrumbControoler;

namespace  {
    class DFMMDCrumbControolerTest : public testing::Test
    {
    public:
        virtual void SetUp() override
        {
        }

        virtual void TearDown() override
        {
        }

        DFMMDCrumbControoler controller;
    };
}

TEST_F(DFMMDCrumbControolerTest, supportedUrl)
{
    DUrl url;
    url.setScheme(DFMMD_SCHEME);
    EXPECT_TRUE(controller.supportedUrl(url));
    url.setScheme(FILE_SCHEME);
    EXPECT_FALSE(controller.supportedUrl(url));
}

