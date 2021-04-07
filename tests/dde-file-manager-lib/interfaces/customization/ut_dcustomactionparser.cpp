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
#include "dcustomactionparser.h"
#include "dcustomactiondefine.h"
#include "dcustomactiondata.h"

#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <QTextStream>
#include <QProcess>
#include "stubext.h"
using namespace DCustomActionDefines;

namespace  {
    QString path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString utDirPath = QString("%1/utTestDir").arg(path);

    class TestDCustomActionParser : public testing::Test
    {
    public:
        void SetUp() override
        {
            //在桌面创建一个测试目录
            QDir utDir(utDirPath);
            if (!utDir.exists()) {
                utDir.mkpath(QString("%1/utTestDir").arg(path));
            }

        }
        void TearDown() override
        {
            //扫荡清除创建的测试目录
            QDir utDir(utDirPath);
            if (utDir.exists()) {
                QProcess temp;
                temp.setProgram("rm");
                temp.setArguments({"-rf",QString("%1/utTestDir").arg(path)});
                temp.start();
                temp.waitForStarted();
                temp.waitForFinished();
                delete m_parser;
                m_parser = nullptr;
            }
        }
    public:
        DCustomActionParser* m_parser = new DCustomActionParser;

    };
}


TEST_F(TestDCustomActionParser, test_loadDir)
{
    //空目录
    auto expectValue1 = m_parser->loadDir("");
    EXPECT_FALSE(expectValue1);

    //无效目录
    auto expectValue2 = m_parser->loadDir("utDirPath/invalidPath");
    EXPECT_FALSE(expectValue2);

    //往测试目录写一个无效配置文件
    auto invalidFilePath = QString("%1/%2").arg(utDirPath).arg("invalid.conf");
    QFile invalidFile(invalidFilePath);
    if (!invalidFile.exists()) {
        invalidFile.open(QIODevice::WriteOnly);
        invalidFile.close();
    }

    auto expectValue3 = m_parser->loadDir(utDirPath);
    EXPECT_TRUE(expectValue3);
}

TEST_F(TestDCustomActionParser, test_get_action_files)
{
    m_parser->m_actionEntry.clear();
    DCustomActionEntry entry;
    m_parser->m_actionEntry << entry; //无限制

    entry.m_notShowIn.append("Desktop"); //不在桌面
    m_parser->m_actionEntry << entry;

    entry.m_notShowIn.append("Filemanager"); //不在桌面和文管
    m_parser->m_actionEntry << entry;

    entry.m_notShowIn.clear();
    entry.m_notShowIn.append("Filemanager"); //不在文管
    m_parser->m_actionEntry << entry;

    EXPECT_EQ(m_parser->getActionFiles(true).size(), 2);
    EXPECT_EQ(m_parser->getActionFiles(false).size(), 2);
}

TEST_F(TestDCustomActionParser, test_parse_file_only_settings_arg)
{
    auto invalidFilePath = QString("%1/%2").arg(utDirPath).arg("invalid.conf");
    //关键入口信息没有，认为是无效的配置文件
    QSettings actionSetting(invalidFilePath, QSettings::IniFormat);
    actionSetting.setIniCodec("UTF-8");
    auto expectValue1 = m_parser->parseFile(actionSetting);
    EXPECT_FALSE(expectValue1);

    //无Version,返回false
    //往刚刚的测试目录下创建的空文件"invalid.conf"写入一个仅含[Menu Entry]
    QFile invalidFile(invalidFilePath);
    invalidFile.open(QIODevice::WriteOnly | QIODevice::Append | QFile::Text);
    QTextStream tsm(&invalidFile);
    tsm.setCodec("UTF-8");
    tsm << QString("[Menu Entry]").toUtf8() << endl;
    tsm.flush();
    invalidFile.flush();
    QSettings actionSetting2(invalidFilePath, QSettings::IniFormat);
    actionSetting2.setIniCodec("UTF-8");
    auto expectValue2 = m_parser->parseFile(actionSetting2);
    EXPECT_FALSE(expectValue2);

    //无Actions返回false
    tsm << QString("Comment=This is a test file!!!").toUtf8() << endl
        << QString("Comment[zh_CN]=这是一个测试文件").toUtf8() << endl
        << QString("Version=Uos1.0").toUtf8() << endl;
    tsm.flush();
    invalidFile.flush();
    QSettings actionSetting3(invalidFilePath, QSettings::IniFormat);
    actionSetting3.setIniCodec("UTF-8");
    auto expectValue3 = m_parser->parseFile(actionSetting3);
    EXPECT_FALSE(expectValue3);

    //基本信息齐全，返回true
    tsm << QString("Actions=Groupzero").toUtf8() << endl;
    QSettings actionSetting4(invalidFilePath, QSettings::IniFormat);
    actionSetting4.setIniCodec("UTF-8");
    auto expectValue4 = m_parser->parseFile(actionSetting4);
    EXPECT_TRUE(expectValue4);
    tsm.flush();
    invalidFile.flush();
    invalidFile.close();

}

TEST_F(TestDCustomActionParser, test_parse_file_more_arg)
{
    auto invalidFilePath = QString("%1/%2").arg(utDirPath).arg("invalid.conf");
    QFile invalidFile(invalidFilePath);
    invalidFile.open(QIODevice::WriteOnly | QIODevice::Append | QFile::Text);
    QTextStream tsm(&invalidFile);
    tsm.setCodec("UTF-8");
    tsm << QString("[Menu Entry]").toUtf8() << endl
        << QString("Comment=This is a test file!!!").toUtf8() << endl
        << QString("Comment[zh_CN]=这是一个测试文件").toUtf8() << endl
        << QString("Version=Uos1.0").toUtf8() << endl
        << QString("Actions=Groupzero:GroupzeroT").toUtf8() << endl;

    QList<DCustomActionData> childrenActions;
    FileBasicInfos basicInfos;
    basicInfos.m_sign = "";
    basicInfos.m_comment = "This is a test file!!!";
    basicInfos.m_package = "invalid.conf";
    basicInfos.m_version = "Uos1.0";
    tsm.flush();
    invalidFile.flush();

    bool needSort = true;
    //无name
    m_parser->m_hierarchyNum = 0;
    m_parser->m_actionEntry.clear();
    m_parser->m_topActionCount = 0;
    QSettings actionSetting1(invalidFilePath, QSettings::IniFormat);
    actionSetting1.setIniCodec("UTF-8");
    m_parser->parseFile(childrenActions, actionSetting1, "Menu Action Groupzero", basicInfos, needSort, true);
    auto expectValue1 = m_parser->m_actionEntry.isEmpty();
    EXPECT_TRUE(expectValue1);


    tsm << QString("[Menu Action Groupzero]").toUtf8() << endl;
    tsm.flush();
    invalidFile.flush();
    QSettings actionSetting2(invalidFilePath, QSettings::IniFormat);
    actionSetting2.setIniCodec("UTF-8");

    m_parser->m_hierarchyNum = 0;
    m_parser->m_actionEntry.clear();
    m_parser->m_topActionCount = 0;
    m_parser->parseFile(childrenActions, actionSetting2, "Menu Action Groupzero", basicInfos, needSort, true);
    auto expectValue2 = m_parser->m_actionEntry.isEmpty();
    EXPECT_TRUE(expectValue2);

    //pos,separator, 无action无动作
    tsm << QString("GenericName=app-one").toUtf8() << endl
        << QString("GenericName[zh_CN]=应用1级").toUtf8() << endl
        << QString("Name=app-one").toUtf8() << endl
        << QString("Name[zh_CN]=应用1级").toUtf8() << endl
        << QString("Icon=/home/ut_test.png") << endl
        << QString("PosNum=1").toUtf8() << endl
        << QString("Separator=None").toUtf8() << endl;
    tsm.flush();
    invalidFile.flush();
    QSettings actionSetting3(invalidFilePath, QSettings::IniFormat);
    actionSetting3.setIniCodec("UTF-8");
    m_parser->m_hierarchyNum = 0;
    m_parser->m_actionEntry.clear();
    m_parser->m_topActionCount = 0;
    m_parser->parseFile(childrenActions, actionSetting3, "Menu Action Groupzero", basicInfos, needSort, true);
    auto expectValue3 = m_parser->m_actionEntry.isEmpty();
    EXPECT_TRUE(expectValue3);


    //actions 父子action级联与动作
    tsm << QString("Exec=/opt/apps/xxxxxx %U").toUtf8() <<endl;//无combo
    tsm.flush();
    invalidFile.flush();
    QSettings actionSetting4(invalidFilePath, QSettings::IniFormat);
    actionSetting4.setIniCodec("UTF-8");

    m_parser->m_hierarchyNum = 0;
    m_parser->m_actionEntry.clear();
    m_parser->m_topActionCount = 0;
    m_parser->parseFile(childrenActions, actionSetting4, "Menu Action Groupzero", basicInfos, needSort, true);
    auto expectValue4 = m_parser->m_actionEntry.isEmpty();
    EXPECT_TRUE(expectValue4);

    //有combo包含多个,一个有效，一个无效，无效的忽略
    tsm << QString("X-DFM-MenuTypes=SingleFile:aaaaa").toUtf8() <<endl;
    tsm.flush();
    invalidFile.flush();
    QSettings actionSetting5(invalidFilePath, QSettings::IniFormat);
    actionSetting5.setIniCodec("UTF-8");

    m_parser->m_hierarchyNum = 0;
    m_parser->m_actionEntry.clear();
    m_parser->m_topActionCount = 0;
    m_parser->parseFile(childrenActions, actionSetting5, "Menu Action Groupzero", basicInfos, needSort, true);
    auto expectValue5 = m_parser->m_actionEntry.isEmpty();
    EXPECT_FALSE(expectValue5);

    //一级菜单未指明支持的combo类型，默认无效
    tsm << QString("[Menu Action GroupzeroT]").toUtf8() << endl
        << QString("GenericName=app-one").toUtf8() << endl
        << QString("GenericName[zh_CN]=应用1级二").toUtf8() << endl
        << QString("Name=app-one").toUtf8() << endl
        << QString("Name[zh_CN]=应用1级二").toUtf8() << endl
        << QString("Name[ko]=韩语").toUtf8() << endl
        << QString("Name[am_ET]=一种语言").toUtf8() << endl
        << QString("PosNum=1").toUtf8() << endl
        << QString("Separator=None").toUtf8() << endl
        << QString("Exec=/opt/apps/xxxxxx %U").toUtf8() <<endl;
    //        << QString("X-DFM-MenuTypes=SingleFile").toUtf8() <<endl;
    tsm.flush();
    invalidFile.flush();

    QSettings actionSetting5_1(invalidFilePath, QSettings::IniFormat);
    actionSetting5_1.setIniCodec("UTF-8");
    m_parser->m_hierarchyNum = 0;
    m_parser->m_actionEntry.clear();
    m_parser->m_topActionCount = 0;
    m_parser->parseFile(childrenActions, actionSetting5_1, "Menu Action Groupzero", basicInfos, needSort, true);
    bool expectValue5_1=false;
    if(!m_parser->m_actionEntry.isEmpty())
        expectValue5_1 = (1 == m_parser->m_actionEntry.size()) && ("应用1级" == m_parser->m_actionEntry.first().m_data.m_name);

    //产品变更，icon暂不考虑
#if 0
    auto expectValueIcon =  "/home/ut_test.png" == m_parser->m_actionEntry.first().m_data.m_icon;
    EXPECT_TRUE(expectValueIcon);
#endif
    EXPECT_TRUE(expectValue5_1);

    tsm<< QString("X-DFM-MenuTypes=SingleFile").toUtf8() <<endl;
    tsm.flush();
    invalidFile.flush();

    {
        QSettings actionSetting(invalidFilePath, QSettings::IniFormat);
        actionSetting.setIniCodec("UTF-8");
        m_parser->m_hierarchyNum = 0;
        m_parser->m_actionEntry.clear();
        m_parser->m_topActionCount = 0;
        stub_ext::StubExt stu;
        stu.set_lamda(ADDR(QLocale, name), [](){return "zh_CN";});
        m_parser->parseFile(childrenActions, actionSetting5_1, "Menu Action GroupzeroT", basicInfos, needSort, true);
        bool expectValue = false;
        if(!m_parser->m_actionEntry.isEmpty())
            expectValue=("应用1级二" == m_parser->m_actionEntry.first().m_data.m_name);
        EXPECT_TRUE(expectValue);

    }

    {
        QSettings actionSetting(invalidFilePath, QSettings::IniFormat);
        actionSetting.setIniCodec("UTF-8");
        m_parser->m_hierarchyNum = 0;
        m_parser->m_actionEntry.clear();
        m_parser->m_topActionCount = 0;
        stub_ext::StubExt stu;
        stu.set_lamda(ADDR(QLocale, name), [](){return "ko_KR";});
        m_parser->parseFile(childrenActions, actionSetting5_1, "Menu Action GroupzeroT", basicInfos, needSort, true);
        bool expectValue = false;
        if(!m_parser->m_actionEntry.isEmpty())
            expectValue=("韩语" == m_parser->m_actionEntry.first().m_data.m_name);
        EXPECT_TRUE(expectValue);

    }

    {
        QSettings actionSetting(invalidFilePath, QSettings::IniFormat);
        actionSetting.setIniCodec("UTF-8");
        m_parser->m_hierarchyNum = 0;
        m_parser->m_actionEntry.clear();
        m_parser->m_topActionCount = 0;
        stub_ext::StubExt stu;
        stu.set_lamda(ADDR(QLocale, name), [](){return "am_ET";});
        m_parser->parseFile(childrenActions, actionSetting5_1, "Menu Action GroupzeroT", basicInfos, needSort, true);
        bool expectValue = false;
        if(!m_parser->m_actionEntry.isEmpty())
            expectValue = ("一种语言" == m_parser->m_actionEntry.first().m_data.m_name);
        EXPECT_TRUE(expectValue);

    }

    QSettings actionSetting5_2(invalidFilePath, QSettings::IniFormat);
    actionSetting5_2.setIniCodec("UTF-8");
    m_parser->m_hierarchyNum = 0;
    m_parser->m_actionEntry.clear();
    m_parser->m_topActionCount = 0;
    //    m_parser->parseFile(childrenActions, actionSetting5_2, "Menu Action Groupzero", basicInfos, true);
    m_parser->parseFile(actionSetting5_2);
    bool expectValue5_2 = false;
    if(!m_parser->m_actionEntry.isEmpty())
        expectValue5_2= 2 == m_parser->m_actionEntry.size();
    EXPECT_TRUE(expectValue5_2);


    //有子级分组，子级信息却未设置，连带一级也被忽略
    tsm << QString("Actions=GroupOne:GroupTwo").toUtf8() <<endl;
    tsm.flush();
    invalidFile.flush();
    QSettings actionSetting6(invalidFilePath, QSettings::IniFormat);
    actionSetting6.setIniCodec("UTF-8");
    m_parser->m_hierarchyNum = 0;
    m_parser->m_actionEntry.clear();
    m_parser->m_topActionCount = 0;
    m_parser->parseFile(actionSetting6);
    bool expectValue6 = false;
    if(!m_parser->m_actionEntry.isEmpty())
        expectValue6= (1 == m_parser->m_actionEntry.size()) && ("应用1级" == m_parser->m_actionEntry.first().m_data.m_name);
    EXPECT_TRUE(expectValue6);

    //有子级,但是全部子级无动作，连带一级也忽略
    tsm << QString("[Menu Action GroupOne]").toUtf8() << endl
        << QString("GenericName=app-one").toUtf8() << endl
        << QString("GenericName[zh_CN]=应用2级菜单项一").toUtf8() << endl
        << QString("Name=app-one").toUtf8() << endl
        << QString("Name[zh_CN]=应用2级菜单项一").toUtf8() << endl
        << QString("PosNum=3").toUtf8() << endl
        << QString("Separator=None").toUtf8() << endl;
    tsm.flush();
    invalidFile.flush();
    QSettings actionSetting6_1(invalidFilePath, QSettings::IniFormat);
    actionSetting6_1.setIniCodec("UTF-8");
    m_parser->m_hierarchyNum = 0;
    m_parser->m_actionEntry.clear();
    m_parser->m_topActionCount = 0;
    m_parser->parseFile(actionSetting6_1);
    bool expectValue6_1 = false;
    if(!m_parser->m_actionEntry.isEmpty())
        expectValue6_1 = (1 == m_parser->m_actionEntry.size()) && ("应用1级" == m_parser->m_actionEntry.first().m_data.m_name);
    EXPECT_TRUE(expectValue6_1);

    //部分子级有效，一级有效，但是仅有有效子级
    tsm << QString("Exec=/opt/apps/xxxxxx %U").toUtf8() <<endl
        << QString("[Menu Action GroupTwo]").toUtf8() << endl
        << QString("GenericName=app-one").toUtf8() << endl
        << QString("GenericName[zh_CN]=应用2级菜单项二").toUtf8() << endl
        << QString("Name=app-one").toUtf8() << endl
        << QString("Name[zh_CN]=应用2级菜单项二").toUtf8() << endl
//        << QString("PosNum=1").toUtf8() << endl
        << QString("Separator=None").toUtf8() << endl;

    tsm.flush();
    invalidFile.flush();
    QSettings actionSetting6_2(invalidFilePath, QSettings::IniFormat);
    actionSetting6_2.setIniCodec("UTF-8");
    m_parser->m_hierarchyNum = 0;
    m_parser->m_actionEntry.clear();
    m_parser->m_topActionCount = 0;
    m_parser->parseFile(actionSetting6_2);
    bool expectValue6_2 = false;
    if(!m_parser->m_actionEntry.isEmpty())
        expectValue6_2 = (2 == m_parser->m_actionEntry.size())
            && ("应用1级二" == m_parser->m_actionEntry.last().m_data.m_name
                && "应用2级菜单项一" == m_parser->m_actionEntry.last().m_data.m_childrenActions.first().name()
                && 1 == m_parser->m_actionEntry.last().m_data.m_childrenActions.size());
    EXPECT_TRUE(expectValue6_2);

    //有子级且子级均有效，子级的顺序自动排序
    tsm << QString("Exec=/opt/apps/xxxxxx %f").toUtf8() << endl;//未指定位置,当前层级顺排序
    tsm.flush();
    invalidFile.flush();
    QSettings actionSetting6_3(invalidFilePath, QSettings::IniFormat);
    actionSetting6_3.setIniCodec("UTF-8");
    m_parser->m_hierarchyNum = 0;
    m_parser->m_actionEntry.clear();
    m_parser->m_topActionCount = 0;
    m_parser->parseFile(actionSetting6_3);
    bool expectValue6_3 = false;
    if(!m_parser->m_actionEntry.isEmpty())
        expectValue6_3 = (2 == m_parser->m_actionEntry.size())
            && ("应用1级二" == m_parser->m_actionEntry.last().m_data.m_name
                && "应用2级菜单项一" == m_parser->m_actionEntry.last().m_data.m_childrenActions.first().name()
                && "应用2级菜单项二" == m_parser->m_actionEntry.last().m_data.m_childrenActions.last().name());
    EXPECT_TRUE(expectValue6_3);

    tsm << QString("PosNum=1").toUtf8() << endl;//所有位置都指定了,以位置排序
    tsm.flush();
    invalidFile.flush();
    QSettings actionSetting6_4(invalidFilePath, QSettings::IniFormat);
    actionSetting6_4.setIniCodec("UTF-8");
    m_parser->m_hierarchyNum = 0;
    m_parser->m_actionEntry.clear();
    m_parser->m_topActionCount = 0;
    m_parser->parseFile(actionSetting6_4);
    bool expectValue6_4 = false;
    if(!m_parser->m_actionEntry.isEmpty())
        expectValue6_4 = (2 == m_parser->m_actionEntry.size())
            && ("应用1级二" == m_parser->m_actionEntry.last().m_data.m_name
                && "应用2级菜单项二" == m_parser->m_actionEntry.last().m_data.m_childrenActions.first().name()
                && "应用2级菜单项一" == m_parser->m_actionEntry.last().m_data.m_childrenActions.last().name());
    EXPECT_TRUE(expectValue6_4);

    //三级菜单
    tsm << QString("Actions=GroupOne_Three").toUtf8() << endl
        << QString("[Menu Action GroupOne_Three]").toUtf8() << endl
        << QString("GenericName=app-one").toUtf8() << endl
        << QString("GenericName[zh_CN]=应用3级菜单项一").toUtf8() << endl
        << QString("Name=app-one").toUtf8() << endl
        << QString("Name[zh_CN]=应用3级菜单项一").toUtf8() << endl
        << QString("PosNum=1").toUtf8() << endl
        << QString("Separator=None").toUtf8() << endl
        << QString("Exec=/opt/apps/xxxxxx %U").toUtf8() <<endl;

    tsm.flush();
    invalidFile.flush();
    QSettings actionSetting7(invalidFilePath, QSettings::IniFormat);
    actionSetting7.setIniCodec("UTF-8");
    m_parser->m_hierarchyNum = 0;
    m_parser->m_actionEntry.clear();
    m_parser->m_topActionCount = 0;
    m_parser->parseFile(actionSetting7);
    bool expectValue7 = false;
    if(!m_parser->m_actionEntry.isEmpty())
        expectValue7 = (2 == m_parser->m_actionEntry.size())
            && ("应用1级二" == m_parser->m_actionEntry.last().m_data.m_name
                && "应用2级菜单项二" == m_parser->m_actionEntry.last().m_data.m_childrenActions.first().name()
                && "应用2级菜单项一" == m_parser->m_actionEntry.last().m_data.m_childrenActions.last().name())
                && "应用3级菜单项一" == m_parser->m_actionEntry.last().m_data.m_childrenActions.first().m_childrenActions.first().name();
    EXPECT_TRUE(expectValue7);

    //4级
    tsm << QString("Actions=GroupOneThree_Four").toUtf8() << endl
        << QString("[Menu Action GroupOneThree_Four]").toUtf8() << endl
        << QString("GenericName=app-one").toUtf8() << endl
        << QString("GenericName[zh_CN]=应用4级菜单项一").toUtf8() << endl
        << QString("Name=app-one").toUtf8() << endl
        << QString("Name[zh_CN]=应用4级菜单项一").toUtf8() << endl
        << QString("PosNum=1").toUtf8() << endl
        << QString("Separator=None").toUtf8() << endl
        << QString("Exec=/opt/apps/xxxxxx %U").toUtf8() <<endl;

    tsm.flush();
    invalidFile.flush();
    QSettings actionSetting8(invalidFilePath, QSettings::IniFormat);
    actionSetting8.setIniCodec("UTF-8");
    m_parser->m_hierarchyNum = 0;
    m_parser->m_actionEntry.clear();
    m_parser->m_topActionCount = 0;
    m_parser->parseFile(actionSetting7);
    bool expectValue8 = false;
    if(!m_parser->m_actionEntry.isEmpty())
        expectValue8 = (2 == m_parser->m_actionEntry.size())
            && ("应用1级二" == m_parser->m_actionEntry.last().m_data.m_name
                && "应用2级菜单项一" == m_parser->m_actionEntry.last().m_data.m_childrenActions.last().name());
    EXPECT_TRUE(expectValue8);
    invalidFile.close();
}

TEST_F(TestDCustomActionParser, test_parse_Level_one_action_args)
{
    auto invalidFilePath = QString("%1/%2").arg(utDirPath).arg("test.conf");
    QFile testFile(invalidFilePath);
    testFile.open(QIODevice::WriteOnly | QIODevice::Append | QFile::Text);
    QTextStream tsm(&testFile);
    tsm.setCodec("UTF-8");
    tsm << QString("[Menu Entry]").toUtf8() << endl
        << QString("Comment=This is a test file!!!").toUtf8() << endl
        << QString("Comment[zh_CN]=这是一个测试文件").toUtf8() << endl
        << QString("Version=Uos1.0").toUtf8() << endl
        << QString("Actions=Level-one").toUtf8() << endl;

    QList<DCustomActionData> childrenActions;
    FileBasicInfos basicInfos;
    basicInfos.m_sign = "";
    basicInfos.m_comment = "This is a test file!!!";
    basicInfos.m_package = "invalid.conf";
    basicInfos.m_version = "Uos1.0";
    tsm.flush();
    testFile.flush();

    //一级菜单相关参数
    tsm << QString("[Menu Action Level-one]")<<endl
        << QString("Name=app-level-one").toUtf8() << endl
        << QString("Name[zh_CN]=应用1级").toUtf8() << endl
        << QString("PosNum=1").toUtf8() << endl
        << QString("Separator=None").toUtf8() << endl
        << QString("X-DFM-MenuTypes=SingleFile").toUtf8() << endl
        << QString("MimeType=test-mimetype1:test-mimetype2") << endl
        << QString("X-DFM-ExcludeMimeTypes=excludeMimtype1:excludeMimtype2").toUtf8() << endl
        << QString("X-DFM-SupportSchemes=file").toUtf8() << endl
        << QString("X-DFM-NotShowIn=desktop").toUtf8() << endl
        << QString("X-DFM-SupportSuffix=*.7z.001").toUtf8() << endl
        << QString("Actions=Level-two").toUtf8() << endl

    //二级菜单
        << QString("[Menu Action Level-two]").toUtf8() << endl
        << QString("Name=app-level-two").toUtf8() << endl
        << QString("Name[zh_CN]=应用2级").toUtf8() << endl
        << QString("PosNum=1").toUtf8() << endl
        << QString("Separator=None").toUtf8() << endl
        << QString("Exec=xxx/xxx").toUtf8() << endl;
    tsm.flush();
    testFile.flush();

    //解析菜单参数
    QSettings actionSetting0(invalidFilePath, QSettings::IniFormat);
    actionSetting0.setIniCodec("UTF-8");
    m_parser->m_hierarchyNum = 0;
    m_parser->m_actionEntry.clear();
    m_parser->m_topActionCount = 0;
    m_parser->parseFile(actionSetting0);

    //combo,选中项类型
    EXPECT_TRUE(DCustomActionDefines::ComboType::SingleDir == m_parser->m_combos.value("SingleDir"));
    EXPECT_FALSE(m_parser->m_actionEntry.isEmpty());
    ASSERT_TRUE(1 == m_parser->m_actionEntry.size());

    //mimetype
    auto utMimetype = m_parser->m_actionEntry.first().mimeTypes();
    ASSERT_TRUE(2 == utMimetype.size());
    EXPECT_TRUE("test-mimetype1" == utMimetype.first());
    EXPECT_TRUE("test-mimetype2" == utMimetype.last());

    //不支持的mimetype
    auto utExcludeMimetype = m_parser->m_actionEntry.first().excludeMimeTypes();
    ASSERT_TRUE(2 == utExcludeMimetype.size());
    EXPECT_TRUE("excludeMimtype1" == utExcludeMimetype.first());
    EXPECT_TRUE("excludeMimtype2" == utExcludeMimetype.last());

    //支持协议
    auto utSupportSchemes = m_parser->m_actionEntry.first().surpportSchemes();
    ASSERT_TRUE(1 == utSupportSchemes.size());
    EXPECT_TRUE("file" == utSupportSchemes.first());

    //不展示在桌面或文管
    auto utNotShowIn = m_parser->m_actionEntry.first().notShowIn();
    ASSERT_TRUE(1 == utNotShowIn.size());
    EXPECT_TRUE("desktop" == utNotShowIn.first());

    //不支持的后缀
    auto utSupportSuffix= m_parser->m_actionEntry.first().supportStuffix();
    ASSERT_TRUE(1 == utSupportSuffix.size());
    EXPECT_TRUE("*.7z.001" == utSupportSuffix.first());
}


TEST_F(TestDCustomActionParser, test_init_hash)
{
    EXPECT_TRUE (6 == m_parser->m_combos.size());
    EXPECT_TRUE (4 == m_parser->m_separtor.size());
    EXPECT_TRUE (3 == m_parser->m_actionNameArg.size());
    EXPECT_TRUE (5 == m_parser->m_actionExecArg.size());

    QHash<QString, ComboType> tCombos;
    QHash<QString, Separator> tSepartor;
    QHash<QString, ActionArg> tActionNameArg;
    QHash<QString, ActionArg> tActionExecArg;
    tCombos.insert("SingleFile", ComboType::SingleFile);
    tCombos.insert("SingleDir", ComboType::SingleDir);
    tCombos.insert("MultiFiles", ComboType::MultiFiles);
    tCombos.insert("MultiDirs", ComboType::MultiDirs);
    tCombos.insert("FileAndDir", ComboType::FileAndDir);
    tCombos.insert("BlankSpace", ComboType::BlankSpace);

    tSepartor.insert("None", Separator::None);
    tSepartor.insert("Top", Separator::Top);
    tSepartor.insert("Both", Separator::Both);
    tSepartor.insert("Bottom", Separator::Bottom);

    //name参数类型仅支持：DirName BaseName FileName
    tActionNameArg.insert(kStrActionArg[DirName], ActionArg::DirName);       //%d
    tActionNameArg.insert(kStrActionArg[BaseName], ActionArg::BaseName);     //%b
    tActionNameArg.insert(kStrActionArg[FileName], ActionArg::FileName);     //"%a",

    //cmd参数类型只支持：DirPath FilePath FilePaths UrlPath UrlPaths
    tActionExecArg.insert(kStrActionArg[DirPath], ActionArg::DirPath);       //"%p"
    tActionExecArg.insert(kStrActionArg[FilePath], ActionArg::FilePath);     //"%f"
    tActionExecArg.insert(kStrActionArg[FilePaths], ActionArg::FilePaths);   //"%F"
    tActionExecArg.insert(kStrActionArg[UrlPath], ActionArg::UrlPath);       //"%u"
    tActionExecArg.insert(kStrActionArg[UrlPaths], ActionArg::UrlPaths);     //"%U"

    auto expectValue1 = tCombos == m_parser->m_combos;
    EXPECT_TRUE(expectValue1);

    auto expectValue2 = tSepartor == m_parser->m_separtor;
    EXPECT_TRUE(expectValue2);

    auto expectValue3 = tActionNameArg == m_parser->m_actionNameArg;
    EXPECT_TRUE(expectValue3);

    auto expectValue4 = tActionExecArg == m_parser->m_actionExecArg;
    EXPECT_TRUE(expectValue4);

}

TEST_F(TestDCustomActionParser, test_get_value)
{
    auto invalidFilePath = QString("%1/%2").arg(utDirPath).arg("invalid.conf");
    QFile invalidFile(invalidFilePath);
    invalidFile.open(QIODevice::WriteOnly | QIODevice::Append | QFile::Text);
    QTextStream tsm(&invalidFile);
    tsm.setCodec("UTF-8");
    tsm << QString("[Menu Entry]").toUtf8() << endl
        << QString("Comment=This is a test file!!!").toUtf8() << endl
        << QString("Comment[zh_CN]=这是一个测试文件").toUtf8() << endl
        << QString("Version=Uos1.0").toUtf8() << endl
        << QString("Actions=Groupzero").toUtf8() << endl;
    tsm.flush();
    invalidFile.flush();
    QSettings actionSetting(invalidFilePath, QSettings::IniFormat);
    actionSetting.setIniCodec("UTF-8");
    auto vOne = "Uos1.0" == m_parser->getValue(actionSetting, "Menu Entry", kConfFileVersion);
    auto vTwo = QVariant() == m_parser->getValue(actionSetting, "Menu Entry", kConfSign);
    EXPECT_TRUE(vOne);
    EXPECT_TRUE(vTwo);
    invalidFile.close();
}

TEST_F(TestDCustomActionParser, test_action_file_infos)
{
    auto invalidFilePath = QString("%1/%2").arg(utDirPath).arg("invalid.conf");
    QFile invalidFile(invalidFilePath);
    invalidFile.open(QIODevice::WriteOnly | QIODevice::Append | QFile::Text);
    QTextStream tsm(&invalidFile);
    tsm.setCodec("UTF-8");
    tsm << QString("[Menu Entry]").toUtf8() << endl
        << QString("Comment=This is a test file!!!").toUtf8() << endl
        << QString("Comment[zh_CN]=这是一个测试文件").toUtf8() << endl;

    tsm.flush();
    invalidFile.flush();
    QSettings actionSetting(invalidFilePath, QSettings::IniFormat);
    actionSetting.setIniCodec("UTF-8");
    FileBasicInfos tempBasicInfos;
    auto tgValue = m_parser->actionFileInfos(tempBasicInfos, actionSetting);
    EXPECT_FALSE(tgValue);

    tsm << QString("Version=Uos1.0").toUtf8() << endl;
    tsm.flush();
    invalidFile.flush();
    QSettings actionSetting2(invalidFilePath, QSettings::IniFormat);
    actionSetting2.setIniCodec("UTF-8");
    auto tgValue2 = m_parser->actionFileInfos(tempBasicInfos, actionSetting2);
    EXPECT_TRUE(tgValue2);
    auto tgValue3 = "Uos1.0" == tempBasicInfos.m_version;
    EXPECT_TRUE(tgValue3);
    invalidFile.close();
}


TEST(DCustomActionParser, action_name_dynamic_arg_empty)
{
    DCustomActionParser parser;
    DCustomActionData actData;

    actData.m_name = "n";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "n %f";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "name";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "name%f";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "name %F";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "\"n\"";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "\"n %F\"";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "\"name\"";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "\"name%f\"";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "\"name %f\"";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

}

TEST(DCustomActionParser, action_name_dynamic_arg_not_empty)
{
    DCustomActionParser parser;
    DCustomActionData actData;

    actData.m_name = "%a";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "n%a";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "%aname";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "na%ame";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "name%a";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "name %a";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "\"n%d\"";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::DirName == actData.m_nameArg);

    actData.m_name = "\"%dname\"";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::DirName == actData.m_nameArg);

    actData.m_name = "\"na%dme\"";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::DirName == actData.m_nameArg);

    actData.m_name = "\"name%d\"";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::DirName == actData.m_nameArg);

    actData.m_name = "\"name %d\"";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::DirName == actData.m_nameArg);

    actData.m_name = "n%a%b";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "%a%bname";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "na%a%bme";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "name%a%b";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "name %a%b";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "\"n%a%b\"";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "\"%a%bname\"";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "\"na%a%bme\"";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "\"name%a%b\"";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "\"name %a%b\"";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "%ana%bme%d";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "%ana%bme %d";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "\"%ana%bme%d\"";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);

    actData.m_name = "\"%ana%bme %d\"";
    parser.actionNameDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FileName == actData.m_nameArg);
}

TEST(DCustomActionParser, exec_dynamic_arg_empty)
{
    DCustomActionParser parser;
    DCustomActionData actData;

    actData.m_name = "n";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "%a/home/test";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "%a/home/test";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "/home%a/test";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "/home/test%a";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "/home/test %a";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "/home/test/%a";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "/home/test/ %a";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "\"n\"";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "\"/home/test\"";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "\"%a/home/test\"";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "\"/home%a/test\"";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "\"/home/test%a\"";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "\"/home/test %a\"";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "\"/home/test\"%a";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "\"/home/test\" %a";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "\"/home/test/%a\"";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

    actData.m_name = "\"/home/test/ %a\"";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::NoneArg == actData.m_nameArg);

}

TEST(DCustomActionParser, exec_dynamic_arg_not_empty)
{
    DCustomActionParser parser;
    DCustomActionData actData;

    actData.m_command = "%f";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "%f%F%u%U";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "%a%f%u%U";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "%f/home/test";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "%a%f/home/test";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "/%fhome/test";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "/%f%ahome/test";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "/%a%fhome/test";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "/home%f/test";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "/home/test%f";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "/home/test/%f";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "/home/test/%a%f";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "/home/test %f";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "/home/test %a%f";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "/home/test --test%f";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "/home/test --test%f%F";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "/home/test --test%a --test01%F";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "/home/test --test%f --test01%F";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "\"/home/test/%f\"";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "\"/home/test/%a%f\"";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "/home/test/ \"%f\"";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "/home/test/ \"%a%f\"";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

    actData.m_command = "/home/test \"--%f%F\"";
    parser.execDynamicArg(actData);
    EXPECT_TRUE(DCustomActionDefines::ActionArg::FilePath == actData.m_cmdArg);

}

TEST(DCustomActionParser, test_is_Action_ShouldShow)
{

    bool ret = DCustomActionParser::isActionShouldShow(QStringList(), true);
    EXPECT_TRUE(ret);

    ret = DCustomActionParser::isActionShouldShow(QStringList() << "*", true);
    EXPECT_FALSE(ret);

    ret = DCustomActionParser::isActionShouldShow(QStringList() << "Filemanager", true);
    EXPECT_TRUE(ret);

    ret = DCustomActionParser::isActionShouldShow(QStringList() << "Desktop", false);
    EXPECT_TRUE(ret);

    ret = DCustomActionParser::isActionShouldShow(QStringList() << "Desktop" << "Filemanager", false);
    EXPECT_FALSE(ret);
}

TEST_F(TestDCustomActionParser, test_read_Conf_One){
    //bug69342对应字段测试
    auto invalidFilePath = QString("%1/%2").arg(utDirPath).arg("invalid.conf");

    QFile invalidFile(invalidFilePath);
    invalidFile.open(QIODevice::WriteOnly | QIODevice::Append | QFile::Text);
    QTextStream tsm(&invalidFile);
    tsm.setCodec("UTF-8");
    tsm << QString("[Menu Entry]").toUtf8() << endl;
    tsm << QString("Comment=This is a test file of compress!!!").toUtf8() <<endl;
    tsm << QString("Comment[zh_CN]=这是一个s测试文件").toUtf8() <<endl;
    tsm << QString("Version=Uos1.0").toUtf8() <<endl;
    tsm << QString("Actions=Zero:decompress").toUtf8() <<endl;
    tsm << QString("[Menu Action Zero]").toUtf8() <<endl;
    tsm << QString("Exec=/usr/bin/deepin-compressor %U compress").toUtf8() <<endl;
    tsm << QString("Name=Compressor").toUtf8() <<endl;
    tsm << QString("Name[zh_CN]=New压缩-压缩到\"%d\"").toUtf8() <<endl;
    tsm << QString("Name[en]=Compress").toUtf8() <<endl;
    tsm << QString("X-DFM-MenuTypes=MultiFiles:MultiDirs:SingleFile:SingleDir").toUtf8() <<endl;
    tsm << QString("X-DFM-SupportSchemes=file").toUtf8() <<endl;
    tsm << QString("PosNum=1").toUtf8() <<endl;
    tsm << QString("Separator=Top").toUtf8() <<endl;
    tsm << QString("MimeType=*").toUtf8() <<endl;

    tsm.flush();
    invalidFile.flush();

    auto expectValue2 = m_parser->loadDir(utDirPath);
    EXPECT_TRUE(expectValue2);

    if (0 < m_parser->m_actionEntry.size())
        EXPECT_TRUE(m_parser->m_actionEntry.at(0).data().name() == QString("New压缩-压缩到\"%d\""));

    invalidFile.close();
}

TEST_F(TestDCustomActionParser, test_read_Conf_Two){
    //入口测试
    auto invalidFilePath = QString("%1/%2").arg(utDirPath).arg("invalid.conf");

    QFile invalidFile(invalidFilePath);
    invalidFile.open(QIODevice::WriteOnly | QIODevice::Append | QFile::Text);
    QTextStream tsm(&invalidFile);
    tsm.setCodec("UTF-8");
    //入口分组不正确，预期拿不到数据
    tsm << QString("[Menu E]n[try～·。、\"’”！@#￥%……&*（）()-+*./{}【】|\\?<>）]").toUtf8() << endl;
    tsm << QString("Comment=This is a test file of compress!!!").toUtf8() <<endl;
    tsm << QString("Comment[zh_CN]=这是一个s测试文件").toUtf8() <<endl;
    tsm << QString("Version=Uos1.0").toUtf8() <<endl;
    tsm << QString("Actions=Zero:decompress").toUtf8() <<endl;
    tsm << QString("[Menu Action Zero]").toUtf8() <<endl;
    tsm << QString("Exec=/usr/bin/deepin-compressor %U compress").toUtf8() <<endl;
    tsm << QString("Name=Compressor").toUtf8() <<endl;
    tsm << QString("Name[zh_CN]=New压缩-压缩到\"%d\"").toUtf8() <<endl;
    tsm << QString("Name[en]=Compress").toUtf8() <<endl;
    tsm << QString("X-DFM-MenuTypes=MultiFiles:MultiDirs:SingleFile:SingleDir").toUtf8() <<endl;
    tsm << QString("X-DFM-SupportSchemes=file").toUtf8() <<endl;
    tsm << QString("PosNum=1").toUtf8() <<endl;
    tsm << QString("Separator=Top").toUtf8() <<endl;
    tsm << QString("MimeType=*").toUtf8() <<endl;

    tsm.flush();
    invalidFile.flush();

    auto expectValue2 = m_parser->loadDir(utDirPath);
    EXPECT_TRUE(expectValue2);
    EXPECT_TRUE(0 == m_parser->m_actionEntry.size());
    invalidFile.close();

}

TEST_F(TestDCustomActionParser, test_read_Conf_Three){
    //分组信息测试
    auto invalidFilePath = QString("%1/%2").arg(utDirPath).arg("invalid.conf");

    QFile invalidFile(invalidFilePath);
    invalidFile.open(QIODevice::WriteOnly | QIODevice::Append | QFile::Text);
    QTextStream tsm(&invalidFile);
    tsm.setCodec("UTF-8");
    //分组正确，预期正确获取到数据
    tsm << QString("[Menu Entry]").toUtf8() << endl;
    tsm << QString("Comment=This is a test file of compress!!!").toUtf8() <<endl;
    tsm << QString("Comment[zh_CN]=这是一个s测试文件").toUtf8() <<endl;
    tsm << QString("Version=Uos1.0").toUtf8() <<endl;
    tsm << QString("Actions=～·。、\"’”！@#￥%……&*（）()-+*./{}【】[]|\\?<>）").toUtf8() <<endl;
    tsm << QString("[Menu Action ～·。、\"’”！@#￥%……&*（）()-+*./{}【】[]|\\?<>）]").toUtf8() <<endl;
    tsm << QString("Exec=/usr/bin/deepin-compressor %U compress").toUtf8() <<endl;
    tsm << QString("Name=Compressor").toUtf8() <<endl;
    tsm << QString("Name[zh_CN]=压缩").toUtf8() <<endl;
    tsm << QString("Name[en]=Compress").toUtf8() <<endl;
    tsm << QString("X-DFM-MenuTypes=MultiFiles:MultiDirs:SingleFile:SingleDir").toUtf8() <<endl;
    tsm << QString("X-DFM-SupportSchemes=file").toUtf8() <<endl;
    tsm << QString("PosNum=1").toUtf8() <<endl;
    tsm << QString("Separator=Top").toUtf8() <<endl;
    tsm << QString("MimeType=*").toUtf8() <<endl;

    tsm.flush();
    invalidFile.flush();

    auto expectValue2 = m_parser->loadDir(utDirPath);
    EXPECT_TRUE(expectValue2);

    if (0 < m_parser->m_actionEntry.size())
        EXPECT_TRUE(m_parser->m_actionEntry.at(0).data().name() == QString("压缩"));
    invalidFile.close();
}

TEST_F(TestDCustomActionParser, test_read_Conf_Four){
    //字段信息测试
    auto invalidFilePath = QString("%1/%2").arg(utDirPath).arg("invalid.conf");

    QFile invalidFile(invalidFilePath);
    invalidFile.open(QIODevice::WriteOnly | QIODevice::Append | QFile::Text);
    QTextStream tsm(&invalidFile);
    tsm.setCodec("UTF-8");
    //分组正确，但是找不到name对应语言区域字段信息，预期只能拿到name:Compressor
    tsm << QString("[Menu Entry]").toUtf8() << endl;
    tsm << QString("Comment=This is a test file of compress!!").toUtf8() <<endl;
    tsm << QString("Comment[zh_CN]=这是一个s测试文件").toUtf8() <<endl;
    tsm << QString("Version=Uos1.0").toUtf8() <<endl;
    tsm << QString("Actions=～·。、\"’”！@#￥%……&*（）()-+*./{}【】[]|\\?<>）").toUtf8() <<endl;
    tsm << QString("[Menu Action ～·。、\"’”！@#￥%……&*（）()-+*./{}【】[]|\\?<>）]").toUtf8() <<endl;
    tsm << QString("Exec=/usr/bin/deepin-compressor %U compress").toUtf8() <<endl;
    tsm << QString("Name=Compressor").toUtf8() <<endl;
    tsm << QString("Name[zh_CN]～·。、\"’”！@#￥%……&*（）()-+*./{}【】[]|\\?<>）=压缩").toUtf8() <<endl;
    tsm << QString("Name[en]=Compress").toUtf8() <<endl;
    tsm << QString("X-DFM-MenuTypes=MultiFiles:MultiDirs:SingleFile:SingleDir").toUtf8() <<endl;
    tsm << QString("X-DFM-SupportSchemes=file").toUtf8() <<endl;
    tsm << QString("PosNum=1").toUtf8() <<endl;
    tsm << QString("Separator=Top").toUtf8() <<endl;
    tsm << QString("MimeType=*").toUtf8() <<endl;

    tsm.flush();
    invalidFile.flush();
    invalidFile.close();

    auto expectValue2 = m_parser->loadDir(utDirPath);
    EXPECT_TRUE(expectValue2);

    if (0 < m_parser->m_actionEntry.size())
        EXPECT_TRUE(m_parser->m_actionEntry.at(0).data().name() == QString("Compressor"));
    invalidFile.close();
}

TEST_F(TestDCustomActionParser, test_read_Conf_Five){
    //字段信息测试
    auto invalidFilePath = QString("%1/%2").arg(utDirPath).arg("invalid.conf");

    QFile invalidFile(invalidFilePath);
    invalidFile.open(QIODevice::WriteOnly | QIODevice::Append | QFile::Text);
    QTextStream tsm(&invalidFile);
    tsm.setCodec("UTF-8");
    //分组正确，语言区域正确，预期拿到信息
    tsm << QString("[Menu Entry]").toUtf8() << endl;
    tsm << QString("Comment=This is a test file of compress!!").toUtf8() <<endl;
    tsm << QString("Comment[zh_CN]=这是一个s测试文件").toUtf8() <<endl;
    tsm << QString("Version=Uos1.0").toUtf8() <<endl;
    tsm << QString("Actions=～·。、\"’”！@#￥%……&*（）()-+*./{}【】[]|\\?<>）").toUtf8() <<endl;
    tsm << QString("[Menu Action ～·。、\"’”！@#￥%……&*（）()-+*./{}【】[]|\\?<>）]").toUtf8() <<endl;
    tsm << QString("Exec=/usr/bin/deepin-compressor %U compress").toUtf8() <<endl;
    tsm << QString("Name=Compressor").toUtf8() <<endl;
    tsm << QString("Name[zh_CN]=～·。、\"’”！@#￥%……&*（）()-+*./{}【】[]|\\?<>）压缩").toUtf8() <<endl;
    tsm << QString("Name[en]=Compress").toUtf8() <<endl;
    tsm << QString("X-DFM-MenuTypes=MultiFiles:MultiDirs:SingleFile:SingleDir").toUtf8() <<endl;
    tsm << QString("X-DFM-SupportSchemes=file").toUtf8() <<endl;
    tsm << QString("PosNum=1").toUtf8() <<endl;
    tsm << QString("Separator=Top").toUtf8() <<endl;
    tsm << QString("MimeType=*").toUtf8() <<endl;

    tsm.flush();
    invalidFile.flush();

    auto expectValue2 = m_parser->loadDir(utDirPath);
    EXPECT_TRUE(expectValue2);

    if (0 < m_parser->m_actionEntry.size())
        EXPECT_TRUE(m_parser->m_actionEntry.at(0).data().name() == QString("～·。、\"’”！@#￥%……&*（）()-+*./{}【】[]|\\?<>）压缩"));
    invalidFile.close();
}

TEST_F(TestDCustomActionParser, test_read_Conf_Six){
    //字段信息测试
    auto invalidFilePath = QString("%1/%2").arg(utDirPath).arg("invalid.conf");

    QFile invalidFile(invalidFilePath);
    invalidFile.open(QIODevice::WriteOnly | QIODevice::Append | QFile::Text);
    QTextStream tsm(&invalidFile);
    tsm.setCodec("UTF-8");
    tsm << QString("[Menu Entry]").toUtf8() << endl;
    tsm << QString("Comment=This is a test file of compress!!").toUtf8() <<endl;
    tsm << QString("Comment[zh_CN]=这是一个s测试文件").toUtf8() <<endl;
    tsm << QString("Version=Uos1.0").toUtf8() <<endl;
    tsm << QString("Actions=～·。、\"’”！@#￥%……&*（）()-+*./{}【】[]|\\?<>）a测试：:\"Test\"").toUtf8() <<endl;
    tsm << QString("[Menu Action ～·。、\"’”！@#￥%……&*（）()-+*./{}【】[]|\\?<>）a测试：]").toUtf8() <<endl;
    tsm << QString("Exec=/usr/bin/deepin-compressor %U compress").toUtf8() <<endl;
    tsm << QString("Name=Compressor").toUtf8() <<endl;
    tsm << QString("Name[zh_CN]=～·。、\"’”！@#￥%……&*（）()-+*./{}【】[]|\\?<>）压缩").toUtf8() <<endl;
    tsm << QString("Name[en]=Compress").toUtf8() <<endl;
    tsm << QString("X-DFM-MenuTypes=MultiFiles:MultiDirs:SingleFile:SingleDir").toUtf8() <<endl;
    tsm << QString("X-DFM-SupportSchemes=file").toUtf8() <<endl;
    tsm << QString("PosNum=1.1").toUtf8() <<endl;
    tsm << QString("Separator=Top").toUtf8() <<endl;
    tsm << QString("MimeType=*").toUtf8() <<endl;

    tsm << QString("[Menu Action \"Test\"]").toUtf8() <<endl;
    tsm << QString("Exec=/usr/bin/deepin-compressor %U compress").toUtf8() <<endl;
    tsm << QString("Name=Compressor").toUtf8() <<endl;
    tsm << QString("Name[zh_CN]=压缩2").toUtf8() <<endl;
    tsm << QString("Name[en]=Compress").toUtf8() <<endl;
    tsm << QString("X-DFM-MenuTypes=MultiFiles:MultiDirs:SingleFile:SingleDir").toUtf8() <<endl;
    tsm << QString("X-DFM-SupportSchemes=file").toUtf8() <<endl;
    tsm << QString("PosNum=4/2").toUtf8() <<endl;
    tsm << QString("Separator=Top").toUtf8() <<endl;
    tsm << QString("MimeType=*").toUtf8() <<endl;

    tsm.flush();
    invalidFile.flush();

    auto expectValue1 = m_parser->loadDir(utDirPath);
    EXPECT_TRUE(expectValue1);

    if (2 == m_parser->m_actionEntry.size()) {
        //不按照规范配置位置信息预期获取位置都为0
        EXPECT_TRUE(m_parser->m_actionEntry.at(0).data().name() == QString("～·。、\"’”！@#￥%……&*（）()-+*./{}【】[]|\\?<>）压缩"));
        EXPECT_TRUE(0 == m_parser->m_actionEntry.at(0).data().position());
        EXPECT_TRUE(m_parser->m_actionEntry.at(1).data().name() == QString("压缩2"));
        EXPECT_TRUE(0 == m_parser->m_actionEntry.at(1).data().position());
    }

    invalidFile.close();
}

TEST(TestRegisterCustomFormat, test_write_Conf){
    QSettings::SettingsMap tempMap;
    EXPECT_TRUE(RegisterCustomFormat::instance().customFormat() != QSettings::defaultFormat());
}
