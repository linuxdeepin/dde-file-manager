#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QSharedPointer>
#include <controllers/vaulterrorcode.h>
#include <QProcess>
#include <QThread>

#include "stub.h"

#define private public
#include "controllers/vaulthandle.h"


namespace  {
    class TestCryFsHandle : public testing::Test
    {
    public:
        CryFsHandle* m_controller;

        virtual void SetUp() override
        {
            void (*st_threadStart)(QThread::Priority) = [](QThread::Priority){
                //do nothing.
            };
            Stub stub;
            stub.set(ADDR(QThread, start), st_threadStart);

            m_controller = new CryFsHandle();
            std::cout << "start TestCryFsHandle" << std::endl;
        }

        virtual void TearDown() override
        {
            if (m_controller) {
                delete m_controller;
                m_controller = nullptr;
            }

            std::cout << "end TestCryFsHandle" << std::endl;
        }
    };
}


TEST_F(TestCryFsHandle, tst_slotReadError)
{
    QByteArray (*st_readAllStandardError)() = []()->QByteArray {
        return QByteArray("mountpoint is not empty");
    };
    Stub stub;
    stub.set(ADDR(QProcess, readAllStandardError), st_readAllStandardError);

    m_controller->m_activeState.insert(1, 1);
    EXPECT_NO_FATAL_FAILURE(m_controller->slotReadError());
    m_controller->m_activeState.clear();

    QByteArray (*st_readAllStandardError_2)() = []()->QByteArray {
        return QByteArray("Permission denied");
    };
    stub.set(ADDR(QProcess, readAllStandardError), st_readAllStandardError_2);

    m_controller->m_activeState.insert(1, 1);
    EXPECT_NO_FATAL_FAILURE(m_controller->slotReadError());
    m_controller->m_activeState.clear();

    stub.set(ADDR(QProcess, readAllStandardError), st_readAllStandardError);
    m_controller->m_activeState.insert(3, 3);
    EXPECT_NO_FATAL_FAILURE(m_controller->slotReadError());
    m_controller->m_activeState.clear();

    stub.set(ADDR(QProcess, readAllStandardError), st_readAllStandardError_2);
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

TEST_F(TestCryFsHandle, tst_createVault)
{
    int (*st_runVaultProcess)(QString, QString, QString) = [](QString, QString, QString)->int {
        // do nothing.
        return 0;
    };
    Stub stub;
    stub.set(ADDR(CryFsHandle, runVaultProcess), st_runVaultProcess);

    m_controller->createVault("", "", "");
}

TEST_F(TestCryFsHandle, tst_unlockVault)
{
    int (*st_runVaultProcess)(QString, QString, QString) = [](QString, QString, QString)->int {
        // do nothing.
        return 0;
    };
    Stub stub;
    stub.set(ADDR(CryFsHandle, runVaultProcess), st_runVaultProcess);

    m_controller->unlockVault("", "", "");
}

TEST_F(TestCryFsHandle, tst_runVaultProcess)
{
    int exitCode = m_controller->runVaultProcess("", "", "");
    if(exitCode == 0)
        EXPECT_EQ(0, exitCode);
    else {
        EXPECT_EQ(1, exitCode);
    }
}

TEST_F(TestCryFsHandle, tst_lockVaultProcess)
{
    QProcess::ExitStatus (*st_exitStatus)() = []()->QProcess::ExitStatus{
        return QProcess::NormalExit;
    };
    Stub stub;
    stub.set(ADDR(QProcess, exitStatus), st_exitStatus);

    int (*st_exitCode)() = []()->int {
        return 0;
    };
    stub.set(ADDR(QProcess, exitCode), st_exitCode);

    EXPECT_EQ(static_cast<int>(0), m_controller->lockVaultProcess(""));
}


