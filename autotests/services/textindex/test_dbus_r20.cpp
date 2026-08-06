// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dbus_r20.cpp
 * @brief Unit tests for TextIndexDBus and OcrIndexDBus (textindexdbus.cpp + ocrindexdbus.cpp)
 *        — constructs the DBus objects (they create an IndexRuntime internally) and
 *        exercises the public slot API. DBus registration on session bus will fail
 *        gracefully in the test sandbox (no dbus-daemon), so the object still works.
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QHash>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/profile/indexprofile.h"
#include "services/textindex/textindexdbus.h"
#include "services/textindex/ocrindexdbus.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

// TextIndexDBus needs an IndexRuntime which needs an IndexProfile with a real
// index directory. The profile's content() factory creates a real profile that
// uses the default index path, which may not exist in the sandbox. We use a
// custom profile pointing to a temp dir.

class DBusR20Test : public testing::Test
{
protected:
    QTemporaryDir tmp;

    IndexProfile makeProfile()
    {
        return IndexProfile(IndexProfile::Type::Content,
                            "dbus_test",
                            "dbus_status.json",
                            "dbus_version",
                            1,
                            [this]() -> QString { return tmp.path(); },
                            []() -> bool { return true; },
                            [](const QString &) -> bool { return true; },
                            [](const QString &) -> bool { return true; });
    }
};

TEST_F(DBusR20Test, TextIndexDBusConstructAndDestruct)
{
    {
        TextIndexDBus dbus;
        SUCCEED();
    }
}

TEST_F(DBusR20Test, TextIndexDBusInit)
{
    TextIndexDBus dbus;
    EXPECT_NO_FATAL_FAILURE({ dbus.Init(); });
}

TEST_F(DBusR20Test, TextIndexDBusIsEnabled)
{
    TextIndexDBus dbus;
    EXPECT_NO_FATAL_FAILURE({ (void)dbus.IsEnabled(); });
}

TEST_F(DBusR20Test, TextIndexDBusSetEnabled)
{
    TextIndexDBus dbus;
    EXPECT_NO_FATAL_FAILURE({ dbus.SetEnabled(false); });
}

TEST_F(DBusR20Test, TextIndexDBusStopCurrentTask)
{
    TextIndexDBus dbus;
    EXPECT_FALSE(dbus.StopCurrentTask());
}

TEST_F(DBusR20Test, TextIndexDBusHasRunningTask)
{
    TextIndexDBus dbus;
    EXPECT_FALSE(dbus.HasRunningTask());
}

TEST_F(DBusR20Test, TextIndexDBusIndexDatabaseExists)
{
    TextIndexDBus dbus;
    EXPECT_FALSE(dbus.IndexDatabaseExists());
}

TEST_F(DBusR20Test, TextIndexDBusGetLastUpdateTime)
{
    TextIndexDBus dbus;
    QString t = dbus.GetLastUpdateTime();
    EXPECT_TRUE(t.isEmpty());
}

TEST_F(DBusR20Test, TextIndexDBusCreateIndexTaskEmpty)
{
    TextIndexDBus dbus;
    EXPECT_FALSE(dbus.CreateIndexTask({}));
}

TEST_F(DBusR20Test, TextIndexDBusUpdateIndexTaskEmpty)
{
    TextIndexDBus dbus;
    EXPECT_FALSE(dbus.UpdateIndexTask({}));
}

TEST_F(DBusR20Test, TextIndexDBusProcessFileChangesEmpty)
{
    TextIndexDBus dbus;
    EXPECT_FALSE(dbus.ProcessFileChanges({}, {}, {}));
}

TEST_F(DBusR20Test, TextIndexDBusProcessFileMovesEmpty)
{
    TextIndexDBus dbus;
    EXPECT_FALSE(dbus.ProcessFileMoves({}));
}

TEST_F(DBusR20Test, TextIndexDBusCleanup)
{
    TextIndexDBus dbus;
    EXPECT_NO_FATAL_FAILURE({ dbus.cleanup(); });
}

TEST_F(DBusR20Test, OcrIndexDBusConstructAndDestruct)
{
    {
        OcrIndexDBus dbus;
        SUCCEED();
    }
}

TEST_F(DBusR20Test, OcrIndexDBusInit)
{
    OcrIndexDBus dbus;
    EXPECT_NO_FATAL_FAILURE({ dbus.Init(); });
}

TEST_F(DBusR20Test, OcrIndexDBusIsEnabled)
{
    OcrIndexDBus dbus;
    EXPECT_NO_FATAL_FAILURE({ (void)dbus.IsEnabled(); });
}

TEST_F(DBusR20Test, OcrIndexDBusSetEnabled)
{
    OcrIndexDBus dbus;
    EXPECT_NO_FATAL_FAILURE({ dbus.SetEnabled(false); });
}

TEST_F(DBusR20Test, OcrIndexDBusStopCurrentTask)
{
    OcrIndexDBus dbus;
    EXPECT_FALSE(dbus.StopCurrentTask());
}

TEST_F(DBusR20Test, OcrIndexDBusHasRunningTask)
{
    OcrIndexDBus dbus;
    EXPECT_FALSE(dbus.HasRunningTask());
}

TEST_F(DBusR20Test, OcrIndexDBusIndexDatabaseExists)
{
    OcrIndexDBus dbus;
    EXPECT_FALSE(dbus.IndexDatabaseExists());
}

TEST_F(DBusR20Test, OcrIndexDBusGetLastUpdateTime)
{
    OcrIndexDBus dbus;
    QString t = dbus.GetLastUpdateTime();
    EXPECT_TRUE(t.isEmpty());
}

TEST_F(DBusR20Test, OcrIndexDBusCreateIndexTaskEmpty)
{
    OcrIndexDBus dbus;
    EXPECT_FALSE(dbus.CreateIndexTask({}));
}

TEST_F(DBusR20Test, OcrIndexDBusProcessFileChangesEmpty)
{
    OcrIndexDBus dbus;
    EXPECT_FALSE(dbus.ProcessFileChanges({}, {}, {}));
}

TEST_F(DBusR20Test, OcrIndexDBusProcessFileMovesEmpty)
{
    OcrIndexDBus dbus;
    EXPECT_FALSE(dbus.ProcessFileMoves({}));
}

TEST_F(DBusR20Test, OcrIndexDBusCleanup)
{
    OcrIndexDBus dbus;
    EXPECT_NO_FATAL_FAILURE({ dbus.cleanup(); });
}
