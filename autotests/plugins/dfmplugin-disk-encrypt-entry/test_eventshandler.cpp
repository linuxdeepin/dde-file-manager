// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "events/eventshandler.h"
#include "dfmplugin_disk_encrypt_global.h"
#include "services/diskencrypt/globaltypesdefine.h"

#include <gtest/gtest.h>

#include <QApplication>
#include <QDBusInterface>
#include <QDBusAbstractInterface>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QFile>
#include <QTemporaryDir>

#include <DDBusSender>

using namespace dfmplugin_diskenc;
using namespace disk_encrypt;

class EventsHandlerImpl : public testing::Test
{
protected:
    void SetUp() override
    {
        ins = EventsHandler::instance();
    }

    void TearDown() override
    {
        stub.clear();
    }

    void stubDaemonInterface(bool valid, const QVariant &replyValue = QVariant())
    {
        stub.set_lamda(&QDBusAbstractInterface::isValid, [valid](QDBusAbstractInterface *) -> bool {
            __DBG_STUB_INVOKE__
            return valid;
        });

        using CallFunc = QDBusMessage (QDBusAbstractInterface::*)(const QString &);
        stub.set_lamda(static_cast<CallFunc>(&QDBusAbstractInterface::call),
                       [replyValue](QDBusAbstractInterface *, const QString &) -> QDBusMessage {
                           __DBG_STUB_INVOKE__
                           QDBusMessage call = QDBusMessage::createMethodCall("s", "/p", "i", "m");
                           return call.createReply(replyValue);
                       });

        // multi-argument call("Method", args...) resolves to the header-inline
        // template overload which delegates to doCall(), so stub that too
        // (covers deviceEncryptStatus/holderDevice which pass an argument).
        using DoCallFunc = QDBusMessage (QDBusAbstractInterface::*)(QDBus::CallMode, const QString &, const QVariant *, size_t);
        stub.set_lamda(static_cast<DoCallFunc>(&QDBusAbstractInterface::doCall),
                       [replyValue](QDBusAbstractInterface *, QDBus::CallMode, const QString &, const QVariant *, size_t) -> QDBusMessage {
                           __DBG_STUB_INVOKE__
                           QDBusMessage call = QDBusMessage::createMethodCall("s", "/p", "i", "m");
                           return call.createReply(replyValue);
                       });
    }

    stub_ext::StubExt stub;
    EventsHandler *ins = nullptr;
};

TEST_F(EventsHandlerImpl, instance)
{
    EXPECT_NE(ins, nullptr);
    EXPECT_EQ(ins, EventsHandler::instance());
}

TEST_F(EventsHandlerImpl, bindDaemonSignals_NonFileManager)
{
    stub.set_lamda(&QApplication::applicationName, []() -> QString {
        __DBG_STUB_INVOKE__
        return "file-dialog";
    });

    bool connected = false;
    using ConnectFunc = bool (QDBusConnection::*)(const QString &, const QString &, const QString &, const QString &, QObject *, const char *);
    stub.set_lamda(static_cast<ConnectFunc>(&QDBusConnection::connect),
                   [&connected](QDBusConnection *, const QString &, const QString &, const QString &, const QString &, QObject *, const char *) -> bool {
                       __DBG_STUB_INVOKE__
                       connected = true;
                       return true;
                   });

    ins->bindDaemonSignals();
    EXPECT_FALSE(connected);
}

TEST_F(EventsHandlerImpl, bindDaemonSignals_FileManager)
{
    stub.set_lamda(&QApplication::applicationName, []() -> QString {
        __DBG_STUB_INVOKE__
        return "dde-file-manager";
    });

    int connectCount = 0;
    using ConnectFunc = bool (QDBusConnection::*)(const QString &, const QString &, const QString &, const QString &, QObject *, const char *);
    stub.set_lamda(static_cast<ConnectFunc>(&QDBusConnection::connect),
                   [&connectCount](QDBusConnection *, const QString &, const QString &, const QString &, const QString &, QObject *, const char *) -> bool {
                       __DBG_STUB_INVOKE__
                       ++connectCount;
                       return true;
                   });

    ins->bindDaemonSignals();
    EXPECT_EQ(connectCount, 8);
}

TEST_F(EventsHandlerImpl, checkPendingOverlayDMNotify_NoFile)
{
    stub.set_lamda(static_cast<bool (*)(const QString &)>(&QFile::exists), [](const QString &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    EXPECT_NO_THROW(ins->checkPendingOverlayDMNotify());
}

TEST_F(EventsHandlerImpl, checkPendingOverlayDMNotify_InvalidJson)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QString notifyFile = dir.path() + "/pending.json";
    QFile f(notifyFile);
    f.open(QIODevice::WriteOnly);
    f.write("not json");
    f.close();

    stub.set_lamda(static_cast<bool (*)(const QString &)>(&QFile::remove), [](const QString &) -> bool { return true; });

    // Redirect the constant path by stubbing exists to only match our file
    stub.set_lamda(static_cast<bool (*)(const QString &)>(&QFile::exists),
                   [&notifyFile](const QString &path) -> bool {
                       __DBG_STUB_INVOKE__
                       return path == notifyFile;
                   });

    EXPECT_NO_THROW(ins->checkPendingOverlayDMNotify());
}

TEST_F(EventsHandlerImpl, isTaskWorking_Valid)
{
    stubDaemonInterface(true, true);
    EXPECT_TRUE(ins->isTaskWorking());
}

TEST_F(EventsHandlerImpl, isTaskWorking_Invalid)
{
    stubDaemonInterface(false);
    EXPECT_FALSE(ins->isTaskWorking());
}

TEST_F(EventsHandlerImpl, hasPendingTask_True)
{
    stubDaemonInterface(true, false);   // IsTaskEmpty returns false -> has pending
    EXPECT_TRUE(ins->hasPendingTask());
}

TEST_F(EventsHandlerImpl, hasPendingTask_False)
{
    stubDaemonInterface(true, true);   // IsTaskEmpty returns true -> no pending
    EXPECT_FALSE(ins->hasPendingTask());
}

TEST_F(EventsHandlerImpl, unfinishedDecryptJob)
{
    stubDaemonInterface(true, QString("/dev/sda1"));
    EXPECT_EQ(ins->unfinishedDecryptJob(), "/dev/sda1");
}

TEST_F(EventsHandlerImpl, deviceEncryptStatus)
{
    stubDaemonInterface(true, 1);
    EXPECT_EQ(ins->deviceEncryptStatus("/dev/sda1"), 1);
}

TEST_F(EventsHandlerImpl, deviceEncryptStatus_Invalid)
{
    stubDaemonInterface(false);
    EXPECT_EQ(ins->deviceEncryptStatus("/dev/sda1"), -1);
}

TEST_F(EventsHandlerImpl, holderDevice)
{
    stubDaemonInterface(true, QString("/dev/mapper/home"));
    EXPECT_EQ(ins->holderDevice("/dev/sda1"), "/dev/mapper/home");
}

TEST_F(EventsHandlerImpl, holderDevice_Invalid)
{
    stubDaemonInterface(false);
    EXPECT_EQ(ins->holderDevice("/dev/sda1"), "/dev/sda1");
}

TEST_F(EventsHandlerImpl, isUnderOperating_False)
{
    EXPECT_FALSE(ins->isUnderOperating("/dev/sda1"));
}

TEST_F(EventsHandlerImpl, resumeEncrypt)
{
    bool asyncCalled = false;
    using AsyncFunc = QDBusPendingCall (QDBusAbstractInterface::*)(const QString &, const QList<QVariant> &);
    stub.set_lamda(static_cast<AsyncFunc>(&QDBusAbstractInterface::asyncCallWithArgumentList),
                   [&asyncCalled](QDBusAbstractInterface *, const QString &method, const QList<QVariant> &) -> QDBusPendingCall {
                       __DBG_STUB_INVOKE__
                       if (method == "ResumeEncryption") asyncCalled = true;
                       return QDBusPendingCall(nullptr);
                   });
    stub.set_lamda(&QDBusAbstractInterface::isValid, [](QDBusAbstractInterface *) -> bool { return true; });

    ins->resumeEncrypt("/dev/sda1");
    EXPECT_TRUE(asyncCalled);
}

TEST_F(EventsHandlerImpl, onOverlayDMModeChanged_Success)
{
    bool called = false;
    using CallFunc = QDBusPendingCall (DDBusCaller::*)();
    stub.set_lamda(static_cast<CallFunc>(&DDBusCaller::call),
                   [&called]() -> QDBusPendingCall {
                       __DBG_STUB_INVOKE__
                       called = true;
                       return QDBusPendingCall(nullptr);
                   });

    ins->onOverlayDMModeChanged(true, OverlayDMSuccess);
    EXPECT_TRUE(called);
}

TEST_F(EventsHandlerImpl, onOverlayDMModeChanged_Failed)
{
    bool called = false;
    using CallFunc = QDBusPendingCall (DDBusCaller::*)();
    stub.set_lamda(static_cast<CallFunc>(&DDBusCaller::call),
                   [&called]() -> QDBusPendingCall {
                       __DBG_STUB_INVOKE__
                       called = true;
                       return QDBusPendingCall(nullptr);
                   });

    ins->onOverlayDMModeChanged(false, OverlayDMFailedUpdateInitramfs);
    EXPECT_TRUE(called);
}

TEST_F(EventsHandlerImpl, onOverlayDMModeChanged_RolledBack)
{
    bool called = false;
    using CallFunc = QDBusPendingCall (DDBusCaller::*)();
    stub.set_lamda(static_cast<CallFunc>(&DDBusCaller::call),
                   [&called]() -> QDBusPendingCall {
                       __DBG_STUB_INVOKE__
                       called = true;
                       return QDBusPendingCall(nullptr);
                   });

    ins->onOverlayDMModeChanged(false, OverlayDMRolledBackInterrupted);
    EXPECT_TRUE(called);
}

TEST_F(EventsHandlerImpl, onAcquireDevicePwd_NullParams)
{
    QString pwd;
    bool cancelled = false;
    EXPECT_FALSE(ins->onAcquireDevicePwd("/dev/sda1", nullptr, &cancelled));
    EXPECT_FALSE(ins->onAcquireDevicePwd("/dev/sda1", &pwd, nullptr));
}
