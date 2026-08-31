// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_eventshandler_1.cpp
 * @brief Unit tests for EventsHandler methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/eventshandler.h"

#include <QTest>

using namespace dfmplugin_disk_encrypt_entry;

class EventsHandlerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new EventsHandler();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    EventsHandler *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(EventsHandlerTest, acquirePassphrase)
{
    // Test method: QString acquirePassphrase((const QString &dev, bool &cancelled))
    QString _arg0{};
    bool _arg1{};
    auto result = obj->acquirePassphrase(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(EventsHandlerTest, acquirePassphraseByPIN)
{
    // Test method: QString acquirePassphraseByPIN((const QString &dev, bool &cancelled))
    QString _arg0{};
    bool _arg1{};
    auto result = obj->acquirePassphraseByPIN(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(EventsHandlerTest, acquirePassphraseByRec)
{
    // Test method: QString acquirePassphraseByRec((const QString &dev, bool &cancelled))
    QString _arg0{};
    bool _arg1{};
    auto result = obj->acquirePassphraseByRec(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(EventsHandlerTest, acquirePassphraseByTPM)
{
    // Test method: QString acquirePassphraseByTPM((const QString &dev, bool &))
    QString _arg0{};
    bool _arg1{};
    auto result = obj->acquirePassphraseByTPM(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(EventsHandlerTest, bindDaemonSignals)
{
    // Test method: void bindDaemonSignals(())
    EXPECT_NO_FATAL_FAILURE(obj->bindDaemonSignals());
}

TEST_F(EventsHandlerTest, canUnlock)
{
    // Test method: bool canUnlock((const QString &device))
    QString _arg0{};
    auto result = obj->canUnlock(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(EventsHandlerTest, deviceEncryptStatus)
{
    // Test method: int deviceEncryptStatus((const QString &device))
    QString _arg0{};
    auto result = obj->deviceEncryptStatus(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(EventsHandlerTest, hasPendingTask)
{
    // Test bool getter: hasPendingTask()
    bool result = obj->hasPendingTask();
    EXPECT_FALSE(result);

}

TEST_F(EventsHandlerTest, holderDevice)
{
    // Test method: QString holderDevice((const QString &device))
    QString _arg0{};
    auto result = obj->holderDevice(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(EventsHandlerTest, hookEvents)
{
    // Test method: void hookEvents(())
    EXPECT_NO_FATAL_FAILURE(obj->hookEvents());
}

TEST_F(EventsHandlerTest, ignoreParamRequest)
{
    // Test method: void ignoreParamRequest(())
    EXPECT_NO_FATAL_FAILURE(obj->ignoreParamRequest());
}

TEST_F(EventsHandlerTest, isTaskWorking)
{
    // Test bool getter: isTaskWorking()
    bool result = obj->isTaskWorking();
    EXPECT_FALSE(result);

}

TEST_F(EventsHandlerTest, isUnderOperating)
{
    // Test method: bool isUnderOperating((const QString &device))
    QString _arg0{};
    auto result = obj->isUnderOperating(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(EventsHandlerTest, onChgPwdFinished)
{
    // Test method: void onChgPwdFinished((const QVariantMap &result))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onChgPwdFinished(_arg0));
}

TEST_F(EventsHandlerTest, onDecryptFinished)
{
    // Test method: void onDecryptFinished((const QVariantMap &result))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onDecryptFinished(_arg0));
}

TEST_F(EventsHandlerTest, onDecryptProgress)
{
    // Test method: void onDecryptProgress((const QString &dev, const QString &devName, double progress))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onDecryptProgress(_arg0, _arg1, 0.0));
}

TEST_F(EventsHandlerTest, onEncryptProgress)
{
    // Test method: void onEncryptProgress((const QString &dev, const QString &devName, double progress))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onEncryptProgress(_arg0, _arg1, 0.0));
}

TEST_F(EventsHandlerTest, onInitEncryptFinished)
{
    // Test method: void onInitEncryptFinished((const QVariantMap &result))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onInitEncryptFinished(_arg0));
}

TEST_F(EventsHandlerTest, requestReboot)
{
    // Test method: void requestReboot(())
    EXPECT_NO_FATAL_FAILURE(obj->requestReboot());
}

TEST_F(EventsHandlerTest, resumeEncrypt)
{
    // Test method: void resumeEncrypt((const QString &device))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->resumeEncrypt(_arg0));
}

TEST_F(EventsHandlerTest, showPreEncryptError)
{
    // Test method: void showPreEncryptError((const QString &dev, const QString &devName, int code))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->showPreEncryptError(_arg0, _arg1, 0));
}

TEST_F(EventsHandlerTest, unfinishedDecryptJob)
{
    // Test getter: QString unfinishedDecryptJob()
    auto result = obj->unfinishedDecryptJob();
    EXPECT_TRUE(result.isEmpty());

}
