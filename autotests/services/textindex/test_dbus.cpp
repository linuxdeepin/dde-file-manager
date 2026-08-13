// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dbus.cpp
 * @brief Unit tests for TextIndexDBus and OcrIndexDBus (textindexdbus.cpp + ocrindexdbus.cpp)
 *        — constructs the DBus objects (they create an IndexRuntime internally) and
 *        exercises the public slot API. DBus registration on session bus will fail
 *        gracefully in the test sandbox (no dbus-daemon), so the object still works.
 *
 * We stub DFMSEARCH::Global directory/availability functions so that
 * IndexProfile::content()/ocr() resolve to a QTemporaryDir instead of the real
 * system index path.  This provides full host-environment isolation while
 * still exercising the real production code paths.
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
#include "services/textindex/textindexdbus.h"
#include "services/textindex/ocrindexdbus.h"

using namespace SERVICETEXTINDEX_NAMESPACE;
using namespace DFMSEARCH;

// ---------------------------------------------------------------------------
// Helper: write a JSON status file into the temp index directory so that
// IndexStateStore::readStatusJson() returns a controlled QJsonObject.
// ---------------------------------------------------------------------------
static void writeStatusJson(const QString &indexDir, const QJsonObject &obj)
{
    QDir().mkpath(indexDir);
    QFile f(indexDir + QLatin1String("/index_status.json"));
    ASSERT_TRUE(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
    f.write(QJsonDocument(obj).toJson());
    f.close();
}

// ---------------------------------------------------------------------------
// DBusTest fixture
// ---------------------------------------------------------------------------
class DBusTest : public testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(tmp.isValid());

        // Redirect Global::contentIndexDirectory to the temp dir
        stub.set_lamda(ADDR(Global, contentIndexDirectory),
                       [this]() -> QString {
                           __DBG_STUB_INVOKE__
                           return tmp.path() + "/content-index";
                       });

        // Let isContentIndexAvailable return true so that IndexDatabaseExists
        // can exercise the deeper branches (version check, lastUpdateTime check).
        stub.set_lamda(ADDR(Global, isContentIndexAvailable),
                       []() -> bool {
                           __DBG_STUB_INVOKE__
                           return true;
                       });

        stub.set_lamda(ADDR(Global, isPathInContentIndexDirectory),
                       [this](const QString &path) -> bool {
                           __DBG_STUB_INVOKE__
                           return path.startsWith(tmp.path());
                       });

        // Redirect Global::ocrTextIndexDirectory to the temp dir
        stub.set_lamda(ADDR(Global, ocrTextIndexDirectory),
                       [this]() -> QString {
                           __DBG_STUB_INVOKE__
                           return tmp.path() + "/ocr-index";
                       });

        stub.set_lamda(ADDR(Global, isOcrTextIndexAvailable),
                       []() -> bool {
                           __DBG_STUB_INVOKE__
                           return true;
                       });

        stub.set_lamda(ADDR(Global, isPathInOcrTextIndexDirectory),
                       [this](const QString &path) -> bool {
                           __DBG_STUB_INVOKE__
                           return path.startsWith(tmp.path());
                       });

        // Prevent handleSlientStart() from scanning real system directories
        stub.set_lamda(ADDR(Global, defaultIndexedDirectory),
                       [this]() -> QStringList {
                           __DBG_STUB_INVOKE__
                           return QStringList { tmp.path() + "/indexed-dir" };
                       });

        // Prevent reading real system blacklist paths from DConfig
        stub.set_lamda(ADDR(Global, defaultBlacklistPaths),
                       []() -> QStringList {
                           __DBG_STUB_INVOKE__
                           return QStringList();
                       });
    }

    void TearDown() override
    {
        // stub_ext::StubExt destructor resets all stubs automatically
    }

    QTemporaryDir tmp;
    stub_ext::StubExt stub;
};

// ========================== TextIndexDBus ================================

TEST_F(DBusTest, TextIndexDBusConstructAndDestruct)
{
    {
        TextIndexDBus dbus;
        SUCCEED();
    }
}

TEST_F(DBusTest, TextIndexDBusInit)
{
    TextIndexDBus dbus;
    EXPECT_NO_FATAL_FAILURE({ dbus.Init(); });
}

TEST_F(DBusTest, TextIndexDBusIsEnabled)
{
    TextIndexDBus dbus;
    EXPECT_NO_FATAL_FAILURE({ (void)dbus.IsEnabled(); });
}

TEST_F(DBusTest, TextIndexDBusSetEnabled)
{
    TextIndexDBus dbus;
    EXPECT_NO_FATAL_FAILURE({ dbus.SetEnabled(false); });
}

TEST_F(DBusTest, TextIndexDBusStopCurrentTask)
{
    TextIndexDBus dbus;
    EXPECT_FALSE(dbus.StopCurrentTask());
}

TEST_F(DBusTest, TextIndexDBusHasRunningTask)
{
    TextIndexDBus dbus;
    EXPECT_FALSE(dbus.HasRunningTask());
}

// --- No status file → isIndexAvailable() passes, but version missing
//     → isCompatibleVersion() fails → returns false
TEST_F(DBusTest, TextIndexDBusIndexDatabaseExistsNoStatusFile)
{
    TextIndexDBus dbus;
    EXPECT_FALSE(dbus.IndexDatabaseExists());
}

// --- Status file with wrong version → isCompatibleVersion() fails
TEST_F(DBusTest, TextIndexDBusIndexDatabaseExistsVersionMismatch)
{
    const QString indexDir = tmp.path() + "/content-index";
    writeStatusJson(indexDir, { { "version", 999 }, { "lastUpdateTime", "2026-01-01T00:00:00" } });

    TextIndexDBus dbus;
    EXPECT_FALSE(dbus.IndexDatabaseExists());
}

// --- Version ok but lastUpdateTime missing → returns false
TEST_F(DBusTest, TextIndexDBusIndexDatabaseExistsNoLastUpdateTime)
{
    const QString indexDir = tmp.path() + "/content-index";
    writeStatusJson(indexDir, { { "version", Defines::kTextIndexVersion } });

    TextIndexDBus dbus;
    EXPECT_FALSE(dbus.IndexDatabaseExists());
}

// --- All checks pass → returns true
TEST_F(DBusTest, TextIndexDBusIndexDatabaseExistsAllGood)
{
    const QString indexDir = tmp.path() + "/content-index";
    writeStatusJson(indexDir, {
        { "version", Defines::kTextIndexVersion },
        { "lastUpdateTime", "2026-03-11T15:34:16" },
        { "state", "clean" }
    });

    TextIndexDBus dbus;
    EXPECT_TRUE(dbus.IndexDatabaseExists());
}

// --- No status file → getLastUpdateTime returns empty
TEST_F(DBusTest, TextIndexDBusGetLastUpdateTimeEmpty)
{
    TextIndexDBus dbus;
    QString t = dbus.GetLastUpdateTime();
    EXPECT_TRUE(t.isEmpty());
}

// --- Status file with lastUpdateTime → returns formatted time
TEST_F(DBusTest, TextIndexDBusGetLastUpdateTimeValid)
{
    const QString indexDir = tmp.path() + "/content-index";
    writeStatusJson(indexDir, {
        { "version", Defines::kTextIndexVersion },
        { "lastUpdateTime", "2026-03-11T15:34:16" }
    });

    TextIndexDBus dbus;
    QString t = dbus.GetLastUpdateTime();
    EXPECT_FALSE(t.isEmpty());
    // IndexStateStore formats as "yyyy-MM-dd hh:mm:ss"
    EXPECT_EQ(t, QString("2026-03-11 15:34:16"));
}

TEST_F(DBusTest, TextIndexDBusCreateIndexTaskEmpty)
{
    TextIndexDBus dbus;
    EXPECT_FALSE(dbus.CreateIndexTask({}));
}

TEST_F(DBusTest, TextIndexDBusUpdateIndexTaskEmpty)
{
    TextIndexDBus dbus;
    EXPECT_FALSE(dbus.UpdateIndexTask({}));
}

TEST_F(DBusTest, TextIndexDBusProcessFileChangesEmpty)
{
    TextIndexDBus dbus;
    EXPECT_FALSE(dbus.ProcessFileChanges({}, {}, {}));
}

TEST_F(DBusTest, TextIndexDBusProcessFileMovesEmpty)
{
    TextIndexDBus dbus;
    EXPECT_FALSE(dbus.ProcessFileMoves({}));
}

TEST_F(DBusTest, TextIndexDBusCleanup)
{
    TextIndexDBus dbus;
    EXPECT_NO_FATAL_FAILURE({ dbus.cleanup(); });
}

// ========================== OcrIndexDBus =================================

TEST_F(DBusTest, OcrIndexDBusConstructAndDestruct)
{
    {
        OcrIndexDBus dbus;
        SUCCEED();
    }
}

TEST_F(DBusTest, OcrIndexDBusInit)
{
    OcrIndexDBus dbus;
    EXPECT_NO_FATAL_FAILURE({ dbus.Init(); });
}

TEST_F(DBusTest, OcrIndexDBusIsEnabled)
{
    OcrIndexDBus dbus;
    EXPECT_NO_FATAL_FAILURE({ (void)dbus.IsEnabled(); });
}

TEST_F(DBusTest, OcrIndexDBusSetEnabled)
{
    OcrIndexDBus dbus;
    EXPECT_NO_FATAL_FAILURE({ dbus.SetEnabled(false); });
}

TEST_F(DBusTest, OcrIndexDBusStopCurrentTask)
{
    OcrIndexDBus dbus;
    EXPECT_FALSE(dbus.StopCurrentTask());
}

TEST_F(DBusTest, OcrIndexDBusHasRunningTask)
{
    OcrIndexDBus dbus;
    EXPECT_FALSE(dbus.HasRunningTask());
}

// --- No status file → version missing → returns false
TEST_F(DBusTest, OcrIndexDBusIndexDatabaseExistsNoStatusFile)
{
    OcrIndexDBus dbus;
    EXPECT_FALSE(dbus.IndexDatabaseExists());
}

// --- Version ok but lastUpdateTime missing → returns false
TEST_F(DBusTest, OcrIndexDBusIndexDatabaseExistsNoLastUpdateTime)
{
    const QString indexDir = tmp.path() + "/ocr-index";
    writeStatusJson(indexDir, { { "version", Defines::kOcrIndexVersion } });

    OcrIndexDBus dbus;
    EXPECT_FALSE(dbus.IndexDatabaseExists());
}

// --- All checks pass → returns true
TEST_F(DBusTest, OcrIndexDBusIndexDatabaseExistsAllGood)
{
    const QString indexDir = tmp.path() + "/ocr-index";
    writeStatusJson(indexDir, {
        { "version", Defines::kOcrIndexVersion },
        { "lastUpdateTime", "2026-03-11T15:34:16" },
        { "state", "clean" }
    });

    OcrIndexDBus dbus;
    EXPECT_TRUE(dbus.IndexDatabaseExists());
}

// --- No status file → returns empty
TEST_F(DBusTest, OcrIndexDBusGetLastUpdateTimeEmpty)
{
    OcrIndexDBus dbus;
    QString t = dbus.GetLastUpdateTime();
    EXPECT_TRUE(t.isEmpty());
}

// --- Valid lastUpdateTime → returns formatted time
TEST_F(DBusTest, OcrIndexDBusGetLastUpdateTimeValid)
{
    const QString indexDir = tmp.path() + "/ocr-index";
    writeStatusJson(indexDir, {
        { "version", Defines::kOcrIndexVersion },
        { "lastUpdateTime", "2026-03-11T15:34:16" }
    });

    OcrIndexDBus dbus;
    QString t = dbus.GetLastUpdateTime();
    EXPECT_FALSE(t.isEmpty());
    EXPECT_EQ(t, QString("2026-03-11 15:34:16"));
}

TEST_F(DBusTest, OcrIndexDBusCreateIndexTaskEmpty)
{
    OcrIndexDBus dbus;
    EXPECT_FALSE(dbus.CreateIndexTask({}));
}

TEST_F(DBusTest, OcrIndexDBusProcessFileChangesEmpty)
{
    OcrIndexDBus dbus;
    EXPECT_FALSE(dbus.ProcessFileChanges({}, {}, {}));
}

TEST_F(DBusTest, OcrIndexDBusProcessFileMovesEmpty)
{
    OcrIndexDBus dbus;
    EXPECT_FALSE(dbus.ProcessFileMoves({}));
}

TEST_F(DBusTest, OcrIndexDBusCleanup)
{
    OcrIndexDBus dbus;
    EXPECT_NO_FATAL_FAILURE({ dbus.cleanup(); });
}

// ---- Additional TextIndexDBus coverage ----

TEST_F(DBusTest, TextIndexDBusCreateIndexTaskWithPaths)
{
    TextIndexDBus dbus;
    // Non-empty paths, but TaskManager::startTask will fail if index not ready
    QVariantMap opts;
    opts["silent"] = true;
    bool result = dbus.CreateIndexTask({ tmp.path() + "/indexed-dir" }, opts);
    // May succeed or fail depending on runtime state, just no crash
    SUCCEED();
}

TEST_F(DBusTest, TextIndexDBusUpdateIndexTaskWithPaths)
{
    TextIndexDBus dbus;
    QVariantMap opts;
    opts["silent"] = true;
    bool result = dbus.UpdateIndexTask({ tmp.path() + "/indexed-dir" }, opts);
    SUCCEED();
}

TEST_F(DBusTest, TextIndexDBusProcessFileChanges_OnlyDeleted)
{
    TextIndexDBus dbus;
    bool result = dbus.ProcessFileChanges(
            {},
            {},
            { "/fake/deleted/file.txt" });
    // May or may not queue task depending on runtime
    SUCCEED();
}

TEST_F(DBusTest, TextIndexDBusProcessFileChanges_OnlyCreated)
{
    TextIndexDBus dbus;
    bool result = dbus.ProcessFileChanges(
            { "/fake/created/file.txt" },
            {},
            {});
    SUCCEED();
}

TEST_F(DBusTest, TextIndexDBusProcessFileChanges_OnlyModified)
{
    TextIndexDBus dbus;
    bool result = dbus.ProcessFileChanges(
            {},
            { "/fake/modified/file.txt" },
            {});
    SUCCEED();
}

TEST_F(DBusTest, TextIndexDBusProcessFileChanges_AllThree)
{
    TextIndexDBus dbus;
    bool result = dbus.ProcessFileChanges(
            { "/fake/created.txt" },
            { "/fake/modified.txt" },
            { "/fake/deleted.txt" });
    SUCCEED();
}

TEST_F(DBusTest, TextIndexDBusProcessFileMoves_NonEmpty)
{
    TextIndexDBus dbus;
    QHash<QString, QString> moves;
    moves["/old/path.txt"] = "/new/path.txt";
    bool result = dbus.ProcessFileMoves(moves);
    SUCCEED();
}

TEST_F(DBusTest, TextIndexDBusIndexDatabaseExists_VersionMismatch)
{
    const QString indexDir = tmp.path() + "/content-index";
    writeStatusJson(indexDir, { { "version", 99999 } });

    TextIndexDBus dbus;
    EXPECT_FALSE(dbus.IndexDatabaseExists());
}

// ---- Additional OcrIndexDBus coverage ----

TEST_F(DBusTest, OcrIndexDBusCreateIndexTaskWithPaths)
{
    OcrIndexDBus dbus;
    QVariantMap opts;
    opts["silent"] = true;
    bool result = dbus.CreateIndexTask({ tmp.path() + "/indexed-dir" }, opts);
    SUCCEED();
}

TEST_F(DBusTest, OcrIndexDBusUpdateIndexTaskWithPaths)
{
    OcrIndexDBus dbus;
    QVariantMap opts;
    bool result = dbus.UpdateIndexTask({ tmp.path() + "/indexed-dir" }, opts);
    SUCCEED();
}

TEST_F(DBusTest, OcrIndexDBusProcessFileChanges_OnlyDeleted)
{
    OcrIndexDBus dbus;
    bool result = dbus.ProcessFileChanges(
            {},
            {},
            { "/fake/deleted/img.png" });
    SUCCEED();
}

TEST_F(DBusTest, OcrIndexDBusProcessFileChanges_OnlyCreated)
{
    OcrIndexDBus dbus;
    bool result = dbus.ProcessFileChanges(
            { "/fake/created/img.png" },
            {},
            {});
    SUCCEED();
}

TEST_F(DBusTest, OcrIndexDBusProcessFileChanges_OnlyModified)
{
    OcrIndexDBus dbus;
    bool result = dbus.ProcessFileChanges(
            {},
            { "/fake/modified/img.png" },
            {});
    SUCCEED();
}

TEST_F(DBusTest, OcrIndexDBusProcessFileChanges_AllThree)
{
    OcrIndexDBus dbus;
    bool result = dbus.ProcessFileChanges(
            { "/fake/created.png" },
            { "/fake/modified.png" },
            { "/fake/deleted.png" });
    SUCCEED();
}

TEST_F(DBusTest, OcrIndexDBusProcessFileMoves_NonEmpty)
{
    OcrIndexDBus dbus;
    QHash<QString, QString> moves;
    moves["/old/img.png"] = "/new/img.png";
    bool result = dbus.ProcessFileMoves(moves);
    SUCCEED();
}

TEST_F(DBusTest, OcrIndexDBusIndexDatabaseExists_VersionMismatch)
{
    const QString indexDir = tmp.path() + "/ocr-index";
    writeStatusJson(indexDir, { { "version", 99999 } });

    OcrIndexDBus dbus;
    EXPECT_FALSE(dbus.IndexDatabaseExists());
}

TEST_F(DBusTest, OcrIndexDBusUpdateIndexTaskWithNonSilent)
{
    OcrIndexDBus dbus;
    QVariantMap opts;
    opts["silent"] = false;
    bool result = dbus.UpdateIndexTask({ tmp.path() + "/indexed-dir" }, opts);
    SUCCEED();
}
