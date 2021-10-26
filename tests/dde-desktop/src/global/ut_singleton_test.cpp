#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#define protected public
#include "global/singleton.h"
#include "../utils/singleton.h"

class MyUTClass : public DDEDesktop::Singleton<MyUTClass>
{
};

TEST(Singleton, call_constructor)
{
    MyUTClass *p = MyUTClass::instance();
    EXPECT_NE(p, nullptr);
}

TEST(Singleton, call_copy_constructor)
{
    MyUTClass *p = new MyUTClass;
    EXPECT_NE(p, nullptr);

    MyUTClass q(*p);
    EXPECT_NE(nullptr, &q);

    delete p;
}

TEST(Singleton, call_assignment_operator)
{
    MyUTClass *p = new MyUTClass;
    EXPECT_NE(p, nullptr);

    MyUTClass q = *p;
    EXPECT_NE(nullptr, &q);

    delete p;
}

TEST(Singleton, conflict)
{
    EXPECT_NE(MyUTClass::instance(), Singleton<MyUTClass>::instance());
}
