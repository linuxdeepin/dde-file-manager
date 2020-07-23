//#include "interfaces/tst_all_interfaces.h"

#include <gtest/gtest.h>
#include <QDebug>

int main(int argc, char *argv[])
{
    qDebug() << "start disk-mount test cases ..............";
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    qDebug() << "end disk-mount test cases ..............";
    return ret;
}
