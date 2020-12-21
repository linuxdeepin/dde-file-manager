#include <gtest/gtest.h>
#include "fileoperations/sort.h"
#include "testhelper.h"
#include "stub.h"

#include <QString>
#include <QProcess>


using namespace testing;


TEST(savedirTest,AnyAgruInput){
    Stub stl;

    direntry_t stl1,stl2;
    char tt[10] = {'a'};
    stl1.name = tt;
    stl1.ino = 9;
    stl2.ino = 10;
    stl2.name = tt;
    EXPECT_TRUE(direntry_cmp_inode(&stl1,&stl2));
    EXPECT_TRUE(direntry_cmp_inode(&stl1,&stl2));
    EXPECT_TRUE(QString(savedir("~/Pictures")).isNull());
    QProcess::execute("mkdir /tmp/test_temp_sourt");
    QProcess::execute("chmode 0000 /tmp/test_temp_sourt");
    EXPECT_FALSE(QString(savedir("/tmp/test_temp_sourt")).isNull());
    EXPECT_FALSE(QString(savedir("/tmp")).isNull());
    EXPECT_FALSE(QString(savedir("/usr")).isNull());
    int (*closedir1)(DIR *) = [](DIR *){return -1;};
    stl.set(closedir,closedir1);
    EXPECT_TRUE(QString(savedir("/tmp/test_temp_sourt")).isNull());
    QProcess::execute("chmode 0700 /tmp/test_temp_sourt");
    QProcess::execute("rm -r /tmp/test_temp_sourt");
}
