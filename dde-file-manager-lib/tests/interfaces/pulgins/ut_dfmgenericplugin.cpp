
#include <gtest/gtest.h>

#include "interfaces/plugins/dfmgenericplugin.h"

DFM_USE_NAMESPACE

namespace  {

    class A : public DFMGenericPlugin
    {
    public:
        A(QObject *parent = 0)
            : DFMGenericPlugin(parent)
        {

        }
        QObject *create(const QString &key)
        {
            Q_UNUSED(key);
            return nullptr;
        }
    };

    class TestDFMGenericPlugin : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new A();
            std::cout << "start TestDFMGenericPlugin";
        }
        void TearDown() override
        {
            if(m_pTester)
                delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestDFMGenericPlugin";
        }
    public:
        A *m_pTester;
    };
}

TEST_F(TestDFMGenericPlugin, testInit)
{
    EXPECT_NE(m_pTester, nullptr);
}
