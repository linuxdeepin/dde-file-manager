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

#include "stub.h"
#include "dfmglobal.h"
#include "usershare/usersharemanager.h"

#include <gtest/gtest.h>

#define private public
#include "dialogs/usersharepasswordsettingdialog.h"

namespace  {
    class TestUserSharePasswordSettingDialog : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new UserSharePasswordSettingDialog();
            std::cout << "start TestUserSharePasswordSettingDialog";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestUserSharePasswordSettingDialog";
        }
    public:
        UserSharePasswordSettingDialog  *m_pTester;
    };
}

TEST_F(TestUserSharePasswordSettingDialog, testInit)
{
    EXPECT_NE(m_pTester, nullptr);
}

TEST_F(TestUserSharePasswordSettingDialog, testOnButtonClicked)
{
    void(*stub_setSambaPassword)(const QString&, const QString&) = [](const QString&, const QString&){};
    Stub stu;
    stu.set(ADDR(UserShareManager, setSambaPassword), stub_setSambaPassword);
    EXPECT_NO_FATAL_FAILURE(m_pTester->onButtonClicked(1));
}

TEST_F(TestUserSharePasswordSettingDialog, testOnButtonClicked2)
{
    DPasswordEdit edit;
    edit.setText("123");
    m_pTester->m_passwordEdit = &edit;

    void(*stub_setSambaPassword)(const QString&, const QString&) = [](const QString&, const QString&){

    };
    Stub stu;
    stu.set(ADDR(UserShareManager, setSambaPassword), stub_setSambaPassword);

    EXPECT_NO_FATAL_FAILURE(m_pTester->onButtonClicked(1));
}

TEST_F(TestUserSharePasswordSettingDialog, testInitUI)
{
    bool(*stub_isWayLand)() = []()->bool{
        return true;
    };
    Stub stu;
    stu.set(ADDR(DFMGlobal, isWayLand), stub_isWayLand);

    m_pTester->initUI();
    int width = m_pTester->width();
    EXPECT_EQ(width, 390);
}
