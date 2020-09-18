#include <gtest/gtest.h>
#include "commandlinemanager.h"
#include <QApplication>
#include <QtDebug>
#include <QTimer>
#include <QWindow>

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

TEST_F(CommandLineManagerTest, isSet)
{

    QStringList arguments;
    arguments << "dde-file-manager" << "-n";

    CommandLineManager::instance()->process(arguments);
    ASSERT_TRUE(CommandLineManager::instance()->isSet("n"));
}

TEST_F(CommandLineManagerTest, value)
{
    QString greeting = "hello";
    QStringList arguments;
    arguments << "dde-file-manager" << "-w" << greeting;

    CommandLineManager::instance()->process(arguments);

    ASSERT_TRUE(CommandLineManager::instance()->isSet("w"));
    ASSERT_EQ(greeting, CommandLineManager::instance()->value("w"));
}

TEST_F(CommandLineManagerTest, addOptions)
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

TEST_F(CommandLineManagerTest, processCommand_set_p)
{
    QStringList arguments;
    arguments << "dde-file-manager" << "-p" << "/home/yngty/Workspace/uos/dde-file-manager";
    int argc = 2;
    char *argv[10];
    argv[0] = const_cast<char *>("CommandLineManager_Test");
    argv[1] = const_cast<char *>("-p");
    argv[2] = const_cast<char *>("/home/yngty/Workspace/uos/dde-file-manager");
    QApplication app(argc, argv);

    CommandLineManager::instance()->process(arguments);
    CommandLineManager::instance()->processCommand();
    QTimer::singleShot(1000, nullptr, [&app] {
         app.closeAllWindows();
    });
    int r = app.exec();
    ASSERT_EQ(r, 0);
}
