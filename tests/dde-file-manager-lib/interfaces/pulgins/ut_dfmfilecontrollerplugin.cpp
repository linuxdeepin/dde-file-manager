#include <gtest/gtest.h>

#include "interfaces/plugins/dfmfilecontrollerplugin.h"

DFM_USE_NAMESPACE

namespace  {

    class A : public DFMFileControllerPlugin
    {
    public:
        A(QObject *parent = nullptr)
            : DFMFileControllerPlugin(parent)
        {

        }
        DAbstractFileController *create(const QString &key)
        {
            Q_UNUSED(key);
            return nullptr;
        }
    };

    class TestDFMFileControllerPlugin : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new A();
            std::cout << "start TestDFMFileControllerPlugin";
        }
        void TearDown() override
        {
            if(m_pTester)
                delete  m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestDFMFileControllerPlugin";
        }
    public:
        A *m_pTester;
    };
}

TEST_F(TestDFMFileControllerPlugin, testInit)
{
    EXPECT_NE(m_pTester, nullptr);
}

