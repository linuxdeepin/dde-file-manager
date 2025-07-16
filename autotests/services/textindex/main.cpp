// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QApplication>
#include "dfm_asan_helper.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    DFM_SETUP_ASAN_REPORT(textindex_service);
    return ret;
} 