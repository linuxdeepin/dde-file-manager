// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-burn/utils/auditlogjob.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>

#include <QDBusInterface>
#include <QThread>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QFile>

#include <gtest/gtest.h>

DPBURN_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

class UT_AuditLogJob : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.set_lamda(ADDR(QDBusAbstractInterface, isValid), []() {
            return true;
        });

        // Register FileInfo classes like in core.cpp
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        InfoFactory::regClass<AsyncFileInfo>(Global::Scheme::kAsyncFile);
    }
    virtual void TearDown() override { stub.clear(); }

public:
    stub_ext::StubExt stub;
};

TEST_F(UT_AuditLogJob, AuditHelper_bunner)
{
    QString result1 = AuditHelper::bunner(QVariant("/dev/sr0"));
    EXPECT_FALSE(result1.isEmpty());

    QString result2 = AuditHelper::bunner(QVariant(""));
    EXPECT_TRUE(result2.isEmpty());
}

TEST_F(UT_AuditLogJob, AuditHelper_opticalMedia)
{
    QString result1 = AuditHelper::opticalMedia(QVariant("optical_dvd_plus_rw"));
    EXPECT_FALSE(result1.isEmpty());
    EXPECT_EQ(result1, "DVD+RW");

    QString result2 = AuditHelper::opticalMedia(QVariant(""));
    EXPECT_TRUE(result2.isEmpty());
}

TEST_F(UT_AuditLogJob, AuditHelper_idGenerator)
{
    qint64 id1 = AuditHelper::idGenerator();
    qint64 id2 = AuditHelper::idGenerator();

    EXPECT_GT(id1, 0);
    EXPECT_GT(id2, 0);
    EXPECT_NE(id1, id2);   // Should generate different IDs
}

TEST_F(UT_AuditLogJob, AbstractAuditLogJob_Constructor)
{
    CopyFromDiscAuditLog job({}, {});

    // Should not crash
    EXPECT_TRUE(true);
}

TEST_F(UT_AuditLogJob, AbstractAuditLogJob_run)
{
    bool doLogCalled = false;

    CopyFromDiscAuditLog job({}, {});
    job.run();
    EXPECT_FALSE(doLogCalled);

    stub.set_lamda(ADDR(QDBusAbstractInterface, isValid), []() {
        return true;
    });

    stub.set_lamda(VADDR(CopyFromDiscAuditLog, doLog), [&doLogCalled]() {
        __DBG_STUB_INVOKE__
        doLogCalled = true;
    });

    job.run();

    EXPECT_TRUE(doLogCalled);
}

TEST_F(UT_AuditLogJob, CopyFromDiscAuditLog_Constructor)
{
    QList<QUrl> srcUrls = { QUrl::fromLocalFile("/media/sr0/file1.txt") };
    QList<QUrl> destUrls = { QUrl::fromLocalFile("/tmp/file1.txt") };

    CopyFromDiscAuditLog job(srcUrls, destUrls);

    // Should not crash
    EXPECT_TRUE(true);
}

TEST_F(UT_AuditLogJob, CopyFromDiscAuditLog_doLog)
{
    QList<QUrl> srcUrls = { QUrl::fromLocalFile("/media/sr0/file1.txt") };
    QList<QUrl> destUrls = { QUrl::fromLocalFile("/tmp/file1.txt") };

    CopyFromDiscAuditLog job(srcUrls, destUrls);

    bool writeLogCalled = false;
    stub.set_lamda(ADDR(CopyFromDiscAuditLog, writeLog), [&writeLogCalled] {
        __DBG_STUB_INVOKE__
        writeLogCalled = true;
    });

    QDBusInterface interface("test.service", "/test/path", "test.interface");
    job.doLog(interface);

    EXPECT_FALSE(writeLogCalled);
}

TEST_F(UT_AuditLogJob, CopyFromDiscAuditLog_doLog_EmptyUrls)
{
    QList<QUrl> emptyUrls;

    CopyFromDiscAuditLog job(emptyUrls, emptyUrls);

    QDBusInterface interface("test.service", "/test/path", "test.interface");

    // Should handle empty URLs gracefully
    job.doLog(interface);
    // Should not crash
}

TEST_F(UT_AuditLogJob, CopyFromDiscAuditLog_writeLog)
{
    QList<QUrl> srcUrls = { QUrl::fromLocalFile("/media/sr0/file1.txt") };
    QList<QUrl> destUrls = { QUrl::fromLocalFile("/tmp/file1.txt") };

    CopyFromDiscAuditLog job(srcUrls, destUrls);

    bool dbusCallCalled = false;
    stub.set_lamda(ADDR(QDBusInterface, doCall), [&dbusCallCalled] {
        __DBG_STUB_INVOKE__
        dbusCallCalled = true;
        return QDBusMessage();
    });

    QDBusInterface interface("test.service", "/test/path", "test.interface");
    job.writeLog(interface, "/media/sr0/file1.txt", "/tmp/file1.txt");

    EXPECT_TRUE(dbusCallCalled);
}

TEST_F(UT_AuditLogJob, BurnFilesAuditLogJob_Constructor)
{
    QUrl stagingUrl = QUrl::fromLocalFile("/tmp/staging");

    BurnFilesAuditLogJob job(stagingUrl, true);

    // Should not crash
    EXPECT_TRUE(true);
}

TEST_F(UT_AuditLogJob, BurnFilesAuditLogJob_Constructor_Failed)
{
    QUrl stagingUrl = QUrl::fromLocalFile("/tmp/staging");

    BurnFilesAuditLogJob job(stagingUrl, false);

    // Should not crash
    EXPECT_TRUE(true);
}

TEST_F(UT_AuditLogJob, BurnFilesAuditLogJob_doLog_Success)
{
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());

    // Create test files
    QFile file1(tempDir.path() + "/file1.txt");
    ASSERT_TRUE(file1.open(QIODevice::WriteOnly));
    file1.write("test content");
    file1.close();

    QUrl stagingUrl = QUrl::fromLocalFile(tempDir.path());
    BurnFilesAuditLogJob job(stagingUrl, true);

    bool writeLogCalled = false;
    stub.set_lamda(ADDR(BurnFilesAuditLogJob, writeLog), [&writeLogCalled] {
        __DBG_STUB_INVOKE__
        writeLogCalled = true;
    });

    QDBusInterface interface("test.service", "/test/path", "test.interface");
    job.doLog(interface);

    EXPECT_TRUE(writeLogCalled);
}

TEST_F(UT_AuditLogJob, BurnFilesAuditLogJob_doLog_Failed)
{
    QUrl stagingUrl = QUrl::fromLocalFile("/tmp/nonexistent");
    BurnFilesAuditLogJob job(stagingUrl, false);

    QDBusInterface interface("test.service", "/test/path", "test.interface");

    // Should handle failure case gracefully
    job.doLog(interface);
    // Should not crash
}

TEST_F(UT_AuditLogJob, BurnFilesAuditLogJob_writeLog)
{
    QUrl stagingUrl = QUrl::fromLocalFile("/tmp/staging");
    BurnFilesAuditLogJob job(stagingUrl, true);

    bool dbusCallCalled = false;

    stub.set_lamda(ADDR(QDBusInterface, doCall), [&dbusCallCalled] {
        __DBG_STUB_INVOKE__
        dbusCallCalled = true;
        return QDBusMessage();
    });

    stub.set_lamda(ADDR(Settings, sync), [](Settings *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    QDBusInterface interface("test.service", "/test/path", "test.interface");
    job.writeLog(interface, "/dev/sr0/file1.txt", "/tmp/staging/file1.txt", 1024);

    EXPECT_TRUE(dbusCallCalled);
}

TEST_F(UT_AuditLogJob, BurnFilesAuditLogJob_burnedFileInfoList)
{
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());

    // Create test files
    QFile file1(tempDir.path() + "/file1.txt");
    ASSERT_TRUE(file1.open(QIODevice::WriteOnly));
    file1.write("test content 1");
    file1.close();

    QFile file2(tempDir.path() + "/file2.txt");
    ASSERT_TRUE(file2.open(QIODevice::WriteOnly));
    file2.write("test content 2");
    file2.close();

    QUrl stagingUrl = QUrl::fromLocalFile(tempDir.path());
    BurnFilesAuditLogJob job(stagingUrl, true);

    QFileInfoList fileList = job.burnedFileInfoList();

    EXPECT_EQ(fileList.size(), 2);
}

TEST_F(UT_AuditLogJob, BurnFilesAuditLogJob_burnedFileInfoList_EmptyDir)
{
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());

    QUrl stagingUrl = QUrl::fromLocalFile(tempDir.path());
    BurnFilesAuditLogJob job(stagingUrl, true);

    QFileInfoList fileList = job.burnedFileInfoList();

    EXPECT_TRUE(fileList.isEmpty());
}

TEST_F(UT_AuditLogJob, EraseDiscAuditLogJob_Constructor_Success)
{
    EraseDiscAuditLogJob job(true);

    // Should not crash
    EXPECT_TRUE(true);
}

TEST_F(UT_AuditLogJob, EraseDiscAuditLogJob_Constructor_Failed)
{
    EraseDiscAuditLogJob job(false);

    // Should not crash
    EXPECT_TRUE(true);
}

TEST_F(UT_AuditLogJob, EraseDiscAuditLogJob_doLog_Success)
{
    EraseDiscAuditLogJob job(true);

    bool dbusCallCalled = false;

    stub.set_lamda(ADDR(QDBusInterface, doCall), [&dbusCallCalled] {
        __DBG_STUB_INVOKE__
        dbusCallCalled = true;
        return QDBusMessage();
    });

    QDBusInterface interface("test.service", "/test/path", "test.interface");
    job.doLog(interface);

    EXPECT_TRUE(dbusCallCalled);
}

TEST_F(UT_AuditLogJob, EraseDiscAuditLogJob_doLog_Failed)
{
    EraseDiscAuditLogJob job(false);

    QDBusInterface interface("test.service", "/test/path", "test.interface");

    // Should handle failure case gracefully
    job.doLog(interface);
    // Should not crash (no D-Bus call expected for failed erase)
}

TEST_F(UT_AuditLogJob, AbstractAuditLogJob_run_DBusInterfaceError)
{
    CopyFromDiscAuditLog job({}, {});

    // Mock QDBusInterface to be invalid
    stub.set_lamda(ADDR(QDBusInterface, isValid), [] {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Should handle invalid D-Bus interface gracefully
    job.run();
    // Should not crash
}
