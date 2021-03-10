/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: gongheng<gongheng@uniontech.com>
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

#include "interfaces/plugins/dfmcrumbfactory.h"
#include "interfaces/dfmcrumbinterface.h"

DFM_USE_NAMESPACE

namespace  {
    class TestDFMCrumbFactory : public testing::Test
    {
    public:
        void SetUp() override
        {
            std::cout << "start TestDFMCrumbFactory";
        }
        void TearDown() override
        {
            std::cout << "end TestDFMCrumbFactory";
        }
    public:

    };
}

TEST_F(TestDFMCrumbFactory, testCreate)
{
    DFMCrumbInterface *pobject = DFM_NAMESPACE::DFMCrumbFactory::create("video/*");
    EXPECT_EQ(pobject, nullptr);
}

TEST_F(TestDFMCrumbFactory, testKeys)
{
    QStringList lst = DFM_NAMESPACE::DFMCrumbFactory::keys();
    EXPECT_EQ(lst.count(), 0);
}
