// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// VaultFileIterator tests enumerate a real temp dir mapped in as the vault
// decrypto directory. fileInfo() asserts a worker thread, mirroring the
// production contract.

#include <gtest/gtest.h>
#include <QUrl>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QThread>
#include <thread>

#include "stubext.h"

#include "fileutils/vaultfileiterator.h"
#include "utils/vaulthelper.h"
#include "utils/pathmanager.h"

DPVAULT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

static QString gIterTemp;

static void installMappingStub(stub_ext::StubExt &stub)
{
    stub.set_lamda(&VaultHelper::vaultToLocalUrl, [](const QUrl &url) -> QUrl {
        return QUrl::fromLocalFile(gIterTemp + url.path());
    });
    stub.set_lamda(&PathManager::makeVaultLocalPath, [](const QString &, const QString &) -> QString {
        return gIterTemp;
    });
}

class VaultFileIteratorTest : public testing::Test
{
protected:
    void SetUp() override
    {
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());
        gIterTemp = tempDir->path();

        for (const char *name : { "alpha.txt", "beta.txt" }) {
            QFile f(QString("%1/%2").arg(tempDir->path(), name));
            ASSERT_TRUE(f.open(QIODevice::WriteOnly));
            f.write("x");
            f.close();
        }
        ASSERT_TRUE(QDir(tempDir->path()).mkdir("subdir"));

        installMappingStub(stub);
    }

    void TearDown() override
    {
        stub.clear();
        tempDir.reset();
        gIterTemp.clear();
    }

    QUrl vaultRoot() const
    {
        QUrl url;
        url.setScheme("dfmvault");
        url.setPath("/");
        return url;
    }

protected:
    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
};

TEST_F(VaultFileIteratorTest, Iterate_DirWithTwoFilesAndDir_VisitsAllEntries)
{
    VaultFileIterator it(vaultRoot(), {}, QDir::AllEntries | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);

    QStringList names;
    while (it.hasNext()) {
        QUrl url = it.next();
        names << url.fileName();
    }
    EXPECT_EQ(names.size(), 3);
    EXPECT_TRUE(names.contains("alpha.txt"));
    EXPECT_TRUE(names.contains("subdir"));
}

TEST_F(VaultFileIteratorTest, FileNameAndFileUrl_TrackLastNextResult)
{
    VaultFileIterator it(vaultRoot(), {}, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
    ASSERT_TRUE(it.hasNext());
    QUrl url = it.next();

    EXPECT_EQ(it.fileUrl(), url);
    EXPECT_EQ(it.fileName(), url.fileName());
}

TEST_F(VaultFileIteratorTest, Url_ReturnsVaultRoot)
{
    VaultFileIterator it(vaultRoot(), {}, QDir::Files, QDirIterator::NoIteratorFlags);
    EXPECT_EQ(it.url(), VaultHelper::instance()->rootUrl());
}

TEST_F(VaultFileIteratorTest, InitIterator_OnRealDir_Succeeds)
{
    VaultFileIterator it(vaultRoot(), {}, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
    EXPECT_TRUE(it.initIterator());
}

TEST_F(VaultFileIteratorTest, FileInfo_FromWorkerThread_ReturnsVaultInfo)
{
    VaultFileIterator *it = new VaultFileIterator(vaultRoot(), {},
                                                  QDir::Files | QDir::NoDotAndDotDot,
                                                  QDirIterator::NoIteratorFlags);
    ASSERT_TRUE(it->hasNext());
    it->next();

    FileInfoPointer info;
    std::thread worker([&]() {
        info = it->fileInfo();
    });
    worker.join();

    EXPECT_NE(info, nullptr);
    EXPECT_EQ(info->urlOf(FileInfo::FileUrlInfoType::kUrl).scheme(), QString("dfmvault"));
    delete it;
}

TEST_F(VaultFileIteratorTest, HasNext_EmptyDir_ReturnsFalseImmediately)
{
    ASSERT_TRUE(QDir(tempDir->path()).mkdir("empty"));
    QUrl url;
    url.setScheme("dfmvault");
    url.setPath("/empty");

    VaultFileIterator it(url, {}, QDir::AllEntries | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
    EXPECT_FALSE(it.hasNext());
}
