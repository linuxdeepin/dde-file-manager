#include <gtest/gtest.h>

#include "stub.h"
#include "dfmfactoryloader.h"
#include "dfmbaseview.h"
#include "views/computerview.h"
#include "interfaces/plugins/dfmviewfactory.h"
#include "interfaces/plugins/private/dfmviewfactory_p.h"

DFM_USE_NAMESPACE

namespace  {
    class TestDFMViewFactory : public testing::Test
    {
    public:
        void SetUp() override
        {
            std::cout << "start TestDFMViewFactory";
        }
        void TearDown() override
        {
            std::cout << "end TestDFMViewFactory";
        }
    public:
    };


}

TEST_F(TestDFMViewFactory, testCreate)
{
    DFMBaseView *pview = DFMViewFactory::create("/views");
    EXPECT_EQ(pview, nullptr);
}

TEST_F(TestDFMViewFactory, testKeys)
{
    QMultiMap<int, QString>(*stub_keyMap)() = []()->QMultiMap<int, QString>{
        QMultiMap<int, QString> map;
        map.insert(0, "utest1");
        return map;
    };
    Stub stu;
    stu.set(ADDR(DFMFactoryLoader, keyMap), stub_keyMap);

    QStringList lst = DFMViewFactory::keys();
    EXPECT_EQ(lst.count(), 1);
}

TEST_F(TestDFMViewFactory, testViewIsSuitedWithUrl)
{
    QMap<const DFMBaseView*, int> map;
    ComputerView* view1 = new ComputerView();
    map.insert(view1, 1);

    DFMViewFactoryPrivate::viewToLoaderIndex = map;

    bool b = DFMViewFactory::viewIsSuitedWithUrl(view1, DUrl("file:///utest"));
    EXPECT_EQ(b, false);
}
