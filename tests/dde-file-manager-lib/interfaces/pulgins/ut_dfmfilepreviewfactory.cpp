#include <gtest/gtest.h>
#include <qmap.h>

#include "stub.h"
#include "plugins/dfmfactoryloader.h"
#include "dialogs/filepreviewdialog.h"
#include "dfmbaseview.h"
#include "views/computerview.h"

#include "interfaces/plugins/dfmfilepreviewfactory.h"
#include "interfaces/plugins/private/dfmfilepreviewfactory_p.h"

DFM_USE_NAMESPACE

namespace  {
    class TestDFMFilePreviewFactory : public testing::Test
    {
    public:
        void SetUp() override
        {
            std::cout << "start TestDFMFilePreviewFactory";
        }
        void TearDown() override
        {
            std::cout << "end TestDFMFilePreviewFactory";
        }
    public:
    };
}

TEST_F(TestDFMFilePreviewFactory, testCreate)
{
    bool(*stub_isRootUser)() = []()->bool{
        return true;
    };
    Stub stu;
    stu.set(ADDR(DFMGlobal, isRootUser), stub_isRootUser);

    DFMFilePreview *pview = DFMFilePreviewFactory::create("/video");
    EXPECT_EQ(pview, nullptr);
}


TEST_F(TestDFMFilePreviewFactory, testKeys)
{
    QMultiMap<int, QString>(*stub_keyMap)() = []()->QMultiMap<int, QString>{
        QMultiMap<int, QString> map;
        map.insert(0, "utest1");
        return map;
    };
    Stub stu;
    stu.set(ADDR(DFMFactoryLoader, keyMap), stub_keyMap);

    QStringList lst = DFMFilePreviewFactory::keys();
    EXPECT_EQ(lst.count(), 1);
}

TEST_F(TestDFMFilePreviewFactory, testIsSuitedWithKey)
{
    QString index("");
    bool b = DFMFilePreviewFactory::isSuitedWithKey(nullptr, index);
    EXPECT_EQ(b, false);
}

TEST_F(TestDFMFilePreviewFactory, testIsSuitedWithKey2)
{
    QMap<const UnknowFilePreview*, int> map;
    UnknowFilePreview* view1 = new UnknowFilePreview();
    DFMFilePreviewFactoryPrivate::previewToLoaderIndex[view1] = 1;

    QString index("");
    bool b = DFMFilePreviewFactory::isSuitedWithKey(view1, index);
    EXPECT_EQ(b, false);
}
