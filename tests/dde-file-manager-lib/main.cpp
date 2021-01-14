#include <gtest/gtest.h>
#include <QDebug>
#include <QApplication>
#include <QProcess>

int main(int argc, char *argv[])
{
    QApplication app(argc,argv);
    qDebug() << "start test cases ..............";
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    qDebug() << "end test cases ..............";
    QProcess::execute("killall dde-file-manager");
    QProcess::execute("killall deepin-editor");
    return ret;
}
