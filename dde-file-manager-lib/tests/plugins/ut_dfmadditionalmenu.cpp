#include "plugins/dfmadditionalmenu.h"

#include <gtest/gtest.h>

DFM_USE_NAMESPACE

namespace  {
    class TestDFMAdditionalMenu : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new DFMAdditionalMenu();
            std::cout << "start TestDFMAdditionalMenu";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestDFMAdditionalMenu";
        }
    public:
        DFMAdditionalMenu   *m_pTester;
    };
}

TEST_F(TestDFMAdditionalMenu, testInit)
{

}

TEST_F(TestDFMAdditionalMenu, testActions)
{
    QStringList files;
    files << "/test1";
    m_pTester->actions(files);
}
