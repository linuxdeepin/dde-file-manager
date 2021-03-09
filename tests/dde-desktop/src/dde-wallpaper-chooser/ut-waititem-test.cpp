/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xinglinkun<xinglinkun@uniontech.com>
 *
 * Maintainer: xinglinkun<xinglinkun@uniontech.com>
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
#include <QTest>
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QLabel>
#define private public

#include "../dde-wallpaper-chooser/waititem.h"
#include "../dde-wallpaper-chooser/waititem.cpp"

using namespace testing;

namespace  {
       class WaitItemTest : public Test {
       public:

           void SetUp() override {
               if (m_testitem == nullptr)
               m_testitem = new WaitItem;
           }

           void TearDown() override {
               if (m_testitem != nullptr)
                   delete m_testitem;
               m_testitem = nullptr;
           }

       private:
           WaitItem* m_testitem = nullptr;
       };
}

TEST_F(WaitItemTest, initSize)
{
    ASSERT_TRUE(m_testitem->m_icon == nullptr);
    ASSERT_TRUE(m_testitem->m_contant == nullptr);
    ASSERT_TRUE(m_testitem->m_animationSpinner == nullptr);

    m_testitem->initSize(QSize(400, 400));
    m_testitem->m_movedistance = m_testitem->width() * m_testitem->m_proportion;
    int sumwidth = m_testitem->m_iconsize.width() + m_testitem->m_contantsize.width() + WAITITEM_ICON_CONTANT_SPACE;
    if ((m_testitem->width() - static_cast<int>(m_testitem->m_movedistance)) < sumwidth) {
        int temp = sumwidth - (m_testitem->width() - static_cast<int>(m_testitem->m_movedistance));
        m_testitem->m_movedistance -= temp;
    }
    int temp = static_cast<int>(m_testitem->m_movedistance);

    EXPECT_EQ(m_testitem->size(), QSize(400, 400));
    EXPECT_EQ(temp, static_cast<int>(m_testitem->m_movedistance));
}

TEST_F(WaitItemTest, setContantText)
{
    ASSERT_TRUE(m_testitem->m_icon == nullptr);
    ASSERT_TRUE(m_testitem->m_contant == nullptr);
    ASSERT_TRUE(m_testitem->m_animationSpinner == nullptr);

    m_testitem->initSize(QSize(400, 400));
    m_testitem->setContantText("test");
    QLabel* temp = m_testitem->m_contant;

    EXPECT_EQ(temp->text(),QString("test"));
}

