#include <gtest/gtest.h>
#include <QDir>
#include <QStringList>

#include "stub.h"

#include "interfaces/plugins/dfmfactoryloader.h"

DFM_USE_NAMESPACE

namespace  {
    class TestDFMFactoryLoader : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new DFMFactoryLoader("com.deepin.filemanager.DFMFileControllerFactoryInterface_iid",
                                             "/controllers",
                                             Qt::CaseInsensitive,
                                             false);
            std::cout << "start TestDFMFactoryLoader";
        }
        void TearDown() override
        {
            if(m_pTester)
                delete  m_pTester;
            std::cout << "end TestDFMFactoryLoader";
        }
    public:
        DFMFactoryLoader *m_pTester;
    };
}

TEST_F(TestDFMFactoryLoader, testInit)
{

}

TEST_F(TestDFMFactoryLoader, testMetaData)
{
    m_pTester->metaData();
}

TEST_F(TestDFMFactoryLoader, testInstance)
{
    m_pTester->instance(0);
}

#if defined(Q_OS_UNIX) && !defined (Q_OS_MAC)
TEST_F(TestDFMFactoryLoader, testPluginLoader)
{
    m_pTester->pluginLoader("video/*");
}

TEST_F(TestDFMFactoryLoader, testPluginLoaderList)
{
    m_pTester->pluginLoaderList("video/*");
}
#endif

TEST_F(TestDFMFactoryLoader, testKeyMap)
{
    m_pTester->keyMap();
}

TEST_F(TestDFMFactoryLoader, testIndexOf)
{
    m_pTester->indexOf("video/*");
}

TEST_F(TestDFMFactoryLoader, testGetAllIndexByKey)
{
    m_pTester->getAllIndexByKey("video/*");
}

bool stub2_contains(const QString &str, Qt::CaseSensitivity cs = Qt::CaseSensitive)
{
    return false;
}

bool stub_exists(const QString& name)
{
    return true;
}

QStringList stub3_entryList(QDir::Filters filters = QDir::NoFilter, QDir::SortFlags sort = QDir::NoSort)
{
    QStringList lst;
    lst << "libdde-image-preview-plugin.so"
        << "libdde-music-preview-plugin.so"
        << "libdde-pdf-preview-plugin.so"
        << "libdde-text-preview-plugin.so"
        << "libdde-video-preview-plugin.so";
    return lst;
}

TEST_F(TestDFMFactoryLoader, testUpdate)
{

//    auto stub2_contains = [](const QString &str, Qt::CaseSensitivity cs = Qt::CaseSensitive)->bool{
//        return true;
//    };

    Stub stub2;
    stub2.set((bool(QStringList::*)(const QString &str, Qt::CaseSensitivity cs)const)ADDR(QStringList, contains), stub2_contains);

//    auto stub_exists = [](const QString& name)->bool{
//        Q_UNUSED(name);
//        return true;
//    };
    Stub stub;
    stub.set((bool(QDir::*)(const QString&)const) ADDR(QDir, exists), stub_exists);

    Stub stub3;
    stub3.set((QStringList(QDir::*)(QDir::Filters, QDir::SortFlags)const)ADDR(QDir, entryList), stub3_entryList);

    m_pTester->update();
}

TEST_F(TestDFMFactoryLoader, testRefreshAll)
{
    m_pTester->refreshAll();
}
