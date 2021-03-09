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

#define private public
#include "views/dfmadvancesearchbar.h"
DFM_USE_NAMESPACE

namespace  {
    class DFMAdvanceSearchBarTest : public testing::Test
    {
    public:        
        virtual void SetUp() override
        {
            m_bar = new DFMAdvanceSearchBar;
        }

        virtual void TearDown() override
        {
            delete m_bar;
        }
        DFMAdvanceSearchBar *m_bar;
    };
}

TEST_F(DFMAdvanceSearchBarTest,reset_form)
{
    ASSERT_NE(nullptr,m_bar);

    bool updateView = true;
    m_bar->resetForm(updateView);
    EXPECT_TRUE(m_bar->allowUpdateView);
}

TEST_F(DFMAdvanceSearchBarTest,slots_option_changed)
{
    ASSERT_NE(nullptr,m_bar);

    bool oldValue = m_bar->needSearchAgain;
    m_bar->onOptionChanged();
    bool result = m_bar->needSearchAgain;
    EXPECT_NE(oldValue, result);
}

TEST_F(DFMAdvanceSearchBarTest,slots_resetbutton_pressed)
{
    ASSERT_NE(nullptr,m_bar);
    m_bar->onResetButtonPressed();
    EXPECT_TRUE(m_bar->allowUpdateView);
}

