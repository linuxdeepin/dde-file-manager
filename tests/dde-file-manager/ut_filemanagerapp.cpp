// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QApplication>
#include <QtDebug>
#include <QTimer>
#include <QWindow>
#include <QJsonDocument>
#include <QCommandLineParser>

#include "dialogs/openwithdialog.h"
#include "testhelper.h"
#define private public
#include "filemanagerapp.h"
#undef private
#include "stub.h"
#include "stubext.h"

#include "dfmeventdispatcher.h"
#include "filemanagerapp.h"
#include "dabstractfileinfo.h"
#include "dfileservices.h"
#include "dfmapplication.h"
#include "dfmstandardpaths.h"
#include "dialogs/dialogmanager.h"
#include "utils/rlog/rlog.h"

using namespace testing;
namespace  {
    class FileManagerAppTest : public Test
    {
    public:
        virtual void SetUp() override{

        }

        virtual void TearDown() override{

        }
    };

}

static bool judge = false;

TEST_F(FileManagerAppTest, test_showPropertyDialog)
{
    TestHelper::runInLoop([]{});

    QString filePath = TestHelper::createTmpFile();

    stub_ext::StubExt stu;
    bool judge = false;
    stu.set_lamda(ADDR(DialogManager, showPropertyDialog), [&judge](){judge = true;return;});
    FileManagerApp::instance()->showPropertyDialog(QStringList() << filePath);
    TestHelper::deleteTmpFile(filePath);
    EXPECT_TRUE(judge);
}

TEST_F(FileManagerAppTest, test_showPropertyDialog_computer)
{
    stub_ext::StubExt stu;
    bool judge = false;
    stu.set_lamda(ADDR(DialogManager, showComputerPropertyDialog), [&judge](){judge = true;return;});
    FileManagerApp::instance()->showPropertyDialog(QStringList() <<  DUrl::fromComputerFile("/").toString() <<  "/usr/share/applications/dde-computer.desktop");
    EXPECT_TRUE(judge);
}

TEST_F(FileManagerAppTest, test_showPropertyDialog_trash)
{
    stub_ext::StubExt stu;
    bool judge = false;
    stu.set_lamda(ADDR(DialogManager, showTrashPropertyDialog), [&judge](){judge = true;return;});
    FileManagerApp::instance()->showPropertyDialog(QStringList() << DUrl::fromTrashFile("/").toString() << "/usr/share/applications/dde-trash.desktop");
    EXPECT_TRUE(judge);
}

TEST_F(FileManagerAppTest, test_showPropertyDialog_empty)
{
    stub_ext::StubExt stu;
    bool judge = false;
    stu.set_lamda(ADDR(DialogManager, showPropertyDialog), [&judge](){judge = true;return;});
    FileManagerApp::instance()->showPropertyDialog(QStringList());
    EXPECT_FALSE(judge);
}

TEST_F(FileManagerAppTest, test_openWithDialog)
{
    QString filePath = TestHelper::createTmpFile();

    stub_ext::StubExt stu;
    bool judge = false;
    stu.set_lamda(ADDR(DialogManager, showOpenFilesWithDialog), [&judge](){judge = true;return;});
    FileManagerApp::instance()->openWithDialog(QStringList() << filePath);
    TestHelper::deleteTmpFile(filePath);
    EXPECT_TRUE(judge);

}

TEST_F(FileManagerAppTest, test_openWithDialog_computer)
{
    stub_ext::StubExt stu;
    bool judge = false;
    stu.set_lamda(ADDR(DialogManager, showComputerPropertyDialog), [&judge](){judge = true;return;});
    FileManagerApp::instance()->openWithDialog(QStringList() <<  DUrl::fromComputerFile("/").toString() <<  "/usr/share/applications/dde-computer.desktop");
    EXPECT_TRUE(judge);
}

TEST_F(FileManagerAppTest, test_openWithDialog_trash)
{
    stub_ext::StubExt stu;
    bool judge = false;
    stu.set_lamda(ADDR(DialogManager, showTrashPropertyDialog), [&judge](){judge = true;return;});
    FileManagerApp::instance()->openWithDialog(QStringList() << DUrl::fromTrashFile("/").toString() << "/usr/share/applications/dde-trash.desktop");
    EXPECT_TRUE(judge);
}

TEST_F(FileManagerAppTest, test_openWithDialog_empty)
{
    stub_ext::StubExt stu;
    bool judge = false;
    stu.set_lamda(ADDR(DialogManager, showPropertyDialog), [&judge](){judge = true;return;});
    FileManagerApp::instance()->openWithDialog(QStringList());
    EXPECT_FALSE(judge);
}

TEST_F(FileManagerAppTest, test_lazyRunTask)
{
    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(FileManagerApp, initService), [](){return;});
    stu.set_lamda(ADDR(RLog, commit), [](){return;});
    EXPECT_NO_FATAL_FAILURE(FileManagerApp::instance()->lazyRunTask());
}
