// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/reportlog/reportlogworker.h"
#include "plugins/common/dfmplugin-utils/reportlog/datas/reportdatainterface.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/device/private/devicehelper.h>

#include <QLibrary>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;
DFMBASE_USE_NAMESPACE

class UT_ReportLogWorker : public testing::Test
{
protected:
    void SetUp() override
    {
        worker = new ReportLogWorker();
    }

    void TearDown() override
    {
        delete worker;
        worker = nullptr;
        stub.clear();
    }

    ReportLogWorker *worker { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_ReportLogWorker, init_LibraryLoadFailed_NoCrash)
{
    stub.set_lamda(ADDR(QLibrary, load),
                   [](QLibrary *) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    EXPECT_NO_THROW(worker->init());
}

TEST_F(UT_ReportLogWorker, init_ResolveFailed_NoCrash)
{
    stub.set_lamda(ADDR(QLibrary, load),
                   [](QLibrary *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(qOverload<const char *>(&QLibrary::resolve),
                   [](QLibrary *, const char *) -> QFunctionPointer {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    EXPECT_NO_THROW(worker->init());
}

TEST_F(UT_ReportLogWorker, commitLog_UnregisteredType_Returns)
{
    worker->commitLog("UnknownType", QVariantMap());
}

TEST_F(UT_ReportLogWorker, registerLogData_DuplicateType_ReturnsFalse)
{
    class MockReportData : public ReportDataInterface
    {
    public:
        QString type() const override { return "MockType"; }
        QJsonObject prepareData(const QVariantMap &) const override { return QJsonObject(); }
    };

    MockReportData *data1 = new MockReportData();
    MockReportData *data2 = new MockReportData();

    bool result1 = worker->registerLogData("MockType", data1);
    bool result2 = worker->registerLogData("MockType", data2);

    EXPECT_TRUE(result1);
    EXPECT_FALSE(result2);

    delete data2;
}

TEST_F(UT_ReportLogWorker, handleMenuData_WithUrls_SetsFileLocation)
{
    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/test.txt") };

    auto mockFileInfo = QSharedPointer<FileInfo>(new FileInfo(urls[0]));
    stub.set_lamda(static_cast<FileInfoPointer(*)(const QUrl&, Global::CreateFileInfoType, QString*)>(&InfoFactory::create<FileInfo>),
                   [&mockFileInfo](const QUrl &, const Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return mockFileInfo;
                   });

    stub.set_lamda(VADDR(FileInfo, nameOf),
                   [](FileInfo *, const NameInfoType) -> QString {
                       __DBG_STUB_INVOKE__
                       return "text/plain";
                   });

    stub.set_lamda(ADDR(ReportLogWorker, commitLog),
                   [](ReportLogWorker *, const QString &, const QVariantMap &) {
                       __DBG_STUB_INVOKE__
                   });

    worker->handleMenuData("TestAction", urls);
}

TEST_F(UT_ReportLogWorker, handleMenuData_EmptyUrls_SetsWorkspaceLocation)
{
    stub.set_lamda(ADDR(ReportLogWorker, commitLog),
                   [](ReportLogWorker *, const QString &, const QVariantMap &) {
                       __DBG_STUB_INVOKE__
                   });

    worker->handleMenuData("TestAction", QList<QUrl>());
}

TEST_F(UT_ReportLogWorker, handleBlockMountData_EmptyId_Returns)
{
    worker->handleBlockMountData("", true);
}

TEST_F(UT_ReportLogWorker, handleBlockMountData_FailedResult_CommitsWithUnknown)
{
    stub.set_lamda(ADDR(ReportLogWorker, commitLog),
                   [](ReportLogWorker *, const QString &, const QVariantMap &) {
                       __DBG_STUB_INVOKE__
                   });

    worker->handleBlockMountData("/dev/sda1", false);
}

TEST_F(UT_ReportLogWorker, handleMountNetworkResult_Success_CommitsData)
{
    stub.set_lamda(ADDR(ReportLogWorker, commitLog),
                   [](ReportLogWorker *, const QString &, const QVariantMap &) {
                       __DBG_STUB_INVOKE__
                   });

    worker->handleMountNetworkResult(true, dfmmount::DeviceError::kNoError, "");
}

TEST_F(UT_ReportLogWorker, handleMountNetworkResult_UserCancelled_SetsErrorId)
{
    stub.set_lamda(ADDR(ReportLogWorker, commitLog),
                   [](ReportLogWorker *, const QString &, const QVariantMap &) {
                       __DBG_STUB_INVOKE__
                   });

    worker->handleMountNetworkResult(false, dfmmount::DeviceError::kUserErrorUserCancelled, "User cancelled");
}

TEST_F(UT_ReportLogWorker, commit_NullArgs_Returns)
{
    worker->commit(QVariant());
}

TEST_F(UT_ReportLogWorker, commit_InvalidArgs_Returns)
{
    QVariant invalid;
    worker->commit(invalid);
}

// ---------------------------------------------------------------------------
// PMS sev-2 regression additions (report log worker)
// ---------------------------------------------------------------------------

// PMS:355011 when the deepin-event-log library fails to load, init() must bail
// out cleanly and later commit calls must stay harmless (no dangling function
// pointer dereference).
TEST_F(UT_ReportLogWorker, BUG355011_InitFailureKeepsCommitSafe)
{
    stub.set_lamda(ADDR(QLibrary, load),
                   [](QLibrary *) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    EXPECT_NO_THROW(worker->init());
    EXPECT_NO_THROW(worker->commitLog(QStringLiteral("CrashProbe"), QVariantMap()));
}

// PMS:355011 init() is invoked on every plugin load; the second call must be a
// no-op for the data registry — datas must not be registered twice.
TEST_F(UT_ReportLogWorker, BUG355011_InitTwiceDoesNotReregisterDatas)
{
    stub.set_lamda(ADDR(QLibrary, load),
                   [](QLibrary *) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    worker->init();   // first run populates the registry

    int registerCalls = 0;
    stub.set_lamda(ADDR(ReportLogWorker, registerLogData),
                   [&registerCalls](ReportLogWorker *, const QString &, ReportDataInterface *) -> bool {
                       __DBG_STUB_INVOKE__
                       ++registerCalls;
                       return true;
                   });

    worker->init();   // second run must short-circuit on the idempotency guard
    EXPECT_EQ(registerCalls, 0);
}
