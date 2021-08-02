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
#include <gmock/gmock-matchers.h>
#include <stub.h>
#include <stubext.h>

#include <QAction>

#define private public
#define protected public
#include "customization/dcustomactionbuilder.h"
#include "interfaces/dfileservices.h"
#include "models/dfmrootfileinfo.h"

TEST(DCustomActionBuilder, set_active_dir_empty)
{
    DCustomActionBuilder builder;
    DUrl url("");
    builder.setActiveDir(url);
    EXPECT_EQ(builder.m_dirName, "");
    EXPECT_EQ(builder.m_dirPath, url);

    url = DUrl("file://");
    builder.setActiveDir(url);
    EXPECT_EQ(builder.m_dirName, "");
    EXPECT_EQ(builder.m_dirPath, url);
}

TEST(DCustomActionBuilder, set_active_dir_root)
{
    DCustomActionBuilder builder;
    DUrl url("file:///");
    builder.setActiveDir(url);
    EXPECT_EQ(builder.m_dirName, "/");
    EXPECT_EQ(builder.m_dirPath, url);
}

TEST(DCustomActionBuilder, set_active_dir_one)
{
    DCustomActionBuilder builder;

    DUrl url("file:///usr");
    builder.setActiveDir(url);
    EXPECT_EQ(builder.m_dirName, "usr");
    EXPECT_EQ(builder.m_dirPath, url);

    url = DUrl("file:///usr/");
    builder.setActiveDir(url);
    EXPECT_EQ(builder.m_dirName, "usr");
    EXPECT_EQ(builder.m_dirPath, url);
}

TEST(DCustomActionBuilder, set_active_dir_two)
{
    DCustomActionBuilder builder;

    DUrl url("file:///usr/bin");
    builder.setActiveDir(url);
    EXPECT_EQ(builder.m_dirName, "bin");
    EXPECT_EQ(builder.m_dirPath, url);

    url = DUrl("file:///usr/bin/");
    builder.setActiveDir(url);
    EXPECT_EQ(builder.m_dirName, "bin");
    EXPECT_EQ(builder.m_dirPath, url);
}

TEST(DCustomActionBuilder, set_foucus_file_empty)
{
    DCustomActionBuilder builder;

    DUrl url("");
    builder.setFocusFile(url);
    EXPECT_EQ(builder.m_fileFullName, "");
    EXPECT_EQ(builder.m_fileBaseName, "");
    EXPECT_EQ(builder.m_filePath, url);

    url = DUrl("file://");
    builder.setFocusFile(url);
    EXPECT_EQ(builder.m_fileFullName, "");
    EXPECT_EQ(builder.m_fileBaseName, "");
    EXPECT_EQ(builder.m_filePath, url);
}

TEST(DCustomActionBuilder, set_foucus_file_hidden)
{
    DCustomActionBuilder builder;

    DUrl url("file:///.tar.gz");
    builder.setFocusFile(url);
    EXPECT_EQ(builder.m_fileFullName, ".tar.gz");
    EXPECT_EQ(builder.m_fileBaseName, ".tar");
    EXPECT_EQ(builder.m_filePath, url);

    url = DUrl("file:///.tar");
    builder.setFocusFile(url);
    EXPECT_EQ(builder.m_fileFullName, ".tar");
    EXPECT_EQ(builder.m_fileBaseName, ".tar");
    EXPECT_EQ(builder.m_filePath, url);
}

TEST(DCustomActionBuilder, set_foucus_file_zero)
{
    DCustomActionBuilder builder;

    DUrl url("file://usr/bin/dde-desktop");
    builder.setFocusFile(url);
    EXPECT_EQ(builder.m_fileFullName, "dde-desktop");
    EXPECT_EQ(builder.m_fileBaseName, "dde-desktop");
    EXPECT_EQ(builder.m_filePath, url);
}

TEST(DCustomActionBuilder, set_foucus_file_one)
{
    DCustomActionBuilder builder;

    DUrl url("file://usr/bin/dde-desktop.run");
    builder.setFocusFile(url);
    EXPECT_EQ(builder.m_fileFullName, "dde-desktop.run");
    EXPECT_EQ(builder.m_fileBaseName, "dde-desktop.run");
    EXPECT_EQ(builder.m_filePath, url);
}

TEST(DCustomActionBuilder, set_foucus_file_two)
{
    DCustomActionBuilder builder;

    DUrl url("file:///usr/bin/dde-desktop.tar.gz");
    builder.setFocusFile(url);
    EXPECT_EQ(builder.m_fileFullName, "dde-desktop.tar.gz");
    EXPECT_EQ(builder.m_fileBaseName, "dde-desktop");
    EXPECT_EQ(builder.m_filePath, url);

    url = DUrl("file:///usr/bin/.dde-desktop.tar.gz");
    builder.setFocusFile(url);
    EXPECT_EQ(builder.m_fileFullName, ".dde-desktop.tar.gz");
    EXPECT_EQ(builder.m_fileBaseName, ".dde-desktop");
    EXPECT_EQ(builder.m_filePath, url);

    url = DUrl("file:///usr/bin");
    builder.setFocusFile(url);
    EXPECT_EQ(builder.m_fileFullName, "bin");
    EXPECT_EQ(builder.m_fileBaseName, "bin");
    EXPECT_EQ(builder.m_filePath, url);

    url = DUrl("file:///usr/.bin");
    builder.setFocusFile(url);
    EXPECT_EQ(builder.m_fileFullName, ".bin");
    EXPECT_EQ(builder.m_fileBaseName, ".bin");
    EXPECT_EQ(builder.m_filePath, url);

    url = DUrl("file:///ut_test.7z.001");
    builder.setFocusFile(url);
    EXPECT_EQ(builder.m_fileFullName, "ut_test.7z.001");
    EXPECT_EQ(builder.m_fileBaseName, "ut_test");
    EXPECT_EQ(builder.m_filePath, url);

    url = DUrl("file:///.ut_test.7z.001");
    builder.setFocusFile(url);
    EXPECT_EQ(builder.m_fileFullName, ".ut_test.7z.001");
    EXPECT_EQ(builder.m_fileBaseName, ".ut_test");
    EXPECT_EQ(builder.m_filePath, url);

    url = DUrl("file:///ut_test.txt");
    builder.setFocusFile(url);
    EXPECT_EQ(builder.m_fileFullName, "ut_test.txt");
    EXPECT_EQ(builder.m_fileBaseName, "ut_test");
    EXPECT_EQ(builder.m_filePath, url);

    url = DUrl("file:///.ut_test.txt");
    builder.setFocusFile(url);
    EXPECT_EQ(builder.m_fileFullName, ".ut_test.txt");
    EXPECT_EQ(builder.m_fileBaseName, ".ut_test");
    EXPECT_EQ(builder.m_filePath, url);

    url = DUrl("file:///ut_test.tar.gz");
    builder.setFocusFile(url);
    EXPECT_EQ(builder.m_fileFullName, "ut_test.tar.gz");
    EXPECT_EQ(builder.m_fileBaseName, "ut_test");
    EXPECT_EQ(builder.m_filePath, url);

    url = DUrl("file:///.ut_test.tar.gz");
    builder.setFocusFile(url);
    EXPECT_EQ(builder.m_fileFullName, ".ut_test.tar.gz");
    EXPECT_EQ(builder.m_fileBaseName, ".ut_test");
    EXPECT_EQ(builder.m_filePath, url);

    //底层返回的是rar,不应该做特殊处理所以这是ut_test.part1
    url = DUrl("file:///ut_test.part1.rar");
    builder.setFocusFile(url);
    EXPECT_EQ(builder.m_fileFullName, "ut_test.part1.rar");
    EXPECT_EQ(builder.m_fileBaseName, "ut_test.part1");
    EXPECT_EQ(builder.m_filePath, url);
}

TEST(DCustomActionBuilder, get_Complete_Suffix)
{
    DCustomActionBuilder builder;

    EXPECT_EQ(builder.getCompleteSuffix("test.7z.001", "7z.*"), "7z.001");
    EXPECT_EQ(builder.getCompleteSuffix(".test.7z.001", "7z.*"), "7z.001");
    EXPECT_EQ(builder.getCompleteSuffix("test.tar.gz", "tar.gz"), "tar.gz");
    EXPECT_EQ(builder.getCompleteSuffix(".test.tar.gz", "tar.gz"), "tar.gz");
    EXPECT_EQ(builder.getCompleteSuffix("test.txt", "txt"), "txt");
    EXPECT_EQ(builder.getCompleteSuffix(".test.txt", "txt"), "txt");
    EXPECT_EQ(builder.getCompleteSuffix("test", ""), "");
    EXPECT_EQ(builder.getCompleteSuffix(".test", ""), "");
}

TEST(DCustomActionBuilder, make_name_none)
{
    DCustomActionBuilder builder;
    DUrl url("file:///usr");
    builder.setActiveDir(url);
    DUrl url2("file://usr/bin/dde-desktop.run");
    builder.setFocusFile(url2);

    QString name = "name";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::NoneArg), name);
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::DirName), name);

    name = "name %d";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::NoneArg), name);

    name = "name %a";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::NoneArg), name);

    name = "name %b";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::NoneArg), name);
}

TEST(DCustomActionBuilder, make_name_dirname_empty)
{
    DCustomActionBuilder builder;
    DUrl url("file://");
    builder.setActiveDir(url);
    DUrl url2("file://usr/bin/dde-desktop.run");
    builder.setFocusFile(url2);

    QString name = "name %d";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::DirName), "name ");

    name = "name%d";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::DirName), "name");

    name = "%dname";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::DirName), "name");

    name = "na%dme";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::DirName), "name");

    name = "na%dm%de";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::DirName), "nam%de");

    name = "%dname %d";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::DirName), "name %d");

    name = "%dname %a%b";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::DirName), "name %a%b");
}

TEST(DCustomActionBuilder, make_name_dirname_one)
{
    DCustomActionBuilder builder;
    DUrl url("file:///usr");
    builder.setActiveDir(url);
    DUrl url2("file://usr/bin/dde-desktop.run");
    builder.setFocusFile(url2);

    QString name = "name %d";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::DirName), "name usr");

    name = "name%d";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::DirName), "nameusr");

    name = "%dname";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::DirName), "usrname");

    name = "na%dme";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::DirName), "nausrme");

    name = "na%dm%de";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::DirName), "nausrm%de");

    name = "%dname %d";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::DirName), "usrname %d");

    name = "%dname %a%b";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::DirName), "usrname %a%b");

    name = "%aname %d%b";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::DirName), "%aname usr%b");
}

TEST(DCustomActionBuilder, make_name_fullname_empty)
{
    DCustomActionBuilder builder;
    DUrl url("file:///usr");
    builder.setActiveDir(url);
    DUrl url2("file://");
    builder.setFocusFile(url2);

    QString name = "name %a";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::FileName), "name ");

    name = "name%a";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::FileName), "name");

    name = "%aname";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::FileName), "name");

    name = "na%ame";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::FileName), "name");

    name = "na%am%ae";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::FileName), "nam%ae");

    name = "%aname %a";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::FileName), "name %a");

    name = "%aname %a%b";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::FileName), "name %a%b");

    name = "%aname %d%b";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::FileName), "name %d%b");
}

TEST(DCustomActionBuilder, make_name_fullname_one)
{
    DCustomActionBuilder builder;
    DUrl url("file:///usr");
    builder.setActiveDir(url);
    DUrl url2("file://usr/bin/dde-desktop.run");
    builder.setFocusFile(url2);

    QString name = "name %a";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::FileName), "name dde-desktop.run");

    name = "name%a";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::FileName), "namedde-desktop.run");

    name = "%aname";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::FileName), "dde-desktop.runname");

    name = "na%ame";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::FileName), "nadde-desktop.runme");

    name = "na%am%ae";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::FileName), "nadde-desktop.runm%ae");

    name = "%aname %a";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::FileName), "dde-desktop.runname %a");

    name = "%aname %a%b";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::FileName), "dde-desktop.runname %a%b");

    name = "%dname %a%b";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::FileName), "%dname dde-desktop.run%b");
}


TEST(DCustomActionBuilder, make_name_basename_empty)
{
    DCustomActionBuilder builder;
    DUrl url("file:///usr");
    builder.setActiveDir(url);
    DUrl url2("file://");
    builder.setFocusFile(url2);

    QString name = "name %b";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::BaseName), "name ");

    name = "name%b";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::BaseName), "name");

    name = "%bname";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::BaseName), "name");

    name = "na%bme";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::BaseName), "name");

    name = "na%bm%be";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::BaseName), "nam%be");

    name = "%bname %b";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::BaseName), "name %b");

    name = "%bname %b%a";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::BaseName), "name %b%a");

    name = "%bname %d%a";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::BaseName), "name %d%a");
}

TEST(DCustomActionBuilder, make_name_basename_one)
{
    DCustomActionBuilder builder;
    DUrl url("file:///usr");
    builder.setActiveDir(url);
    DUrl url2("file://usr/bin/dde-desktop.run");
    builder.setFocusFile(url2);

    QString name = "name %b";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::BaseName), "name dde-desktop.run");

    name = "name%b";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::BaseName), "namedde-desktop.run");

    name = "%bname";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::BaseName), "dde-desktop.runname");

    name = "na%bme";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::BaseName), "nadde-desktop.runme");

    name = "na%bm%be";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::BaseName), "nadde-desktop.runm%be");

    name = "%bname %b";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::BaseName), "dde-desktop.runname %b");

    name = "%bname %b%a";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::BaseName), "dde-desktop.runname %b%a");

    name = "%dname %a%b";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::BaseName), "%dname %adde-desktop.run");
}

TEST(DCustomActionBuilder, split_cmd_empty)
{
    QString cmd = "";
    QStringList ret = DCustomActionBuilder::splitCommand(cmd);
    EXPECT_TRUE(ret.isEmpty());
}

TEST(DCustomActionBuilder, split_cmd_none_arg)
{
    QString cmd = "/usr/bin/dde-desktop";
    QStringList exp = {"/usr/bin/dde-desktop"};
    QStringList ret = DCustomActionBuilder::splitCommand(cmd);
    EXPECT_EQ(ret, exp);

    cmd = "/usr/bin/dde-desktop ";
    exp = QStringList({"/usr/bin/dde-desktop"});
    ret = DCustomActionBuilder::splitCommand(cmd);
    EXPECT_EQ(ret, exp);
}

TEST(DCustomActionBuilder, split_cmd_arg)
{
    QString cmd = "/usr/bin/dde-desktop --filedialog-only";
    QStringList exp = {"/usr/bin/dde-desktop","--filedialog-only"};
    QStringList ret = DCustomActionBuilder::splitCommand(cmd);
    EXPECT_EQ(ret, exp);

    cmd = "/usr/bin/dde-desktop  --filedialog-only ";
    exp = QStringList({"/usr/bin/dde-desktop","--filedialog-only"});
    ret = DCustomActionBuilder::splitCommand(cmd);
    EXPECT_EQ(ret, exp);
}

TEST(DCustomActionBuilder, split_cmd_arg_one)
{
    QString cmd = "/usr/bin/dde-desktop --filedialog-only %u";
    QStringList exp = {"/usr/bin/dde-desktop","--filedialog-only","%u"};
    QStringList ret = DCustomActionBuilder::splitCommand(cmd);
    EXPECT_EQ(ret, exp);

    cmd = "/usr/bin/dde-desktop  --filedialog-%u";
    exp = QStringList({"/usr/bin/dde-desktop","--filedialog-%u"});
    ret = DCustomActionBuilder::splitCommand(cmd);
    EXPECT_EQ(ret, exp);
}

TEST(DCustomActionBuilder, split_cmd_arg_quotes)
{
    QString cmd = "/usr/bin/dde-desktop \"--filedialog-only %u\"";
    QStringList exp = {"/usr/bin/dde-desktop","--filedialog-only %u"};
    QStringList ret = DCustomActionBuilder::splitCommand(cmd);
    EXPECT_EQ(ret, exp);

    cmd = "/usr/bin/dde-desktop \"--filedialog-%u end\" --test";
    exp = QStringList({"/usr/bin/dde-desktop","--filedialog-%u end","--test"});
    ret = DCustomActionBuilder::splitCommand(cmd);
    EXPECT_EQ(ret, exp);

    cmd = "/usr/bin/dde-desktop '--filedialog-%u end' --test";
    exp = QStringList({"/usr/bin/dde-desktop","--filedialog-%u end","--test"});
    ret = DCustomActionBuilder::splitCommand(cmd);
    EXPECT_EQ(ret, exp);
}

TEST(DCustomActionBuilder, make_command_empty)
{
    DUrl dir("file:///usr/bin");
    DUrl foucs("file:///usr/bin/dde-desktop");
    DUrlList selected = {DUrl("file:///usr/bin/dde-desktop"),DUrl("file:///usr/bin/dde-file-manager")};

    QString cmd = "";
    auto ret = DCustomActionBuilder::makeCommand(cmd, DCustomActionDefines::NoneArg, dir, foucs, selected);
    EXPECT_EQ(ret.first, "");
    EXPECT_EQ(ret.second, QStringList());
}

TEST(DCustomActionBuilder, make_command_none)
{
    DUrl dir("file:///usr/bin");
    DUrl foucs("file:///usr/bin/dde-desktop");
    DUrlList selected = {DUrl("file:///usr/bin/dde-desktop"),DUrl("file:///usr/bin/dde-file-manager")};

    QString cmd = "/usr/bin/dde-desktop --file --test";
    auto ret = DCustomActionBuilder::makeCommand(cmd, DCustomActionDefines::NoneArg, dir, foucs, selected);
    EXPECT_EQ(ret.first, "/usr/bin/dde-desktop");
    EXPECT_EQ(ret.second, QStringList({"--file","--test"}));

    cmd = "/usr/bin/dde-desktop --file --test %u";
    ret = DCustomActionBuilder::makeCommand(cmd, DCustomActionDefines::DirPath, dir, foucs, selected);
    EXPECT_EQ(ret.first, "/usr/bin/dde-desktop");
    EXPECT_EQ(ret.second, QStringList({"--file","--test","%u"}));
}


TEST(DCustomActionBuilder, make_command_dir)
{
    DUrl dir("file:///usr/bin");
    DUrl foucs("file:///usr/bin/dde-desktop");
    DUrlList selected = {DUrl("file:///usr/bin/dde-desktop"),DUrl("file:///usr/bin/dde-file-manager")};

    QString cmd = "/usr/bin/dde-desktop %p --file --test";
    auto ret = DCustomActionBuilder::makeCommand(cmd, DCustomActionDefines::DirPath, dir, foucs, selected);
    EXPECT_EQ(ret.first, "/usr/bin/dde-desktop");
    EXPECT_EQ(ret.second, QStringList({"/usr/bin","--file","--test"}));

    cmd = "/usr/bin/dde-desktop --file '--test %p'";
    ret = DCustomActionBuilder::makeCommand(cmd, DCustomActionDefines::DirPath, dir, foucs, selected);
    EXPECT_EQ(ret.first, "/usr/bin/dde-desktop");
    EXPECT_EQ(ret.second, QStringList({"--file","--test /usr/bin"}));
}

TEST(DCustomActionBuilder, make_command_file)
{
    DUrl dir("file:///usr/bin");
    DUrl foucs("file:///usr/bin/dde-desktop");
    DUrlList selected = {DUrl("file:///usr/bin/dde-desktop"),DUrl("file:///usr/bin/dde-file-manager")};

    QString cmd = "/usr/bin/dde-desktop --file %f --test";
    auto ret = DCustomActionBuilder::makeCommand(cmd, DCustomActionDefines::FilePath, dir, foucs, selected);
    EXPECT_EQ(ret.first, "/usr/bin/dde-desktop");
    EXPECT_EQ(ret.second, QStringList({"--file","/usr/bin/dde-desktop","--test"}));

    cmd = "/usr/bin/dde-desktop '--file%f' --test";
    ret = DCustomActionBuilder::makeCommand(cmd, DCustomActionDefines::FilePath, dir, foucs, selected);
    EXPECT_EQ(ret.first, "/usr/bin/dde-desktop");
    EXPECT_EQ(ret.second, QStringList({"--file/usr/bin/dde-desktop","--test"}));
}

TEST(DCustomActionBuilder, make_command_files)
{
    DUrl dir("file:///usr/bin");
    DUrl foucs("file:///usr/bin/dde-desktop");
    DUrlList selected = {DUrl("file:///usr/bin/dde-desktop"),DUrl("file:///usr/bin/dde-file-manager")};

    QString cmd = "/usr/bin/dde-desktop --file %F --test";
    auto ret = DCustomActionBuilder::makeCommand(cmd, DCustomActionDefines::FilePaths, dir, foucs, selected);
    EXPECT_EQ(ret.first, "/usr/bin/dde-desktop");
    EXPECT_EQ(ret.second, QStringList({"--file","/usr/bin/dde-desktop","/usr/bin/dde-file-manager","--test"}));

    cmd = "/usr/bin/dde-desktop '--file %F' --test";
    ret = DCustomActionBuilder::makeCommand(cmd, DCustomActionDefines::FilePaths, dir, foucs, selected);
    EXPECT_EQ(ret.first, "/usr/bin/dde-desktop");
    EXPECT_EQ(ret.second, QStringList({"--file %F","--test"}));
}

TEST(DCustomActionBuilder, make_command_file_url)
{
    DUrl dir("file:///usr/bin");
    DUrl foucs("file:///usr/bin/dde-desktop");
    DUrlList selected = {DUrl("file:///usr/bin/dde-desktop"),DUrl("file:///usr/bin/dde-file-manager")};

    QString cmd = "/usr/bin/dde-desktop --file %u --test";
    auto ret = DCustomActionBuilder::makeCommand(cmd, DCustomActionDefines::UrlPath, dir, foucs, selected);
    EXPECT_EQ(ret.first, "/usr/bin/dde-desktop");
    EXPECT_EQ(ret.second, QStringList({"--file","file:///usr/bin/dde-desktop","--test"}));

    cmd = "/usr/bin/dde-desktop '--file%u' --test";
    ret = DCustomActionBuilder::makeCommand(cmd, DCustomActionDefines::UrlPath, dir, foucs, selected);
    EXPECT_EQ(ret.first, "/usr/bin/dde-desktop");
    EXPECT_EQ(ret.second, QStringList({"--filefile:///usr/bin/dde-desktop","--test"}));
}

TEST(DCustomActionBuilder, make_command_files_url)
{
    DUrl dir("file:///usr/bin");
    DUrl foucs("file:///usr/bin/dde-desktop");
    DUrlList selected = {DUrl("file:///usr/bin/dde-desktop"),DUrl("file:///usr/bin/dde-file-manager")};

    QString cmd = "/usr/bin/dde-desktop --file %U --test";
    auto ret = DCustomActionBuilder::makeCommand(cmd, DCustomActionDefines::UrlPaths, dir, foucs, selected);
    EXPECT_EQ(ret.first, "/usr/bin/dde-desktop");
    EXPECT_EQ(ret.second, QStringList({"--file","file:///usr/bin/dde-desktop","file:///usr/bin/dde-file-manager","--test"}));

    cmd = "/usr/bin/dde-desktop '--file %U' --test";
    ret = DCustomActionBuilder::makeCommand(cmd, DCustomActionDefines::UrlPaths, dir, foucs, selected);
    EXPECT_EQ(ret.first, "/usr/bin/dde-desktop");
    EXPECT_EQ(ret.second, QStringList({"--file %U","--test"}));
}

TEST(DCustomActionBuilder, buildAciton)
{
    DCustomActionBuilder builder;
    DCustomActionData ad;

    Stub st;
    QAction *(*createAciton)() = [](){return new QAction();};
    QAction *(*createMenu)() = [](){return new QAction();};
    st.set(&DCustomActionBuilder::createAciton,createAciton);
    st.set(&DCustomActionBuilder::createMenu,createMenu);

    QAction *ac = builder.buildAciton(ad, nullptr);

    EXPECT_TRUE(ac != nullptr);

    delete ac;
    ac = nullptr;

    bool (*stub_isAction)() = [](){return true;};
    st.set(&DCustomActionData::isAction, stub_isAction);

    ac = builder.buildAciton(ad, nullptr);

    EXPECT_TRUE(ac != nullptr);

    delete ac;
    ac = nullptr;
}

TEST(DCustomActionBuilder, checkFileCombo_empty)
{
    EXPECT_EQ(DCustomActionDefines::BlankSpace, DCustomActionBuilder::checkFileCombo({}));
}

TEST(DCustomActionBuilder, checkFileCombo_onefile)
{
    DUrlList list;
    list.append({"file:///usr/bin/ls"});
    EXPECT_EQ(DCustomActionDefines::SingleFile, DCustomActionBuilder::checkFileCombo(list));

    list.append({""});
    EXPECT_EQ(DCustomActionDefines::SingleFile, DCustomActionBuilder::checkFileCombo(list));
}

TEST(DCustomActionBuilder, checkFileCombo_mulfile)
{
    DUrlList list;
    list.append({"file:///usr/bin/ls"});
    list.append({"file:///usr/bin/ls"});
    EXPECT_EQ(DCustomActionDefines::MultiFiles, DCustomActionBuilder::checkFileCombo(list));

    list.append({"file:///usr/bin/ls"});
    EXPECT_EQ(DCustomActionDefines::MultiFiles, DCustomActionBuilder::checkFileCombo(list));
}

TEST(DCustomActionBuilder, checkFileCombo_onedir)
{
    DUrlList list;
    list.append({"file:///usr/bin"});
    EXPECT_EQ(DCustomActionDefines::SingleDir, DCustomActionBuilder::checkFileCombo(list));

    list.append({""});
    EXPECT_EQ(DCustomActionDefines::SingleDir, DCustomActionBuilder::checkFileCombo(list));
}

TEST(DCustomActionBuilder, checkFileCombo_muldir)
{
    DUrlList list;
    list.append({"file:///usr/bin"});
    list.append({"file:///usr/bin"});
    EXPECT_EQ(DCustomActionDefines::MultiDirs, DCustomActionBuilder::checkFileCombo(list));

    list.append({"file:///usr/bin"});
    EXPECT_EQ(DCustomActionDefines::MultiDirs, DCustomActionBuilder::checkFileCombo(list));
}

TEST(DCustomActionBuilder, checkFileCombo_file_and_dir)
{
    DUrlList list;
    list.append({"file:///usr/bin"});
    list.append({"file:///usr/bin/ls"});
    EXPECT_EQ(DCustomActionDefines::FileAndDir, DCustomActionBuilder::checkFileCombo(list));

    list.append({"file:///usr/bin"});
    EXPECT_EQ(DCustomActionDefines::FileAndDir, DCustomActionBuilder::checkFileCombo(list));

    list.append({"file:///usr/bin/ls"});
    EXPECT_EQ(DCustomActionDefines::FileAndDir, DCustomActionBuilder::checkFileCombo(list));
}

TEST(DCustomActionBuilder, getIcon_invalid_path)
{
    DCustomActionBuilder build;
    QString iconName = "/temp/dsadsa.das.ds.ad.sad.sa.";
    QIcon icon = build.getIcon(iconName);
    EXPECT_TRUE(icon.name().isEmpty());
}

TEST(DCustomActionBuilder, createAciton)
{
    DCustomActionBuilder builder;
    DCustomActionData ad;
    ad.m_icon = "dde-file-manager";
    ad.m_name = "test";
    ad.m_command = "dde-file-manager %u";
    ad.m_cmdArg = DCustomActionDefines::UrlPath;

    auto ac = builder.createAciton(ad);
    ASSERT_NE(ac, nullptr);
    EXPECT_EQ(ac->text(), ad.m_name);
    EXPECT_TRUE(ac->property(DCustomActionDefines::kCustomActionFlag).toBool());
    EXPECT_EQ(ac->property(DCustomActionDefines::kCustomActionCommand).toString(), ad.m_command);
    EXPECT_EQ(ac->property(DCustomActionDefines::kCustomActionCommandArgFlag).toInt(), ad.m_cmdArg);
    delete ac;
}

TEST(DCustomActionBuilder, isMimeTypeSupport)
{
    QString tgMimetype {"type0"};
    QStringList sptMimeType {"type1","type2"};
    DCustomActionBuilder builder;

    EXPECT_FALSE(builder.isMimeTypeSupport(tgMimetype, sptMimeType));

    sptMimeType.append("type0");
    EXPECT_TRUE(builder.isMimeTypeSupport(tgMimetype, sptMimeType));
}

TEST(DCustomActionBuilder, isMimeTypeMatch)
{
    QStringList tgMimetype {"type0"};
    QStringList sptMimeType {"type1","type2"};
    DCustomActionBuilder builder;

    EXPECT_FALSE(builder.isMimeTypeMatch(tgMimetype, sptMimeType));

    sptMimeType.append("type0");
    EXPECT_TRUE(builder.isMimeTypeMatch(tgMimetype, sptMimeType));

    sptMimeType.clear();
    sptMimeType.append("*");
    EXPECT_TRUE(builder.isMimeTypeMatch(tgMimetype, sptMimeType));
}

TEST(DCustomActionBuilder, isSchemeSupport)
{
    DCustomActionBuilder builder;
    DCustomActionEntry entry;

    DUrl utUrlOne("file:///home/xxx/Desktop/zip.001");
    DUrl utUrlTwo("trush:///home/xxx/Desktop/zip.001");


    EXPECT_TRUE(builder.isSchemeSupport(entry, utUrlOne));
    EXPECT_TRUE(builder.isSchemeSupport(entry, utUrlTwo));

    entry.m_supportSchemes.append("*");
    EXPECT_TRUE(builder.isSchemeSupport(entry, utUrlOne));
    EXPECT_TRUE(builder.isSchemeSupport(entry, utUrlTwo));

    entry.m_supportSchemes.clear();
    entry.m_supportSchemes.append("file");
    EXPECT_TRUE(builder.isSchemeSupport(entry, utUrlOne));
    EXPECT_FALSE(builder.isSchemeSupport(entry, utUrlTwo));

    entry.m_supportSchemes.append("*");
    EXPECT_TRUE(builder.isSchemeSupport(entry, utUrlOne));
    EXPECT_TRUE(builder.isSchemeSupport(entry, utUrlTwo));

}

TEST(DCustomActionBuilder, isSuffixSupport)
{
    DCustomActionBuilder builder;
    DCustomActionEntry entry;
    DUrl utUrl("usr/bin/dde-desktop");
    EXPECT_TRUE(builder.isSuffixSupport(entry, utUrl));

    DAbstractFileInfoPointer(*tempFunc)(const QObject *, const DUrl &) = [](const QObject *, const DUrl &) ->DAbstractFileInfoPointer {
        return DAbstractFileInfoPointer(new DAbstractFileInfo(DUrl("usr/bin/dde-desktop")));
    };
    Stub stub;
    stub.set(ADDR(DFileService, createFileInfo), tempFunc);

    EXPECT_TRUE(builder.isSuffixSupport(entry, utUrl));
    entry.m_supportSuffix.append("7z.*");
    EXPECT_FALSE(builder.isSuffixSupport(entry, utUrl));
}

TEST(DCustomActionBuilder, appendParentMimeType)
{
    DCustomActionBuilder builder;
    QStringList fileMimeTypes;
    fileMimeTypes.append("application/wps-office.xlsx");
    QStringList fileMimeTypesParent;
    builder.appendParentMimeType(fileMimeTypesParent, fileMimeTypes);
    EXPECT_TRUE(1 == fileMimeTypes.size());
    EXPECT_TRUE("application/wps-office.xlsx" == fileMimeTypes.first());

    fileMimeTypesParent.append("application/vnd.ms-excel");
    fileMimeTypesParent.append("application/msexcel");
    fileMimeTypesParent.append("application/x-msexcel");
    fileMimeTypesParent.append("application/vnd.ms-excel.sheet.macroenabled.12");
    fileMimeTypesParent.append("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet");

    int tempSize = fileMimeTypes.size() + fileMimeTypesParent.size();
    builder.appendParentMimeType(fileMimeTypesParent, fileMimeTypes);
    EXPECT_TRUE(tempSize < fileMimeTypes.size());
}

TEST(DCustomActionBuilder, matchActions)
{
    QList<DCustomActionEntry> oriActions;
    DUrlList selects;
    DCustomActionBuilder builder;
    DCustomActionEntry actEntryZone;
    actEntryZone.m_version = "1.0";
    actEntryZone.m_fileCombo = DCustomActionDefines::ComboType::AllFile;
    actEntryZone.data().name() = "utTestOne";
    actEntryZone.m_supportSchemes.append("*");
    actEntryZone.m_notShowIn.append("Desktop");
    actEntryZone.m_supportSuffix.append("xlsx");
    actEntryZone.m_mimeTypes.append("application/wps-office.xlsx");
    oriActions.append(actEntryZone);
    auto temp = builder.matchActions(selects, oriActions);
    EXPECT_TRUE(1 == temp.size());

    Stub stub;
    //createFile
    DAbstractFileInfoPointer(*tempFuncFile)(const QObject *, const DUrl &) = [](const QObject *, const DUrl &) ->DAbstractFileInfoPointer {
        return DAbstractFileInfoPointer(nullptr);
    };
    stub.set(ADDR(DFileService, createFileInfo), tempFuncFile);
    DUrl utUrl("usr/bin/dde-desktop");
    selects.append(utUrl);
    auto temp_1 = builder.matchActions(selects, oriActions);
    EXPECT_TRUE(1 == temp_1.size());
    DAbstractFileInfoPointer(*tempFuncFile_1)(const QObject *, const DUrl &) = [](const QObject *, const DUrl &) ->DAbstractFileInfoPointer {
        return DAbstractFileInfoPointer(new DAbstractFileInfo(DUrl("usr/bin/dde-desktop")));
    };
    stub.set(ADDR(DFileService, createFileInfo), tempFuncFile_1);

    //支持类型
    void(*tempFunc)(const DAbstractFileInfoPointer &, QStringList &, QStringList &) = [](const DAbstractFileInfoPointer &fileInfo, QStringList &noParentmimeTypes, QStringList &allMimeTypes){
        Q_UNUSED(fileInfo)
        noParentmimeTypes.append("text/plain");
        allMimeTypes.append("text/plain");
    };
    stub.set(ADDR(DCustomActionBuilder, appendAllMimeTypes), tempFunc);

    //协议
    bool(*tempSchemeFunc)(const DCustomActionEntry &, const DUrl &) = [](const DCustomActionEntry &, const DUrl &)->bool{
        return false;
    };
    stub.set(ADDR(DCustomActionBuilder, isSchemeSupport), tempSchemeFunc);
    auto tempSchemeFalse = builder.matchActions(selects, oriActions);
    EXPECT_TRUE(0 == tempSchemeFalse.size());

    //是否在文管或桌面
    bool(*tempSchemeFunc_1)(const DCustomActionEntry &, const DUrl &) = [](const DCustomActionEntry &, const DUrl &)->bool{
        return true;
    };
    bool(*tempShouldShowFunc)(const DCustomActionEntry &, bool) = [](const DCustomActionEntry &, bool )->bool{
        return false;
    };
    stub.set(ADDR(DCustomActionBuilder, isSchemeSupport), tempSchemeFunc_1);
    auto tempShouldShowFalse = builder.matchActions(selects, oriActions);
    EXPECT_TRUE(0 == tempShouldShowFalse.size());

    //后缀
    bool(*tempShouldShowFunc_1)(const DCustomActionEntry &, bool) = [](const DCustomActionEntry &, bool )->bool{
        return true;
    };
    bool(*tempSuffixSupportFunc)(const DCustomActionEntry &, const DUrl &) = [](const DCustomActionEntry &, const DUrl &)->bool{
        return false;
    };
    stub.set(ADDR(DCustomActionBuilder, isSuffixSupport), tempSuffixSupportFunc);
    auto tempSuffixFalse = builder.matchActions(selects, oriActions);
    EXPECT_TRUE(0 == tempSuffixFalse.size());

    bool(*tempSuffixSupportFunc_1)(const DCustomActionEntry &, const DUrl &) = [](const DCustomActionEntry &, const DUrl &)->bool{
        return true;
    };
    stub.set(ADDR(DCustomActionBuilder, isSuffixSupport), tempSuffixSupportFunc_1);

    ASSERT_TRUE(1 <= oriActions.size());
    //不支持的mimetypes,不包含父类型的mimetype集合
    oriActions[0].m_mimeTypes.clear();
    oriActions[0].m_excludeMimeTypes.append("text/plain");
    auto tempExcludeMimeTypes = builder.matchActions(selects, oriActions);
    EXPECT_TRUE(0 == tempExcludeMimeTypes.size());

    //父类mimetype类型
    void(*tempFunc_1)(const DAbstractFileInfoPointer &, QStringList &, QStringList &) = [](const DAbstractFileInfoPointer &fileInfo, QStringList &noParentmimeTypes, QStringList &allMimeTypes){
        Q_UNUSED(fileInfo)
        noParentmimeTypes.append("application/wps-office.xlsx");
        allMimeTypes = noParentmimeTypes;
        allMimeTypes.append("application/vnd.ms-excel");
        allMimeTypes.append("application/msexcel");
        allMimeTypes.append("application/x-msexcel");
        allMimeTypes.append("application/vnd.ms-excel.sheet.macroenabled.12");
        allMimeTypes.append("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet");
    };
    stub.set(ADDR(DCustomActionBuilder, appendAllMimeTypes), tempFunc_1);
    auto tempExcludeMimeTypes_1 = builder.matchActions(selects, oriActions);
    EXPECT_TRUE(1 == tempExcludeMimeTypes_1.size());

    oriActions[0].m_mimeTypes.append("text/plain");
    auto tempMimeTypesNo = builder.matchActions(selects, oriActions);
    EXPECT_TRUE(0 == tempMimeTypesNo.size());

    oriActions[0].m_mimeTypes.clear();
    oriActions[0].m_mimeTypes.append("application/wps-office.xlsx");
    auto tempMimeTypesYes = builder.matchActions(selects, oriActions);
    EXPECT_TRUE(1 == tempMimeTypesYes.size());

    oriActions[0].m_mimeTypes.append("*");
    auto tempMimeTypesYes_1 = builder.matchActions(selects, oriActions);
    EXPECT_TRUE(1 == tempMimeTypesYes_1.size());

    oriActions[0].m_mimeTypes.clear();
    oriActions[0].m_mimeTypes.append("*");
    auto tempMimeTypesYes_2 = builder.matchActions(selects, oriActions);
    EXPECT_TRUE(1 == tempMimeTypesYes_2.size());
}
