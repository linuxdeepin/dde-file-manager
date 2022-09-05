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
    QApplication a(argc, argv); // 这里必须用QApplication，否则UI相关东西要挂掉

    qDebug() << "start disk-mount test cases ..............";

    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();

    qDebug() << "end disk-mount test cases ..............";

    QProcess::execute("killall dde-file-manager");

#ifdef ENABLE_TSAN_TOOL
    __sanitizer_set_report_path("../../asan_dde-disk-mount.log");
#endif

    return ret;
}
