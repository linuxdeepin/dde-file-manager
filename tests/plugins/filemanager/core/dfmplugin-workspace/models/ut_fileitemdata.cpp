// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "plugins/filemanager/core/dfmplugin-workspace/models/fileitemdata.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/thumbnail/thumbnailfactory.h>
#include <dfm-base/interfaces/sortfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/syncfileinfo.h>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DPWORKSPACE_USE_NAMESPACE

class UT_FileItemData : public testing::Test
{
protected:
    void SetUp() override
    {
        UrlRoute::regScheme(Global::Scheme::kFile, "/", QIcon(), false, QObject::tr("System Disk"));
        UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/", QIcon(), false,
                            QObject::tr("System Disk"));

        InfoFactory::regClass<dfmbase::SyncFileInfo>(Global::Scheme::kFile);

        QUrl url(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
        url.setScheme(Scheme::kFile);

        itemData = new FileItemData(url);
    }

    void TearDown() override
    {
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
    FileItemData *itemData;
};

TEST_F(UT_FileItemData, SetParentData){
    FileItemData *parent = new FileItemData(QUrl());
    itemData->setParentData(parent);

    EXPECT_NE(itemData->parent, nullptr);
}

TEST_F(UT_FileItemData, SetSortFileInfo){
    SortInfoPointer info(new SortFileInfo);
    itemData->setSortFileInfo(info);

    EXPECT_FALSE(itemData->sortInfo.isNull());
}

TEST_F(UT_FileItemData, RefreshInfo){
    bool fileRefreshed = false;
    stub.set_lamda(VADDR(FileInfo, refresh), [&fileRefreshed]{ fileRefreshed = true; });

    itemData->refreshInfo();
    EXPECT_FALSE(fileRefreshed);

    QUrl url(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
    url.setScheme(Scheme::kFile);
    itemData->info = InfoFactory::create<FileInfo>(url);

    itemData->refreshInfo();
//    EXPECT_TRUE(fileRefreshed);
}

TEST_F(UT_FileItemData, ClearThumbnail){
    QVariant thumbnailValue;
    stub.set_lamda(VADDR(FileInfo, setExtendedAttributes), [&thumbnailValue](FileInfo*, const ExtInfoType &key, const QVariant &value){
        if (key == ExtInfoType::kFileThumbnail)
            thumbnailValue = QVariant("clearThumbnail");
    });

    itemData->clearThumbnail();
    EXPECT_FALSE(thumbnailValue.isValid());

    QUrl url(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
    url.setScheme(Scheme::kFile);
    itemData->info = InfoFactory::create<FileInfo>(url);

    itemData->clearThumbnail();
    EXPECT_EQ(thumbnailValue.toString(), "clearThumbnail");
}

TEST_F(UT_FileItemData, FileInfo){
    QUrl url(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
    url.setScheme(Scheme::kFile);
    FileInfoPointer info = InfoFactory::create<FileInfo>(url);
    itemData->info = info;

    EXPECT_EQ(itemData->fileInfo(), info);
}

TEST_F(UT_FileItemData, ParentData){
    FileItemData *parent = new FileItemData(QUrl());
    itemData->setParentData(parent);

    EXPECT_EQ(itemData->parentData(), parent);
}

TEST_F(UT_FileItemData, FileIcon){
    QIcon fileIcon = itemData->fileIcon();
    EXPECT_EQ(fileIcon.name(), "empty");

    QUrl url(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
    url.setScheme(Scheme::kFile);
    itemData->info = InfoFactory::create<FileInfo>(url);

    QVariant thumbnailValue;
    QVariant invalidValue;
    stub.set_lamda(VADDR(FileInfo, setExtendedAttributes), [&thumbnailValue](FileInfo*, const ExtInfoType &key, const QVariant &){
        if (key == ExtInfoType::kFileThumbnail)
            thumbnailValue = QVariant("setThumbnail");
    });
    stub.set_lamda(VADDR(FileInfo, extendAttributes), [&thumbnailValue, &invalidValue](FileInfo*, const ExtInfoType &key){
        if (key == ExtInfoType::kFileThumbnail)
            return thumbnailValue;
        return invalidValue;
    });
    QIcon defaultIcon("defaultIcon");
    stub.set_lamda(VADDR(FileInfo, fileIcon), [&defaultIcon]{ return defaultIcon; });
    bool joinedJob = false;
    stub.set_lamda(ADDR(ThumbnailFactory, joinThumbnailJob), [=, &joinedJob](ThumbnailFactory*, const QUrl &url, ThumbnailSize size){
        if (UniversalUtils::urlEquals(url, itemData->url))
            joinedJob = true;
    });

    fileIcon = itemData->fileIcon();
    EXPECT_TRUE(joinedJob);
    EXPECT_EQ(thumbnailValue.toString(), "setThumbnail");
//    EXPECT_EQ(fileIcon.name(), defaultIcon.name());

    thumbnailValue.clear();
    thumbnailValue.setValue(QIcon("thumbnailIcon"));

    fileIcon = itemData->fileIcon();
    EXPECT_EQ(fileIcon.name(), thumbnailValue.value<QIcon>().name());
}

TEST_F(UT_FileItemData, CreateFileInfoData){
    itemData->data(kItemCreateFileInfoRole);
    EXPECT_FALSE(itemData->info.isNull());
}

TEST_F(UT_FileItemData, FilePathData){
    QString path = itemData->data(kItemFilePathRole).toString();
    EXPECT_EQ(path, itemData->url.path());

    itemData->data(kItemCreateFileInfoRole);
    path = itemData->data(kItemFilePathRole).toString();
    EXPECT_EQ(path, itemData->info->displayOf(DisPlayInfoType::kFileDisplayPath));
}

TEST_F(UT_FileItemData, FileLastModifiedData){
    QVariant data = itemData->data(kItemFileLastModifiedRole);
    EXPECT_EQ(data.toString(), "-");

    itemData->data(kItemCreateFileInfoRole);
    data = itemData->data(kItemFileLastModifiedRole).toString();

    QDateTime timeData = itemData->info->timeOf(TimeInfoType::kLastModified).value<QDateTime>();
    if (timeData.isValid()) {
        EXPECT_EQ(data.toString(), timeData.toString(FileUtils::dateTimeFormat()));
    } else {
        EXPECT_EQ(data.toString(), "-");
    }
}

TEST_F(UT_FileItemData, IconData){
    QIcon defaultIcon("defaultIcon");
    stub.set_lamda(ADDR(FileItemData, fileIcon), [&defaultIcon]{
        return defaultIcon;
    });

    QIcon fileIcon = itemData->data(kItemIconRole).value<QIcon>();
    EXPECT_EQ(fileIcon.name(), defaultIcon.name());
}

TEST_F(UT_FileItemData, FileSizeData){
    QVariant data = itemData->data(kItemFileSizeRole);
    EXPECT_EQ(data.toString(), "-");

    itemData->data(kItemCreateFileInfoRole);
    data = itemData->data(kItemFileSizeRole);
    EXPECT_EQ(data.toString(), itemData->info->displayOf(DisPlayInfoType::kSizeDisplayName));
}

TEST_F(UT_FileItemData, FileMimeTypeData){
    QVariant data = itemData->data(kItemFileMimeTypeRole);
    EXPECT_TRUE(data.toString().isEmpty());

    itemData->data(kItemCreateFileInfoRole);
    data = itemData->data(kItemFileMimeTypeRole);
    EXPECT_EQ(data.toString(), itemData->info->displayOf(DisPlayInfoType::kMimeTypeDisplayName));
}

TEST_F(UT_FileItemData, NameData){
    QVariant data = itemData->data(kItemNameRole);
    EXPECT_EQ(data.toString(), itemData->url.fileName());

    itemData->data(kItemCreateFileInfoRole);
    data = itemData->data(kItemNameRole);
    EXPECT_EQ(data.toString(), itemData->info->nameOf(NameInfoType::kFileName));
}

TEST_F(UT_FileItemData, DisplayNameData){
    QVariant data = itemData->data(kItemFileDisplayNameRole);
    EXPECT_EQ(data.toString(), itemData->url.fileName());

    itemData->data(kItemCreateFileInfoRole);
    data = itemData->data(kItemFileDisplayNameRole);
    EXPECT_EQ(data.toString(), itemData->info->displayOf(DisPlayInfoType::kFileDisplayName));
}

TEST_F(UT_FileItemData, LastReadData){
    QVariant data = itemData->data(kItemFileLastReadRole);
    EXPECT_TRUE(data.toString().isEmpty());

    itemData->data(kItemCreateFileInfoRole);
    data = itemData->data(kItemFileLastReadRole);
    EXPECT_EQ(data.toString(), itemData->info->customData(dfmbase::Global::kItemFileLastReadRole).toString());
}

TEST_F(UT_FileItemData, PinyinNameData){
    QVariant data = itemData->data(kItemFilePinyinNameRole);
    EXPECT_EQ(data.toString(), itemData->url.fileName());

    itemData->data(kItemCreateFileInfoRole);
    data = itemData->data(kItemFilePinyinNameRole);
    EXPECT_EQ(data.toString(), itemData->info->displayOf(DisPlayInfoType::kFileDisplayPinyinName));
}

TEST_F(UT_FileItemData, BaseNameData){
    QVariant data = itemData->data(kItemFileBaseNameRole);
    EXPECT_EQ(data.toString(), itemData->url.fileName());

    itemData->data(kItemCreateFileInfoRole);
    data = itemData->data(kItemFileBaseNameRole);
    EXPECT_EQ(data.toString(), itemData->info->nameOf(NameInfoType::kCompleteBaseName));
}

TEST_F(UT_FileItemData, SuffixData){
    QVariant data = itemData->data(kItemFileSuffixRole);
    EXPECT_EQ(data.toString(), itemData->url.fileName());

    itemData->data(kItemCreateFileInfoRole);
    data = itemData->data(kItemFileSuffixRole);
    EXPECT_EQ(data.toString(), itemData->info->nameOf(NameInfoType::kSuffix));
}

TEST_F(UT_FileItemData, NameOfRenameData){
    QVariant data = itemData->data(kItemFileNameOfRenameRole);
    EXPECT_EQ(data.toString(), itemData->url.fileName());

    itemData->data(kItemCreateFileInfoRole);
    data = itemData->data(kItemFileNameOfRenameRole);
    EXPECT_EQ(data.toString(), itemData->info->nameOf(NameInfoType::kFileNameOfRename));
}

TEST_F(UT_FileItemData, BaseNameOfRenameData){
    QVariant data = itemData->data(kItemFileBaseNameOfRenameRole);
    EXPECT_EQ(data.toString(), itemData->url.fileName());

    itemData->data(kItemCreateFileInfoRole);
    data = itemData->data(kItemFileBaseNameOfRenameRole);
    EXPECT_EQ(data.toString(), itemData->info->nameOf(NameInfoType::kBaseNameOfRename));
}

TEST_F(UT_FileItemData, SuffixOfRenameData){
    QVariant data = itemData->data(kItemFileSuffixOfRenameRole);
    EXPECT_EQ(data.toString(), itemData->url.fileName());

    itemData->data(kItemCreateFileInfoRole);
    data = itemData->data(kItemFileSuffixOfRenameRole);
    EXPECT_EQ(data.toString(), itemData->info->nameOf(NameInfoType::kSuffixOfRename));
}

TEST_F(UT_FileItemData, UrlData){
    QVariant data = itemData->data(kItemUrlRole);
    EXPECT_EQ(data.toUrl(), itemData->url);

    itemData->data(kItemCreateFileInfoRole);
    data = itemData->data(kItemUrlRole);
    EXPECT_EQ(data.toUrl(), itemData->info->urlOf(UrlInfoType::kUrl));
}

TEST_F(UT_FileItemData, IsWritableData){
    QVariant data = itemData->data(kItemFileIsWritableRole);
    EXPECT_TRUE(data.toBool());

    SortInfoPointer info(new SortFileInfo);
    itemData->setSortFileInfo(info);
    data = itemData->data(kItemFileIsWritableRole);
    EXPECT_EQ(data.toBool(), info->isWriteable());

    itemData->data(kItemCreateFileInfoRole);
    data = itemData->data(kItemFileIsWritableRole);
    EXPECT_EQ(data.toBool(), itemData->info->isAttributes(OptInfoType::kIsWritable));
}

TEST_F(UT_FileItemData, IsDirData){
    QVariant data = itemData->data(kItemFileIsDirRole);
    EXPECT_TRUE(data.toBool());

    SortInfoPointer info(new SortFileInfo);
    itemData->setSortFileInfo(info);
    data = itemData->data(kItemFileIsDirRole);
    EXPECT_EQ(data.toBool(), info->isDir());

    itemData->data(kItemCreateFileInfoRole);
    data = itemData->data(kItemFileIsDirRole);
    EXPECT_EQ(data.toBool(), itemData->info->isAttributes(OptInfoType::kIsDir));
}

TEST_F(UT_FileItemData, CanRenameData){
    QVariant data = itemData->data(kItemFileCanRenameRole);
    EXPECT_TRUE(data.toBool());

    itemData->data(kItemCreateFileInfoRole);
    data = itemData->data(kItemFileCanRenameRole);
    EXPECT_EQ(data.toBool(), itemData->info->canAttributes(CanableInfoType::kCanRename));
}

TEST_F(UT_FileItemData, CanDropData){
    QVariant data = itemData->data(kItemFileCanDropRole);
    EXPECT_TRUE(data.toBool());

    itemData->data(kItemCreateFileInfoRole);
    data = itemData->data(kItemFileCanDropRole);
    EXPECT_EQ(data.toBool(), itemData->info->canAttributes(CanableInfoType::kCanDrop));
}

TEST_F(UT_FileItemData, CanDragData){
    QVariant data = itemData->data(kItemFileCanDragRole);
    EXPECT_TRUE(data.toBool());

    itemData->data(kItemCreateFileInfoRole);
    data = itemData->data(kItemFileCanDragRole);
    EXPECT_EQ(data.toBool(), itemData->info->canAttributes(CanableInfoType::kCanDrag));
}

TEST_F(UT_FileItemData, SizeIntData){
    QVariant data = itemData->data(kItemFileSizeIntRole);
    EXPECT_EQ(data.toInt(), 0);

    SortInfoPointer info(new SortFileInfo);
    itemData->setSortFileInfo(info);
    data = itemData->data(kItemFileSizeIntRole);
    EXPECT_EQ(data.toInt(), info->fileSize());

    itemData->data(kItemCreateFileInfoRole);
    data = itemData->data(kItemFileSizeIntRole);
    EXPECT_EQ(data.toInt(), itemData->info->size());
}

TEST_F(UT_FileItemData, AvailableStateData){
    itemData->setAvailableState(true);
    EXPECT_TRUE(itemData->data(kItemFileIsAvailableRole).toBool());

    itemData->setAvailableState(false);
    EXPECT_FALSE(itemData->data(kItemFileIsAvailableRole).toBool());
}
