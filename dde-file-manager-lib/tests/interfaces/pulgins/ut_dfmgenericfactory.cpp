#include <gtest/gtest.h>

#include "stub.h"
#include "dfmfactoryloader.h"
#include "interfaces/plugins/dfmgenericfactory.h"

DFM_USE_NAMESPACE

namespace  {
    class TestDFMGenericFactory : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new DFMGenericFactory();
            std::cout << "start TestDFMGenericFactory";
        }
        void TearDown() override
        {
            std::cout << "end TestDFMGenericFactory";
        }
    public:
        DFMGenericFactory *m_pTester;

    };
}

TEST_F(TestDFMGenericFactory, testCreate)
{
    QObject * pr = DFMGenericFactory::create("video/*");
    EXPECT_EQ(pr, nullptr);
}

TEST_F(TestDFMGenericFactory, testcreateAll)
{
    QObjectList lst = DFMGenericFactory::createAll("video/*");
    EXPECT_EQ(lst.count(), 0);
}

TEST_F(TestDFMGenericFactory, testKeys)
{
    QMultiMap<int, QString>(*stub_keyMap)() = []()->QMultiMap<int, QString>{
        QMultiMap<int, QString> map;
        map.insert(0, "utest1");
        return map;
    };
    Stub stu;
    stu.set(ADDR(DFMFactoryLoader, keyMap), stub_keyMap);
    QStringList lst = DFMGenericFactory::keys();
    EXPECT_EQ(lst.count(), 1);
}
