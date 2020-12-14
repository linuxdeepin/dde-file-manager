#include <gtest/gtest.h>

#include "interfaces/plugins/dfmviewplugin.h"

DFM_USE_NAMESPACE

namespace  {

    class A : public DFMViewPlugin
    {
    public:
        A(QObject *parent = nullptr)
            : DFMViewPlugin(parent)
        {

        }

        DFMBaseView *create(const QString &key)
        {
            Q_UNUSED(key);
            return nullptr;
        }

    };

    class TestDFMViewPlugin : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new A();
            std::cout << "start TestDFMViewPlugin";
        }
        void TearDown() override
        {
            if(m_pTester)
                delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestDFMViewPlugin";
        }
    public:
        A *m_pTester;
    };
}

TEST_F(TestDFMViewPlugin, testInit)
{
    EXPECT_NE(m_pTester, nullptr);
}
