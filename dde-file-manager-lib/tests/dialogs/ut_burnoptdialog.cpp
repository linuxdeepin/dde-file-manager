#include <gtest/gtest.h>

#include "dialogs/burnoptdialog.h"

namespace  {
    class TestBurnOptDialog : public testing::Test{
    public:
        void SetUp() override
        {
            m_pTester = new BurnOptDialog("file:///file");
            std::cout << "start TestBurnOptDialog";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestBurnOptDialog";
        }
    public:
        BurnOptDialog *m_pTester;
    };
}

TEST_F(TestBurnOptDialog, testInit)
{

}

TEST_F(TestBurnOptDialog, testSetIOSImage)
{
    DUrl image("file://test1/image");
    m_pTester->setISOImage(image);
}

TEST_F(TestBurnOptDialog, testSetJobWindowId)
{
    int wid(12345678);
    m_pTester->setJobWindowId(wid);
}

TEST_F(TestBurnOptDialog, testSetDefaultVolName)
{
    QString volName("test UT");
    m_pTester->setDefaultVolName(volName);
}

