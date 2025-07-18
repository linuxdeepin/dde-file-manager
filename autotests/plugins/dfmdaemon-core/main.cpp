// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QCoreApplication>
#include "dfm_asan_helper.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    ::testing::InitGoogleTest(&argc, argv);

    int ret = RUN_ALL_TESTS();

    DFM_SETUP_ASAN_REPORT(dfmdaemon_core);

    return ret;
}
