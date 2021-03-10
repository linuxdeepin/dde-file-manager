/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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

#define private public
#include "views/dfmvaultactiveview.h"


namespace  {
    class TestDFMVaultActiveView : public testing::Test
    {
    public:
        DFMVaultActiveView* m_view;

        virtual void SetUp() override
        {
            m_view = DFMVaultActiveView::getInstance();
            m_view->show();
            std::cout << "start TestDFMVaultActiveView" << std::endl;
        }

        virtual void TearDown() override
        {
            m_view->close();
            std::cout << "end TestDFMVaultActiveView" << std::endl;
        }
    };
}


TEST_F(TestDFMVaultActiveView, tst_slotNextWidget)
{
    EXPECT_NO_FATAL_FAILURE(m_view->slotNextWidget());
}
