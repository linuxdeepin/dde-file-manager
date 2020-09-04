#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QApplication>

static void noMessageHandler(QtMsgType type, const QMessageLogContext &context,
                                   const QString &message)
{
    return;
}

int main(int argc, char *argv[])
{
    QApplication app(argc,argv);
    testing::InitGoogleTest(&argc,argv);
    //不打印应用的输出日志
    qInstallMessageHandler(noMessageHandler);
    return RUN_ALL_TESTS();
}
