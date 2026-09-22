// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================================
// Coverage map for src/plugins/filemanager/dfmplugin-search/utils/abstractindexclient.cpp
// (uncovered functions -> test case)
//   checkServiceStatus(){lambda#1} ................ CheckServiceStatus_WithInterface_EmitsErrorAfterFailedCall
//   handleServiceTestReply ........................ HandleServiceTestReply_CompletedReply_EmitsAvailable /
//                                                   CheckServiceStatus_WithInterface_EmitsErrorAfterFailedCall (error branch)
//   handleServiceTestReply(){lambda#1} ............ error branch via FinallyUtil in the same calls
//   checkIndexExists(){lambda#1} .................. CheckIndexExists_WithInterface_EmitsFailure
//   handleIndexExistsReply ........................ HandleIndexExistsReply_True/False + failure case
//   handleIndexExistsReply(){lambda#1} ............ same
//   startTask(){lambda#1/#2} ...................... StartTask_Create_WithInterface_EmitsTaskFailedAfterError
//   stringToTaskType .............................. StringToTaskType_KnownAndUnknownStrings
//   checkHasRunningTask(){lambda#1} ............... CheckHasRunningTask_WithInterface_EmitsFailure
//   handleHasRunningTaskReply ..................... HandleHasRunningTaskReply_CompletedReply_EmitsValues
//   handleHasRunningTaskReply(){lambda#1} ......... same
//   checkHasRunningRootTask(){lambda#1/#1::lambda#1}  CheckHasRunningRootTask_WithInterface_EmitsFailure
//   getLastUpdateTime(){lambda#1} ................. GetLastUpdateTime_WithInterface_EmitsFailure
//   handleGetLastUpdateTimeReply .................. HandleGetLastUpdateTimeReply_CompletedReply_EmitsTime
//   handleGetLastUpdateTimeReply(){lambda#1} ...... same
//   getIndexStatus(){lambda#1/#1::lambda#1} ....... GetIndexStatus_WithInterface_EmitsFailure
//   onDBusIndexStatusChanged ...................... OnDBusIndexStatusChanged_EmitsIndexStatusChanged
// The client gets a real QDBusInterface pointing at an unregistered session-bus
// service; QDBusAbstractInterface::isValid is stubbed true so ensureInterface()
// succeeds and the async paths run (the bus then answers with an error).
// handle*Reply success branches are driven with QDBusPendingCall::fromCompletedCall.
// ============================================================================

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QtTest>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusAbstractInterface>
#include <QDBusAbstractAdaptor>
#include <QVariant>
#include <QList>
#include <QStringList>
#include <QCoreApplication>

#include "stubext.h"

#include <QApplication>
#include <QThread>

#include "utils/abstractindexclient.h"

using namespace dfmplugin_search;

namespace {

constexpr const char *kCovService[] = { "com.deepin.covtest.FakeService" };

// Real (session-bus) mock backend used to drive the *success* branches of the
// watcher lambdas and handle*Reply methods with genuine DBus replies.
class ReplyServerAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.covtest.ReplyIface")

public:
    explicit ReplyServerAdaptor(QObject *parent)
        : QDBusAbstractAdaptor(parent)
    {
    }

public slots:
    bool HasRunningTask() { return true; }
    bool IndexDatabaseExists() { return true; }
    QString GetLastUpdateTime() { return QStringLiteral("2026-01-02 03:04:05"); }
    QVariantMap GetIndexStatus()
    {
        return { { "state", QVariant(QStringLiteral("Idle")) },
                 { "grade", QVariant(QStringLiteral("Full")) } };
    }
    bool CreateIndexTask(const QStringList &, const QVariantMap &) { return true; }
    bool UpdateIndexTask(const QStringList &, const QVariantMap &) { return true; }
};

}   // namespace

class UT_AbstractIndexClientCov : public testing::Test
{
protected:
    void SetUp() override
    {
        IndexClientDescriptor desc;
        desc.clientName = "covclient";
        desc.dbusServiceName = kCovService[0];
        desc.dbusObjectPath = "/com/deepin/covtest/FakeService";
        desc.interfaceFactory = [](QObject *parent) -> QDBusAbstractInterface * {
            return new QDBusInterface(QString(kCovService[0]),
                                      QStringLiteral("/com/deepin/covtest/FakeService"),
                                      QStringLiteral("com.deepin.covtest.FakeService"),
                                      QDBusConnection::sessionBus(),
                                      parent);
        };

        client = new AbstractIndexClient(desc);
    }

    void TearDown() override
    {
        stub.clear();
        delete client;
        client = nullptr;
        delete serverHost;
        serverHost = nullptr;
    }

    // Build a client whose interface points at the real session-bus mock above.
    void makeRealServiceClient()
    {
        const QString service = QStringLiteral("com.deepin.covtest.Reply%1").arg(QCoreApplication::applicationPid());
        const QString path = QStringLiteral("/com/deepin/covtest/Reply");

        serverHost = new QObject();
        new ReplyServerAdaptor(serverHost);
        ASSERT_TRUE(QDBusConnection::sessionBus().registerService(service));
        ASSERT_TRUE(QDBusConnection::sessionBus().registerObject(path, serverHost, QDBusConnection::ExportAdaptors));

        IndexClientDescriptor desc;
        desc.clientName = "covreal";
        desc.dbusServiceName = service;
        desc.dbusObjectPath = path;
        desc.interfaceFactory = [service, path](QObject *parent) -> QDBusAbstractInterface * {
            return new QDBusInterface(service, path, QStringLiteral("com.deepin.covtest.ReplyIface"),
                                      QDBusConnection::sessionBus(), parent);
        };
        delete client;
        client = new AbstractIndexClient(desc);
    }

    // Let ensureInterface() accept the (unregistered) fake service interface.
    void makeInterfaceValid()
    {
        stub.set_lamda(static_cast<bool (QDBusAbstractInterface::*)() const>(&QDBusAbstractInterface::isValid),
                       [](QDBusAbstractInterface *) -> bool {
                           return true;
                       });
    }

    void pumpEvents(int ms = 300)
    {
        if (qApp)
            QTest::qWait(ms);
    }

    stub_ext::StubExt stub;
    AbstractIndexClient *client = nullptr;
    QObject *serverHost = nullptr;
};

// ---------- ensureInterface ----------

TEST_F(UT_AbstractIndexClientCov, EnsureInterface_StubbedValidInterface_ReturnsTrueTwice)
{
    // Arrange
    makeInterfaceValid();

    // Act
    bool first = client->ensureInterface();
    bool second = client->ensureInterface();   // cached "already valid" shortcut

    // Assert
    EXPECT_TRUE(first);
    EXPECT_TRUE(second);
    EXPECT_NE(client->interface.get(), nullptr);
}

TEST_F(UT_AbstractIndexClientCov, EnsureInterface_MissingFactory_ReturnsFalse)
{
    // Arrange
    IndexClientDescriptor desc = client->descriptor();
    desc.interfaceFactory = nullptr;
    auto *noFactoryClient = new AbstractIndexClient(desc);

    // Act
    bool ok = noFactoryClient->ensureInterface();

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_EQ(noFactoryClient->descriptor().clientName, QString("covclient"));
    delete noFactoryClient;
}

// ---------- async paths with a failing fake service ----------

TEST_F(UT_AbstractIndexClientCov, CheckServiceStatus_WithInterface_EmitsErrorAfterFailedCall)
{
    // Arrange
    makeInterfaceValid();
    QSignalSpy spy(client, &AbstractIndexClient::serviceStatusResult);

    // Act
    client->checkServiceStatus();
    pumpEvents();

    // Assert
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).value<AbstractIndexClient::ServiceStatus>(),
              AbstractIndexClient::ServiceStatus::Error);
}

TEST_F(UT_AbstractIndexClientCov, CheckIndexExists_WithInterface_EmitsFailure)
{
    // Arrange
    makeInterfaceValid();
    QSignalSpy spy(client, &AbstractIndexClient::indexExistsResult);

    // Act
    client->checkIndexExists();
    pumpEvents();

    // Assert
    EXPECT_EQ(spy.count(), 1);
    const auto args = spy.takeFirst();
    EXPECT_FALSE(args.at(0).toBool());   // exists
    EXPECT_FALSE(args.at(1).toBool());   // success
}

TEST_F(UT_AbstractIndexClientCov, StartTask_Create_WithInterface_EmitsTaskFailedAfterError)
{
    // Arrange
    makeInterfaceValid();
    QSignalSpy spy(client, &AbstractIndexClient::taskFailed);

    // Act
    client->startTask(AbstractIndexClient::TaskType::Create, { "/tmp", "/home" });
    pumpEvents();

    // Assert
    EXPECT_EQ(spy.count(), 1);
    const auto args = spy.takeFirst();
    EXPECT_EQ(args.at(0).value<AbstractIndexClient::TaskType>(), AbstractIndexClient::TaskType::Create);
    EXPECT_EQ(args.at(1).toString(), QString("/tmp|/home"));
}

TEST_F(UT_AbstractIndexClientCov, CheckHasRunningTask_WithInterface_EmitsFailure)
{
    // Arrange
    makeInterfaceValid();
    QSignalSpy spy(client, &AbstractIndexClient::hasRunningTaskResult);

    // Act
    client->checkHasRunningTask();
    pumpEvents();

    // Assert
    EXPECT_EQ(spy.count(), 1);
    const auto args = spy.takeFirst();
    EXPECT_FALSE(args.at(0).toBool());
    EXPECT_FALSE(args.at(1).toBool());
}

TEST_F(UT_AbstractIndexClientCov, CheckHasRunningRootTask_WithInterface_EmitsFailure)
{
    // Arrange
    makeInterfaceValid();
    QSignalSpy spy(client, &AbstractIndexClient::hasRunningRootTaskResult);

    // Act
    client->checkHasRunningRootTask();
    pumpEvents();

    // Assert
    EXPECT_EQ(spy.count(), 1);
    const auto args = spy.takeFirst();
    EXPECT_FALSE(args.at(0).toBool());
    EXPECT_FALSE(args.at(1).toBool());
}

TEST_F(UT_AbstractIndexClientCov, GetLastUpdateTime_WithInterface_EmitsFailure)
{
    // Arrange
    makeInterfaceValid();
    QSignalSpy spy(client, &AbstractIndexClient::lastUpdateTimeResult);

    // Act
    client->getLastUpdateTime();
    pumpEvents();

    // Assert
    EXPECT_EQ(spy.count(), 1);
    const auto args = spy.takeFirst();
    EXPECT_TRUE(args.at(0).toString().isEmpty());
    EXPECT_FALSE(args.at(1).toBool());
}

TEST_F(UT_AbstractIndexClientCov, GetIndexStatus_WithInterface_EmitsFailure)
{
    // Arrange
    makeInterfaceValid();
    QSignalSpy spy(client, &AbstractIndexClient::indexStatusResult);

    // Act
    client->getIndexStatus();
    pumpEvents();

    // Assert
    EXPECT_EQ(spy.count(), 1);
    const auto args = spy.takeFirst();
    EXPECT_TRUE(args.at(0).toString().isEmpty());
    EXPECT_FALSE(args.at(2).toBool());
}

TEST_F(UT_AbstractIndexClientCov, ForceUpdateIndex_WithInterface_NoFailureSignal)
{
    // Arrange
    makeInterfaceValid();
    QSignalSpy failedSpy(client, &AbstractIndexClient::taskFailed);

    // Act
    client->forceUpdateIndex({ "/tmp" });
    client->updateIndexBypassEnv({ "/tmp" });
    pumpEvents();

    // Assert
    EXPECT_EQ(failedSpy.count(), 0);   // fire-and-forget calls, no watcher path
    EXPECT_EQ(client->descriptor().clientName, QString("covclient"));
}

// ---------- handle*Reply success branches ----------

TEST_F(UT_AbstractIndexClientCov, CheckServiceStatus_RealBackend_EmitsAvailable)
{
    // Arrange
    makeRealServiceClient();
    QSignalSpy spy(client, &AbstractIndexClient::serviceStatusResult);

    // Act
    client->checkServiceStatus();
    pumpEvents();

    // Assert
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).value<AbstractIndexClient::ServiceStatus>(),
              AbstractIndexClient::ServiceStatus::Available);
}

TEST_F(UT_AbstractIndexClientCov, CheckIndexExists_RealBackend_EmitsExistsAndSuccess)
{
    // Arrange
    makeRealServiceClient();
    QSignalSpy spy(client, &AbstractIndexClient::indexExistsResult);

    // Act
    client->checkIndexExists();
    pumpEvents();

    // Assert
    EXPECT_EQ(spy.count(), 1);
    const auto args = spy.takeFirst();
    EXPECT_TRUE(args.at(0).toBool());
    EXPECT_TRUE(args.at(1).toBool());
}

TEST_F(UT_AbstractIndexClientCov, CheckHasRunningTask_RealBackend_EmitsValues)
{
    // Arrange
    makeRealServiceClient();
    QSignalSpy spy(client, &AbstractIndexClient::hasRunningTaskResult);

    // Act
    client->checkHasRunningTask();
    pumpEvents();

    // Assert
    EXPECT_EQ(spy.count(), 1);
    const auto args = spy.takeFirst();
    EXPECT_TRUE(args.at(0).toBool());
    EXPECT_TRUE(args.at(1).toBool());
}

TEST_F(UT_AbstractIndexClientCov, GetLastUpdateTime_RealBackend_EmitsTime)
{
    // Arrange
    makeRealServiceClient();
    QSignalSpy spy(client, &AbstractIndexClient::lastUpdateTimeResult);

    // Act
    client->getLastUpdateTime();
    pumpEvents();

    // Assert
    EXPECT_EQ(spy.count(), 1);
    const auto args = spy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), QString("2026-01-02 03:04:05"));
    EXPECT_TRUE(args.at(1).toBool());
}

TEST_F(UT_AbstractIndexClientCov, GetIndexStatus_RealBackend_EmitsStatus)
{
    // Arrange
    makeRealServiceClient();
    QSignalSpy spy(client, &AbstractIndexClient::indexStatusResult);

    // Act
    client->getIndexStatus();
    pumpEvents();

    // Assert
    EXPECT_EQ(spy.count(), 1);
    const auto args = spy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), QString("Idle"));
    EXPECT_EQ(args.at(1).toString(), QString("Full"));
    EXPECT_TRUE(args.at(2).toBool());
}

TEST_F(UT_AbstractIndexClientCov, StartTask_RealBackend_EmitsTaskStarted)
{
    // Arrange
    makeRealServiceClient();
    QSignalSpy startedSpy(client, &AbstractIndexClient::taskStarted);
    QSignalSpy failedSpy(client, &AbstractIndexClient::taskFailed);

    // Act
    client->startTask(AbstractIndexClient::TaskType::Create, { "/tmp" });
    pumpEvents();

    // Assert
    EXPECT_EQ(failedSpy.count(), 0);
    EXPECT_EQ(startedSpy.count(), 1);
    const auto args = startedSpy.takeFirst();
    EXPECT_EQ(args.at(0).value<AbstractIndexClient::TaskType>(), AbstractIndexClient::TaskType::Create);
    EXPECT_EQ(args.at(1).toString(), QString("/tmp"));
}

TEST_F(UT_AbstractIndexClientCov, CheckHasRunningRootTask_RealBackend_EmitsValues)
{
    // Arrange
    makeRealServiceClient();
    QSignalSpy spy(client, &AbstractIndexClient::hasRunningRootTaskResult);

    // Act
    client->checkHasRunningRootTask();
    pumpEvents();

    // Assert
    EXPECT_EQ(spy.count(), 1);
    const auto args = spy.takeFirst();
    EXPECT_TRUE(args.at(0).toBool());
    EXPECT_TRUE(args.at(1).toBool());
}

// ---------- type mapping / DBus slots ----------

TEST_F(UT_AbstractIndexClientCov, StringToTaskType_KnownAndUnknownStrings)
{
    // Arrange
    const QString create = QStringLiteral("create");

    // Act
    const AbstractIndexClient::TaskType mapped = client->stringToTaskType(create);

    // Assert
    EXPECT_EQ(mapped, AbstractIndexClient::TaskType::Create);
    EXPECT_EQ(client->stringToTaskType(QStringLiteral("create")), AbstractIndexClient::TaskType::Create);
    EXPECT_EQ(client->stringToTaskType(QStringLiteral("update")), AbstractIndexClient::TaskType::Update);
    EXPECT_EQ(client->stringToTaskType(QStringLiteral("create-file-list")), AbstractIndexClient::TaskType::CreateFileList);
    EXPECT_EQ(client->stringToTaskType(QStringLiteral("update-file-list")), AbstractIndexClient::TaskType::UpdateFileList);
    EXPECT_EQ(client->stringToTaskType(QStringLiteral("remove-file-list")), AbstractIndexClient::TaskType::RemoveFileList);
    EXPECT_EQ(client->stringToTaskType(QStringLiteral("move-file-list")), AbstractIndexClient::TaskType::MoveFileList);
    EXPECT_EQ(client->stringToTaskType(QStringLiteral("bogus")), AbstractIndexClient::TaskType::Create);   // fallback
}

TEST_F(UT_AbstractIndexClientCov, OnDBusTaskFinished_SupportedType_EmitsFinishedOrFailed)
{
    // Arrange
    QSignalSpy okSpy(client, &AbstractIndexClient::taskFinished);
    QSignalSpy failSpy(client, &AbstractIndexClient::taskFailed);

    // Act
    client->onDBusTaskFinished(QStringLiteral("update"), QStringLiteral("/tmp"), true);
    client->onDBusTaskFinished(QStringLiteral("create"), QStringLiteral("/var"), false);
    client->onDBusTaskFinished(QStringLiteral("bogus"), QStringLiteral("/tmp"), true);   // ignored

    // Assert
    EXPECT_EQ(okSpy.count(), 1);
    const auto okArgs = okSpy.takeFirst();
    EXPECT_EQ(okArgs.at(0).value<AbstractIndexClient::TaskType>(), AbstractIndexClient::TaskType::Update);
    EXPECT_TRUE(okArgs.at(2).toBool());

    EXPECT_EQ(failSpy.count(), 1);
    const auto failArgs = failSpy.takeFirst();
    EXPECT_EQ(failArgs.at(1).toString(), QString("/var"));
}

TEST_F(UT_AbstractIndexClientCov, OnDBusTaskProgressChanged_EmitsProgress)
{
    // Arrange
    QSignalSpy spy(client, &AbstractIndexClient::taskProgressChanged);

    // Act
    client->onDBusTaskProgressChanged(QStringLiteral("move-file-list"), QStringLiteral("/tmp"), 3, 9);

    // Assert
    EXPECT_EQ(spy.count(), 1);
    const auto args = spy.takeFirst();
    EXPECT_EQ(args.at(0).value<AbstractIndexClient::TaskType>(), AbstractIndexClient::TaskType::MoveFileList);
    EXPECT_EQ(args.at(2).toLongLong(), 3);
    EXPECT_EQ(args.at(3).toLongLong(), 9);
}

TEST_F(UT_AbstractIndexClientCov, OnDBusIndexStatusChanged_EmitsIndexStatusChanged)
{
    // Arrange
    QSignalSpy spy(client, &AbstractIndexClient::indexStatusChanged);

    // Act
    client->onDBusIndexStatusChanged(QStringLiteral("Running"), QStringLiteral("Full"));

    // Assert
    EXPECT_EQ(spy.count(), 1);
    const auto args = spy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), QString("Running"));
    EXPECT_EQ(args.at(1).toString(), QString("Full"));
}

#include "test_abstractindexclient_cov.moc"
