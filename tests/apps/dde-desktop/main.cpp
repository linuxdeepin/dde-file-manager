// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAIN_CPP
#define MAIN_CPP

#include <gtest/gtest.h>
#include <sanitizer/asan_interface.h>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc,argv);

    ::testing::InitGoogleTest(&argc, argv);

    int ret = RUN_ALL_TESTS();

#ifdef ENABLE_TSAN_TOOL
    __sanitizer_set_report_path("../../../asan_dde-desktop.log");
#endif

    return ret;
}

#endif // MAIN_CPP
