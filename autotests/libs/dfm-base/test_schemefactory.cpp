// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_schemefactory.cpp
 * @brief Unit tests for the non-template instance() / create() / scheme()
 *        functions defined in schemefactory.cpp
 *
 * The factory singletons (InfoFactory, WatcherFactory, DirIteratorFactory,
 * SortFilterFactory) are reached through their public static create() entry
 * points. With no concrete class registered for the "file" scheme, create()
 * deterministically returns null — exercising instance(), scheme(),
 * getFileInfoFromCache() and the SchemeFactory::create() guard paths without
 * any registered plugin or hardware.
 */

#include <gtest/gtest.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_global_defines.h>

#include <QUrl>
#include <QDir>

using namespace dfmbase;

TEST(SchemeFactoryTest, InfoFactoryCreateInvalidUrlReturnsNull)
{
    auto info = InfoFactory::create<FileInfo>(QUrl());
    EXPECT_EQ(info, nullptr);
}

TEST(SchemeFactoryTest, InfoFactoryCreateFileUrlAutoDoesNotCrash)
{
    // Default (Auto) branch calls InfoFactory::scheme(url) internally.
    // /tmp exists and is not a symlink nor remote, so scheme() returns "file".
    // Whether create() returns null depends on whether another test in the
    // same binary registered a "file" creator; the call itself exercises
    // scheme() + the Auto cache path regardless.
    QUrl fileUrl = QUrl::fromLocalFile(QDir::tempPath());
    auto info = InfoFactory::create<FileInfo>(fileUrl);
    (void)info;
    SUCCEED();
}

TEST(SchemeFactoryTest, InfoFactoryCreateFileUrlSyncCacheDoesNotCrash)
{
    // SyncAndCache branch routes through getFileInfoFromCache().
    QUrl fileUrl = QUrl::fromLocalFile(QDir::tempPath());
    auto info = InfoFactory::create<FileInfo>(fileUrl,
                                              Global::CreateFileInfoType::kCreateFileInfoSyncAndCache);
    (void)info;
    SUCCEED();
}

TEST(SchemeFactoryTest, WatcherFactoryCreateReturnsNullWithoutRegistration)
{
    QUrl fileUrl = QUrl::fromLocalFile(QDir::tempPath());
    auto watcher = WatcherFactory::create<AbstractFileWatcher>(fileUrl);
    EXPECT_EQ(watcher, nullptr);
}


