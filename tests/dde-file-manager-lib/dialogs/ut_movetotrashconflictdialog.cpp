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

#include "dialogs/movetotrashconflictdialog.h"
#include "stub.h"
#include "dfmglobal.h"

#include <gtest/gtest.h>

namespace  {
    class TestMoveToTrashConflictDialog : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new MoveToTrashConflictDialog();
            std::cout << "start TestMoveToTrashConflictDialog";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestMoveToTrashConflictDialog";
        }
    public:
        MoveToTrashConflictDialog   *m_pTester;
    };
}

TEST_F(TestMoveToTrashConflictDialog, testInit)
{
    EXPECT_NE(m_pTester, nullptr);
}

TEST_F(TestMoveToTrashConflictDialog, testInit2)
{
    bool(*stub_isWayLand)() = []()->bool{
        return true;
    };
    Stub stu;
    stu.set(ADDR(DFMGlobal, isWayLand), stub_isWayLand);

    DUrlList lst;
    lst << DUrl("file:///jerry");
    MoveToTrashConflictDialog dlg(nullptr, lst);
    QString str = dlg.title();
    EXPECT_TRUE(str == "This file is too big for the trash");
}

