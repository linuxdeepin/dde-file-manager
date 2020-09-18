#include <gtest/gtest.h>
#include "singleapplication.h"

using namespace testing;

namespace  {
    class SingleApplicationTest : public Test
    {
    public:
    private:
        virtual void SetUp() override{
        }

        virtual void TearDown() override{
        }
    };

}

TEST_F(SingleApplicationTest, registerLogger)
{
//    SingleApplication::initSources();
//    SingleApplication app(argc, argv);
}
