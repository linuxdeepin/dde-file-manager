// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "musicpreview.h"
#include "musicmessageview.h"
#include "toolbarframe.h"

#include <gtest/gtest.h>

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/file/local/syncfileinfo.h>

DFMBASE_USE_NAMESPACE
PREVIEW_USE_NAMESPACE

TEST(UT_MusicPreview, setFileUrl_one)
{
    MusicPreview preview;
    preview.currentUrl = QUrl("file:///UT_TEST");
    bool re = preview.setFileUrl(QUrl("file:///UT_TEST"));

    EXPECT_TRUE(re);
}

TEST(UT_MusicPreview, setFileUrl_two)
{
    MusicPreview preview;
    bool re = preview.setFileUrl(QUrl("dfmvault:///UT_TEST"));

    EXPECT_FALSE(re);
}

TEST(UT_MusicPreview, setFileUrl_three)
{
    MusicPreview preview;
    MusicMessageView view;
    preview.musicView = &view;
    bool re = preview.setFileUrl(QUrl("file:///UT_TEST"));
    preview.musicView = nullptr;

    EXPECT_FALSE(re);
}

TEST(UT_MusicPreview, setFileUrl_four)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&MusicPreview::canPreview, []{
        return false;
    });

    MusicPreview preview;
    bool re = preview.setFileUrl(QUrl("file:///UT_TEST"));

    EXPECT_FALSE(re);
}

TEST(UT_MusicPreview, setFileUrl_five)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&MusicPreview::canPreview, []{
        return true;
    });

    MusicPreview preview;
    bool re = preview.setFileUrl(QUrl("file:///UT_TEST"));

    EXPECT_TRUE(re);
}

TEST(UT_MusicPreview, fileUrl)
{
    QUrl url("file:///UT_TEST");
    MusicPreview preview;
    preview.currentUrl = url;

    EXPECT_TRUE(preview.fileUrl() == url);
}

TEST(UT_MusicPreview, contentWidget)
{
    MusicMessageView view;
    MusicPreview preview;
    preview.musicView = &view;

    EXPECT_TRUE(preview.contentWidget() == &view);
    preview.musicView = nullptr;
}

TEST(UT_MusicPreview, statusBarWidget)
{
    ToolBarFrame tool("/UT_TEST");
    MusicPreview preview;
    preview.statusBarFrame = &tool;

    EXPECT_TRUE(preview.statusBarWidget() == &tool);
}

TEST(UT_MusicPreview, statusBarWidgetAlignment)
{
    MusicPreview preview;

    EXPECT_TRUE(preview.statusBarWidgetAlignment() == Qt::Alignment());
}

TEST(UT_MusicPreview, play)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&ToolBarFrame::play, [ &isOk ]{
        isOk = true;
    });

    MusicPreview preview;
    ToolBarFrame tool("/UT_TEST");
    preview.statusBarFrame = &tool;
    preview.play();

    EXPECT_TRUE(isOk);
}

TEST(UT_MusicPreview, pause)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&ToolBarFrame::pause, [ &isOk ]{
        isOk = true;
    });

    MusicPreview preview;
    ToolBarFrame tool("/UT_TEST");
    preview.statusBarFrame = &tool;
    preview.pause();

    EXPECT_TRUE(isOk);
}

TEST(UT_MusicPreview, stop)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&ToolBarFrame::stop, [ &isOk ]{
        isOk = true;
    });

    MusicPreview preview;
    ToolBarFrame tool("/UT_TEST");
    preview.statusBarFrame = &tool;
    preview.stop();

    EXPECT_TRUE(isOk);
}

TEST(UT_MusicPreview, canPreview_one)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&InfoFactory::create<FileInfo>, [] {
        return QSharedPointer<SyncFileInfo>(nullptr);
    });

    MusicPreview preview;
    bool re = preview.canPreview(QUrl("file:///UT_TEST"));

    EXPECT_FALSE(re);
}

TEST(UT_MusicPreview, canPreview_two)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&InfoFactory::create<FileInfo>, [] {
        return QSharedPointer<SyncFileInfo>(new SyncFileInfo(QUrl("file:///UT_TEST")));
    });

    MusicPreview preview;
    bool re = preview.canPreview(QUrl("file:///UT_TEST"));

    EXPECT_FALSE(re);
}
