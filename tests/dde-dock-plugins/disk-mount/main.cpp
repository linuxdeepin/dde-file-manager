/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     max-lv<lvwujun@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
