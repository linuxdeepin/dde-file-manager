#include "dfmglobal.h"

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

namespace  {
class TestDListItemDelegate : public testing::Test
{
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
TEST_F(TestDListItemDelegate, fileNameRule)
{
}
