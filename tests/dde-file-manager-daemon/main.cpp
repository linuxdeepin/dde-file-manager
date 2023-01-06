// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QDebug>

int main(int argc, char *argv[])
{
    qDebug() << "start dde-file-manager-daemon test cases ..............";
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    qDebug() << "end dde-file-manager-daemon test cases ..............";
    return ret;
}
