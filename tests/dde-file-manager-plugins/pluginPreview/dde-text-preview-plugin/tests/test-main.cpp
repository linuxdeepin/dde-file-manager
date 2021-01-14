#include "interfaces/tst_all_interfaces.h"

#include <sanitizer/asan_interface.h>
#include <gtest/gtest.h>
#include <QDebug>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qDebug() << "start test cases ..............";
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    qDebug() << "end test cases ..............";

#ifdef ENABLE_TSAN_TOOL
    __sanitizer_set_report_path("../../../../../build-ut/asan_dde-text-preview-plugin.log");
#endif
    return ret;
}
