// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-base/base/application/application.h>

#include <gtest/gtest.h>
#include <sanitizer/asan_interface.h>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    auto appins = new dfmbase::Application();

    ::testing::InitGoogleTest(&argc, argv);

    int ret = RUN_ALL_TESTS();

    delete appins;

#ifdef ENABLE_TSAN_TOOL
    __sanitizer_set_report_path("../../../asan_base.log");
#endif

    return ret;
}
