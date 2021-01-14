
#include <gtest/gtest.h>

#include "interfaces/plugins/dfmfilepreviewplugin.h"

DFM_USE_NAMESPACE

namespace  {

    class A : public DFMFilePreviewPlugin
    {
    public:
        A(QObject *parent = nullptr)
            : DFMFilePreviewPlugin(parent)
        {

        }
        DFMFilePreview *create(const QString &key)
        {
            Q_UNUSED(key);
            return nullptr;
        }
    };

    class TestDFMFilePreviewPlugin : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new A();
            std::cout << "start TestDFMFilePreviewPlugin";
        }
        void TearDown() override
        {
            if(m_pTester)
                delete  m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestDFMFilePreviewPlugin";
        }
    public:
        A *m_pTester;
    };
}

TEST_F(TestDFMFilePreviewPlugin, testCreate)
{
    EXPECT_NE(m_pTester, nullptr);
}
