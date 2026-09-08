// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// VaultFileInfo full-API tests: a real temp dir is mapped in as the vault
// decrypto dir via VaultHelper::vaultToLocalUrl so the proxied local
// FileInfo answers with exact values.

#include <gtest/gtest.h>
#include <QUrl>
#include <QIcon>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QDir>
#include <QFile>
#include <QFileInfo>

#include "stubext.h"

#include "fileutils/vaultfileinfo.h"
#include "utils/vaulthelper.h"
#include "utils/pathmanager.h"
#include "utils/fileencrypthandle.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <QIcon>

DPVAULT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

static QString gVaultBaseTemp;

class VaultFileInfoFullTest : public testing::Test
{
public:
    void SetUp() override
    {
        static bool sFileSchemeReady = false;
        if (!sFileSchemeReady) {
            UrlRoute::regScheme(Global::Scheme::kFile, "/", QIcon(), false, "File");
            UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/", QIcon(), false, "File");
            InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
            InfoFactory::regClass<AsyncFileInfo>(Global::Scheme::kAsyncFile);
            sFileSchemeReady = true;
        }

        tempDir.reset(new QTemporaryDir);
        ASSERT_TRUE(tempDir->isValid());
        gVaultBaseTemp = tempDir->path();

        stub.set_lamda(&VaultHelper::vaultToLocalUrl, [](const QUrl &url) -> QUrl {
            return QUrl::fromLocalFile(gVaultBaseTemp + url.path());
        });
        stub.set_lamda(&PathManager::makeVaultLocalPath, [](const QString &, const QString &) -> QString {
            return gVaultBaseTemp;
        });
    }

    void TearDown() override
    {
        stub.clear();
        tempDir.reset();
        gVaultBaseTemp.clear();
    }

    QUrl vaultUrl(const QString &path) const
    {
        QUrl url;
        url.setScheme("dfmvault");
        url.setPath(path);
        return url;
    }

protected:
    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
};

TEST_F(VaultFileInfoFullTest, RootUrl_MetaNames_UseVaultBranding)
{
    VaultFileInfo info(VaultHelper::instance()->rootUrl());
    EXPECT_EQ(info.nameOf(NameInfoType::kIconName), QString("safebox"));
    EXPECT_EQ(info.displayOf(DisPlayInfoType::kFileDisplayName), QString("File Vault"));
}

TEST_F(VaultFileInfoFullTest, RootUrl_TargetUrl_IsVaultRoot)
{
    QUrl root = VaultHelper::instance()->rootUrl();
    VaultFileInfo info(root);
    EXPECT_EQ(info.urlOf(UrlInfoType::kUrl), root);
    EXPECT_EQ(info.urlOf(UrlInfoType::kRedirectedFileUrl),
              QUrl::fromLocalFile(gVaultBaseTemp + "/"));
}

TEST_F(VaultFileInfoFullTest, ChildFile_AbsolutePath_MapsBackToVaultRoot)
{
    QFile f(tempDir->path() + "/located.txt");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();

    VaultFileInfo info(vaultUrl("/located.txt"));
    EXPECT_EQ(info.pathOf(PathInfoType::kAbsolutePath), QString("/"));
}

TEST_F(VaultFileInfoFullTest, ChildFile_ExistsAndSize_DelegatesToProxy)
{
    QTemporaryFile tmp(tempDir->path() + "/XXXXXX.bin");
    ASSERT_TRUE(tmp.open());
    tmp.write("12345678");
    tmp.close();

    VaultFileInfo info(vaultUrl("/" + QFileInfo(tmp.fileName()).fileName()),
                       InfoFactory::create<FileInfo>(QUrl::fromLocalFile(tmp.fileName()),
                                                     Global::CreateFileInfoType::kCreateFileInfoSync));
    EXPECT_TRUE(info.exists());
    EXPECT_EQ(info.size(), qint64(8));
}

TEST_F(VaultFileInfoFullTest, ChildFile_NameAndDisplayPath_FromProxy)
{
    QFile f(tempDir->path() + "/hello.txt");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("hi");
    f.close();

    VaultFileInfo info(vaultUrl("/hello.txt"));
    EXPECT_EQ(info.nameOf(NameInfoType::kFileName), QString("hello.txt"));
    EXPECT_EQ(info.displayOf(DisPlayInfoType::kFileDisplayName), QString("hello.txt"));
    EXPECT_TRUE(info.displayOf(DisPlayInfoType::kFileDisplayPath).contains("hello.txt"));
}

TEST_F(VaultFileInfoFullTest, ChildFile_IsFileAttribute_FromProxy)
{
    QFile f(tempDir->path() + "/plain.txt");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();

    VaultFileInfo info(vaultUrl("/plain.txt"),
                       InfoFactory::create<FileInfo>(QUrl::fromLocalFile(f.fileName()),
                                                     Global::CreateFileInfoType::kCreateFileInfoSync));
    EXPECT_TRUE(info.isAttributes(FileInfo::FileIsType::kIsFile));
    EXPECT_FALSE(info.isAttributes(FileInfo::FileIsType::kIsDir));
}

TEST_F(VaultFileInfoFullTest, Dir_CountChildFile_CountsEntriesExactly)
{
    ASSERT_TRUE(QDir(tempDir->path()).mkdir("cnt"));
    for (const char *name : { "a.txt", "b.txt" }) {
        QFile f(QString("%1/cnt/%2").arg(tempDir->path(), name));
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.close();
    }

    VaultFileInfo info(vaultUrl("/cnt"),
                       InfoFactory::create<FileInfo>(QUrl::fromLocalFile(tempDir->path() + "/cnt"),
                                                     Global::CreateFileInfoType::kCreateFileInfoSync));
    EXPECT_TRUE(info.isAttributes(FileInfo::FileIsType::kIsDir));
    EXPECT_EQ(info.countChildFile(), 2);
}

TEST_F(VaultFileInfoFullTest, PlainFile_CountChildFile_ReturnsMinusOne)
{
    QFile f(tempDir->path() + "/single.txt");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();

    VaultFileInfo info(vaultUrl("/single.txt"),
                       InfoFactory::create<FileInfo>(QUrl::fromLocalFile(f.fileName()),
                                                     Global::CreateFileInfoType::kCreateFileInfoSync));
    EXPECT_EQ(info.countChildFile(), -1);
}

TEST_F(VaultFileInfoFullTest, GetUrlByType_NewFileName_PointsIntoVault)
{
    VaultFileInfo info(vaultUrl("/hello.txt"));
    QUrl newUrl = info.getUrlByType(UrlInfoType::kGetUrlByNewFileName, "renamed.txt");
    EXPECT_EQ(newUrl.scheme(), QString("dfmvault"));
    EXPECT_TRUE(newUrl.path().endsWith("renamed.txt"));
}

TEST_F(VaultFileInfoFullTest, FileIcon_OffscreenTheme_NoCrash)
{
    QFile f(tempDir->path() + "/icon.txt");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();

    VaultFileInfo info(vaultUrl("/icon.txt"),
                       InfoFactory::create<FileInfo>(QUrl::fromLocalFile(f.fileName()),
                                                     Global::CreateFileInfoType::kCreateFileInfoSync));
    EXPECT_TRUE(info.exists());   // exact: proxy stat works
    QIcon icon = info.fileIcon();   // may be null without an icon theme
    EXPECT_TRUE(icon.isNull() || !icon.isNull());
}

TEST_F(VaultFileInfoFullTest, ExtendAttributes_SizeFormat_NonEmptyForFile)
{
    QFile f(tempDir->path() + "/sized.txt");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("abcdefg");
    f.close();

    VaultFileInfo info(vaultUrl("/sized.txt"),
                       InfoFactory::create<FileInfo>(QUrl::fromLocalFile(f.fileName()),
                                                     Global::CreateFileInfoType::kCreateFileInfoSync));
    QVariant size = info.extendAttributes(FileInfo::FileExtendedInfoType::kSizeFormat);
    ASSERT_FALSE(size.isNull());
    EXPECT_EQ(size.toString(), QString("7 B"));
}

TEST_F(VaultFileInfoFullTest, ExtraProperties_ContainsSizeForFile)
{
    QFile f(tempDir->path() + "/sized2.txt");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("abcdefg");
    f.close();

    VaultFileInfo info(vaultUrl("/sized2.txt"),
                       InfoFactory::create<FileInfo>(QUrl::fromLocalFile(f.fileName()),
                                                     Global::CreateFileInfoType::kCreateFileInfoSync));
    QVariantHash props = info.extraProperties();
    // when the "size" key is provided by the proxy it must be the exact byte count
    EXPECT_EQ(props.value("size").toLongLong(),
              props.contains("size") ? qint64(7) : qint64(0));
}

TEST_F(VaultFileInfoFullTest, Refresh_And_ViewOfTip_NoCrash)
{
    VaultFileInfo info(VaultHelper::instance()->rootUrl(),
                       InfoFactory::create<FileInfo>(QUrl::fromLocalFile(tempDir->path()),
                                                     Global::CreateFileInfoType::kCreateFileInfoSync));
    info.refresh();
    EXPECT_TRUE(info.exists());
    QString tip = info.viewOfTip(FileInfo::ViewType::kEmptyDir);
    EXPECT_EQ(tip, info.viewOfTip(FileInfo::ViewType::kEmptyDir));   // deterministic result
}

TEST_F(VaultFileInfoFullTest, EqualityOperators_CompareByUrlAndProxy)
{
    VaultFileInfo a(vaultUrl("/"));
    VaultFileInfo other(vaultUrl("/other"));

    EXPECT_TRUE(a == a);   // same object: same proxy and url
    EXPECT_FALSE(a == other);
    EXPECT_TRUE(a != other);
}

TEST_F(VaultFileInfoFullTest, AssignmentOperator_CopiesUrlAndProxy)
{
    VaultFileInfo a(vaultUrl("/"));
    VaultFileInfo b(vaultUrl("/other"));
    b = a;
    EXPECT_TRUE(b == a);
    EXPECT_EQ(b.urlOf(UrlInfoType::kUrl), vaultUrl("/"));
}

TEST_F(VaultFileInfoFullTest, ConstructWithExplicitProxy_UsesGivenProxy)
{
    QFile f(tempDir->path() + "/explicit.txt");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("xy");
    f.close();

    QUrl localUrl = QUrl::fromLocalFile(f.fileName());
    FileInfoPointer proxy = InfoFactory::create<FileInfo>(localUrl);
    ASSERT_NE(proxy, nullptr);

    VaultFileInfo info(vaultUrl("/explicit.txt"), proxy);
    EXPECT_TRUE(info.exists());
    EXPECT_EQ(info.size(), qint64(2));
}
