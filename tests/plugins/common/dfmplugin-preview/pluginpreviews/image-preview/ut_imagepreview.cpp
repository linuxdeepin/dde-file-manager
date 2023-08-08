// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "imagepreview.h"
#include "imageview.h"

#include <gtest/gtest.h>

#include <QByteArray>
#include <QImageReader>
#include <QLabel>
#include <QByteArray>

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/utils/fileutils.h>

DFMBASE_USE_NAMESPACE
PREVIEW_USE_NAMESPACE

TEST(UT_imagePreview, canPreview_one)
{
    ImagePreview view;
    QByteArray arry;
    bool isOk = view.canPreview(QUrl("file:///UT_TEST"), &arry);

    EXPECT_FALSE(isOk);
}

TEST(UT_imagePreview, canPreview_two)
{
    stub_ext::StubExt stub;
    typedef QByteArray(*FuncType)(const QString &);
    stub.set_lamda(static_cast<FuncType>(&QImageReader::imageFormat), []{
        return QByteArray("png");
    });

    ImagePreview view;
    QByteArray arry("png");
    bool isOk = view.canPreview(QUrl("file:///UT_TEST"), &arry);

    EXPECT_TRUE(isOk);
}

TEST(UT_imagePreview, initialize)
{
    QWidget window, statusBar;
    ImagePreview view;
    view.initialize(&window, &statusBar);

    EXPECT_FALSE(view.messageStatusBar.isNull());
}

TEST(UT_imagePreview, setFileUrl_one)
{
    ImagePreview view;
    view.currentFileUrl = QUrl("file:///UT_TEST");
    bool isOk = view.setFileUrl(QUrl("file:///UT_TEST"));

    EXPECT_TRUE(isOk);
}

TEST(UT_imagePreview, setFileUrl_two)
{
    ImagePreview view;
    bool isOk = view.setFileUrl(QUrl("file:///UT_TEST"));

    EXPECT_FALSE(isOk);
}

TEST(UT_imagePreview, setFileUrl_three)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&InfoFactory::create<FileInfo>, [] {
        return QSharedPointer<SyncFileInfo>(new SyncFileInfo(QUrl("file:///UT_TEST")));
    });
    stub.set_lamda(VADDR(SyncFileInfo, canAttributes), []{
        return true;
    });
    stub.set_lamda(&FileUtils::isLocalFile, []{
        return false;
    });

    ImagePreview view;
    bool isOk = view.setFileUrl(QUrl("file:///UT_TEST1"));

    EXPECT_FALSE(isOk);
}

TEST(UT_imagePreview, setFileUrl_four)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&InfoFactory::create<FileInfo>, [] {
        return QSharedPointer<SyncFileInfo>(new SyncFileInfo(QUrl("file:///UT_TEST")));
    });
    stub.set_lamda(VADDR(SyncFileInfo, canAttributes), []{
        return true;
    });
    stub.set_lamda(&FileUtils::isLocalFile, []{
        return true;
    });
    stub.set_lamda(&ImagePreview::canPreview, []{
        return false;
    });

    ImagePreview view;
    bool isOk = view.setFileUrl(QUrl("file:///UT_TEST1"));

    EXPECT_FALSE(isOk);
}

TEST(UT_imagePreview, setFileUrl_five)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&InfoFactory::create<FileInfo>, [] {
        return QSharedPointer<SyncFileInfo>(new SyncFileInfo(QUrl("file:///UT_TEST")));
    });
    stub.set_lamda(VADDR(SyncFileInfo, canAttributes), []{
        return true;
    });
    stub.set_lamda(&FileUtils::isLocalFile, []{
        return true;
    });
    stub.set_lamda(&ImagePreview::canPreview, []{
        return true;
    });

    ImagePreview view;
    QLabel *statusBar = new QLabel;
    view.messageStatusBar = statusBar;
    bool isOk = view.setFileUrl(QUrl("file:///UT_TEST1"));

    EXPECT_TRUE(isOk);
}

TEST(UT_imagePreview, setFileUrl_six)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&InfoFactory::create<FileInfo>, [] {
        return QSharedPointer<SyncFileInfo>(new SyncFileInfo(QUrl("file:///UT_TEST")));
    });
    stub.set_lamda(VADDR(SyncFileInfo, canAttributes), []{
        return true;
    });
    stub.set_lamda(&FileUtils::isLocalFile, []{
        return true;
    });
    stub.set_lamda(&ImagePreview::canPreview, []{
        return true;
    });
    typedef bool(*FuncType)(const QUrl &);
    stub.set_lamda((FuncType)(bool(ImagePreview::*)(const QUrl &))&ImagePreview::setFileUrl, []{
        return true;
    });

    ImagePreview view;
    QLabel *statusBar = new QLabel;
    view.messageStatusBar = statusBar;
    ImageView v("file:///UT_TEST2", QByteArray(""));
    bool isOk = view.setFileUrl(QUrl("file:///UT_TEST1"));

    EXPECT_TRUE(isOk);
}

TEST(UT_imagePreview, fileUrl)
{
    QUrl url("file:///UT_TEST");

    ImagePreview view;
    view.currentFileUrl = url;

    EXPECT_TRUE(view.fileUrl() == url);
}

TEST(UT_imagePreview, contentWidget)
{
    stub_ext::StubExt stub;
    typedef bool(*FuncType)(const QUrl &);
    stub.set_lamda((FuncType)(bool(ImagePreview::*)(const QUrl &))&ImagePreview::setFileUrl, []{
        return true;
    });

    ImageView v("file:///UT_TEST2", QByteArray(""));
    ImagePreview view;
    view.imageView = &v;

    EXPECT_TRUE(view.contentWidget() == &v);
}

TEST(UT_imagePreview, title)
{
    QString title("UT_TEST");

    ImagePreview view;
    view.imageTitle = title;

    EXPECT_TRUE(view.title() == title);
}
