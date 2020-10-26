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

TEST_F(SingleApplicationTest, setSingleInstance)
{
    int argc = 1;
    char *argv[10];
    argv[0] = const_cast<char *>("SingleApplicationTest");
    SingleApplication::initSources();
    SingleApplication app(argc, argv);

    QString uniqueKey = app.applicationName();

    bool isSingleInstance = app.setSingleInstance(uniqueKey);
    ASSERT_TRUE(isSingleInstance);
    isSingleInstance = app.setSingleInstance(uniqueKey);
    ASSERT_FALSE(isSingleInstance);
}
