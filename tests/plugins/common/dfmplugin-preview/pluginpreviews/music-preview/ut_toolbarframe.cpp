// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "toolbarframe.h"

#include <gtest/gtest.h>

#include <DSlider>

#include <QTimer>
#include <QLabel>

PREVIEW_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

TEST(UT_toolBarFrame, init)
{
    ToolBarFrame bar("/UT_TEST");

    EXPECT_TRUE(bar.mediaPlayer);
}

TEST(UT_toolBarFrame, onPlayDurationChanged)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&ToolBarFrame::durationToLabel, [ &isOk ]{
        isOk = true;
    });

    ToolBarFrame bar("/UT_TEST");
    bar.onPlayDurationChanged(1);

    EXPECT_TRUE(isOk);
}

TEST(UT_toolBarFrame, onPlayStateChanged_one)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&ToolBarFrame::stop, [ &isOk ]{
        isOk = true;
    });

    ToolBarFrame bar("/UT_TEST");
    bar.onPlayStateChanged(QMediaPlayer::StoppedState);

    EXPECT_TRUE(isOk);
}

TEST(UT_toolBarFrame, onPlayStateChanged_two)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&ToolBarFrame::stop, [ &isOk ]{
        isOk = true;
    });

    ToolBarFrame bar("/UT_TEST");
    bar.onPlayStateChanged(QMediaPlayer::PlayingState);

    EXPECT_FALSE(isOk);
}

TEST(UT_toolBarFrame, onPlayStatusChanged)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&ToolBarFrame::durationToLabel, [ &isOk ]{
        isOk = true;
    });

    ToolBarFrame bar("/UT_TEST");
    bar.onPlayStatusChanged(QMediaPlayer::LoadedMedia);

    EXPECT_TRUE(isOk);
}

TEST(UT_toolBarFrame, onPlayControlButtonClicked_one)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&QMediaPlayer::state, []{
        return QMediaPlayer::PlayingState;
    });
    stub.set_lamda(&ToolBarFrame::pause, [ &isOk ]{
        isOk = true;
    });

    ToolBarFrame bar("/UT_TEST");
    bar.onPlayControlButtonClicked();

    EXPECT_TRUE(isOk);
}

TEST(UT_toolBarFrame, onPlayControlButtonClicked_two)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&QMediaPlayer::state, []{
        return QMediaPlayer::StoppedState;
    });
    stub.set_lamda(&ToolBarFrame::play, [ &isOk ]{
        isOk = true;
    });

    ToolBarFrame bar("/UT_TEST");
    bar.onPlayControlButtonClicked();

    EXPECT_TRUE(isOk);
}

TEST(UT_toolBarFrame, onPlayControlButtonClicked_three)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&QMediaPlayer::state, []{
        return QMediaPlayer::PausedState;
    });
    stub.set_lamda(&ToolBarFrame::play, [ &isOk ]{
        isOk = true;
    });

    ToolBarFrame bar("/UT_TEST");
    bar.onPlayControlButtonClicked();

    EXPECT_TRUE(isOk);
}

TEST(UT_toolBarFrame, updateProgress)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&DSlider::setValue, [ &isOk ]{
        isOk = true;
    });

    ToolBarFrame bar("/UT_TEST");
    bar.updateProgress();

    EXPECT_TRUE(isOk);
}

TEST(UT_toolBarFrame, seekPosition)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&QMediaPlayer::setPosition, [ &isOk ]{
        isOk = true;
    });

    ToolBarFrame bar("/UT_TEST");
    bar.seekPosition(10);

    EXPECT_TRUE(isOk);
}

TEST(UT_toolBarFrame, play)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&QMediaPlayer::play, [ &isOk ]{
        isOk = true;
    });
    typedef void(QTimer::*FuncType)();
    stub.set_lamda(static_cast<FuncType>(&QTimer::start), []{});

    ToolBarFrame bar("/UT_TEST");
    bar.play();

    EXPECT_TRUE(isOk);
}

TEST(UT_toolBarFrame, pause)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&QMediaPlayer::pause, [ &isOk ]{
        isOk = true;
    });
    stub.set_lamda(&QTimer::stop, []{});

    ToolBarFrame bar("/UT_TEST");
    bar.pause();

    EXPECT_TRUE(isOk);
}

TEST(UT_toolBarFrame, stop)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&QMediaPlayer::stop, [ &isOk ]{
        isOk = true;
    });
    stub.set_lamda(&QTimer::stop, []{});

    ToolBarFrame bar("/UT_TEST");
    bar.stop();

    EXPECT_TRUE(isOk);
}

TEST(UT_toolBarFrame, durationToLabel_one)
{
    ToolBarFrame bar("/UT_TEST");
    bar.durationToLabel(9111);

    EXPECT_FALSE(bar.durationLabel->text().isEmpty());
}

TEST(UT_toolBarFrame, durationToLabel_two)
{
    ToolBarFrame bar("/UT_TEST");
    bar.durationToLabel(700111);

    EXPECT_FALSE(bar.durationLabel->text().isEmpty());
}
