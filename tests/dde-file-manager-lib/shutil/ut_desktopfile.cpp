/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     max-lv<lvwujun@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#include "shutil/desktopfile.h"

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <memory>
#include <QDir>
#include <QDebug>

namespace  {
    class TestDesktopFile : public testing::Test {
    public:
        void SetUp() override
        {
        }
        void TearDown() override
        {
        }

    };
}

TEST_F(TestDesktopFile, can_parse_the_desktop_computer)
{
    QString desktop_test_file = QDir::homePath() + "/Desktop/dde-computer.desktop";
    DesktopFile computerfile(desktop_test_file);
    computerfile.getCategories();
    computerfile.getMimeType();
    EXPECT_EQ(desktop_test_file, computerfile.getFileName());
    EXPECT_EQ("", computerfile.getName());
    EXPECT_EQ("computer", computerfile.getIcon());
    EXPECT_EQ("Application", computerfile.getType());
    EXPECT_EQ("dde-computer", computerfile.getDeepinId());
    EXPECT_TRUE(computerfile.getExec().contains("computer:///"));
    EXPECT_TRUE(computerfile.getPureFileName().contains("dde-computer"));
    EXPECT_TRUE(!computerfile.getLocalName().isEmpty());
    EXPECT_TRUE(!computerfile.getDisplayName().isEmpty());
    EXPECT_FALSE(computerfile.getNoShow());
}

TEST_F(TestDesktopFile, can_parse_the_desktop_trash)
{
    QString desktop_test_file = QDir::homePath() + "/Desktop/dde-trash.desktop";
    DesktopFile trashfile(desktop_test_file);
    trashfile.getCategories();
    trashfile.getMimeType();
    EXPECT_EQ(desktop_test_file, trashfile.getFileName());
    EXPECT_EQ("", trashfile.getName());
    EXPECT_EQ("user-trash", trashfile.getIcon());
    EXPECT_EQ("Application", trashfile.getType());
    EXPECT_EQ("dde-trash", trashfile.getDeepinId());
    EXPECT_TRUE(trashfile.getExec().contains("trash:///"));
    EXPECT_TRUE(trashfile.getPureFileName().contains("dde-trash"));
    EXPECT_TRUE(!trashfile.getLocalName().isEmpty());
    EXPECT_TRUE(!trashfile.getDisplayName().isEmpty());
    EXPECT_FALSE(trashfile.getNoShow());
}
