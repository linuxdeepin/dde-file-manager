// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-base/base/application/application.h>

#include <gtest/gtest.h>
#include <QApplication>
#include "dfm_asan_helper.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    auto appins = new dfmbase::Application();

    ::testing::InitGoogleTest(&argc, argv);

    int ret = RUN_ALL_TESTS();

    delete appins;

    DFM_SETUP_ASAN_REPORT(dfm_base);

    return ret;
}
