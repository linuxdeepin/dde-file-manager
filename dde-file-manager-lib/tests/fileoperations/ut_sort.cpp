#include <gtest/gtest.h>
#include "fileoperations/sort.h"

#include <QString>
#include <QProcess>


using namespace testing;


TEST(savedirTest,AnyAgruInput){
    EXPECT_FALSE(QString(savedir("~/Pictures")).isNull());
    QProcess::execute("mkdir ~/Pictures/test_temp");
    QProcess::execute("chmode 0000 ~/Pictures/test_temp");
    EXPECT_TRUE(QString(savedir("~/Pictures/test_temp")).isNull());
    QProcess::execute("chmode 0700 ~/Pictures/test_temp");
    QProcess::execute("rm -r ~/Pictures/test_temp");
}
