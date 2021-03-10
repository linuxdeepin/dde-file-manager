/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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
#include <DTitlebar>

#include "stub.h"

#include "dfmglobal.h"
#define private public
#include "dialogs/basedialog.h"

namespace  {
    class TestBaseDialog : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new BaseDialog();
            std::cout << "start TestBaseDialog";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestBaseDialog";
        }
    public:
        BaseDialog *m_pTester;
    };
}

TEST_F(TestBaseDialog, testInit)
{
    bool(*stub_isWayLand)() = []()->bool{
        return true;
    };
    Stub stu;
    stu.set(ADDR(DFMGlobal, isWayLand), stub_isWayLand);
    BaseDialog dlg;
    EXPECT_NE(nullptr, m_pTester->m_titlebar);
}

TEST_F(TestBaseDialog, testSetTitle)
{
    QString strTitle("TitleName");
    m_pTester->setTitle(strTitle);
    QString str = m_pTester->m_titlebar->windowTitle();
    EXPECT_TRUE(str.isEmpty());
}
