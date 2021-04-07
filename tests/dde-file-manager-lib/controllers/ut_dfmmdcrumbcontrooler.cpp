/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     dengkeyun<dengkeyun@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
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

