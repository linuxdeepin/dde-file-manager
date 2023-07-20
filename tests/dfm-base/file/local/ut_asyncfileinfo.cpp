// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_AAsyncFileInfo
#define UT_AAsyncFileInfo

#include <stubext.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/dfm_global_defines.h>

#include <QDir>

#include <gtest/gtest.h>
#include <unistd.h>
#include <pwd.h>

DFMBASE_USE_NAMESPACE

class UT_AsyncFileInfo : public testing::Test
{
public:
    virtual void SetUp() override
    {
        // 注册路由
        UrlRoute::regScheme(dfmbase::Global::Scheme::kFile, "/", QIcon(), false, QObject::tr("System Disk"));
        pool.setMaxThreadCount(10);
    }

    virtual void TearDown() override
    {
    }

    ~UT_AsyncFileInfo() override;

    void queryAsync(QSharedPointer<AsyncFileInfo> info);

    QSharedPointer<AsyncFileInfo> info{ nullptr };
    QThreadPool pool;
};

UT_AsyncFileInfo::~UT_AsyncFileInfo() {

}

void UT_AsyncFileInfo::queryAsync(QSharedPointer<AsyncFileInfo> info)
{
    bool isMain = false;
    do {
        auto future = QtConcurrent::run(&pool, [&]{
            isMain = qApp->thread() == QThread::currentThread();
            if (isMain)
                return;
            info->cacheAsyncAttributes();
        });
        future.waitForFinished();
    }
    while (isMain);

}

TEST_F(UT_AsyncFileInfo, testAsyncFileInfo)
{
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    info.reset(new AsyncFileInfo(url));
    QSharedPointer<dfmio::DFileInfo> dfileinfo (new dfmio::DFileInfo(url));

    info.reset(new AsyncFileInfo(url, dfileinfo));
    queryAsync(info);
    EXPECT_TRUE(info->exists());
    info->cacheAttribute(dfmio::DFileInfo::AttributeID::kStandardIsHidden, true);
    EXPECT_TRUE(info->isAttributes(OptInfoType::kIsHidden));
}

TEST_F(UT_AsyncFileInfo, testAsyncFileInfoNameOf)
{
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    QProcess::execute("touch testAsyncFileInfo.txt");
    url.setPath(url.path() + QDir::separator() + "testAsyncFileInfo.txt");
    QSharedPointer<dfmio::DFileInfo> dfileinfo (new dfmio::DFileInfo(url));
    info.reset(new AsyncFileInfo(url, dfileinfo));
    queryAsync(info);
    EXPECT_EQ("testAsyncFileInfo.txt", info->nameOf(NameInfoType::kFileName));
    EXPECT_EQ("testAsyncFileInfo", info->nameOf(NameInfoType::kBaseName));
    EXPECT_EQ("testAsyncFileInfo", info->nameOf(NameInfoType::kBaseNameOfRename));
    EXPECT_EQ("testAsyncFileInfo", info->nameOf(NameInfoType::kCompleteBaseName));
    EXPECT_EQ("txt", info->nameOf(NameInfoType::kSuffix));
    EXPECT_EQ("txt", info->nameOf(NameInfoType::kCompleteSuffix));
    EXPECT_EQ("testAsyncFileInfo.txt", info->nameOf(NameInfoType::kFileCopyName));
    EXPECT_EQ("text-plain", info->nameOf(NameInfoType::kIconName));
    EXPECT_EQ("text-x-generic", info->nameOf(NameInfoType::kGenericIconName));
    EXPECT_EQ("text/plain", info->nameOf(NameInfoType::kMimeTypeName));
    EXPECT_EQ("testAsyncFileInfo.txt", info->nameOf(NameInfoType::kFileNameOfRename));
    EXPECT_EQ("", info->nameOf(NameInfoType::kCustomerStartName));
    QProcess::execute("rm testAsyncFileInfo.txt");

}


TEST_F(UT_AsyncFileInfo, testAsyncFileInfoPathOf)
{
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    QProcess::execute("touch testAsyncFileInfo.txt");
    auto parent = url.path();
    url.setPath(url.path() + QDir::separator() + "testAsyncFileInfo.txt");
    QSharedPointer<dfmio::DFileInfo> dfileinfo (new dfmio::DFileInfo(url));
    info.reset(new AsyncFileInfo(url, dfileinfo));
    queryAsync(info);
    EXPECT_EQ(url.path(), info->pathOf(PathInfoType::kFilePath));
    EXPECT_EQ(url.path(), info->pathOf(PathInfoType::kAbsoluteFilePath));
    EXPECT_EQ(url.path(), info->pathOf(PathInfoType::kCanonicalPath));
    EXPECT_EQ(parent, info->pathOf(PathInfoType::kPath));
    EXPECT_EQ(parent, info->pathOf(PathInfoType::kAbsolutePath));
    EXPECT_EQ(parent + "/", info->pathOf(PathInfoType::kSymLinkTarget));
    EXPECT_EQ("", info->pathOf(PathInfoType::kCustomerStartPath));

    QProcess::execute("ln -s  testAsyncFileInfo.txt testInfott");
    info.reset(new AsyncFileInfo(QUrl::fromLocalFile(parent + QDir::separator() + "testInfott")));
    queryAsync(info);
    EXPECT_EQ(url.path(), info->pathOf(PathInfoType::kSymLinkTarget));

    QProcess::execute("rm testInfott testAsyncFileInfo.txt");
}

TEST_F(UT_AsyncFileInfo, testAsyncFileInfoUrlOf)
{
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    QProcess::execute("touch testAsyncFileInfo.txt");
    auto parent = url.path();
    url.setPath(url.path() + QDir::separator() + "testAsyncFileInfo.txt");
    info.reset(new AsyncFileInfo(url));
    queryAsync(info);
    EXPECT_EQ(url, info->urlOf(UrlInfoType::kRedirectedFileUrl));
    EXPECT_EQ(url, info->urlOf(UrlInfoType::kUrl));
    EXPECT_EQ(url, info->urlOf(UrlInfoType::kOriginalUrl));
    EXPECT_EQ(parent, info->urlOf(UrlInfoType::kParentUrl).path());
    QProcess::execute("ln -s  testAsyncFileInfo.txt testInfott");
    info.reset(new AsyncFileInfo(QUrl::fromLocalFile(parent + QDir::separator() + "testInfott")));
    queryAsync(info);
    EXPECT_EQ(url, info->urlOf(UrlInfoType::kRedirectedFileUrl));
    EXPECT_FALSE(info->canAttributes(CanableInfoType::kCanRedirectionFileUrl));
    EXPECT_TRUE(0 == info->size());
    EXPECT_TRUE(info->isAttributes(OptInfoType::kIsSymLink));
    EXPECT_EQ(QUrl(), info->urlOf(UrlInfoType::kCustomerStartUrl));
    QProcess::execute("rm testInfott testAsyncFileInfo.txt");
}

TEST_F(UT_AsyncFileInfo, testAsyncFileInfoIsAttributes)
{
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    info.reset(new AsyncFileInfo(url));
    queryAsync(info);
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

TEST_F(UT_AsyncFileInfo, testAsyncFileInfoCanAttributes)
{
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    info.reset(new AsyncFileInfo(url));
    queryAsync(info);
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

TEST_F(UT_AsyncFileInfo, testAsyncFileInfoExtendAttributes)
{
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    info.reset(new AsyncFileInfo(url));
    queryAsync(info);
    EXPECT_EQ(true,info->extendAttributes(ExtInfoType::kFileLocalDevice).toBool());
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

TEST_F(UT_AsyncFileInfo, testAsyncFileInfoTimeOf)
{
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    QProcess::execute("touch testAsyncFileInfo.txt");
    url.setPath(url.path() + QDir::separator() + "testAsyncFileInfo.txt");
    info.reset(new AsyncFileInfo(url));
    queryAsync(info);
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

    QProcess::execute("rm testAsyncFileInfo.txt");
}

TEST_F(UT_AsyncFileInfo, testAsyncFileInfoFileType)
{
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    info.reset(new AsyncFileInfo(url));
    queryAsync(info);
    EXPECT_TRUE(FileInfo::FileType::kDirectory == info->fileType());
    EXPECT_TRUE(FileInfo::FileType::kDirectory == info->fileType());
}

TEST_F(UT_AsyncFileInfo, testAsyncFileInfoChildren)
{
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    url.setPath(url.path() + QDir::separator() + "testAsyncFileInfo");
    QProcess::execute("rm -rf testAsyncFileInfo");
    info.reset(new AsyncFileInfo(url));
    queryAsync(info);
    EXPECT_EQ(QObject::tr("File has been moved or deleted"), info->viewOfTip(ViewInfoType::kEmptyDir));
    EXPECT_EQ(QObject::tr("Loading..."), info->viewOfTip(ViewInfoType::kLoading));
    EXPECT_EQ("", info->viewOfTip(ViewInfoType::kCustomerStartView));
    EXPECT_TRUE(-1 == info->countChildFile());
    EXPECT_TRUE(-1 == info->countChildFileAsync());
    QProcess::execute("mkdir testAsyncFileInfo");
    queryAsync(info);
    while (-1 == info->countChildFile()) {
        info->countChildFileAsync();
    }
    EXPECT_TRUE(0 == info->countChildFile());
    EXPECT_TRUE(0 == info->countChildFileAsync());
    EXPECT_EQ("testAsyncFileInfo", info->displayOf(DisPlayInfoType::kFileDisplayName));
    EXPECT_EQ("-", info->displayOf(DisPlayInfoType::kSizeDisplayName));
    EXPECT_EQ(url.path(), info->displayOf(DisPlayInfoType::kFileDisplayPath));
    EXPECT_EQ("Directory (inode/directory)", info->displayOf(DisPlayInfoType::kMimeTypeDisplayName));
    EXPECT_EQ("0", info->displayOf(DisPlayInfoType::kFileTypeDisplayName));
    EXPECT_EQ("testAsyncFileInfo", info->displayOf(DisPlayInfoType::kFileDisplayPinyinName));

    QProcess::execute("touch ./testAsyncFileInfo/testAsyncFileInfo.txt");
    info.reset(new AsyncFileInfo(url));
    info.dynamicCast<AsyncFileInfo>()->setNotifyUrl(url, QString(""));
    queryAsync(info);
    while (-1 == info->countChildFileAsync()) {
        QThread::msleep(10);
    }
    EXPECT_TRUE(1 == info->countChildFileAsync());

    QSharedPointer<AsyncFileInfo> tmp(new AsyncFileInfo(QUrl::fromLocalFile(url.path() + "/testAsyncFileInfo.txt")));
    queryAsync(tmp);
    EXPECT_EQ("0 B", tmp->displayOf(DisPlayInfoType::kSizeDisplayName));

    QProcess::execute("rm -rf testAsyncFileInfo");
}

TEST_F(UT_AsyncFileInfo, testAsyncFileInfoMimType)
{
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    info.reset(new AsyncFileInfo(url));
    EXPECT_TRUE(!info->fileMimeTypeAsync().isValid());
    queryAsync(info);
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

    EXPECT_TRUE(info->supportedOfAttributes(SupportedType::kDrag).testFlag(Qt::CopyAction));
    EXPECT_TRUE(info->supportedOfAttributes(SupportedType::kDrop).testFlag(Qt::CopyAction));
    EXPECT_TRUE(info->supportedOfAttributes(SupportedType::kCustomerSupport).testFlag(Qt::IgnoreAction));
}

#endif
