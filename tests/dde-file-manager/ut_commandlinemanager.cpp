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

#include "dialogs/openwithdialog.h"
#include "testhelper.h"
#define private public
#include "commandlinemanager.h"
#undef private
#include "stub.h"
#include "stubext.h"

#include "dfmeventdispatcher.h"
#include "filemanagerapp.h"
#include "dabstractfileinfo.h"
#include "dfileservices.h"
#include "dfmapplication.h"
#include "dfmstandardpaths.h"

#include <QApplication>
#include <QtDebug>
#include <QTimer>
#include <QWindow>
#include <QJsonDocument>
#include <QCommandLineParser>

using namespace testing;
namespace  {
    class CommandLineManagerTest : public Test
    {
    public:
        virtual void SetUp() override{

        }

        virtual void TearDown() override{

        }
    };

}

static bool judge = false;
TEST_F(CommandLineManagerTest, test_isSet)
{
    TestHelper::runInLoop([]{});
    QStringList arguments;
    arguments << "dde-file-manager" << "-n";
    CommandLineManager::instance()->process(arguments);
    ASSERT_TRUE(CommandLineManager::instance()->isSet("n"));
}

TEST_F(CommandLineManagerTest, test_value)
{
    QString greeting = "hello";
    QStringList arguments;
    arguments << "dde-file-manager" << "-w" << greeting;
    CommandLineManager::instance()->process(arguments);
    ASSERT_TRUE(CommandLineManager::instance()->isSet("w"));
    ASSERT_EQ(greeting, CommandLineManager::instance()->value("w"));
}
TEST_F(CommandLineManagerTest, test_unknown_option)
{
    QString greeting = "hello";
    QStringList arguments;
    arguments << "dde-file-manager" << "-x";
    CommandLineManager::instance()->m_commandParser->parse(arguments);
    QStringList unknownOptionNames = CommandLineManager::instance()->unknownOptionNames();
    ASSERT_TRUE(unknownOptionNames.contains("x"));
}

TEST_F(CommandLineManagerTest, test_addOptions)
{

    QCommandLineOption option1(QStringList() << "x" << "open-x", "description x", "x");
    QCommandLineOption option2(QStringList() << "y" << "open-y", "description y", "y");
    QList<QCommandLineOption> newOptions;
    newOptions << option1 << option2;
    CommandLineManager::instance()->addOptions(newOptions);

    QString greeting = "hello";
    QStringList arguments;
    arguments << "dde-file-manager" << "-x" << greeting << "-y" << "y";

    CommandLineManager::instance()->process(arguments);

    ASSERT_TRUE(CommandLineManager::instance()->isSet("x"));
    ASSERT_TRUE(CommandLineManager::instance()->isSet("y"));
    ASSERT_EQ(greeting, CommandLineManager::instance()->value("x"));
}

TEST_F(CommandLineManagerTest, test_processCommand_set_p)
{
    QStringList arguments;
    QString tmpDir = TestHelper::createTmpDir();
    arguments << "dde-file-manager" << "-p" << tmpDir;
    CommandLineManager::instance()->process(arguments);

    stub_ext::StubExt stu;
    bool judge = false;
    stu.set_lamda(ADDR(FileManagerApp, showPropertyDialog), [&judge](){
        judge = true;
        return
    ;});
    CommandLineManager::instance()->processCommand();
    TestHelper::deleteTmpFile(tmpDir);
    EXPECT_TRUE(judge);
}

TEST_F(CommandLineManagerTest, test_processCommand_set_o)
{
    QStringList arguments;
    QString tmpFile = TestHelper::createTmpFile(".txt");
    arguments << "dde-file-manager" << "-o" << tmpFile;
    QWindowList windows =  qApp->topLevelWindows();
    CommandLineManager::instance()->process(arguments);
    stub_ext::StubExt stu;
    bool judge = false;
    stu.set_lamda(ADDR(FileManagerApp, openWithDialog), [&judge](){judge = true;return;});
    CommandLineManager::instance()->processCommand();
    TestHelper::deleteTmpFile(tmpFile);
    EXPECT_TRUE(judge);
}

TEST_F(CommandLineManagerTest, test_process)
{
    void(*stub_process)(const QStringList &) = [](const QStringList &) {
        judge = true;
        return;
    };

    Stub stub;
    auto addr = (void(QCommandLineParser::*)(const QStringList &))ADDR(QCommandLineParser, process);
    stub.set(addr, stub_process);
    CommandLineManager::instance()->process();
    EXPECT_TRUE(judge);
}

TEST_F(CommandLineManagerTest, test_processCommand_set_e)
{
    QStringList arguments;
    QJsonObject object;
    object.insert("eventType", "TouchFile");
    QString filePath = "/tmp/test_processCommand_set_e";
    object.insert("url", filePath);
    QString jsonStr = QString(QJsonDocument(object).toJson());
    arguments << "dde-file-manager" << "-e" << jsonStr;
    CommandLineManager::instance()->process(arguments);

    judge = false;

    QVariant(*stub_processEvent)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = [](const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        judge = true;
        return QVariant();
    };

    Stub stub;
    stub.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))ADDR(DFMEventDispatcher, processEvent), stub_processEvent);

    CommandLineManager::instance()->processCommand();

    EXPECT_TRUE(judge);

}

TEST_F(CommandLineManagerTest, test_processCommand_set_O)
{
    QWindowList windows =  qApp->topLevelWindows();
    QStringList arguments;
    arguments << "dde-file-manager" << "-O";
    CommandLineManager::instance()->process(arguments);

    stub_ext::StubExt stu;

    judge = false;

    QVariant(*stub_processEvent)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = [](const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        judge = true;
        return QVariant();
    };

    Stub stub;
    stub.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))ADDR(DFMEventDispatcher, processEvent), stub_processEvent);

    CommandLineManager::instance()->processCommand();

    EXPECT_TRUE(judge);
}

TEST_F(CommandLineManagerTest, test_processCommand_show_item)
{
    QWindowList windows =  qApp->topLevelWindows();
    QStringList arguments;
    QString dirPath = TestHelper::createTmpDir();
    arguments << "dde-file-manager" << "--show-item" << dirPath;
    CommandLineManager::instance()->process(arguments);
    judge = false;

    QVariant(*stub_processEvent)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = [](const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        judge = true;
        return QVariant();
    };

    Stub stub;
    stub.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))ADDR(DFMEventDispatcher, processEvent), stub_processEvent);

    CommandLineManager::instance()->processCommand();

    EXPECT_TRUE(judge);
}


