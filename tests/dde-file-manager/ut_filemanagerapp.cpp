/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: yanghao<yanghao@uniontech.com>
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
