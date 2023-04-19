// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_SYNCFILEINFO
#define UT_SYNCFILEINFO

#include <stubext.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/dfm_global_defines.h>

#include <QDir>

#include <gtest/gtest.h>
#include <unistd.h>
#include <pwd.h>

DFMBASE_USE_NAMESPACE

class UT_SyncFileInfo : public testing::Test
{
public:
    virtual void SetUp() override
    {
        // 注册路由
        UrlRoute::regScheme(dfmbase::Global::Scheme::kFile, "/", QIcon(), false, QObject::tr("System Disk"));
    }

    virtual void TearDown() override
    {
    }

    ~UT_SyncFileInfo() override;

    FileInfoPointer info{ nullptr };
};

UT_SyncFileInfo::~UT_SyncFileInfo() {

}

TEST_F(UT_SyncFileInfo, testSyncFileInfo)
{
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    info.reset(new SyncFileInfo(url));
    QSharedPointer<dfmio::DFileInfo> dfileinfo (new dfmio::DFileInfo(url));
    dfileinfo->initQuerier();
    info.reset(new SyncFileInfo(url, dfileinfo));
    SyncFileInfo info1(url, dfileinfo),info2(url, dfileinfo),info3(url);
    EXPECT_TRUE(info1 == info2);
    EXPECT_TRUE(info1 != info3);
    EXPECT_TRUE(info3.initQuerier());
    EXPECT_TRUE(info->exists());
    info->cacheAttribute(dfmio::DFileInfo::AttributeID::kStandardIsHidden, true);
    EXPECT_TRUE(info->isAttributes(OptInfoType::kIsHidden));
    info->refresh();
    EXPECT_FALSE(info->isAttributes(OptInfoType::kIsHidden));
}

TEST_F(UT_SyncFileInfo, testSyncFileInfoNameOf)
{
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    QProcess::execute("touch testSyncFileInfo.txt");
    url.setPath(url.path() + QDir::separator() + "testSyncFileInfo.txt");
    info.reset(new SyncFileInfo(url));
    EXPECT_EQ("testSyncFileInfo.txt", info->nameOf(NameInfoType::kFileName));
    EXPECT_EQ("testSyncFileInfo", info->nameOf(NameInfoType::kBaseName));
    EXPECT_EQ("testSyncFileInfo", info->nameOf(NameInfoType::kBaseNameOfRename));
    EXPECT_EQ("testSyncFileInfo", info->nameOf(NameInfoType::kCompleteBaseName));
    EXPECT_EQ("txt", info->nameOf(NameInfoType::kSuffix));
    EXPECT_EQ("txt", info->nameOf(NameInfoType::kCompleteSuffix));
    EXPECT_EQ("testSyncFileInfo.txt", info->nameOf(NameInfoType::kFileCopyName));
    EXPECT_EQ("text-plain", info->nameOf(NameInfoType::kIconName));
    EXPECT_EQ("text-x-generic", info->nameOf(NameInfoType::kGenericIconName));
    EXPECT_EQ("text/plain", info->nameOf(NameInfoType::kMimeTypeName));
    EXPECT_EQ("testSyncFileInfo.txt", info->nameOf(NameInfoType::kFileNameOfRename));
    EXPECT_EQ("", info->nameOf(NameInfoType::kCustomerStartName));
    QProcess::execute("rm testSyncFileInfo.txt");

}


TEST_F(UT_SyncFileInfo, testSyncFileInfoPathOf)
{
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    QProcess::execute("touch testSyncFileInfo.txt");
    auto parent = url.path();
    url.setPath(url.path() + QDir::separator() + "testSyncFileInfo.txt");
    info.reset(new SyncFileInfo(url));
    EXPECT_EQ(url.path(), info->pathOf(PathInfoType::kFilePath));
    EXPECT_EQ(url.path(), info->pathOf(PathInfoType::kAbsoluteFilePath));
    EXPECT_EQ(url.path(), info->pathOf(PathInfoType::kCanonicalPath));
    EXPECT_EQ(parent, info->pathOf(PathInfoType::kPath));
    EXPECT_EQ(parent, info->pathOf(PathInfoType::kAbsolutePath));
    EXPECT_EQ(parent + "/", info->pathOf(PathInfoType::kSymLinkTarget));
    EXPECT_EQ("", info->pathOf(PathInfoType::kCustomerStartPath));

    QProcess::execute("ln -s  testSyncFileInfo.txt testInfott");
    info.reset(new SyncFileInfo(QUrl::fromLocalFile(parent + QDir::separator() + "testInfott")));
    EXPECT_EQ(url.path(), info->pathOf(PathInfoType::kSymLinkTarget));

    QProcess::execute("rm testInfott testSyncFileInfo.txt");
}

TEST_F(UT_SyncFileInfo, testSyncFileInfoUrlOf)
{
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    QProcess::execute("touch testSyncFileInfo.txt");
    auto parent = url.path();
    url.setPath(url.path() + QDir::separator() + "testSyncFileInfo.txt");
    info.reset(new SyncFileInfo(url));
    EXPECT_EQ(url, info->urlOf(UrlInfoType::kRedirectedFileUrl));
    EXPECT_EQ(url, info->urlOf(UrlInfoType::kUrl));
    EXPECT_EQ(url, info->urlOf(UrlInfoType::kOriginalUrl));
    EXPECT_EQ(parent, info->urlOf(UrlInfoType::kParentUrl).path());
    QProcess::execute("ln -s  testSyncFileInfo.txt testInfott");
    info.reset(new SyncFileInfo(QUrl::fromLocalFile(parent + QDir::separator() + "testInfott")));
    EXPECT_EQ(url, info->urlOf(UrlInfoType::kRedirectedFileUrl));
    EXPECT_TRUE(info->canAttributes(CanableInfoType::kCanRedirectionFileUrl));
    EXPECT_TRUE(0 == info->size());
    EXPECT_TRUE(info->isAttributes(OptInfoType::kIsSymLink));
    EXPECT_EQ(QUrl(), info->urlOf(UrlInfoType::kCustomerStartUrl));
    QProcess::execute("rm testInfott testSyncFileInfo.txt");
}

TEST_F(UT_SyncFileInfo, testSyncFileInfoIsAttributes)
{
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    info.reset(new SyncFileInfo(url));
    EXPECT_TRUE(info->isAttributes(OptInfoType::kIsDir));
    EXPECT_FALSE(info->isAttributes(OptInfoType::kIsFile));
    EXPECT_TRUE(info->isAttributes(OptInfoType::kIsReadable));
    EXPECT_TRUE(info->isAttributes(OptInfoType::kIsWritable));
    EXPECT_TRUE(info->isAttributes(OptInfoType::kIsExecutable));
    EXPECT_FALSE(info->isAttributes(OptInfoType::kIsRoot));
    EXPECT_FALSE(info->isAttributes(OptInfoType::kIsHidden));
    EXPECT_FALSE(info->isAttributes(OptInfoType::kIsBundle));
    EXPECT_FALSE(info->isAttributes(OptInfoType::kIsPrivate));
    EXPECT_FALSE(info->isAttributes(OptInfoType::kCustomerFileIs));
}

TEST_F(UT_SyncFileInfo, testSyncFileInfoCanAttributes)
{
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    info.reset(new SyncFileInfo(url));
    EXPECT_FALSE(info->canAttributes(CanableInfoType::kCanRedirectionFileUrl));
    EXPECT_TRUE(info->canAttributes(CanableInfoType::kCanDelete));
    EXPECT_TRUE(info->canAttributes(CanableInfoType::kCanTrash));
    EXPECT_TRUE(info->canAttributes(CanableInfoType::kCanRename));
    EXPECT_TRUE(info->canAttributes(CanableInfoType::kCanHidden));
    EXPECT_TRUE(info->canAttributes(CanableInfoType::kCanFetch));
    EXPECT_TRUE(info->canAttributes(CanableInfoType::kCanDrop));
    EXPECT_TRUE(info->canAttributes(CanableInfoType::kCanMoveOrCopy));
    EXPECT_TRUE(info->canAttributes(CanableInfoType::kCanDrag));
    EXPECT_FALSE(info->canAttributes(CanableInfoType::kCustomerFileCan));
}

TEST_F(UT_SyncFileInfo, testSyncFileInfoExtendAttributes)
{
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    info.reset(new SyncFileInfo(url));
    EXPECT_EQ(false,info->extendAttributes(ExtInfoType::kFileLocalDevice).toBool());
    EXPECT_EQ(false,info->extendAttributes(ExtInfoType::kFileCdRomDevice).toBool());
    EXPECT_EQ("-",info->extendAttributes(ExtInfoType::kSizeFormat).toString());
    EXPECT_TRUE(0 != info->extendAttributes(ExtInfoType::kInode).toUInt());
    struct passwd *pwd { nullptr };
    pwd = getpwuid(getuid());
    EXPECT_TRUE(QString(pwd->pw_name) == info->extendAttributes(ExtInfoType::kOwner).toString());
    EXPECT_TRUE(!info->extendAttributes(ExtInfoType::kFileIsHid).toBool());
    EXPECT_TRUE(getuid() == info->extendAttributes(ExtInfoType::kOwnerId).toUInt());
    EXPECT_TRUE(getgid() == info->extendAttributes(ExtInfoType::kGroupId).toUInt());
    EXPECT_TRUE(!info->extendAttributes(ExtInfoType::kCustomerStartExtended).isValid());
    EXPECT_TRUE(info->permission(QFile::ReadUser));
}

TEST_F(UT_SyncFileInfo, testSyncFileInfoTimeOf)
{
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    QProcess::execute("touch testSyncFileInfo.txt");
    url.setPath(url.path() + QDir::separator() + "testSyncFileInfo.txt");
    info.reset(new SyncFileInfo(url));
    dfmio::DFile file(url);
    if (file.open(dfmio::DFile::OpenFlag::kReadWrite)) {
        QByteArray tt{"------"};
        file.write(tt);
        file.close();
    }
    EXPECT_TRUE(info->timeOf(TimeInfoType::kCreateTime).isValid());
    EXPECT_TRUE(info->timeOf(TimeInfoType::kBirthTime).isValid());
    EXPECT_TRUE(info->timeOf(TimeInfoType::kMetadataChangeTime).isValid());
    EXPECT_TRUE(info->timeOf(TimeInfoType::kLastModified).isValid());
    EXPECT_TRUE(info->timeOf(TimeInfoType::kLastRead).isValid());

    EXPECT_TRUE(info->timeOf(TimeInfoType::kCreateTimeSecond).isValid());
    EXPECT_TRUE(info->timeOf(TimeInfoType::kBirthTimeSecond).isValid());
    EXPECT_TRUE(info->timeOf(TimeInfoType::kMetadataChangeTimeSecond).isValid());
    EXPECT_TRUE(info->timeOf(TimeInfoType::kLastModifiedSecond).isValid());
    EXPECT_TRUE(info->timeOf(TimeInfoType::kLastReadSecond).isValid());

    EXPECT_NO_FATAL_FAILURE(info->timeOf(TimeInfoType::kCreateTimeMSecond));
    EXPECT_NO_FATAL_FAILURE(info->timeOf(TimeInfoType::kBirthTimeMSecond));
    EXPECT_NO_FATAL_FAILURE(info->timeOf(TimeInfoType::kMetadataChangeTimeMSecond));
    EXPECT_NO_FATAL_FAILURE(info->timeOf(TimeInfoType::kLastModifiedMSecond));
    EXPECT_NO_FATAL_FAILURE(info->timeOf(TimeInfoType::kLastReadMSecond));

    QProcess::execute("rm testSyncFileInfo.txt");
}

TEST_F(UT_SyncFileInfo, testSyncFileInfoFileType)
{
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    info.reset(new SyncFileInfo(url));
    EXPECT_TRUE(FileInfo::FileType::kDirectory == info->fileType());
    EXPECT_TRUE(FileInfo::FileType::kDirectory == info->fileType());
}

TEST_F(UT_SyncFileInfo, testSyncFileInfoChildren)
{
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    url.setPath(url.path() + QDir::separator() + "testSyncFileInfo");
    info.reset(new SyncFileInfo(url));
    EXPECT_EQ(QObject::tr("File has been moved or deleted"), info->viewOfTip(ViewInfoType::kEmptyDir));
    EXPECT_EQ(QObject::tr("Loading..."), info->viewOfTip(ViewInfoType::kLoading));
    EXPECT_EQ("", info->viewOfTip(ViewInfoType::kCustomerStartView));
    QProcess::execute("mkdir testSyncFileInfo");
    info->refresh();
    EXPECT_TRUE(0 == info->countChildFile());
    EXPECT_TRUE(0 == info->countChildFileAsync());
    EXPECT_EQ("testSyncFileInfo", info->displayOf(DisPlayInfoType::kFileDisplayName));
    EXPECT_EQ("-", info->displayOf(DisPlayInfoType::kSizeDisplayName));
    EXPECT_EQ(url.path(), info->displayOf(DisPlayInfoType::kFileDisplayPath));
    EXPECT_EQ("Directory (inode/directory)", info->displayOf(DisPlayInfoType::kMimeTypeDisplayName));
    EXPECT_EQ("0", info->displayOf(DisPlayInfoType::kFileTypeDisplayName));
    EXPECT_EQ("testSyncFileInfo", info->displayOf(DisPlayInfoType::kFileDisplayPinyinName));

    QProcess::execute("touch ./testSyncFileInfo/testSyncFileInfo.txt");
    while (0 == info->countChildFileAsync()) {
        QThread::msleep(10);
    }
    EXPECT_TRUE(1 == info->countChildFileAsync());

    FileInfoPointer tmp(new SyncFileInfo(QUrl::fromLocalFile(url.path() + "/testSyncFileInfo.txt")));
    qInfo() << tmp->displayOf(DisPlayInfoType::kSizeDisplayName);
    EXPECT_EQ("0 B", tmp->displayOf(DisPlayInfoType::kSizeDisplayName));

    QProcess::execute("rm -rf testSyncFileInfo");
}

TEST_F(UT_SyncFileInfo, testSyncFileInfoMimType)
{
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    info.reset(new SyncFileInfo(url));
    EXPECT_TRUE(!info->fileMimeTypeAsync().isValid());
    info->refresh();
    EXPECT_TRUE(info->fileMimeType().isValid());
    EXPECT_TRUE(info->fileMimeType().isValid());
    EXPECT_TRUE(!info->fileIcon().isNull());
    info->setExtendedAttributes(ExtInfoType::kFileIsHid, true);
    EXPECT_TRUE(info->extendAttributes(ExtInfoType::kFileIsHid).toBool());
    QList<dfmio::DFileInfo::AttributeExtendID> extenList;
    extenList << dfmio::DFileInfo::AttributeExtendID::kExtendMediaWidth <<
                 dfmio::DFileInfo::AttributeExtendID::kExtendMediaHeight << dfmio::DFileInfo::AttributeExtendID::kExtendMediaDuration;

    const QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> &mediaAttributes = info->mediaInfoAttributes(dfmio::DFileInfo::MediaType::kVideo, extenList);
    EXPECT_TRUE(mediaAttributes.isEmpty());
}

#endif
