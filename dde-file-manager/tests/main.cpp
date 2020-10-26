//#include "interfaces/tst_all_interfaces.h"

#include <gtest/gtest.h>
#include <QDebug>

static void noMessageHandler(QtMsgType type, const QMessageLogContext &context,
                                   const QString &message)
{
    return;
}
int main(int argc, char *argv[])
{
    qDebug() << "start dde-file-manager test cases ..............";
    //不打印应用的输出日志
    qInstallMessageHandler(noMessageHandler);
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    qDebug() << "end dde-file-manager test cases ..............";
    return ret;
}
