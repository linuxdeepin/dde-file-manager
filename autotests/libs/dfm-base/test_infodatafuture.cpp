// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_infodatafuture.cpp
 * @brief Unit tests for InfoDataFuture (file/local/private/infodatafuture.cpp)
 *        — ctor with null DFileFuture (sets up connect), mediaInfo default,
 *        isFinished default, D0/D2 destructor, infoMedia slot.
 *        Private ctor invoked via -fno-access-control.
 */

#include <gtest/gtest.h>
#include <QUrl>
#include <QVariant>

#include <dfm-base/file/local/private/infodatafuture.h>

using namespace dfmbase;

TEST(InfoDataFutureTest, CtorWithNullFuture)
{
    InfoDataFuture f(nullptr, nullptr);
    // ctor sets up connect on null future — no crash
    SUCCEED();
}

TEST(InfoDataFutureTest, MediaInfoDefaultEmpty)
{
    InfoDataFuture f(nullptr, nullptr);
    auto info = f.mediaInfo();
    EXPECT_TRUE(info.isEmpty());
}

TEST(InfoDataFutureTest, IsFinishedDefaultFalse)
{
    InfoDataFuture f(nullptr, nullptr);
    EXPECT_FALSE(f.isFinished());
}

TEST(InfoDataFutureTest, D0DestructorPath)
{
    auto *ptr = new InfoDataFuture(nullptr, nullptr);
    EXPECT_NO_FATAL_FAILURE({ delete ptr; });
}

TEST(InfoDataFutureTest, InfoMediaSlotSetsFinished)
{
    InfoDataFuture f(nullptr, nullptr);
    QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> map;
    f.infoMedia(QUrl("file:///tmp/test.png"), map);
    EXPECT_TRUE(f.isFinished());
}