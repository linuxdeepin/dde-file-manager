#include <gtest/gtest.h>

#include "interfaces/plugins/dfmcrumbplugin.h"

namespace  {

    class A : public DFM_NAMESPACE::DFMCrumbPlugin
    {
    public:
        A(QObject *parent = 0)
            : DFM_NAMESPACE::DFMCrumbPlugin(parent)
        {
        }
        DFM_NAMESPACE::DFMCrumbInterface *create(const QString &key)
        {
        }
    };

    class TestDFMCrumbPlugin : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new A();
            std::cout << "start TestDFMCrumbPlugin";
        }
        void TearDown() override
        {
            if(m_pTester)
                delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestDFMCrumbPlugin";
        }
    public:
        A *m_pTester;
    };
}

TEST_F(TestDFMCrumbPlugin, testInit)
{
    EXPECT_NE(m_pTester, nullptr);
}
