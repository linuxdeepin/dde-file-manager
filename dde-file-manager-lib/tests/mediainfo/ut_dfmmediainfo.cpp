#include <gtest/gtest.h>
#include "mediainfo/dfmmediainfo.h"

DFM_USE_NAMESPACE

namespace {
class TestDFMMediaInfo : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestDFMMediaInfo";
        info = new DFMMediaInfo("test.mp3", nullptr);
    }

    void TearDown() override
    {
        std::cout << "end TestDFMMediaInfo";
        delete info;
        info = nullptr;
    }

public:
    DFMMediaInfo *info = nullptr;
};
} // namespace

TEST_F(TestDFMMediaInfo, StringPropertyTest)
{
    info->startReadInfo();
    QThread::msleep(200);
    EXPECT_STREQ("", info->Value("").toStdString().c_str());
}
