// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <sanitizer/asan_interface.h>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    ::testing::InitGoogleTest(&argc, argv);

    int ret = RUN_ALL_TESTS();

#ifdef ENABLE_TSAN_TOOL
    __sanitizer_set_report_path("../../../asan_dde-file-manager.log");
#endif

    return ret;
}
