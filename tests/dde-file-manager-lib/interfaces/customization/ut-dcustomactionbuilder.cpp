#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <stub.h>

#include <QAction>

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
    QAction *(*createAciton)() = [](){return (QAction* )1;};
    QAction *(*createMenu)() = [](){return (QAction* )2;};
    st.set(&DCustomActionBuilder::createAciton,createAciton);
    st.set(&DCustomActionBuilder::createMenu,createMenu);
    QAction *ac = builder.buildAciton(ad, nullptr);

    EXPECT_EQ(ac, (QAction *)1);

    ad.m_childrenActions.append(ad);
    ac = builder.buildAciton(ad, nullptr);
    EXPECT_EQ(ac, (QAction *)2);
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
