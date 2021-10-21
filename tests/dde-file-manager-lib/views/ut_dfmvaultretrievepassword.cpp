#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QSharedPointer>

#include "views/dfmvaultretrievepassword.h"

namespace  {
    class TestDFMVaultRetrievePassword : public testing::Test
    {
    public:
        DFMVaultRetrievePassword* m_view;

        virtual void SetUp() override
        {
            m_view = DFMVaultRetrievePassword::instance();
            m_view->show();
            std::cout << "start TestDFMVaultRetrievePassword" << std::endl;
        }

        virtual void TearDown() override
        {
            m_view->close();
            std::cout << "end TestDFMVaultRetrievePassword" << std::endl;
        }
    };
}

TEST_F(TestDFMVaultRetrievePassword, callverificationKey)
{
    EXPECT_NO_FATAL_FAILURE(m_view->verificationKey());
}

TEST_F(TestDFMVaultRetrievePassword, callonButtonClicked)
{
    QString text = QString(QObject::tr("close", "button"));
    EXPECT_NO_FATAL_FAILURE(m_view->onButtonClicked(0, text));
    EXPECT_NO_FATAL_FAILURE(m_view->onButtonClicked(1, text));
    text = QString(QObject::tr("returnUnlock", "button"));
    EXPECT_NO_FATAL_FAILURE(m_view->onButtonClicked(1, text));
}

TEST_F(TestDFMVaultRetrievePassword, callonComboBoxIndex)
{
    EXPECT_NO_FATAL_FAILURE(m_view->onComboBoxIndex(0));
    EXPECT_NO_FATAL_FAILURE(m_view->onComboBoxIndex(1));
}
