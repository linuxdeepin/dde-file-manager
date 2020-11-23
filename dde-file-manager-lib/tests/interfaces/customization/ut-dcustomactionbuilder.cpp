#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#define private public
#define protected public
#include "customization/dcustomactionbuilder.h"

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
    EXPECT_EQ(builder.m_dirName, "");
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
    EXPECT_EQ(builder.m_fileBaseName, "dde-desktop");
    EXPECT_EQ(builder.m_filePath, url);
}

TEST(DCustomActionBuilder, set_foucus_file_two)
{
    DCustomActionBuilder builder;

    DUrl url("file://usr/bin/dde-desktop.tar.gz");
    builder.setFocusFile(url);
    EXPECT_EQ(builder.m_fileFullName, "dde-desktop.tar.gz");
    EXPECT_EQ(builder.m_fileBaseName, "dde-desktop.tar");
    EXPECT_EQ(builder.m_filePath, url);
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
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::BaseName), "name dde-desktop");

    name = "name%b";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::BaseName), "namedde-desktop");

    name = "%bname";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::BaseName), "dde-desktopname");

    name = "na%bme";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::BaseName), "nadde-desktopme");

    name = "na%bm%be";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::BaseName), "nadde-desktopm%be");

    name = "%bname %b";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::BaseName), "dde-desktopname %b");

    name = "%bname %b%a";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::BaseName), "dde-desktopname %b%a");

    name = "%dname %a%b";
    EXPECT_EQ(builder.makeName(name, DCustomActionDefines::BaseName), "%dname %adde-desktop");
}
