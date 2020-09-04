//#include "interfaces/tst_all_interfaces.h"

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
