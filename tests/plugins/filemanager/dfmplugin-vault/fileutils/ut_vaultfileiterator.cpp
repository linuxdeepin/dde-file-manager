// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "fileutils/vaultfileiterator.h"
#include "utils/vaulthelper.h"

#include <gtest/gtest.h>

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>

DPVAULT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

TEST(UT_VaultFileIterator, next_one)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&QDirIterator::hasNext, [ &isOk ]{
        isOk = true;
        return false;
    });

    VaultFileIterator iterator(QUrl("dfmvault:///"), QStringList(), QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
    QUrl url = iterator.next();

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileIterator, hasNext)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&QDirIterator::hasNext, []{
        return false;
    });

    VaultFileIterator iterator(QUrl("dfmvault:///"), QStringList(), QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
    bool result = iterator.hasNext();

    EXPECT_FALSE(result);
}

TEST(UT_VaultFileIterator, fileName)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&QDirIterator::fileName, []{
        return "UT_TEST";
    });

    VaultFileIterator iterator(QUrl("dfmvault:///"), QStringList(), QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
    QString name = iterator.fileName();

    EXPECT_TRUE(name == "UT_TEST");
}

TEST(UT_VaultFileIterator, fileUrl)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::pathToVaultVirtualUrl, []{
        return QUrl("dfmvault:///");
    });

    VaultFileIterator iterator(QUrl("dfmvault:///"), QStringList(), QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
    QUrl url = iterator.fileUrl();

    EXPECT_TRUE(url == QUrl("dfmvault:///"));
}

TEST(UT_VaultFileIterator, fileInfo)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&InfoFactory::create<FileInfo>, [ &isOk ]{
        isOk = true;
        return QSharedPointer<SyncFileInfo>(new SyncFileInfo(QUrl("file:///home/UT_TEST")));
    });

    VaultFileIterator iterator(QUrl("dfmvault:///"), QStringList(), QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
    iterator.fileInfo();

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileIterator, url)
{
    VaultFileIterator iterator(QUrl("dfmvault:///"), QStringList(), QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
    QUrl url = iterator.url();

    EXPECT_TRUE(url == QUrl("dfmvault:///"));
}
