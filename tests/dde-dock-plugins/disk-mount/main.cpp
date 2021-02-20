#include <gtest/gtest.h>
#include <QDebug>
#include <QApplication>
#include <QProcess>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv); // 这里必须用QApplication，否则UI相关东西要挂掉

    qDebug() << "start disk-mount test cases ..............";

    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();

    qDebug() << "end disk-mount test cases ..............";

    QProcess::execute("killall dde-file-manager");

    return ret;
}
