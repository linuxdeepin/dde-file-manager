// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "fileoperations/sort.h"
#include "testhelper.h"
#include "stub.h"

#include <QString>
#include <QProcess>

#define Free(x) if(x){free(x);x=nullptr;}

using namespace testing;

#ifndef __arm__
TEST(savedirTest,AnyAgruInput){
    Stub stl;
    TestHelper::runInLoop([](){});

    direntry_t stl1,stl2;
    char tt[10] = {'a'};
    stl1.name = tt;
    stl1.ino = 9;
    stl2.ino = 10;
    stl2.name = tt;
    EXPECT_TRUE(direntry_cmp_inode(&stl1,&stl2));
    EXPECT_TRUE(direntry_cmp_inode(&stl1,&stl2));
    char* name_space = savedir("~/Pictures");
    EXPECT_TRUE(QString(name_space).isNull());
    Free(name_space);

    QProcess::execute("mkdir /tmp/test_temp_sourt");
    QProcess::execute("chmode 0000 /tmp/test_temp_sourt");
    name_space = savedir("/tmp/test_temp_sourt");
    EXPECT_FALSE(QString(name_space).isNull());
    Free(name_space);

    name_space = savedir("/tmp");
    EXPECT_FALSE(QString(name_space).isNull());
    Free(name_space);

    name_space = savedir("/usr");
    EXPECT_FALSE(QString(name_space).isNull());
    Free(name_space);

    int (*closedir1)(DIR *) = [](DIR *){return -1;};
    stl.set(closedir,closedir1);

    name_space = savedir("/tmp/test_temp_sourt");
    EXPECT_TRUE(QString(name_space).isNull());
    QProcess::execute("chmode 0700 /tmp/test_temp_sourt");
    QProcess::execute("rm -r /tmp/test_temp_sourt");

    Free(name_space);
}
#endif
