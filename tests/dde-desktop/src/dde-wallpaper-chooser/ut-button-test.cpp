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
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QTest>

#include <QEventLoop>
#include <QObject>
#include <QTimer>

#define private public
#define protected public

#include "../dde-wallpaper-chooser/button.h"

using namespace testing;

namespace  {
     class ButtonTest : public Test {
     public:
         ButtonTest() : Test()
         {

         }

         virtual void SetUp() override
         {
             m_button = new Button;
         }

         virtual void TearDown() override
         {
             delete m_button;
         }

         Button *m_button = nullptr;
     };
}

TEST_F(ButtonTest, key_pressevent_of_space)
{
    bool bjudge = false;
    QObject::connect(m_button, &Button::clicked, m_button, [&]{
        bjudge = true;
    });
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    m_button->keyPressEvent(&event);
    EXPECT_TRUE(bjudge);
}
