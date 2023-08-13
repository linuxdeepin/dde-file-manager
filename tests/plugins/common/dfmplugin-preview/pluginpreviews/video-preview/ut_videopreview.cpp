// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "videopreview.h"

#include <gtest/gtest.h>

PREVIEW_USE_NAMESPACE

TEST(UT_videoPreview, setFileUrl_one)
{
    stub_ext::StubExt stub;
    typedef bool(dmr::PlayerEngine::*FuncType)(const QUrl &);
    stub.set_lamda(static_cast<FuncType>(&dmr::PlayerEngine::isPlayableFile), []{
        return false;
    });

    VideoPreview preview;

    EXPECT_FALSE(preview.setFileUrl(QUrl("file:///UT_TEST")));
}

TEST(UT_videoPreview, setFileUrl_two)
{
    dmr::MovieInfo info;

    stub_ext::StubExt stub;
    typedef bool(dmr::PlayerEngine::*FuncType)(const QUrl &);
    stub.set_lamda(static_cast<FuncType>(&dmr::PlayerEngine::isPlayableFile), []{
        return true;
    });
    stub.set_lamda(&dmr::MovieInfo::parseFromFile, [ &info ](const QFileInfo &fi, bool *ok){
        *ok = false;
        return info;
    });

    VideoPreview preview;

    EXPECT_FALSE(preview.setFileUrl(QUrl("file:///UT_TEST")));
}

TEST(UT_videoPreview, setFileUrl_three)
{
    dmr::MovieInfo info;
    stub_ext::StubExt stub;
    typedef bool(dmr::PlayerEngine::*FuncType)(const QUrl &);
    stub.set_lamda(static_cast<FuncType>(&dmr::PlayerEngine::isPlayableFile), []{
        return true;
    });
    stub.set_lamda(&dmr::MovieInfo::parseFromFile, [ &info ](const QFileInfo &fi, bool *ok){
        *ok = true;
        return info;
    });

    VideoPreview preview;

    EXPECT_TRUE(preview.setFileUrl(QUrl("file:///UT_TEST")));
}

TEST(UT_videoPreview, fileUrl)
{
    QUrl url("file:///UT_TEST");
    VideoPreview preview;
    preview.videoUrl = url;

    EXPECT_TRUE(preview.fileUrl() == url);
}

TEST(UT_videoPreview, contentWidget)
{
    VideoPreview preview;

    EXPECT_TRUE(preview.contentWidget());
}

TEST(UT_videoPreview, statusBarWidget)
{
    VideoPreview preview;

    EXPECT_TRUE(preview.statusBarWidget());
}

TEST(UT_videoPreview, showStatusBarSeparator)
{
    VideoPreview preview;

    EXPECT_FALSE(preview.showStatusBarSeparator());
}

TEST(UT_videoPreview, statusBarWidgetAlignment)
{
    VideoPreview preview;

    EXPECT_TRUE(preview.statusBarWidgetAlignment() == Qt::Alignment());
}

TEST(UT_videoPreview, play)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&dmr::PlayerWidget::play, [ &isOk ]{
        isOk = true;
    });

    VideoPreview preview;
    preview.videoUrl = QUrl("file:///UT_TEST");
    preview.play();

    EXPECT_TRUE(isOk);
}

TEST(UT_videoPreview, pause)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&dmr::PlayerEngine::pauseResume, [ &isOk ]{
        isOk = true;
    });

    VideoPreview preview;
    preview.pause();

    EXPECT_TRUE(isOk);
}

TEST(UT_videoPreview, stop)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&dmr::PlayerEngine::stop, [ &isOk ]{
        isOk = true;
    });

    VideoPreview preview;
    preview.stop();

    EXPECT_TRUE(isOk);
}
