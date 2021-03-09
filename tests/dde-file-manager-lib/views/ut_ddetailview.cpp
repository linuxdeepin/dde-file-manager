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

#include "views/ddetailview.h"


namespace  {
    class TestDDetailView : public testing::Test
    {
    public:
        QSharedPointer<DDetailView> m_detailView;

        virtual void SetUp() override
        {
            m_detailView = QSharedPointer<DDetailView>(new DDetailView());

            std::cout << "start TestDDetailView" << std::endl;
        }

        virtual void TearDown() override
        {
            std::cout << "end TestDDetailView" << std::endl;
        }
    };
}

TEST_F(TestDDetailView, can_create_obj)
{
    ASSERT_NE(nullptr, m_detailView);
}

