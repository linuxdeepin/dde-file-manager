#include <gtest/gtest.h>

#include "shutil/checknetwork.h"
#include <QSharedPointer>
#include "testhelper.h"

namespace  {
    class TestCheckNetwork : public testing::Test {
    public:
        void SetUp() override
        {
           check.reset(new CheckNetwork());
        }
        void TearDown() override
        {
        }

    public:
            QSharedPointer<CheckNetwork> check = nullptr;
    };
}

TEST_F(TestCheckNetwork,can_isHostAndPortConnect){
    TestHelper::runInLoop([](){});
    EXPECT_FALSE(check->isHostAndPortConnect("127.0.0.1","20"));
    EXPECT_TRUE(check->isHostAndPortConnect("127.0.0.1","139"));
    EXPECT_FALSE(check->isHostAndPortConnect("10.8.40.125","20"));

}
