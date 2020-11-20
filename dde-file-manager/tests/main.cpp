#include <gtest/gtest.h>
#include <gtest/gtest.h>
#define private public
#include "singleapplication.h"
#undef private

static void noMessageHandler(QtMsgType type, const QMessageLogContext &context,
                                   const QString &message)
{
    return;
}
int main(int argc, char *argv[])
{
    //不打印应用的输出日志
    SingleApplication app(argc,argv);
    SingleApplication::initSources();
    ::testing::InitGoogleTest(&argc, argv);
    qInstallMessageHandler(noMessageHandler);
    int ret = RUN_ALL_TESTS();
    app.handleQuitAction();
    return ret;
}
