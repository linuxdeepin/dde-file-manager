// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <sanitizer/asan_interface.h>
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
    //qInstallMessageHandler(noMessageHandler);
    int res = RUN_ALL_TESTS();

#ifdef ENABLE_TSAN_TOOL
    __sanitizer_set_report_path("../../asan_dde-desktop.log");
#endif

    return res;
}
