#include <gtest/gtest.h>
#include "commandlinemanager.h"
#include <QApplication>
#include <QtDebug>
#include <QTimer>
#include <QWindow>
#include "dialogs/openwithdialog.h"
#include "testhelper.h"

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
    QEventLoop loop;
    QTimer timer;
    timer.start(2000);
    QStringList arguments;
    QString tmpDir = TestHelper::createTmpDir();
    arguments << "dde-file-manager" << "-p" << tmpDir;
    QWindowList windows =  qApp->topLevelWindows();
    CommandLineManager::instance()->process(arguments);
    CommandLineManager::instance()->processCommand();
    QObject::connect(&timer, &QTimer::timeout, [&]{
        QWindowList newWindows =  qApp->topLevelWindows();
        EXPECT_GT(newWindows.count(), windows.count());
        timer.stop();
        loop.exit();
        TestHelper::deleteTmpFile(tmpDir);
    });
    loop.exec();
}

TEST_F(CommandLineManagerTest, processCommand_set_o)
{
    QEventLoop loop;
    QTimer timer;
    timer.start(2000);
    QStringList arguments;
    QString tmpFile = TestHelper::createTmpFile(".txt");
    arguments << "dde-file-manager" << "-o" << tmpFile;
    QWindowList windows =  qApp->topLevelWindows();

    CommandLineManager::instance()->process(arguments);
    CommandLineManager::instance()->processCommand();
    QObject::connect(&timer, &QTimer::timeout, [&]{
        QWindowList newWindows =  qApp->topLevelWindows();
        EXPECT_GT(newWindows.count(), windows.count());
        timer.stop();
        loop.exit();
        TestHelper::deleteTmpFile(tmpFile);
    });
    loop.exec();
}

TEST_F(CommandLineManagerTest, processCommand_show_computer)
{
    QEventLoop loop;
    QTimer timer;
    timer.start(2000);
    QStringList arguments;
    arguments << "dde-file-manager" << "-p" << "computer:///";
    QWindowList windows =  qApp->topLevelWindows();
    CommandLineManager::instance()->process(arguments);
    CommandLineManager::instance()->processCommand();
    QObject::connect(&timer, &QTimer::timeout, [&]{
        QWindowList newWindows =  qApp->topLevelWindows();
        EXPECT_GT(newWindows.count(), windows.count());
        timer.stop();
        loop.exit();
    });
    loop.exec();
}

TEST_F(CommandLineManagerTest, processCommand_show_trash)
{
    QEventLoop loop;
    QTimer timer;
    timer.start(2000);
    QStringList arguments;
    arguments << "dde-file-manager" << "-p" << "trash:///";
    QWindowList windows =  qApp->topLevelWindows();
    CommandLineManager::instance()->process(arguments);
    CommandLineManager::instance()->processCommand();
    QObject::connect(&timer, &QTimer::timeout, [&]{
        QWindowList newWindows =  qApp->topLevelWindows();
        EXPECT_GT(newWindows.count(), windows.count());
        timer.stop();
        loop.exit();
    });
    loop.exec();
}
