// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_traversaldirthread.cpp
 * @brief Unit tests for TraversalDirThread (utils/traversaldirthread.cpp)
 */

#include <gtest/gtest.h>
#include <QUrl>
#include <QStringList>
#include <QDir>

#include <dfm-base/utils/traversaldirthread.h>

using namespace dfmbase;

TEST(TraversalDirThreadTest, ConstructAndDestruct)
{
    // Constructing on a non-existent path exercises the ctor; the thread is
    // never started, so the dtor runs without blocking.
    EXPECT_NO_FATAL_FAILURE({
        TraversalDirThread t(QUrl::fromLocalFile(QDir::homePath()));
    });
}

TEST(TraversalDirThreadTest, ConstructWithFiltersAndDestruct)
{
    EXPECT_NO_FATAL_FAILURE({
        TraversalDirThread t(QUrl::fromLocalFile(QDir::homePath()),
                             QStringList("*.cpp"),
                             QDir::Files | QDir::Dirs,
                             QDirIterator::NoIteratorFlags);
    });
}

TEST(TraversalDirThreadTest, StopBeforeStartSetsStopFlag)
{
    TraversalDirThread t(QUrl::fromLocalFile(QDir::homePath()));
    // Calling stop() on a thread that was never started sets stopFlag to true.
    t.stop();
    EXPECT_TRUE(t.stopFlag);
}

TEST(TraversalDirThreadTest, StopAndDeleteLaterBeforeStart)
{
    TraversalDirThread t(QUrl::fromLocalFile(QDir::homePath()));
    EXPECT_NO_FATAL_FAILURE({ t.stopAndDeleteLater(); });
}

TEST(TraversalDirThreadTest, StopFlagDefaultIsFalse)
{
    TraversalDirThread t(QUrl::fromLocalFile(QDir::homePath()));
    EXPECT_FALSE(t.stopFlag);
}