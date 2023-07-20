// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "fileutils/vaultfileinfo.h"
#include "fileutils/private/vaultfileinfo_p.h"
#include "utils/vaulthelper.h"

#include <gtest/gtest.h>

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/interfaces/proxyfileinfo.h>
#include <dfm-base/file/local/syncfileinfo.h>

DPVAULT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

TEST(UT_VaultFileInfo, operator_fileInfo_one)
{
    VaultFileInfo info1(QUrl("dfmvault:///UT_TEST1"));
    VaultFileInfo info2(QUrl("dfmvault:///UT_TEST2"));

    info1 = info2;

    EXPECT_TRUE(info1.url == QUrl("dfmvault:///UT_TEST2"));
}

TEST(UT_VaultFileInfo, operator_fileInfo_two)
{
    VaultFileInfo info1(QUrl("dfmvault:///UT_TEST1"));
    VaultFileInfo info2(QUrl("dfmvault:///UT_TEST1"));

    EXPECT_TRUE(info1 == info2);
}

TEST(UT_VaultFileInfo, operator_fileInfo_three)
{
    VaultFileInfo info1(QUrl("dfmvault:///UT_TEST1"));
    VaultFileInfo info2(QUrl("dfmvault:///UT_TEST2"));

    EXPECT_TRUE(info1 != info2);
}

TEST(UT_VaultFileInfo, pathof_one)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultFileInfoPrivate::absolutePath, []{
        return "/UT_TEST";
    });

    VaultFileInfo info(QUrl("dfmvault:///"));
    QString path = info.pathOf(FileInfo::FilePathInfoType::kAbsolutePath);

    EXPECT_FALSE(path.isNull());
}

TEST(UT_VaultFileInfo, pathof_two)
{
    VaultFileInfo info(QUrl("dfmvault:///UT_TEST"));
    QString path = info.pathOf(FileInfo::FilePathInfoType::kFilePath);

    EXPECT_FALSE(path.isNull());
}

TEST(UT_VaultFileInfo, urlof_one)
{
    VaultFileInfo info(QUrl("dfmvault:///UT_TEST"));
    QUrl url = info.urlOf(FileInfo::FileUrlInfoType::kUrl);

    EXPECT_TRUE(url == QUrl("dfmvault:///UT_TEST"));
}

TEST(UT_VaultFileInfo, urlof_two)
{
    VaultFileInfo info(QUrl("dfmvault:///UT_TEST"));
    QUrl url = info.urlOf(FileInfo::FileUrlInfoType::kRedirectedFileUrl);

    EXPECT_TRUE(url == info.d->localUrl);
}

TEST(UT_VaultFileInfo, urlof_three)
{
    VaultFileInfo info(QUrl("dfmvault:///UT_TEST"));
    QUrl url = info.urlOf(FileInfo::FileUrlInfoType::kOriginalUrl);

    EXPECT_TRUE(url == QUrl("dfmvault:///UT_TEST"));
}

TEST(UT_VaultFileInfo, exists_one)
{
    VaultFileInfo info(QUrl("dfmvault:///UT_TEST"));
    bool isOk = info.exists();

    EXPECT_FALSE(isOk);
}

TEST(UT_VaultFileInfo, exists_two)
{
    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(VaultFileInfo, urlOf), []{
        return QString("");
    });

    VaultFileInfo info(QUrl("dfmvault:///"));
    bool isOk = info.exists();

    EXPECT_FALSE(isOk);
}

TEST(UT_VaultFileInfo, refresh)
{
    bool isOk = { false };

    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(SyncFileInfo, refresh), [ &isOk ]{
        isOk = false;
    });

    VaultFileInfo info(QUrl("dfmvault:///"));
    info.refresh();

    EXPECT_FALSE(isOk);
}

TEST(UT_VaultFileInfo, isAttributes_one)
{
    bool isOk { false };

    VaultFileInfo info(QUrl("dfmvault:///UT_TEST"));
    info.isAttributes(FileInfo::FileIsType::kIsFile);
    info.isAttributes(FileInfo::FileIsType::kIsDir);
    info.isAttributes(FileInfo::FileIsType::kIsReadable);
    info.isAttributes(FileInfo::FileIsType::kIsWritable);
    info.isAttributes(FileInfo::FileIsType::kIsExecutable);
    info.isAttributes(FileInfo::FileIsType::kIsSymLink);
    isOk = info.isAttributes(FileInfo::FileIsType::kIsHidden);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileInfo, isAttributes_two)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(ProxyFileInfo, isAttributes), [ &isOk ]{
        isOk = true;
        return true;
    });

    VaultFileInfo info(QUrl("dfmvault:///UT_TEST"));
    info.isAttributes(FileInfo::FileIsType::kUnknowFileIsInfo);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileInfo, canAttributes_one)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::state, []{
        return VaultState::kEncrypted;
    });

    VaultFileInfo info(QUrl("dfmvault:///UT_TEST"));
    bool isOk = info.canAttributes(FileInfo::FileCanType::kCanDrop);

    EXPECT_FALSE(isOk);
}

TEST(UT_VaultFileInfo, canAttributes_two)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::state, []{
        return VaultState::kUnlocked;
    });

    VaultFileInfo info(QUrl("dfmvault:///UT_TEST"));
    bool isOk = info.canAttributes(FileInfo::FileCanType::kCanDrop);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileInfo, canAttributes_three)
{
    VaultFileInfo info(QUrl("dfmvault:///UT_TEST"));
    bool isOk = info.canAttributes(FileInfo::FileCanType::kCanRedirectionFileUrl);

    EXPECT_FALSE(isOk);
}

TEST(UT_VaultFileInfo, canAttributes_four)
{
    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(ProxyFileInfo, canAttributes), []{
        return true;
    });

    VaultFileInfo info(QUrl("dfmvault:///UT_TEST"));
    bool isOk = info.canAttributes(FileInfo::FileCanType::kCanHidden);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileInfo, extraProperties)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(ProxyFileInfo, extraProperties), [ &isOk ]{
        isOk = true;
        return QVariantHash();
    });

    VaultFileInfo info(QUrl("dfmvault:///UT_TEST"));
    info.extraProperties();

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileInfo, getUrlByType_one)
{
    VaultFileInfo info(QUrl("dfmvault:///UT_TEST"));
    QUrl newUrl = info.getUrlByType(FileInfo::FileUrlInfoType::kGetUrlByNewFileName, "UT_TEST1");

    EXPECT_TRUE(newUrl == QUrl("dfmvault:///UT_TEST1"));
}

TEST(UT_VaultFileInfo, getUrlByType_two)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(ProxyFileInfo, getUrlByType), [ &isOk ]{
        isOk = true;
        return QUrl();
    });

    VaultFileInfo info(QUrl("dfmvault:///UT_TEST"));
    QUrl newUrl = info.getUrlByType(FileInfo::FileUrlInfoType::kUrl, "UT_TEST1");

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileInfo, fileIcon_one)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    typedef QIcon(*FuncType)(const QString &);
    stub.set_lamda(static_cast<FuncType>(&QIcon::fromTheme), [ &isOk ](){
        isOk = true;
        return QIcon();
    });

    VaultFileInfo info(QUrl("dfmvault:///"));
    info.d->isRoot = true;
    info.fileIcon();

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileInfo, fileIcon_two)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(ProxyFileInfo, fileIcon), [ &isOk ]{
        isOk = true;
        return QIcon();
    });

    VaultFileInfo info(QUrl("dfmvault:///"));
    info.d->isRoot = false;
    info.fileIcon();

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileInfo, size)
{
    VaultFileInfo info(QUrl("dfmvault:///"));
    quint64 result = info.size();

    EXPECT_TRUE(result == 0);
}

TEST(UT_VaultFileInfo, countChildFile_one)
{
    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(VaultFileInfo, isAttributes), []{
        return true;
    });
    typedef  QStringList(QDir::*FuncType)(QDir::Filters, QDir::SortFlags)const;
    stub.set_lamda(static_cast<FuncType>(&QDir::entryList), []{
        return QStringList() << "1";
    });

    VaultFileInfo info(QUrl("dfmvault:///"));
    int result = info.countChildFile();

    EXPECT_TRUE(result == 1);
}

TEST(UT_VaultFileInfo, countChildFile_two)
{
    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(VaultFileInfo, isAttributes), []{
        return false;
    });

    VaultFileInfo info(QUrl("dfmvault:///"));
    int result = info.countChildFile();

    EXPECT_TRUE(result == -1);
}

TEST(UT_VaultFileInfo, extendAttributes_one)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(ProxyFileInfo, extendAttributes), [ &isOk ]{
        isOk = true;
        return QVariant();
    });

    VaultFileInfo info(QUrl("dfmvault:///"));
    info.extendAttributes(FileInfo::FileExtendedInfoType::kSizeFormat);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileInfo, extendAttributes_two)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(ProxyFileInfo, extendAttributes), [ &isOk ]{
        isOk = true;
        return QVariant();
    });

    VaultFileInfo info(QUrl("dfmvault:///"));
    info.extendAttributes(FileInfo::FileExtendedInfoType::kOwner);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileInfo, nameof_one)
{
    bool isOk { false };
    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(VaultFileInfo, displayOf), [ &isOk ]{
        isOk = true;
        return "UT_TEST";
    });

    VaultFileInfo info(QUrl("dfmvault:///UT_TEST"));
    info.nameOf(NameInfoType::kFileCopyName);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileInfo, nameof_two)
{
    VaultFileInfo info(QUrl("dfmvault:///"));
    info.d->isRoot = true;
    QString name = info.nameOf(NameInfoType::kIconName);

    EXPECT_TRUE(name == "dfm_safebox");
}

TEST(UT_VaultFileInfo, nameof_three)
{
    VaultFileInfo info(QUrl("dfmvault:///UT_TEST"));
    info.d->isRoot = false;
    QString name = info.nameOf(NameInfoType::kIconName);

    EXPECT_TRUE(name.isEmpty());
}

TEST(UT_VaultFileInfo, nameof_four)
{
    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(ProxyFileInfo, nameOf), []{
        return "UT";
    });

    VaultFileInfo info(QUrl("dfmvault:///UT_TEST"));
    QString name = info.nameOf(NameInfoType::kUnknowNameInfo);

    EXPECT_TRUE(name == "UT");
}

TEST(UT_VaultFileInfo, displayOf_one)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(ProxyFileInfo, displayOf), [ &isOk ]{
        isOk = true;
        return "";
    });

    VaultFileInfo info(QUrl("dfmvault:///"));
    info.displayOf(DisPlayInfoType::kFileDisplayName);

    VaultFileInfo info2(QUrl("dfmvault:///UT_TEST"));
    info2.displayOf(DisPlayInfoType::kFileDisplayName);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultFileInfo, displayOf_two)
{
    VaultFileInfo info(QUrl("dfmvault:///UT_TEST"));
    QString display = info.displayOf(DisPlayInfoType::kFileDisplayPath);

    EXPECT_TRUE(display == "dfmvault:///UT_TEST");
}

TEST(UT_VaultFileInfo, displayOf_three)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(ProxyFileInfo, displayOf), [ &isOk ]{
        isOk = true;
        return "";
    });

    VaultFileInfo info(QUrl("dfmvault:///UT_TEST"));
    info.displayOf(DisPlayInfoType::kUnknowDisplayInfo);

    EXPECT_TRUE(isOk);
}
