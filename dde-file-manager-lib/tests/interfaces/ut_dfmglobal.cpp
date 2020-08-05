#include "dfmglobal.h"

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

namespace  {
    class TestDFMGlobal : public testing::Test {
    public:
        void SetUp() override
        {
           std::cout << "start TestDFMGlobal";
        }
        void TearDown() override
        {
            std::cout << "end TestDFMGlobal";
        }
    };
}

//！ 测试文件名的命名规范
TEST_F(TestDFMGlobal, fileNameRule)
{
    // 剔除的字符 (^\\s+|[\"'/\\\\\[\\\]:|<>+=;,?*\r\n])
    QString strInput("\"\'/\\[]:|<>+=;,?*");
    QString strOutput = DFMGlobal::preprocessingFileName(strInput);
    EXPECT_STREQ(strOutput.toStdString().c_str(), "");
}
