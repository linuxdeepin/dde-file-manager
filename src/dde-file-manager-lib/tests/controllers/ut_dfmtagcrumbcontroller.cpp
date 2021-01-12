#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "controllers/dfmtagcrumbcontroller.h"

using DFM_NAMESPACE::DFMTagCrumbController;
using DFM_NAMESPACE::CrumbData;

namespace  {
    class TestDFMTagCrumbController : public testing::Test
    {
    public:
        virtual void SetUp() override
        {
            m_pController = new DFMTagCrumbController;
        }

        virtual void TearDown() override
        {
            delete m_pController;
            m_pController = nullptr;
        }

        DFMTagCrumbController *m_pController;
    };
}

TEST_F(TestDFMTagCrumbController, supportedUrl)
{
    ASSERT_NE(m_pController, nullptr);

    DUrl url;
    url.setScheme(TAG_SCHEME);
    EXPECT_TRUE(m_pController->supportedUrl(url));
    url.setScheme(FILE_SCHEME);
    EXPECT_FALSE(m_pController->supportedUrl(url));
}

TEST_F(TestDFMTagCrumbController, seprateUrl)
{
    ASSERT_NE(m_pController, nullptr);

    DUrl url;
    url.setScheme(TAG_SCHEME);
    QList<CrumbData> list = m_pController->seprateUrl(url);
    EXPECT_TRUE(!list.empty());
}
