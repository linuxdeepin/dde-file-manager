// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <sanitizer/asan_interface.h>
#include <QDebug>
#include <QApplication>
#include <QProcess>

int main(int argc, char *argv[])
{
    //fix icon get is empty ut test
    if (qEnvironmentVariableIsEmpty("XDG_CURRENT_DESKTOP")) {
        qputenv("XDG_CURRENT_DESKTOP", "Deepin");
    }

    QApplication app(argc,argv);

    qInfo() << "start test cases ..............";
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    qInfo() << "end test cases ..............";
    QProcess::execute("killall dde-file-manager");

#ifdef ENABLE_TSAN_TOOL
    __sanitizer_set_report_path("../../asan_dde-file-manager-lib.log");
#endif

    return ret;
}
