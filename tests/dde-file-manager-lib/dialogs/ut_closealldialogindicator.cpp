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
#include <QTest>
#include <QDebug>

#define protected public
#define private public
#include "dialogs/closealldialogindicator.h"

namespace  {
    class TestCloseAllDialogIndicator : public testing::Test{
    public:
        void SetUp() override
        {
            m_pTester = new CloseAllDialogIndicator();
            std::cout << "start TestCloseAllDialogIndicator";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestCloseAllDialogIndicator";
        }
    public:
        CloseAllDialogIndicator *m_pTester;
    };
}

TEST_F(TestCloseAllDialogIndicator, testInit)
{
    EXPECT_NE(nullptr, m_pTester);
}

TEST_F(TestCloseAllDialogIndicator, testSetTotalMessage)
{
    qint64 size = 1024;
    int count = 2048;
    m_pTester->setTotalMessage(size, count);
    QString str = m_pTester->m_messageLabel->text();
    EXPECT_NE("", str.toStdString().c_str());
}

TEST_F(TestCloseAllDialogIndicator, testKeyPressEvent_Escape)
{
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(m_pTester->keyPressEvent(&event));
}

TEST_F(TestCloseAllDialogIndicator, testKeyPressEvent_Escape2)
{
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(m_pTester->keyPressEvent(&event));
}

TEST_F(TestCloseAllDialogIndicator, testShowEvent)
{
    QShowEvent event;
    EXPECT_NO_FATAL_FAILURE(m_pTester->showEvent(&event));
}
