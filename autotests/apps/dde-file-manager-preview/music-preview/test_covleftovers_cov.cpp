// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage leftovers for the music-preview plugin: the CusMediaPlayer
// destructor (normally only reachable through the static singleton at
// process exit).
//
// Note: the theme-change lambda inside ToolBarFrame::initConnections() is
// deliberately NOT exercised here: broadcasting DGuiApplicationHelper::
// themeTypeChanged in this suite (dozens of live widgets + the media player
// backend) corrupts global state at process exit (QMutex destroying locked
// mutex -> segfault after tear-down), so that single lambda is left
// uncovered on purpose.

#include "cusmediaplayer.h"

#include <gtest/gtest.h>

#include <QApplication>
#include <QTemporaryDir>

using namespace plugin_filepreview;

class MusicLeftoversCovTest : public testing::Test
{
protected:
    void SetUp() override
    {
        dir.reset(new QTemporaryDir());
        ASSERT_TRUE(dir->isValid());
    }
    void TearDown() override
    {
        qApp->processEvents();
        qApp->sendPostedEvents(nullptr, QEvent::DeferredDelete);
        dir.reset();
    }

    std::unique_ptr<QTemporaryDir> dir;
};

TEST_F(MusicLeftoversCovTest, CusMediaPlayer_Destructor_QuitsIdleWorkerThread)
{
    // Arrange: the singleton (whose lifetime is the whole process) stays valid
    CusMediaPlayer *singleton = CusMediaPlayer::instance();
    ASSERT_NE(singleton, nullptr);

    {
        // Act: a second instance is created and destroyed on the stack; the
        // destructor quits/waits its (never used) worker thread
        CusMediaPlayer shortLived;
        EXPECT_NE(&shortLived, singleton);
    }

    // Assert: the process-wide singleton survived the scoped destruction
    EXPECT_EQ(CusMediaPlayer::instance(), singleton);
}
