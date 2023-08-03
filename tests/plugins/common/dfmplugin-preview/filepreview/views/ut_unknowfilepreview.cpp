// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "views/unknowfilepreview.h"

#include <gtest/gtest.h>

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/file/local/syncfileinfo.h>

DPFILEPREVIEW_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

TEST(UT_UnknowFilePreview, setFileUrl_one)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&InfoFactory::create<FileInfo>, [] {
        return QSharedPointer<SyncFileInfo>(new SyncFileInfo(QUrl("file:///UT_TEST")));
    });
    stub.set_lamda(&UnknowFilePreview::setFileInfo, []{});


    UnknowFilePreview view;
    bool isOk = view.setFileUrl(QUrl("file:///UT_TEST"));

    EXPECT_TRUE(isOk);
}

TEST(UT_UnknowFilePreview, setFileUrl_two)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&InfoFactory::create<FileInfo>, [] {
        return QSharedPointer<SyncFileInfo>(nullptr);
    });
    stub.set_lamda(&UnknowFilePreview::setFileInfo, []{});


    UnknowFilePreview view;
    bool isOk = view.setFileUrl(QUrl("file:///UT_TEST"));

    EXPECT_FALSE(isOk);
}

TEST(UT_UnknowFilePreview, fileUrl)
{
    UnknowFilePreview view;
    view.setFileUrl(QUrl("file:///UT_TEST"));

    EXPECT_TRUE(view.fileUrl() == QUrl("file:///UT_TEST"));
}

TEST(UT_UnknowFilePreview, contentWidget)
{
    UnknowFilePreview view;

    EXPECT_TRUE(view.contentWidget());
}

TEST(UT_UnknowFilePreview, setFileInfo_one)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(SyncFileInfo, isAttributes), [ &isOk ](void*, const OptInfoType type){
        isOk = true;
        if (type == OptInfoType::kIsFile)
            return true;
        return false;
    });

    FileInfoPointer info = QSharedPointer<SyncFileInfo>(new SyncFileInfo(QUrl("file:///UT_TEST")));
    UnknowFilePreview view;
    view.setFileInfo(info);

    EXPECT_TRUE(isOk);
}

TEST(UT_UnknowFilePreview, setFileInfo_two)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(SyncFileInfo, isAttributes), [ &isOk ](void*, const OptInfoType type){
        isOk = true;
        if (type == OptInfoType::kIsDir)
            return true;
        return false;
    });

    FileInfoPointer info = QSharedPointer<SyncFileInfo>(new SyncFileInfo(QUrl("file:///UT_TEST")));
    UnknowFilePreview view;
    view.setFileInfo(info);

    EXPECT_TRUE(isOk);
}

TEST(UT_UnknowFilePreview, updateFolderSizeCount)
{
    UnknowFilePreview view;
    view.updateFolderSizeCount(1, 1, 1);

    EXPECT_FALSE(view.sizeLabel->text().isEmpty());
}
