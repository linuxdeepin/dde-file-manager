// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/core/dfmplugin-trashcore/trashfileinfo.h"
#include "plugins/common/core/dfmplugin-trashcore/private/trashfileinfo_p.h"
#include "plugins/common/core/dfmplugin-trashcore/utils/trashcorehelper.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/fileutils.h>

#include <dfm-io/dfileinfo.h>

#include <gtest/gtest.h>


DPTRASHCORE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
USING_IO_NAMESPACE
class UT_TrashFileInfo : public testing::Test
{
public:
    void SetUp() override {
        // 注册路由
        UrlRoute::regScheme(Global::Scheme::kFile, "/", QIcon(), false, QObject::tr("System Disk"));
        // 注册Scheme为"file"的扩展的文件信息 本地默认文件的
        InfoFactory::regClass<dfmbase::SyncFileInfo>(Global::Scheme::kFile);
    }
    void TearDown() override {}
};

TEST_F(UT_TrashFileInfo, testTrashFileInfo)
{
    QUrl url(QUrl::fromLocalFile(QDir::currentPath()));
    TrashFileInfo info(url);
    EXPECT_TRUE(info.proxy.isNull());

    stub_ext::StubExt stub;
    stub.set_lamda(&DFileInfo::initQuerier, []{ __DBG_STUB_INVOKE__ return false;});
    TrashFileInfo info1(url);
    EXPECT_TRUE(info1.proxy.isNull());

    stub.set_lamda(&TrashFileInfoPrivate::initTarget, [url]{ __DBG_STUB_INVOKE__ return url;});
    TrashFileInfo info2(url);
    EXPECT_TRUE(info2.proxy.isNull());

    stub.reset(&TrashFileInfoPrivate::initTarget);
    EXPECT_FALSE(info2.d->initTarget().isValid());

    stub.set_lamda(&DFileInfo::attribute,[url]{ __DBG_STUB_INVOKE__ return QVariant(url);});
    EXPECT_TRUE(info2.d->initTarget().isValid());

    stub.set_lamda(&UniversalUtils::urlEquals, []{ __DBG_STUB_INVOKE__ return true;});
    stub.reset(&DFileInfo::initQuerier);
    EXPECT_TRUE(info2.d->initTarget().isValid());

    stub.reset(&DFileInfo::attribute);
    info2.d->targetUrl = QUrl();
    stub.set_lamda(VADDR(TrashFileInfo, fileUrl), [url]{ __DBG_STUB_INVOKE__ return url;});
    int index(0);
    stub.set_lamda(&DFileInfo::attribute,[url, &index]{ __DBG_STUB_INVOKE__
        index++;
        if (index == 3)
            return QVariant(url);
        return QVariant();});
    stub.set_lamda(&UrlRoute::urlParent, [url]{ __DBG_STUB_INVOKE__ return url;});
    stub.set_lamda(&QUrl::isParentOf, []{ __DBG_STUB_INVOKE__ return true;});
    int index1(0);
    stub.set_lamda(&UniversalUtils::urlEquals, [&index1]{ __DBG_STUB_INVOKE__ index1++; return index1 > 1;});
    EXPECT_TRUE(info2.d->initTarget().isValid());
}

TEST_F(UT_TrashFileInfo, testTrashFileInfoPrivate)
{
    QUrl url(QUrl::fromLocalFile(QDir::currentPath() + QDir::separator() + "TrashFileInfoPrivateFileName.txt"));
    TrashFileInfo info(url);
    EXPECT_EQ("TrashFileInfoPrivateFileName.txt", info.nameOf(NameInfoType::kFileName));

    info.d->targetUrl = url;
    stub_ext::StubExt stub;
    stub.set_lamda(&FileUtils::isDesktopFile, []{ __DBG_STUB_INVOKE__ return true;});
    EXPECT_FALSE(info.d->copyName().isEmpty());

    stub.set_lamda(&FileUtils::isDesktopFile, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_EQ("TrashFileInfoPrivateFileName.txt", info.d->copyName());

    EXPECT_TRUE(info.d->mimeTypeName().isEmpty());

    EXPECT_FALSE(info.d->lastRead().isValid());
    EXPECT_FALSE(info.d->lastModified().isValid());
    EXPECT_FALSE(info.d->deletionTime().isValid());
    EXPECT_TRUE(info.d->symLinkTarget().isEmpty());

    info.d->dAncestorsFileInfo.reset(new DFileInfo(QUrl::fromLocalFile(QDir::currentPath())));
    EXPECT_TRUE(info.d->lastRead().isValid());
    EXPECT_TRUE(info.d->lastModified().isValid());
    EXPECT_FALSE(info.d->deletionTime().isValid());

    info.d->dFileInfo = info.d->dAncestorsFileInfo;
    EXPECT_TRUE(info.d->lastRead().isValid());
    EXPECT_TRUE(info.d->lastModified().isValid());
    EXPECT_FALSE(info.d->deletionTime().isValid());

    info.d->dFileInfo = nullptr;
    EXPECT_TRUE(info.nameOf(NameInfoType::kFileName).isEmpty());
    EXPECT_TRUE(info.d->copyName().isEmpty());
    EXPECT_TRUE(info.d->mimeTypeName().isEmpty());
    EXPECT_FALSE(info.d->lastRead().isValid());
    EXPECT_FALSE(info.d->lastModified().isValid());
    EXPECT_FALSE(info.d->deletionTime().isValid());
    EXPECT_TRUE(info.d->symLinkTarget().isEmpty());
}

TEST_F(UT_TrashFileInfo, testTrashFileInfoExists)
{
    QUrl url(QUrl::fromLocalFile(QDir::currentPath() + QDir::separator() + "TrashFileInfoPrivateFileName.txt"));
    TrashFileInfo info(url);
    EXPECT_FALSE(info.exists());

    stub_ext::StubExt stub;
    stub.set_lamda(&FileUtils::isTrashRootFile, []{ __DBG_STUB_INVOKE__ return true;});
    EXPECT_TRUE(info.exists());

    info.d->dFileInfo = nullptr;
    EXPECT_TRUE(info.exists());
}

TEST_F(UT_TrashFileInfo, testTrashFileInfoSupportedOfAttributes)
{
    QUrl url(QUrl::fromLocalFile(QDir::currentPath() + QDir::separator() + "TrashFileInfoPrivateFileName.txt"));
    TrashFileInfo info(url);

    EXPECT_EQ(Qt::IgnoreAction, info.supportedOfAttributes(SupportedType::kDrop));
    EXPECT_EQ(Qt::CopyAction | Qt::MoveAction, info.supportedOfAttributes(SupportedType::kDrag));

    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(ProxyFileInfo, supportedOfAttributes), []{ __DBG_STUB_INVOKE__ return Qt::IgnoreAction;});
    EXPECT_EQ(Qt::IgnoreAction, info.supportedOfAttributes(SupportedType::kCustomerSupport));

    info.refresh();
}

TEST_F(UT_TrashFileInfo, testTrashFileInfoNameOf)
{
    QUrl url(QUrl::fromLocalFile(QDir::currentPath() + QDir::separator() + "TrashFileInfoPrivateFileName.txt"));
    TrashFileInfo info(url);
    stub_ext::StubExt stub;
    stub.set_lamda(&TrashFileInfoPrivate::copyName, []{ __DBG_STUB_INVOKE__ return "ttttt";});
    info.d->targetUrl = url;
    stub.set_lamda(&FileUtils::isDesktopFile, []{ __DBG_STUB_INVOKE__ return true;});
    EXPECT_EQ("ttttt", info.nameOf(NameInfoType::kFileCopyName));

    stub.set_lamda(&FileUtils::isDesktopFile, []{ __DBG_STUB_INVOKE__ return false;});
    stub.set_lamda(VADDR(TrashFileInfo, displayOf), []{ __DBG_STUB_INVOKE__ return "ttttt";});
    EXPECT_EQ("ttttt", info.nameOf(NameInfoType::kFileCopyName));

    EXPECT_EQ("", info.nameOf(NameInfoType::kMimeTypeName));

    stub.set_lamda(VADDR(ProxyFileInfo, nameOf), []{ __DBG_STUB_INVOKE__ return "cutom";});
    EXPECT_EQ("cutom", info.nameOf(NameInfoType::kIconName));
}

TEST_F(UT_TrashFileInfo, testTrashFileInfoDisplayOf)
{
    TrashFileInfo infoRoot(FileUtils::trashRootUrl());
    EXPECT_EQ("Trash", infoRoot.displayOf(DisPlayInfoType::kFileDisplayName));

    QUrl url(QUrl::fromLocalFile(QDir::currentPath() + QDir::separator() + "TrashFileInfoPrivateFileName.txt"));
    TrashFileInfo info(url);
    stub_ext::StubExt stub;
    info.d->targetUrl = url;
    stub.set_lamda(&FileUtils::isDesktopFile, []{ __DBG_STUB_INVOKE__ return true;});
    EXPECT_EQ("TrashFileInfoPrivateFileName.txt", info.displayOf(DisPlayInfoType::kFileDisplayName));

    stub.set_lamda(&FileUtils::isDesktopFile, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_EQ("TrashFileInfoPrivateFileName.txt", info.displayOf(DisPlayInfoType::kFileDisplayName));

    stub.set_lamda(VADDR(ProxyFileInfo, displayOf), []{ __DBG_STUB_INVOKE__ return "cutom";});
    EXPECT_EQ("cutom", info.displayOf(DisPlayInfoType::kFileDisplayPinyinName));

    info.d->dFileInfo = nullptr;
    EXPECT_TRUE(info.displayOf(DisPlayInfoType::kFileDisplayName).isEmpty());
}

TEST_F(UT_TrashFileInfo, testTrashFileInfoPathOf)
{
    QUrl url(QUrl::fromLocalFile(QDir::currentPath() + QDir::separator() + "TrashFileInfoPrivateFileName.txt"));
    TrashFileInfo info(url);
    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(ProxyFileInfo, pathOf), []{ __DBG_STUB_INVOKE__ return "cutom";});
    EXPECT_TRUE(info.pathOf(PathInfoType::kSymLinkTarget).isEmpty());

    EXPECT_EQ("cutom", info.pathOf(PathInfoType::kCanonicalPath));
}

TEST_F(UT_TrashFileInfo, testTrashFileInfoUrlOf)
{
    QUrl url(QUrl::fromLocalFile(QDir::currentPath() + QDir::separator() + "TrashFileInfoPrivateFileName.txt"));
    TrashFileInfo info(url);
    stub_ext::StubExt stub;

    EXPECT_FALSE(info.urlOf(UrlInfoType::kRedirectedFileUrl).isValid());
    EXPECT_FALSE(info.urlOf(UrlInfoType::kOriginalUrl).isValid());
    EXPECT_EQ(url, info.urlOf(UrlInfoType::kUrl));

    stub.set_lamda(VADDR(ProxyFileInfo, urlOf), []{ __DBG_STUB_INVOKE__ return QUrl();});
    EXPECT_FALSE(info.urlOf(UrlInfoType::kParentUrl).isValid());
}

TEST_F(UT_TrashFileInfo, testTrashFileInfoCanAttributes)
{
    QUrl url(QUrl::fromLocalFile(QDir::currentPath() + QDir::separator() + "TrashFileInfoPrivateFileName.txt"));
    TrashFileInfo info(url);
    stub_ext::StubExt stub;
    EXPECT_FALSE(info.canAttributes(CanableInfoType::kCanDelete));
    EXPECT_FALSE(info.canAttributes(CanableInfoType::kCanTrash));
    EXPECT_FALSE(info.canAttributes(CanableInfoType::kCanRename));
    EXPECT_FALSE(info.canAttributes(CanableInfoType::kCanDrop));
    EXPECT_FALSE(info.canAttributes(CanableInfoType::kCanHidden));
    EXPECT_TRUE(info.canAttributes(CanableInfoType::kCanRedirectionFileUrl));
    EXPECT_FALSE(info.permissions().testFlag(QFileDevice::WriteOwner));

    stub.set_lamda(VADDR(ProxyFileInfo, canAttributes), []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(info.canAttributes(CanableInfoType::kCanDragCompress));

    info.d->dFileInfo = nullptr;
    EXPECT_FALSE(info.canAttributes(CanableInfoType::kCanDelete));
    EXPECT_FALSE(info.canAttributes(CanableInfoType::kCanTrash));
    EXPECT_FALSE(info.canAttributes(CanableInfoType::kCanRename));
}

TEST_F(UT_TrashFileInfo, testTrashFileInfofileIcon)
{
    QUrl url(QUrl::fromLocalFile(QDir::currentPath() + QDir::separator() + "TrashFileInfoPrivateFileName.txt"));
    TrashFileInfo info(url);
    stub_ext::StubExt stub;
    info.d->targetUrl = url;
    stub.set_lamda(&FileUtils::isDesktopFile, []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_TRUE(info.fileIcon().isNull());

    stub.set_lamda(&FileUtils::isDesktopFile, []{ __DBG_STUB_INVOKE__ return true;});
    EXPECT_FALSE(info.fileIcon().isNull());
}

TEST_F(UT_TrashFileInfo, testTrashFileInfoSize)
{
    TrashFileInfo infoRoot(FileUtils::trashRootUrl());
    stub_ext::StubExt stub;
    stub.set_lamda(&TrashCoreHelper::calculateTrashRoot, []{ __DBG_STUB_INVOKE__ return std::make_pair<qint64, int>(qint64(0), int(0));});
    EXPECT_TRUE(0 == infoRoot.size());

    QUrl url(QUrl::fromLocalFile(QDir::currentPath() + QDir::separator() + "TrashFileInfoPrivateFileName.txt"));
    TrashFileInfo info(url);
    EXPECT_TRUE(0 == info.size());

    info.d->dFileInfo = nullptr;
    EXPECT_TRUE(qint64() == info.size());
}

TEST_F(UT_TrashFileInfo, testTrashFileInfoCountChildFile)
{
    TrashFileInfo infoRoot(FileUtils::trashRootUrl());
    stub_ext::StubExt stub;
    stub.set_lamda(&DFileInfo::attribute,[]{ __DBG_STUB_INVOKE__ return QVariant(0);});
    EXPECT_TRUE(0 == infoRoot.countChildFile());

    QUrl url(QUrl::fromLocalFile(QDir::currentPath() + QDir::separator() + "TrashFileInfoPrivateFileName.txt"));
    TrashFileInfo info(url);
    EXPECT_TRUE(-1 == info.countChildFile());

    stub.set_lamda(VADDR(TrashFileInfo, isAttributes), []{ __DBG_STUB_INVOKE__ return true;});
    EXPECT_TRUE(0 == info.countChildFile());
}

TEST_F(UT_TrashFileInfo, testTrashFileInfoIsAttributes)
{
    TrashFileInfo infoRoot(FileUtils::trashRootUrl());
    stub_ext::StubExt stub;
    EXPECT_TRUE(infoRoot.isAttributes(OptInfoType::kIsDir));

    QUrl url(QUrl::fromLocalFile(QDir::currentPath() + QDir::separator() + "TrashFileInfoPrivateFileName.txt"));
    TrashFileInfo info(url);
    stub.set_lamda(VADDR(ProxyFileInfo, isAttributes), []{ __DBG_STUB_INVOKE__ return true;});
    EXPECT_TRUE(info.isAttributes(OptInfoType::kIsDir));

    EXPECT_FALSE(info.isAttributes(OptInfoType::kIsReadable));
    EXPECT_FALSE(info.isAttributes(OptInfoType::kIsWritable));

    info.d->targetUrl = url;
    EXPECT_TRUE(info.isAttributes(OptInfoType::kIsReadable));
    EXPECT_TRUE(info.isAttributes(OptInfoType::kIsWritable));

    EXPECT_FALSE(info.isAttributes(OptInfoType::kIsHidden));
    EXPECT_FALSE(info.isAttributes(OptInfoType::kIsSymLink));
    EXPECT_TRUE(info.isAttributes(OptInfoType::kIsRoot));

    info.d->dFileInfo = nullptr;
    EXPECT_FALSE(info.isAttributes(OptInfoType::kIsReadable));
    EXPECT_FALSE(info.isAttributes(OptInfoType::kIsWritable));
    EXPECT_FALSE(info.isAttributes(OptInfoType::kIsSymLink));
}

TEST_F(UT_TrashFileInfo, testTrashFileInfoTimeOf)
{
    QUrl url(QUrl::fromLocalFile(QDir::currentPath() + QDir::separator() + "TrashFileInfoPrivateFileName.txt"));
    TrashFileInfo info(url);
    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(ProxyFileInfo, timeOf), []{ __DBG_STUB_INVOKE__ return QVariant();});
    EXPECT_FALSE(info.timeOf(TimeInfoType::kBirthTimeMSecond).isValid());

    stub.set_lamda(&TrashFileInfoPrivate::lastRead, []{ __DBG_STUB_INVOKE__ return QDateTime();});
    stub.set_lamda(&TrashFileInfoPrivate::lastModified, []{ __DBG_STUB_INVOKE__ return QDateTime();});
    stub.set_lamda(&TrashFileInfoPrivate::deletionTime, []{ __DBG_STUB_INVOKE__ return QDateTime();});
    EXPECT_TRUE(info.timeOf(TimeInfoType::kLastRead).isValid());
    EXPECT_TRUE(info.timeOf(TimeInfoType::kLastModified).isValid());
    EXPECT_TRUE(info.timeOf(TimeInfoType::kDeletionTime).isValid());

    EXPECT_TRUE(info.customData(dfmbase::Global::kItemFileOriginalPath).toString().isEmpty());
    EXPECT_TRUE(info.customData(dfmbase::Global::kItemFileDeletionDate).toString().isEmpty());
    EXPECT_FALSE(info.customData(dfmbase::Global::kItemFileRefreshIcon).isValid());
}
