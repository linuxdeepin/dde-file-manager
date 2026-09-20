// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filenameindexdbus.cpp
 * @brief Unit tests for FileNameIndexDBus (filenameindexdbus.cpp)
 *        — constructs the DBus object (which creates a filename-profile IndexRuntime
 *        internally) and exercises the public slot API. DBus registration on the
 *        session bus fails gracefully in the sandbox, so the object still works.
 *
 * Global::fileNameIndexDirectory / availability / scope functions are stubbed to
 * point at a QTemporaryDir so the test is fully host-isolated.
 *
 * Covers (design 9.1): DBus construction, IsEnabled/SetEnabled, StopCurrentTask,
 * HasRunningTask, IndexDatabaseExists (version/lastUpdateTime checks), GetLastUpdateTime,
 * ProcessFileChanges/ProcessFileMoves, GetIndexStatus, ForceUpdateIndex, cleanup.
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QHash>

#include "stubext.h"
#include <dfm-search/dsearch_global.h>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/profile/indexprofile.h"
#include "services/textindex/filenameindexdbus.h"

using namespace SERVICETEXTINDEX_NAMESPACE;
using namespace DFMSEARCH;

namespace {

void writeStatusJson(const QString &indexDir, const QJsonObject &obj)
{
    QDir().mkpath(indexDir);
    QFile f(indexDir + QLatin1String("/index_status.json"));
    ASSERT_TRUE(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
    f.write(QJsonDocument(obj).toJson());
    f.close();
}

}   // namespace

class FileNameIndexDBusTest : public testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(tmp.isValid());

        // Redirect filename-index directory to the temp dir.
        stub.set_lamda(ADDR(Global, fileNameIndexDirectory),
                       [this]() -> QString {
                           __DBG_STUB_INVOKE__
                           return tmp.path() + "/filename-index";
                       });
        stub.set_lamda(ADDR(Global, isFileNameIndexDirectoryAvailable),
                       []() -> bool {
                           __DBG_STUB_INVOKE__
                           return true;
                       });
        stub.set_lamda(ADDR(Global, isPathInFileNameIndexDirectory),
                       [this](const QString &path) -> bool {
                           __DBG_STUB_INVOKE__
                           return path.startsWith(tmp.path());
                       });

        // Also stub content/ocr dirs so the filename blacklistProvider (which
        // includes them) resolves to deterministic temp paths, not $HOME.
        stub.set_lamda(ADDR(Global, contentIndexDirectory),
                       [this]() -> QString {
                           __DBG_STUB_INVOKE__
                           return tmp.path() + "/content-index";
                       });
        stub.set_lamda(ADDR(Global, ocrTextIndexDirectory),
                       [this]() -> QString {
                           __DBG_STUB_INVOKE__
                           return tmp.path() + "/ocr-index";
                       });

        // Prevent handleSilentStart() from scanning real system directories.
        stub.set_lamda(ADDR(Global, defaultIndexedDirectory),
                       [this]() -> QStringList {
                           __DBG_STUB_INVOKE__
                           return QStringList { tmp.path() + "/indexed-dir" };
                       });

        // Prevent reading real system blacklist paths from DConfig.
        stub.set_lamda(ADDR(Global, defaultBlacklistPaths),
                       []() -> QStringList {
                           __DBG_STUB_INVOKE__
                           return QStringList();
                       });

        // isFileNameIndexReadyForSearch stubbed to false so FSMonitorWorker fast-scan
        // does not attempt a real index query.
        stub.set_lamda(ADDR(Global, isFileNameIndexReadyForSearch),
                       []() -> bool {
                           __DBG_STUB_INVOKE__
                           return false;
                       });
    }

    QTemporaryDir tmp;
    stub_ext::StubExt stub;
};

// --- Construction / destruction ---

TEST_F(FileNameIndexDBusTest, ConstructAndDestruct)
{
    {
        FileNameIndexDBus dbus;
        SUCCEED();
    }
}

TEST_F(FileNameIndexDBusTest, CleanupOnFreshInstance)
{
    FileNameIndexDBus dbus;
    EXPECT_NO_FATAL_FAILURE({ dbus.cleanup(); });
}

// --- IsEnabled / SetEnabled ---

TEST_F(FileNameIndexDBusTest, IsEnabled_ReturnsBool)
{
    FileNameIndexDBus dbus;
    bool enabled = false;
    EXPECT_NO_FATAL_FAILURE({ enabled = dbus.IsEnabled(); });
    (void)enabled;
}

TEST_F(FileNameIndexDBusTest, SetEnabled_DoesNotCrash)
{
    FileNameIndexDBus dbus;
    EXPECT_NO_FATAL_FAILURE({ dbus.SetEnabled(false); });
    EXPECT_NO_FATAL_FAILURE({ dbus.SetEnabled(true); });
}

// --- StopCurrentTask / HasRunningTask ---

TEST_F(FileNameIndexDBusTest, StopCurrentTask_NoTask_ReturnsFalse)
{
    FileNameIndexDBus dbus;
    EXPECT_FALSE(dbus.StopCurrentTask());
}

TEST_F(FileNameIndexDBusTest, HasRunningTask_NoTask_ReturnsFalse)
{
    FileNameIndexDBus dbus;
    EXPECT_FALSE(dbus.HasRunningTask());
}

// --- IndexDatabaseExists ---

TEST_F(FileNameIndexDBusTest, IndexDatabaseExists_NoStatusFile_ReturnsFalse)
{
    FileNameIndexDBus dbus;
    EXPECT_FALSE(dbus.IndexDatabaseExists());
}

TEST_F(FileNameIndexDBusTest, IndexDatabaseExists_VersionMismatch_ReturnsFalse)
{
    const QString indexDir = tmp.path() + "/filename-index";
    writeStatusJson(indexDir, { { "version", 999 },
                                { "lastUpdateTime", "2026-01-01T00:00:00" } });
    FileNameIndexDBus dbus;
    EXPECT_FALSE(dbus.IndexDatabaseExists());
}

TEST_F(FileNameIndexDBusTest, IndexDatabaseExists_MissingLastUpdateTime_ReturnsFalse)
{
    const QString indexDir = tmp.path() + "/filename-index";
    // version matches kFilenameIndexVersion (1) but lastUpdateTime empty
    writeStatusJson(indexDir, { { "version", Defines::kFilenameIndexVersion } });
    FileNameIndexDBus dbus;
    EXPECT_FALSE(dbus.IndexDatabaseExists());
}

TEST_F(FileNameIndexDBusTest, IndexDatabaseExists_ValidStatus_ReturnsTrue)
{
    const QString indexDir = tmp.path() + "/filename-index";
    writeStatusJson(indexDir, { { "version", Defines::kFilenameIndexVersion },
                                { "lastUpdateTime", "2026-01-01T00:00:00" },
                                { "state", "clean" } });
    FileNameIndexDBus dbus;
    EXPECT_TRUE(dbus.IndexDatabaseExists());
}

// --- GetLastUpdateTime ---

TEST_F(FileNameIndexDBusTest, GetLastUpdateTime_NoStatusFile_ReturnsEmpty)
{
    FileNameIndexDBus dbus;
    EXPECT_TRUE(dbus.GetLastUpdateTime().isEmpty());
}

TEST_F(FileNameIndexDBusTest, GetLastUpdateTime_WithStatus_ReturnsTime)
{
    const QString indexDir = tmp.path() + "/filename-index";
    writeStatusJson(indexDir, { { "lastUpdateTime", "2026-01-01T00:00:00" } });
    FileNameIndexDBus dbus;
    QString t = dbus.GetLastUpdateTime();
    EXPECT_FALSE(t.isEmpty());
}

// --- ProcessFileChanges / ProcessFileMoves ---

TEST_F(FileNameIndexDBusTest, ProcessFileChanges_AllEmpty_NoTasksQueued)
{
    FileNameIndexDBus dbus;
    EXPECT_NO_FATAL_FAILURE({ (void)dbus.ProcessFileChanges({}, {}, {}); });
}

TEST_F(FileNameIndexDBusTest, ProcessFileChanges_WithCreatedFiles)
{
    FileNameIndexDBus dbus;
    QStringList created { tmp.path() + "/new1.txt", tmp.path() + "/new2.txt" };
    EXPECT_NO_FATAL_FAILURE({ (void)dbus.ProcessFileChanges(created, {}, {}); });
}

TEST_F(FileNameIndexDBusTest, ProcessFileChanges_WithModifiedFiles)
{
    FileNameIndexDBus dbus;
    QStringList modified { tmp.path() + "/mod1.txt" };
    EXPECT_NO_FATAL_FAILURE({ (void)dbus.ProcessFileChanges({}, modified, {}); });
}

TEST_F(FileNameIndexDBusTest, ProcessFileChanges_WithDeletedFiles)
{
    FileNameIndexDBus dbus;
    QStringList deleted { tmp.path() + "/gone1.txt" };
    EXPECT_NO_FATAL_FAILURE({ (void)dbus.ProcessFileChanges({}, {}, deleted); });
}

TEST_F(FileNameIndexDBusTest, ProcessFileMoves_EmptyMap_ReturnsFalse)
{
    FileNameIndexDBus dbus;
    QHash<QString, QString> empty;
    EXPECT_FALSE(dbus.ProcessFileMoves(empty));
}

TEST_F(FileNameIndexDBusTest, ProcessFileMoves_WithMoves)
{
    FileNameIndexDBus dbus;
    QHash<QString, QString> moves { { tmp.path() + "/old.txt", tmp.path() + "/new.txt" } };
    EXPECT_NO_FATAL_FAILURE({ (void)dbus.ProcessFileMoves(moves); });
}

// --- GetIndexStatus ---

TEST_F(FileNameIndexDBusTest, GetIndexStatus_ReturnsNonEmptyMap)
{
    FileNameIndexDBus dbus;
    QVariantMap status = dbus.GetIndexStatus();
    EXPECT_FALSE(status.isEmpty());
    EXPECT_TRUE(status.contains("state"));
    EXPECT_TRUE(status.contains("grade"));
}

// --- CreateIndexTask / UpdateIndexTask ---

TEST_F(FileNameIndexDBusTest, CreateIndexTask_WithPaths)
{
    FileNameIndexDBus dbus;
    QStringList paths { tmp.path() + "/indexed-dir" };
    EXPECT_NO_FATAL_FAILURE({ (void)dbus.CreateIndexTask(paths, {}); });
}

TEST_F(FileNameIndexDBusTest, UpdateIndexTask_WithPaths)
{
    FileNameIndexDBus dbus;
    QStringList paths { tmp.path() + "/indexed-dir" };
    EXPECT_NO_FATAL_FAILURE({ (void)dbus.UpdateIndexTask(paths, {}); });
}

// --- ForceUpdateIndex ---

TEST_F(FileNameIndexDBusTest, ForceUpdateIndex_WithManualOption)
{
    FileNameIndexDBus dbus;
    QStringList paths { tmp.path() + "/indexed-dir" };
    QVariantMap opts { { "manual", true } };
    EXPECT_NO_FATAL_FAILURE({ (void)dbus.ForceUpdateIndex(paths, opts); });
}

TEST_F(FileNameIndexDBusTest, ForceUpdateIndex_NonManual)
{
    FileNameIndexDBus dbus;
    QStringList paths { tmp.path() + "/indexed-dir" };
    QVariantMap opts { { "manual", false } };
    EXPECT_NO_FATAL_FAILURE({ (void)dbus.ForceUpdateIndex(paths, opts); });
}
