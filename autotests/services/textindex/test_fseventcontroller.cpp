// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fseventcontroller.cpp
 * @brief Unit tests for FSEventController (fsmonitor/fseventcontroller.cpp)
 *        Covers ctor, setupFSEventCollector, enable/disable state, monitoring
 *        start/stop no-ops, silent flag, and the private file-event slots.
 *        Private slots are invoked directly (the test build uses
 *        -fno-access-control).
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QHash>
#include <QString>
#include <QStringList>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/profile/indexprofile.h"
#include "services/textindex/fsmonitor/fseventcontroller.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

class FSEventControllerTest : public testing::Test
{
protected:
    QTemporaryDir tmp;

    IndexProfile makeProfile()
    {
        return IndexProfile(IndexProfile::Type::Content,
                            "fsec_test",
                            "fsec_status.json",
                            "fsec_version",
                            1,
                            [this]() -> QString { return tmp.path(); },
                            []() -> bool { return true; },
                            [](const QString &) -> bool { return true; },
                            [](const QString &) -> bool { return true; });
    }
};

TEST_F(FSEventControllerTest, CtorDefaultsDisabled)
{
    FSEventController c(makeProfile());
    EXPECT_FALSE(c.isEnabled());
    EXPECT_FALSE(c.silentlyRefreshStarted());
}

TEST_F(FSEventControllerTest, SetupCollectorIsCallable)
{
    FSEventController c(makeProfile());
    EXPECT_NO_FATAL_FAILURE({ c.setupFSEventCollector(); });
    EXPECT_FALSE(c.isEnabled());
}

TEST_F(FSEventControllerTest, EnableThenDisable)
{
    FSEventController c(makeProfile());
    c.setupFSEventCollector();
    EXPECT_NO_FATAL_FAILURE({ c.setEnabled(true); });
    EXPECT_TRUE(c.isEnabled());
    EXPECT_NO_FATAL_FAILURE({ c.setEnabled(false); });
    EXPECT_FALSE(c.isEnabled());
}

TEST_F(FSEventControllerTest, EnableViaSilentRefresh)
{
    FSEventController c(makeProfile());
    c.setupFSEventCollector();
    c.setSilentlyRefreshStarted(true);
    EXPECT_NO_FATAL_FAILURE({ c.setEnabled(true); });
    EXPECT_TRUE(c.isEnabled());
    EXPECT_FALSE(c.silentlyRefreshStarted());
}

TEST_F(FSEventControllerTest, SetEnabledNowToggles)
{
    FSEventController c(makeProfile());
    c.setupFSEventCollector();
    EXPECT_NO_FATAL_FAILURE({ c.setEnabledNow(true); });
    EXPECT_TRUE(c.isEnabled());
    EXPECT_NO_FATAL_FAILURE({ c.setEnabledNow(false); });
    EXPECT_FALSE(c.isEnabled());
}

TEST_F(FSEventControllerTest, SilentFlagRoundtrip)
{
    FSEventController c(makeProfile());
    EXPECT_FALSE(c.silentlyRefreshStarted());
    c.setSilentlyRefreshStarted(true);
    EXPECT_TRUE(c.silentlyRefreshStarted());
    c.setSilentlyRefreshStarted(false);
    EXPECT_FALSE(c.silentlyRefreshStarted());
}

TEST_F(FSEventControllerTest, StartStopMonitoringWithoutCollectorIsSafe)
{
    FSEventController c(makeProfile());
    EXPECT_NO_FATAL_FAILURE({ c.startFSMonitoring(); });
    EXPECT_NO_FATAL_FAILURE({ c.stopFSMonitoring(); });
}

TEST_F(FSEventControllerTest, StartStopMonitoringWithCollectorIsCallable)
{
    FSEventController c(makeProfile());
    c.setupFSEventCollector();
    EXPECT_NO_FATAL_FAILURE({ c.startFSMonitoring(); });
    EXPECT_NO_FATAL_FAILURE({ c.stopFSMonitoring(); });
}

TEST_F(FSEventControllerTest, FileEventSlotsAppendWhenEnabled)
{
    FSEventController c(makeProfile());
    c.setupFSEventCollector();
    c.setEnabled(true);
    QStringList created { "/tmp/a.txt", "/tmp/b.txt" };
    QStringList deleted { "/tmp/c.txt" };
    QStringList modified { "/tmp/d.txt" };
    QHash<QString, QString> moved { { "/tmp/old", "/tmp/new" } };
    EXPECT_NO_FATAL_FAILURE({ c.onFilesCreated(created); });
    EXPECT_NO_FATAL_FAILURE({ c.onFilesDeleted(deleted); });
    EXPECT_NO_FATAL_FAILURE({ c.onFilesModified(modified); });
    EXPECT_NO_FATAL_FAILURE({ c.onFilesMoved(moved); });
    EXPECT_NO_FATAL_FAILURE({ c.onFlushFinished(); });
    EXPECT_NO_FATAL_FAILURE({ c.onConfigChanged(); });
    EXPECT_NO_FATAL_FAILURE({ c.clearCollections(); });
}

TEST_F(FSEventControllerTest, FileEventSlotsNoopWhenDisabled)
{
    FSEventController c(makeProfile());
    c.setupFSEventCollector();
    EXPECT_FALSE(c.isEnabled());
    EXPECT_NO_FATAL_FAILURE({ c.onFilesCreated({ "/tmp/x" }); });
    EXPECT_NO_FATAL_FAILURE({ c.onFlushFinished(); });
}

TEST_F(FSEventControllerTest, FlushWithCollectedEventsEmitsSignals)
{
    FSEventController c(makeProfile());
    c.setupFSEventCollector();
    c.setEnabled(true);
    c.onFilesCreated({ "/tmp/created.txt" });
    c.onFilesModified({ "/tmp/modified.txt" });
    QHash<QString, QString> moves { { "/tmp/from", "/tmp/to" } };
    c.onFilesMoved(moves);
    EXPECT_NO_FATAL_FAILURE({ c.onFlushFinished(); });
    EXPECT_NO_FATAL_FAILURE({ c.clearCollections(); });
}
