#include <gtest/gtest.h>
#include <sanitizer/asan_interface.h>
#define private public
#include "singleapplication.h"
#undef private
#include "testhelper.h"

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
    TestHelper::runInLoop([&]{
        app.handleQuitAction();
    });
    app.closeServer();

#ifdef ENABLE_TSAN_TOOL
    __sanitizer_set_report_path("../../asan_dde-file-manager.log");
#endif

    return ret;
}
