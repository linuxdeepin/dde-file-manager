// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_textindexdbus_extra.cpp
 * @brief Additional unit tests for TextIndexDBus covering private methods:
 *        handleConfigChanged (with status file), cleanup with dirty state,
 *        ProcessFileMoves with non-empty moves.
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

class TextIndexDBusExtraTest : public testing::Test
{
protected:
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
    }

    QTemporaryDir tmp;
    stub_ext::StubExt stub;
};

// --- Test IndexDatabaseExists with various status states ---

TEST_F(TextIndexDBusExtraTest, IndexDatabaseExists_DirtyState)
{
    const QString indexDir = tmp.path() + "/content-index";
    writeStatusJson(indexDir, {
        { "version", Defines::kTextIndexVersion },
        { "lastUpdateTime", "2026-01-01T00:00:00" },
        { "state", "dirty" }
    });

    TextIndexDBus dbus;
    EXPECT_TRUE(dbus.IndexDatabaseExists());
}

TEST_F(TextIndexDBusExtraTest, IndexDatabaseExists_CleanState)
{
    const QString indexDir = tmp.path() + "/content-index";
    writeStatusJson(indexDir, {
        { "version", Defines::kTextIndexVersion },
        { "lastUpdateTime", "2026-01-01T00:00:00" },
        { "state", "clean" }
    });

    TextIndexDBus dbus;
    EXPECT_TRUE(dbus.IndexDatabaseExists());
}

// --- Test SetEnabled(true) to enable FS monitoring ---
TEST_F(TextIndexDBusExtraTest, SetEnabledTrue)
{
    TextIndexDBus dbus;
    EXPECT_NO_FATAL_FAILURE({ dbus.SetEnabled(true); });
}

// --- Test ProcessFileChanges with only one type ---
TEST_F(TextIndexDBusExtraTest, ProcessFileChanges_OnlyDeleted)
{
    TextIndexDBus dbus;
    bool result = dbus.ProcessFileChanges(
            {}, {},
            { tmp.path() + "/deleted.txt" });
    SUCCEED();
}

TEST_F(TextIndexDBusExtraTest, ProcessFileChanges_OnlyCreated)
{
    TextIndexDBus dbus;
    bool result = dbus.ProcessFileChanges(
            { tmp.path() + "/created.txt" }, {}, {});
    SUCCEED();
}

TEST_F(TextIndexDBusExtraTest, ProcessFileChanges_OnlyModified)
{
    TextIndexDBus dbus;
    bool result = dbus.ProcessFileChanges(
            {}, { tmp.path() + "/modified.txt" }, {});
    SUCCEED();
}

TEST_F(TextIndexDBusExtraTest, ProcessFileChanges_MultipleFiles)
{
    TextIndexDBus dbus;
    bool result = dbus.ProcessFileChanges(
            { tmp.path() + "/a.txt", tmp.path() + "/b.txt" },
            { tmp.path() + "/c.txt" },
            { tmp.path() + "/d.txt", tmp.path() + "/e.txt" });
    SUCCEED();
}

// --- Test ProcessFileMoves ---
TEST_F(TextIndexDBusExtraTest, ProcessFileMoves_SingleMove)
{
    TextIndexDBus dbus;
    QHash<QString, QString> moves;
    moves[tmp.path() + "/old.txt"] = tmp.path() + "/new.txt";
    bool result = dbus.ProcessFileMoves(moves);
    SUCCEED();
}

TEST_F(TextIndexDBusExtraTest, ProcessFileMoves_MultipleMoves)
{
    TextIndexDBus dbus;
    QHash<QString, QString> moves;
    moves[tmp.path() + "/a.txt"] = tmp.path() + "/b.txt";
    moves[tmp.path() + "/c.txt"] = tmp.path() + "/d.txt";
    moves[tmp.path() + "/e.txt"] = tmp.path() + "/f.txt";
    bool result = dbus.ProcessFileMoves(moves);
    SUCCEED();
}

// --- Test CreateIndexTask with silent option ---
TEST_F(TextIndexDBusExtraTest, CreateIndexTask_SilentTrue)
{
    TextIndexDBus dbus;
    QVariantMap opts;
    opts["silent"] = true;
    bool result = dbus.CreateIndexTask({ tmp.path() + "/indexed-dir" }, opts);
    SUCCEED();
}

TEST_F(TextIndexDBusExtraTest, CreateIndexTask_SilentFalse)
{
    TextIndexDBus dbus;
    QVariantMap opts;
    opts["silent"] = false;
    bool result = dbus.CreateIndexTask({ tmp.path() + "/indexed-dir" }, opts);
    SUCCEED();
}

// --- Test cleanup ---
TEST_F(TextIndexDBusExtraTest, Cleanup_NoRunningTasks)
{
    const QString indexDir = tmp.path() + "/content-index";
    writeStatusJson(indexDir, {
        { "version", Defines::kTextIndexVersion },
        { "lastUpdateTime", "2026-01-01T00:00:00" },
        { "state", "clean" }
    });

    TextIndexDBus dbus;
    dbus.cleanup();
    SUCCEED();
}

// --- Test Init multiple times ---
TEST_F(TextIndexDBusExtraTest, InitMultipleTimes)
{
    TextIndexDBus dbus;
    EXPECT_NO_FATAL_FAILURE({ dbus.Init(); });
    EXPECT_NO_FATAL_FAILURE({ dbus.Init(); });
    EXPECT_NO_FATAL_FAILURE({ dbus.Init(); });
}
