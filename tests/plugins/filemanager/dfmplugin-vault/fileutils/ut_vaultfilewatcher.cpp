// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "fileutils/vaultfilewatcher.h"

#include <gtest/gtest.h>

#include <QUrl>

DPVAULT_USE_NAMESPACE

TEST(UT_VaultFileWatcher, onFileDeleted)
{
    bool isOk { false };

    VaultFileWatcher watcher(QUrl("dfmvault:///UT_TEST"));
    QObject::connect(&watcher, &VaultFileWatcher::fileDeleted, [ &isOk ]{
        isOk = true;
    });
    watcher.onFileDeleted(QUrl("file:///UT_TEST"));

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileWatcher, onFileAttributeChanged)
{
    bool isOk { false };

    VaultFileWatcher watcher(QUrl("dfmvault:///UT_TEST"));
    QObject::connect(&watcher, &VaultFileWatcher::fileAttributeChanged, [ &isOk ]{
        isOk = true;
    });
    watcher.onFileAttributeChanged(QUrl("file:///UT_TEST"));

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileWatcher, onFileRename)
{
    bool isOk { false };

    VaultFileWatcher watcher(QUrl("dfmvault:///UT_TEST"));
    QObject::connect(&watcher, &VaultFileWatcher::fileRename, [ &isOk ]{
        isOk = true;
    });
    watcher.onFileRename(QUrl("dfmvault:///UT_TEST1"), QUrl("dfmvault:///UT_TEST2"));

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileWatcher, onSubfileCreated)
{
    bool isOk { false };

    VaultFileWatcher watcher(QUrl("dfmvault:///UT_TEST"));
    QObject::connect(&watcher, &VaultFileWatcher::subfileCreated, [ &isOk ]{
        isOk = true;
    });
    watcher.onSubfileCreated(QUrl("file:///UT_TEST"));

    EXPECT_TRUE(isOk);
}
