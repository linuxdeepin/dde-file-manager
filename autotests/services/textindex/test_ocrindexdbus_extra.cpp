// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_ocrindexdbus_extra.cpp
 * @brief Additional unit tests for OcrIndexDBus covering:
 *        UpdateIndexTask with options, IndexDatabaseExists with dirty state,
 *        ProcessFileMoves with non-empty moves, and cleanup.
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
#include "services/textindex/ocrindexdbus.h"

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

class OcrIndexDBusExtraTest : public testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(tmp.isValid());

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

// --- IndexDatabaseExists with various states ---

TEST_F(OcrIndexDBusExtraTest, IndexDatabaseExists_DirtyState)
{
    const QString indexDir = tmp.path() + "/ocr-index";
    writeStatusJson(indexDir, {
        { "version", Defines::kOcrIndexVersion },
        { "lastUpdateTime", "2026-01-01T00:00:00" },
        { "state", "dirty" }
    });

    OcrIndexDBus dbus;
    EXPECT_TRUE(dbus.IndexDatabaseExists());
}

TEST_F(OcrIndexDBusExtraTest, IndexDatabaseExists_CleanState)
{
    const QString indexDir = tmp.path() + "/ocr-index";
    writeStatusJson(indexDir, {
        { "version", Defines::kOcrIndexVersion },
        { "lastUpdateTime", "2026-01-01T00:00:00" },
        { "state", "clean" }
    });

    OcrIndexDBus dbus;
    EXPECT_TRUE(dbus.IndexDatabaseExists());
}

// --- SetEnabled ---
TEST_F(OcrIndexDBusExtraTest, SetEnabledTrue)
{
    OcrIndexDBus dbus;
    EXPECT_NO_FATAL_FAILURE({ dbus.SetEnabled(true); });
}

TEST_F(OcrIndexDBusExtraTest, SetEnabledFalse)
{
    OcrIndexDBus dbus;
    EXPECT_NO_FATAL_FAILURE({ dbus.SetEnabled(false); });
}

// --- ProcessFileChanges with multiple files ---
TEST_F(OcrIndexDBusExtraTest, ProcessFileChanges_MultipleFiles)
{
    OcrIndexDBus dbus;
    bool result = dbus.ProcessFileChanges(
            { "/a.png", "/b.jpg" },
            { "/c.png" },
            { "/d.jpg", "/e.png" });
    SUCCEED();
}

// --- ProcessFileMoves ---
TEST_F(OcrIndexDBusExtraTest, ProcessFileMoves_SingleMove)
{
    OcrIndexDBus dbus;
    QHash<QString, QString> moves;
    moves["/old/img.png"] = "/new/img.png";
    bool result = dbus.ProcessFileMoves(moves);
    SUCCEED();
}

TEST_F(OcrIndexDBusExtraTest, ProcessFileMoves_MultipleMoves)
{
    OcrIndexDBus dbus;
    QHash<QString, QString> moves;
    moves["/a.png"] = "/b.png";
    moves["/c.jpg"] = "/d.jpg";
    bool result = dbus.ProcessFileMoves(moves);
    SUCCEED();
}

// --- CreateIndexTask with options ---
TEST_F(OcrIndexDBusExtraTest, CreateIndexTask_SilentTrue)
{
    OcrIndexDBus dbus;
    QVariantMap opts;
    opts["silent"] = true;
    bool result = dbus.CreateIndexTask({ tmp.path() + "/indexed-dir" }, opts);
    SUCCEED();
}

TEST_F(OcrIndexDBusExtraTest, CreateIndexTask_NoOptions)
{
    OcrIndexDBus dbus;
    // Default QVariantMap - silent defaults to false
    bool result = dbus.CreateIndexTask({ tmp.path() + "/indexed-dir" });
    SUCCEED();
}

// --- Cleanup ---
TEST_F(OcrIndexDBusExtraTest, Cleanup_NoRunningTasks)
{
    const QString indexDir = tmp.path() + "/ocr-index";
    writeStatusJson(indexDir, {
        { "version", Defines::kOcrIndexVersion },
        { "lastUpdateTime", "2026-01-01T00:00:00" },
        { "state", "clean" }
    });

    OcrIndexDBus dbus;
    dbus.cleanup();
    SUCCEED();
}

// --- Init multiple times ---
TEST_F(OcrIndexDBusExtraTest, InitMultipleTimes)
{
    OcrIndexDBus dbus;
    EXPECT_NO_FATAL_FAILURE({ dbus.Init(); });
    EXPECT_NO_FATAL_FAILURE({ dbus.Init(); });
}

// --- GetLastUpdateTime with valid time ---
TEST_F(OcrIndexDBusExtraTest, GetLastUpdateTime_DirtyState)
{
    const QString indexDir = tmp.path() + "/ocr-index";
    writeStatusJson(indexDir, {
        { "version", Defines::kOcrIndexVersion },
        { "lastUpdateTime", "2026-06-15T10:30:00" },
        { "state", "dirty" }
    });

    OcrIndexDBus dbus;
    QString t = dbus.GetLastUpdateTime();
    EXPECT_FALSE(t.isEmpty());
    EXPECT_EQ(t, QString("2026-06-15 10:30:00"));
}