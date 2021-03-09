/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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

#include <QAction>

#include "views/dfmactionbutton.h"
namespace  {
    class DFMActionButtonTest : public testing::Test
    {
    public:        
        virtual void SetUp() override
        {
            m_button = new DFMActionButton;
        }

        virtual void TearDown() override
        {
            delete m_button;
        }
        DFMActionButton *m_button;
    };
}

TEST_F(DFMActionButtonTest,set_current_action)
{
    ASSERT_NE(nullptr,m_button);

    QAction action("test");
    m_button->setAction(&action);

    QAction *result = m_button->action();
    EXPECT_EQ(result, &action);

}

TEST_F(DFMActionButtonTest,get_current_action)
{
    ASSERT_NE(nullptr,m_button);

    QAction action("copy");
    m_button->setAction(&action);

    QAction *result = m_button->action();
    EXPECT_EQ(result, &action);
}



