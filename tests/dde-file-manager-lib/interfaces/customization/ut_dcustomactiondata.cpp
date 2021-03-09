/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
 *             zhangyu<zhangyub@uniontech.com>
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

#define private public
#define protected public
#include "dcustomactiondefine.h"
#include "dcustomactiondata.h"

TEST(DCustomActionData, dcustomactiondata_copy_constructor)
{
    DCustomActionData tempOld;
    tempOld.m_name = "菜单名字";
    DCustomActionData tempNew(tempOld);
    EXPECT_TRUE("菜单名字" == tempNew.name());
}

TEST(DCustomActionData, dcustomactiondata_copy_operator)
{
    DCustomActionData tempOld;
    tempOld.m_name = "菜单名字";
    DCustomActionData tempNew;
    tempNew = tempOld;
    EXPECT_TRUE("菜单名字" == tempNew.name());
}

TEST(DCustomActionData, is_menu)
{
     DCustomActionData tempOld;
     DCustomActionData tempNew;

     EXPECT_FALSE(tempOld.isMenu());
     tempOld.m_childrenActions.append(tempNew);
     EXPECT_TRUE(tempOld.isMenu());
}

TEST(DCustomActionData, is_action)
{
     DCustomActionData tempOld;
     DCustomActionData tempNew;

     EXPECT_TRUE(tempOld.isAction());
     tempOld.m_childrenActions.append(tempNew);
     EXPECT_FALSE(tempOld.isAction());
}

TEST(DCustomActionData, test_name)
{
    DCustomActionData temp;
    temp.m_name = "菜单名字";
    EXPECT_TRUE("菜单名字" == temp.name());
}

TEST(DCustomActionData, test_position_no_combotype)
{
    DCustomActionData temp;
    temp.m_position = 1;
    EXPECT_TRUE(1 == temp.position());
}

TEST(DCustomActionData, test_position_combotype)
{
    DCustomActionData temp;
    QMap<DCustomActionDefines::ComboType,int> comboPos;
    temp.m_position = 1;
    EXPECT_TRUE(1 == temp.position(DCustomActionDefines::ComboType::SingleDir));
    comboPos.insert(DCustomActionDefines::ComboType::SingleDir, 2);
    temp.m_comboPos = comboPos;
    EXPECT_TRUE(2 == temp.position(DCustomActionDefines::ComboType::SingleDir));
}

TEST(DCustomActionData, test_icon)
{
    DCustomActionData temp;
    temp.m_icon = "/home/utTest/xxx.icon";
    EXPECT_TRUE("/home/utTest/xxx.icon" == temp.icon());
}

TEST(DCustomActionData, test_command)
{
    DCustomActionData temp;
    temp.m_command = "/home/utTest/xxx.run";
    EXPECT_TRUE("/home/utTest/xxx.run" == temp.command());
}

TEST(DCustomActionData, test_separator)
{
    DCustomActionData temp;
    temp.m_separator = DCustomActionDefines::Separator::Top;
    EXPECT_TRUE(DCustomActionDefines::Separator::Top == temp.separator());
}

TEST(DCustomActionData, test_acitons)
{
    DCustomActionData temp;
    QList<DCustomActionData> tempActions;
    EXPECT_TRUE(0 == temp.acitons().size());
    DCustomActionData tp1;
    DCustomActionData tp2;
    tempActions.append(tp1);
    tempActions.append(tp2);
    temp.m_childrenActions = tempActions;
    EXPECT_TRUE(2 == temp.acitons().size());

}

TEST(DCustomActionData, test_name_Arg)
{
    DCustomActionData temp;
    temp.m_nameArg = DCustomActionDefines::ActionArg::FileName;
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == temp.nameArg());
}

TEST(DCustomActionData, test_command_arg)
{
    DCustomActionData temp;
    temp.m_cmdArg = DCustomActionDefines::ActionArg::FilePath;
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == temp.commandArg());
}

TEST(DCustomActionEntry, dcustomactionentry_copy_constructor)
{
    DCustomActionEntry tempOld;
    tempOld.m_version = "Uos1.0";
    DCustomActionEntry tempNew(tempOld);
    EXPECT_TRUE("Uos1.0" == tempNew.version());
}

TEST(DCustomActionEntry, dcustomactionentry_copy_operator)
{
    DCustomActionEntry tempOld;
    tempOld.m_version = "Uos1.0";
    DCustomActionEntry tempNew;
    tempNew = tempOld;
    EXPECT_TRUE("Uos1.0" == tempNew.version());
}

TEST(DCustomActionEntry, test_package)
{
    DCustomActionEntry temp;
    temp.m_package = "xxx.conf";
    EXPECT_TRUE("xxx.conf" == temp.package());
}

TEST(DCustomActionEntry, test_version)
{
    DCustomActionEntry temp;
    temp.m_version= "Uos1.0";
    EXPECT_TRUE("Uos1.0" == temp.version());
}

TEST(DCustomActionEntry, test_comment)
{
    DCustomActionEntry temp;
    temp.m_comment = "this is a comment test!!!";
    EXPECT_TRUE("this is a comment test!!!" == temp.comment());
}

TEST(DCustomActionEntry, test_file_combo)
{
    DCustomActionEntry temp;
    temp.m_fileCombo = DCustomActionDefines::ComboType::AllFile;
    EXPECT_TRUE(DCustomActionDefines::ComboType::AllFile == temp.fileCombo());
}

TEST(DCustomActionEntry, test_mimetypes)
{
    DCustomActionEntry temp;
    EXPECT_TRUE(temp.mimeTypes().isEmpty());
    QStringList mimetype{"type_one", "type_two", "type_three"};
    temp.m_mimeTypes = mimetype;
    EXPECT_TRUE("type_three" == temp.mimeTypes().at(2));
}

TEST(DCustomActionEntry, test_data)
{
    DCustomActionEntry temp;
    EXPECT_TRUE(temp.data().name().isEmpty());
    DCustomActionData tpData;
    tpData.m_name = "一级菜单";
    temp.m_data = tpData;
    EXPECT_TRUE("一级菜单" == temp.data().name());
}

