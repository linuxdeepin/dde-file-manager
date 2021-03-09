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

#include <QFileInfo>
#include <QThread>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QPixmap>
#include <QEventLoop>
#include <QObject>
#include <QTimer>

#define private public

#include "../dde-wallpaper-chooser/checkbox.h"

using namespace testing;

namespace {
      class CheckBoxTest : public Test {
      public:
          CheckBoxTest() : Test() {}

          virtual void SetUp() override
          {
              m_box = new CheckBox;
              m_box1 = new CheckBox("test");
          }

          virtual void TearDown() override
          {
              delete m_box;
              delete m_box1;
          }

          CheckBox *m_box = nullptr;
          CheckBox *m_box1 = nullptr;
      };
}
TEST_F(CheckBoxTest, checkbox)
{
    EXPECT_EQ(m_box1->text(), "test");
    EXPECT_NE(m_box, nullptr);
}

TEST_F(CheckBoxTest, key_pressevent_of_enter)
{
    m_box->setChecked(false);
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
    m_box->keyPressEvent(&event);
    EXPECT_EQ(m_box->isChecked(), true);

    m_box->setChecked(true);
    m_box->keyPressEvent(&event);
    EXPECT_FALSE(m_box->isChecked());
}
