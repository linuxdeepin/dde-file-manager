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
#include "commandlinemanager.h"
#undef private

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

TEST_F(CommandLineManagerTest, test_isSet)
{
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
    QWindowList windows =  qApp->topLevelWindows();
    CommandLineManager::instance()->process(arguments);
    TestHelper::runInLoop([]{
        CommandLineManager::instance()->processCommand();
    }, 500);
    QWindowList newWindows =  qApp->topLevelWindows();
    EXPECT_GT(newWindows.count(), windows.count());
    TestHelper::deleteTmpFile(tmpDir);
}

TEST_F(CommandLineManagerTest, test_processCommand_set_o)
{
    QStringList arguments;
    QString tmpFile = TestHelper::createTmpFile(".txt");
    arguments << "dde-file-manager" << "-o" << tmpFile;
    QWindowList windows =  qApp->topLevelWindows();
    CommandLineManager::instance()->process(arguments);
    TestHelper::runInLoop([]{
        CommandLineManager::instance()->processCommand();
    }, 500);
    QWindowList newWindows =  qApp->topLevelWindows();
    EXPECT_GT(newWindows.count(), windows.count());
    TestHelper::deleteTmpFile(tmpFile);
}

TEST_F(CommandLineManagerTest, test_processCommand_show_computer)
{
    QStringList arguments;
    arguments << "dde-file-manager" << "-p" << "computer:///";
    QWindowList windows =  qApp->topLevelWindows();
    CommandLineManager::instance()->process(arguments);
    TestHelper::runInLoop([]{
        CommandLineManager::instance()->processCommand();
    }, 500);
    QWindowList newWindows =  qApp->topLevelWindows();
    EXPECT_GT(newWindows.count(), windows.count());
}

TEST_F(CommandLineManagerTest, test_processCommand_show_trash)
{
    QStringList arguments;
    arguments << "dde-file-manager" << "-p" << "trash:///";
    QWindowList windows =  qApp->topLevelWindows();
    CommandLineManager::instance()->process(arguments);
    TestHelper::runInLoop([]{
        CommandLineManager::instance()->processCommand();
    }, 500);
    QWindowList newWindows =  qApp->topLevelWindows();
    EXPECT_GT(newWindows.count(), windows.count());
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
    TestHelper::runInLoop([]{
        CommandLineManager::instance()->processCommand();
    }, 500);
    QFile file(filePath);
    ASSERT_TRUE(file.exists());
    file.remove();
}

TEST_F(CommandLineManagerTest, test_processCommand_set_O)
{
    QWindowList windows =  qApp->topLevelWindows();
    QStringList arguments;
    arguments << "dde-file-manager" << "-O";
    CommandLineManager::instance()->process(arguments);
    TestHelper::runInLoop([]{
        CommandLineManager::instance()->processCommand();
    }, 500);
    QWindowList newWindows =  qApp->topLevelWindows();
    EXPECT_GT(newWindows.count(), windows.count());
}

TEST_F(CommandLineManagerTest, test_processCommand)
{
    QWindowList windows =  qApp->topLevelWindows();
    QStringList arguments;
    QString dirPath = TestHelper::createTmpDir("111@");
    DUrl::fromLocalFile(dirPath);
    arguments << "dde-file-manager" <<  DUrl::fromLocalFile(dirPath).toString();
    CommandLineManager::instance()->process(arguments);
    TestHelper::runInLoop([]{
        CommandLineManager::instance()->processCommand();
    }, 500);
    TestHelper::deleteTmpFile(dirPath);
    QWindowList newWindows =  qApp->topLevelWindows();
    EXPECT_GT(newWindows.count(), windows.count());
}

TEST_F(CommandLineManagerTest, test_processCommand_show_item)
{
    QWindowList windows =  qApp->topLevelWindows();
    QStringList arguments;
    QString dirPath = TestHelper::createTmpDir();
    arguments << "dde-file-manager" << "--show-item" << dirPath;
    CommandLineManager::instance()->process(arguments);
    TestHelper::runInLoop([]{
        CommandLineManager::instance()->processCommand();
    }, 500);
    TestHelper::deleteTmpFile(dirPath);
    QWindowList newWindows =  qApp->topLevelWindows();
    EXPECT_GT(newWindows.count(), windows.count());
}

TEST_F(CommandLineManagerTest, test_processCommand_empty_arguments)
{
    QWindowList windows =  qApp->topLevelWindows();
    CommandLineManager::instance()->process();
    TestHelper::runInLoop([]{
        CommandLineManager::instance()->processCommand();
    }, 500);
    QWindowList newWindows =  qApp->topLevelWindows();
    EXPECT_GT(newWindows.count(), windows.count());
}

