#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QSharedPointer>
#include <controllers/vaulterrorcode.h>

#define private public
#include "controllers/vaulthandle.h"


namespace  {
    class TestCryFsHandle : public testing::Test
    {
    public:
        QSharedPointer<CryFsHandle> m_controller;

        virtual void SetUp() override
        {
            m_controller = QSharedPointer<CryFsHandle>(new CryFsHandle());
            std::cout << "start TestCryFsHandle" << std::endl;
        }

        virtual void TearDown() override
        {
            std::cout << "end TestCryFsHandle" << std::endl;
        }
    };
}


TEST_F(TestCryFsHandle, tst_slotReadError)
{
    m_controller->m_activeState.insert(1, 1);
    EXPECT_NO_FATAL_FAILURE(m_controller->slotReadError());
    m_controller->m_activeState.clear();

    m_controller->m_activeState.insert(3, 3);
    EXPECT_NO_FATAL_FAILURE(m_controller->slotReadError());
    m_controller->m_activeState.clear();

    m_controller->m_activeState.insert(7, 7);
    EXPECT_NO_FATAL_FAILURE(m_controller->slotReadError());
    m_controller->m_activeState.clear();
}

TEST_F(TestCryFsHandle, tst_slotReadOut)
{
    EXPECT_NO_FATAL_FAILURE(m_controller->slotReadOutput());
}

TEST_F(TestCryFsHandle, tst_lockVault)
{
    EXPECT_NO_FATAL_FAILURE(m_controller->lockVault("/test/path"));
}


