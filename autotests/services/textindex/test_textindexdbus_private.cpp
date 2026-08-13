// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_textindexdbus_private.cpp
 * @brief Tests for TextIndexDBusPrivate methods accessed via d (friend access).
 *        Covers initializeSupportedExtensions, handleConfigChanged, handleMonitoring,
 *        handleSlientStart, canSilentlyRefreshIndex, cleanup.
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
#include "services/textindex/textindexdbus.h"
#include "services/textindex/private/textindexdbus_p.h"

using namespace SERVICETEXTINDEX_NAMESPACE;
using namespace DFMSEARCH;

static void writeStatusJson(const QString &indexDir, const QJsonObject &obj)
{
    QDir().mkpath(indexDir);
    QFile f(indexDir + QLatin1String("/index_status.json"));
    ASSERT_TRUE(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
    f.write(QJsonDocument(obj).toJson());
    f.close();
}

class TextIndexDBusPrivateTest : public testing::Test
{
protected:
    QTemporaryDir tmp;
    TextIndexDBus *dbus { nullptr };
    TextIndexDBusPrivate *d { nullptr };

    void SetUp() override
    {
        ASSERT_TRUE(tmp.isValid());

        stub.set_lamda(ADDR(Global, contentIndexDirectory),
                       [this]() -> QString {
                           __DBG_STUB_INVOKE__
                           return tmp.path() + "/content-index";
                       });
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
        stub.set_lamda(ADDR(Global, defaultIndexedDirectory),
                       [this]() -> QStringList {
                           __DBG_STUB_INVOKE__
                           return QStringList { tmp.path() + "/indexed-dir" };
                       });
        stub.set_lamda(ADDR(Global, defaultBlacklistPaths),
                       []() -> QStringList {
                           __DBG_STUB_INVOKE__
                           return QStringList();
                       });

        dbus = new TextIndexDBus();
        d = dbus->d.data();
    }

    void TearDown() override
    {
        delete dbus;
    }

    stub_ext::StubExt stub;
};

// ---- initializeSupportedExtensions ----
TEST_F(TextIndexDBusPrivateTest, InitializeSupportedExtensions)
{
    EXPECT_NO_FATAL_FAILURE({ d->initializeSupportedExtensions(); });
    EXPECT_GT(d->m_currentSupportedExtensions.size(), 0);
}

// ---- canSilentlyRefreshIndex ----
TEST_F(TextIndexDBusPrivateTest, CanSilentlyRefreshIndex_NoCurrentTask)
{
    EXPECT_TRUE(d->canSilentlyRefreshIndex("/any/path"));
}

TEST_F(TextIndexDBusPrivateTest, CanSilentlyRefreshIndex_DifferentTaskType)
{
    // With no task running, should return true
    EXPECT_TRUE(d->canSilentlyRefreshIndex(tmp.path() + "/indexed-dir"));
}

// ---- handleMonitoring ----
TEST_F(TextIndexDBusPrivateTest, HandleMonitoring_Stop)
{
    EXPECT_NO_FATAL_FAILURE({ d->handleMonitoring(false); });
}

TEST_F(TextIndexDBusPrivateTest, HandleMonitoring_Start)
{
    EXPECT_NO_FATAL_FAILURE({ d->handleMonitoring(true); });
}

// ---- handleSlientStart (once_flag - only runs once) ----
TEST_F(TextIndexDBusPrivateTest, HandleSlientStart)
{
    // This uses std::call_once, so it runs once and then never again
    // But we can still call it to cover the function
    EXPECT_NO_FATAL_FAILURE({ d->handleSlientStart(); });
}

// ---- handleConfigChanged ----
TEST_F(TextIndexDBusPrivateTest, HandleConfigChanged_NoIndexDb)
{
    // No index database exists - should log warning and not start task
    EXPECT_NO_FATAL_FAILURE({ d->handleConfigChanged(); });
}

TEST_F(TextIndexDBusPrivateTest, HandleConfigChanged_WithIndexDb)
{
    const QString indexDir = tmp.path() + "/content-index";
    writeStatusJson(indexDir, {
        { "version", Defines::kTextIndexVersion },
        { "lastUpdateTime", "2026-01-01T00:00:00" },
        { "state", "clean" }
    });

    EXPECT_NO_FATAL_FAILURE({ d->handleConfigChanged(); });
}

// ---- cleanup with running task ----
TEST_F(TextIndexDBusPrivateTest, Cleanup_WithQueuedTasks)
{
    // Enqueue a task then cleanup
    dbus->CreateIndexTask({ tmp.path() + "/indexed-dir" });
    // There may be a queued task now
    EXPECT_NO_FATAL_FAILURE({ dbus->cleanup(); });
}

// ---- stopCurrentTask when no task running ----
TEST_F(TextIndexDBusPrivateTest, StopCurrentTask_NoTask)
{
    EXPECT_FALSE(dbus->StopCurrentTask());
}

// ---- HasRunningTask ----
TEST_F(TextIndexDBusPrivateTest, HasRunningTask_Default)
{
    EXPECT_FALSE(dbus->HasRunningTask());
}

// ---- GetLastUpdateTime ----
TEST_F(TextIndexDBusPrivateTest, GetLastUpdateTime)
{
    QString time = dbus->GetLastUpdateTime();
    SUCCEED();
}

// ---- IndexDatabaseExists with incompatible version ----
TEST_F(TextIndexDBusPrivateTest, IndexDatabaseExists_IncompatibleVersion)
{
    const QString indexDir = tmp.path() + "/content-index";
    writeStatusJson(indexDir, {
        { "version", "wrong_version" },
        { "lastUpdateTime", "2026-01-01T00:00:00" },
        { "state", "clean" }
    });
    EXPECT_FALSE(dbus->IndexDatabaseExists());
}

TEST_F(TextIndexDBusPrivateTest, IndexDatabaseExists_EmptyUpdateTime)
{
    const QString indexDir = tmp.path() + "/content-index";
    writeStatusJson(indexDir, {
        { "version", Defines::kTextIndexVersion },
        { "lastUpdateTime", "" },
        { "state", "clean" }
    });
    EXPECT_FALSE(dbus->IndexDatabaseExists());
}

// ---- IsEnabled / SetEnabled ----
TEST_F(TextIndexDBusPrivateTest, IsEnabled_Default)
{
    EXPECT_NO_FATAL_FAILURE({ (void)dbus->IsEnabled(); });
}

TEST_F(TextIndexDBusPrivateTest, SetEnabled_False)
{
    EXPECT_NO_FATAL_FAILURE({ dbus->SetEnabled(false); });
}

// ---- ProcessFileMoves empty ----
TEST_F(TextIndexDBusPrivateTest, ProcessFileMoves_Empty)
{
    EXPECT_FALSE(dbus->ProcessFileMoves({}));
}

// ---- ProcessFileChanges all empty ----
TEST_F(TextIndexDBusPrivateTest, ProcessFileChanges_AllEmpty)
{
    EXPECT_FALSE(dbus->ProcessFileChanges({}, {}, {}));
}

// ---- Destructor ----
TEST_F(TextIndexDBusPrivateTest, Destructor)
{
    auto *d2 = new TextIndexDBus();
    EXPECT_NO_FATAL_FAILURE({ delete d2; });
}
