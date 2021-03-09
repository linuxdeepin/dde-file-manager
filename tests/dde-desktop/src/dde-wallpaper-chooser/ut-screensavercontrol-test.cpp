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
#include <QDebug>
#include <QProcess>
#include <QDBusConnection>
#include <QDBusMessage>
#include <dgiosettings.h>
#include <QString>
#include <QProcessEnvironment>
#include "../dde-wallpaper-chooser/screensavercontrol.h"
#include "stubext.h"
using namespace testing;
using namespace ScreenSaverCtrlFunction;

TEST(ScreenSaverCtrlTest, needShowScreensaver)
{
    stub_ext::StubExt stu;
    QString str("NN");
    QStringList strlist;
    strlist << QString("show-screen-saver");
    stu.set_lamda(ADDR(DGioSettings, keys), [&strlist](){return strlist;});
    stu.set_lamda(ADDR(DGioSettings, value), [](){return false;});
    bool ret = needShowScreensaver();
    EXPECT_FALSE(ret);

    stu.set_lamda(ADDR(QDBusMessage, type), [](){return QDBusMessage::MessageType::InvalidMessage;});
    ret = needShowScreensaver();
    EXPECT_FALSE(ret);

    stu.set_lamda(ADDR(QProcessEnvironment, contains),[](){return true;});
    stu.set_lamda(ADDR(QProcessEnvironment, value), [&str](){return str;});
    ret = needShowScreensaver();
    EXPECT_EQ(ret, false);
}
