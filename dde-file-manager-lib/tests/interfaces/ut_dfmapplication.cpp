#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "interfaces/dfmapplication.h"

DFM_USE_NAMESPACE
namespace  {
    static DFMApplication m_application;
    class TestDFMApplication : public testing::Test
    {
        void SetUp() override
        {
            std::cout << "start DFMApplication" << std::endl;
        }

        void TearDown() override
        {
            std::cout << "end DFMApplication" << std::endl;
        }
    };


TEST_F(TestDFMApplication, test_appAttribute)
{
   EXPECT_FALSE(m_application.appAttribute(DFMApplication::ApplicationAttribute::AA_AllwayOpenOnNewWindow).toBool());
}

TEST_F(TestDFMApplication, test_appUrlAttribute)
{
    DUrl url = m_application.appUrlAttribute(DFMApplication::AA_UrlOfNewWindow);
    EXPECT_EQ(url.toString(), COMPUTER_ROOT);
}

TEST_F(TestDFMApplication, test_setAppAttribute)
{
    EXPECT_FALSE(m_application.appAttribute(DFMApplication::ApplicationAttribute::AA_AllwayOpenOnNewWindow).toBool());
    m_application.setAppAttribute(DFMApplication::ApplicationAttribute::AA_AllwayOpenOnNewWindow, true);
    EXPECT_TRUE(m_application.appAttribute(DFMApplication::ApplicationAttribute::AA_AllwayOpenOnNewWindow).toBool());
    m_application.setAppAttribute(DFMApplication::ApplicationAttribute::AA_AllwayOpenOnNewWindow, false);
    EXPECT_FALSE(m_application.appAttribute(DFMApplication::ApplicationAttribute::AA_AllwayOpenOnNewWindow).toBool());
}

TEST_F(TestDFMApplication, test_syncAppAttribute)
{
    EXPECT_TRUE(m_application.syncAppAttribute());
}

TEST_F(TestDFMApplication, test_genericAttribute)
{
   EXPECT_TRUE(m_application.genericAttribute(DFMApplication::GenericAttribute::GA_AutoMount).toBool());
}

TEST_F(TestDFMApplication, test_setGenericAttribute)
{
    EXPECT_TRUE(m_application.genericAttribute(DFMApplication::GenericAttribute::GA_AutoMount).toBool());
    m_application.setGenericAttribute(DFMApplication::GenericAttribute::GA_AutoMount, false);
    EXPECT_FALSE(m_application.genericAttribute(DFMApplication::GenericAttribute::GA_AutoMount).toBool());
    m_application.setGenericAttribute(DFMApplication::GenericAttribute::GA_AutoMount, true);
    EXPECT_TRUE(m_application.genericAttribute(DFMApplication::GenericAttribute::GA_AutoMount).toBool());
}

TEST_F(TestDFMApplication, test_syncGenericAttribute)
{
    EXPECT_TRUE(m_application.syncGenericAttribute());
}

TEST_F(TestDFMApplication, test_instance)
{
    EXPECT_NE(m_application.instance(), nullptr);
}

TEST_F(TestDFMApplication, test_genericSetting)
{
    EXPECT_NE(m_application.genericSetting(), nullptr);
}

TEST_F(TestDFMApplication, test_appSetting)
{
    EXPECT_NE(m_application.appSetting(), nullptr);
}

TEST_F(TestDFMApplication, test_enericObtuselySetting)
{
    EXPECT_NE(m_application.genericObtuselySetting(), nullptr);
}

TEST_F(TestDFMApplication, test_appObtuselySetting)
{
    EXPECT_NE(m_application.appObtuselySetting(), nullptr);
}

}
