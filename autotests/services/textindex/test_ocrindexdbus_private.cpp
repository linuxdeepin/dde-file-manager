// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_ocrindexdbus_private.cpp
 * @brief Tests for OcrIndexDBusPrivate methods accessed via d (friend access).
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
#include "services/textindex/ocrindexdbus.h"
#include "services/textindex/private/ocrindexdbus_p.h"

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

class OcrIndexDBusPrivateTest : public testing::Test
{
protected:
    QTemporaryDir tmp;
    OcrIndexDBus *dbus { nullptr };
    OcrIndexDBusPrivate *d { nullptr };

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

        dbus = new OcrIndexDBus();
        d = dbus->d.data();
    }

    void TearDown() override
    {
        delete dbus;
    }

    stub_ext::StubExt stub;
};

// ---- initializeSupportedExtensions ----
TEST_F(OcrIndexDBusPrivateTest, InitializeSupportedExtensions)
{
    EXPECT_NO_FATAL_FAILURE({ d->initializeSupportedExtensions(); });
    EXPECT_NO_FATAL_FAILURE({ (void)d->m_currentSupportedExtensions.size(); });
}

// ---- canSilentlyRefreshIndex ----
TEST_F(OcrIndexDBusPrivateTest, CanSilentlyRefreshIndex_NoCurrentTask)
{
    EXPECT_TRUE(d->canSilentlyRefreshIndex("/any/path"));
}

// ---- handleMonitoring ----
TEST_F(OcrIndexDBusPrivateTest, HandleMonitoring_Stop)
{
    EXPECT_NO_FATAL_FAILURE({ d->handleMonitoring(false); });
}

TEST_F(OcrIndexDBusPrivateTest, HandleMonitoring_Start)
{
    EXPECT_NO_FATAL_FAILURE({ d->handleMonitoring(true); });
}

// ---- handleSlientStart ----
TEST_F(OcrIndexDBusPrivateTest, HandleSlientStart)
{
    EXPECT_NO_FATAL_FAILURE({ d->handleSlientStart(); });
}

// ---- handleConfigChanged ----
TEST_F(OcrIndexDBusPrivateTest, HandleConfigChanged_NoIndexDb)
{
    EXPECT_NO_FATAL_FAILURE({ d->handleConfigChanged(); });
}

TEST_F(OcrIndexDBusPrivateTest, HandleConfigChanged_WithIndexDb)
{
    const QString indexDir = tmp.path() + "/ocr-index";
    writeStatusJson(indexDir, {
        { "version", Defines::kOcrIndexVersion },
        { "lastUpdateTime", "2026-01-01T00:00:00" },
        { "state", "clean" }
    });

    EXPECT_NO_FATAL_FAILURE({ d->handleConfigChanged(); });
}

// ---- cleanup ----
TEST_F(OcrIndexDBusPrivateTest, Cleanup_NoRunningTasks)
{
    const QString indexDir = tmp.path() + "/ocr-index";
    writeStatusJson(indexDir, {
        { "version", Defines::kOcrIndexVersion },
        { "lastUpdateTime", "2026-01-01T00:00:00" },
        { "state", "clean" }
    });

    EXPECT_NO_FATAL_FAILURE({ dbus->cleanup(); });
}

// ---- stopCurrentTask when no task running ----
TEST_F(OcrIndexDBusPrivateTest, StopCurrentTask_NoTask)
{
    EXPECT_FALSE(dbus->StopCurrentTask());
}

// ---- HasRunningTask ----
TEST_F(OcrIndexDBusPrivateTest, HasRunningTask_Default)
{
    EXPECT_FALSE(dbus->HasRunningTask());
}

// ---- GetLastUpdateTime ----
TEST_F(OcrIndexDBusPrivateTest, GetLastUpdateTime)
{
    QString time = dbus->GetLastUpdateTime();
    SUCCEED();
}

// ---- IndexDatabaseExists with incompatible version ----
TEST_F(OcrIndexDBusPrivateTest, IndexDatabaseExists_IncompatibleVersion)
{
    const QString indexDir = tmp.path() + "/ocr-index";
    writeStatusJson(indexDir, {
        { "version", "wrong_version" },
        { "lastUpdateTime", "2026-01-01T00:00:00" },
        { "state", "clean" }
    });
    EXPECT_FALSE(dbus->IndexDatabaseExists());
}

TEST_F(OcrIndexDBusPrivateTest, IndexDatabaseExists_EmptyUpdateTime)
{
    const QString indexDir = tmp.path() + "/ocr-index";
    writeStatusJson(indexDir, {
        { "version", Defines::kOcrIndexVersion },
        { "lastUpdateTime", "" },
        { "state", "clean" }
    });
    EXPECT_FALSE(dbus->IndexDatabaseExists());
}

// ---- IsEnabled / SetEnabled ----
TEST_F(OcrIndexDBusPrivateTest, IsEnabled_Default)
{
    EXPECT_NO_FATAL_FAILURE({ (void)dbus->IsEnabled(); });
}

TEST_F(OcrIndexDBusPrivateTest, SetEnabled_False)
{
    EXPECT_NO_FATAL_FAILURE({ dbus->SetEnabled(false); });
}

// ---- ProcessFileMoves empty ----
TEST_F(OcrIndexDBusPrivateTest, ProcessFileMoves_Empty)
{
    EXPECT_FALSE(dbus->ProcessFileMoves({}));
}

// ---- ProcessFileChanges all empty ----
TEST_F(OcrIndexDBusPrivateTest, ProcessFileChanges_AllEmpty)
{
    EXPECT_FALSE(dbus->ProcessFileChanges({}, {}, {}));
}

// ---- Destructor ----
TEST_F(OcrIndexDBusPrivateTest, Destructor)
{
    auto *d2 = new OcrIndexDBus();
    EXPECT_NO_FATAL_FAILURE({ delete d2; });
}

// ---- Init multiple times ----
TEST_F(OcrIndexDBusPrivateTest, InitMultipleTimes)
{
    EXPECT_NO_FATAL_FAILURE({ dbus->Init(); });
    EXPECT_NO_FATAL_FAILURE({ dbus->Init(); });
}
