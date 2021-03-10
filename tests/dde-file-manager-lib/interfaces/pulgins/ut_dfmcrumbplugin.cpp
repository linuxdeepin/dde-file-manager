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

#include "interfaces/plugins/dfmcrumbplugin.h"

namespace  {

    class A : public DFM_NAMESPACE::DFMCrumbPlugin
    {
    public:
        A(QObject *parent = 0)
            : DFM_NAMESPACE::DFMCrumbPlugin(parent)
        {
        }
        DFM_NAMESPACE::DFMCrumbInterface *create(const QString &key)
        {
        }
    };

    class TestDFMCrumbPlugin : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new A();
            std::cout << "start TestDFMCrumbPlugin";
        }
        void TearDown() override
        {
            if(m_pTester)
                delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestDFMCrumbPlugin";
        }
    public:
        A *m_pTester;
    };
}

TEST_F(TestDFMCrumbPlugin, testInit)
{
    EXPECT_NE(m_pTester, nullptr);
}
