// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "toolbarframe.h"
#include "cusmediaplayer.h"
#include "mediawork.h"
#include "cover.h"

#include <gtest/gtest.h>

#include <DSlider>

#include <QApplication>
#include <QPainter>
#include <QEvent>
#include <QLabel>
#include <QPushButton>
#include <QSignalSpy>
#include <QSlider>
#include <QTemporaryDir>
#include <QThread>
#include <QUrl>

#include <atomic>

using namespace plugin_filepreview;

static std::atomic<int> g_mediaWorkPlayCount { 0 };

class UT_ToolBarFrame : public testing::Test
{
protected:
    void SetUp() override
    {
        frame = new ToolBarFrame(QUrl::fromLocalFile(dir.filePath("song.mp3")).toString());
    }

    void TearDown() override
    {
        delete frame;
        frame = nullptr;
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        qApp->processEvents();
        stub.clear();
    }

protected:
    ToolBarFrame *frame { nullptr };
    QTemporaryDir dir;
    stub_ext::StubExt stub;
};

TEST_F(UT_ToolBarFrame, Constructor_CreatesAllUiElements)
{
    EXPECT_NE(frame->playControlButton, nullptr);
    EXPECT_NE(frame->progressSlider, nullptr);
    EXPECT_NE(frame->durationLabel, nullptr);
    EXPECT_EQ(frame->playControlButton->accessibleName(), QString("PlayControlButton"));
    EXPECT_EQ(frame->progressSlider->accessibleName(), QString("ProgressSlider"));
    EXPECT_EQ(frame->curState, QMediaPlayer::StoppedState);
}

TEST_F(UT_ToolBarFrame, DurationToLabel_ZeroDuration_ShowsPlaceholderAndDisablesSlider)
{
    frame->durationToLabel(0);
    EXPECT_EQ(frame->durationLabel->text(), QString("--"));
    EXPECT_EQ(frame->progressSlider->minimum(), 0);
    EXPECT_EQ(frame->progressSlider->maximum(), 0);
}

TEST_F(UT_ToolBarFrame, DurationToLabel_NegativeDuration_ShowsPlaceholder)
{
    frame->durationToLabel(-5);
    EXPECT_EQ(frame->durationLabel->text(), QString("--"));
}

TEST_F(UT_ToolBarFrame, DurationToLabel_65Seconds_ShowsFormattedTime)
{
    frame->durationToLabel(65000);
    EXPECT_EQ(frame->durationLabel->text(), QString("01: 05"));
    EXPECT_EQ(frame->progressSlider->maximum(), 65000);
}

TEST_F(UT_ToolBarFrame, DurationToLabel_TenMinutes_ShowsTwoDigitMinutes)
{
    frame->durationToLabel(599000);
    EXPECT_EQ(frame->durationLabel->text(), QString("09: 59"));
}

TEST_F(UT_ToolBarFrame, DurationToLabel_ExactMinute_ShowsDoubleZeroSeconds)
{
    frame->durationToLabel(60000);
    EXPECT_EQ(frame->durationLabel->text(), QString("01: 00"));
}

TEST_F(UT_ToolBarFrame, OnPlayDurationChanged_ForwardsToDurationLabel)
{
    frame->onPlayDurationChanged(125000);
    EXPECT_EQ(frame->durationLabel->text(), QString("02: 05"));
}

TEST_F(UT_ToolBarFrame, OnPlayStateChanged_Playing_ShowsPauseIcon)
{
    frame->onPlayDurationChanged(1000);
    frame->onPlayStateChanged(QMediaPlayer::PlayingState);
    EXPECT_TRUE(frame->controlButtonShowsPause);
    EXPECT_EQ(frame->curState, QMediaPlayer::PlayingState);
}

TEST_F(UT_ToolBarFrame, OnPlayStateChanged_Stopped_ResetsSlider)
{
    frame->onPlayDurationChanged(1000);
    frame->progressSlider->setValue(500);
    frame->onPlayStateChanged(QMediaPlayer::StoppedState);
    EXPECT_EQ(frame->progressSlider->value(), 0);
    EXPECT_FALSE(frame->controlButtonShowsPause);
    EXPECT_EQ(frame->curState, QMediaPlayer::StoppedState);
}

TEST_F(UT_ToolBarFrame, OnPlayStateChanged_Paused_ShowsPlayIcon)
{
    frame->onPlayStateChanged(QMediaPlayer::PausedState);
    EXPECT_FALSE(frame->controlButtonShowsPause);
    EXPECT_EQ(frame->curState, QMediaPlayer::PausedState);
}

TEST_F(UT_ToolBarFrame, OnPlayPositionChanged_Playing_UpdatesSliderValue)
{
    frame->onPlayStateChanged(QMediaPlayer::PlayingState);
    frame->onPlayDurationChanged(1000);
    frame->onPlayPositionChanged(300);
    EXPECT_EQ(frame->progressSlider->value(), 300);
}

TEST_F(UT_ToolBarFrame, OnPlayPositionChanged_SamePositionTwice_UpdatesOnlyOnce)
{
    frame->onPlayStateChanged(QMediaPlayer::PlayingState);
    frame->onPlayDurationChanged(1000);
    frame->onPlayPositionChanged(300);
    frame->onPlayPositionChanged(300);
    EXPECT_EQ(frame->progressSlider->value(), 300);
    EXPECT_EQ(frame->lastPos, 300);
}

TEST_F(UT_ToolBarFrame, OnPlayPositionChanged_StoppedState_Ignored)
{
    frame->onPlayDurationChanged(1000);
    frame->onPlayPositionChanged(300);
    EXPECT_EQ(frame->progressSlider->value(), 0);
}

TEST_F(UT_ToolBarFrame, OnPlayStatusChanged_LoadedMedia_ShowsDurationFromPlayer)
{
    stub.set_lamda(&CusMediaPlayer::duration, [](const CusMediaPlayer *) -> qint64 {
        __DBG_STUB_INVOKE__
        return 65000;
    });
    frame->onPlayStatusChanged(QMediaPlayer::LoadedMedia);
    EXPECT_EQ(frame->durationLabel->text(), QString("01: 05"));
}

TEST_F(UT_ToolBarFrame, OnPlayStatusChanged_OtherStatus_LabelUntouched)
{
    frame->onPlayStatusChanged(QMediaPlayer::LoadingMedia);
    EXPECT_TRUE(frame->durationLabel->text().isEmpty());
}

TEST_F(UT_ToolBarFrame, OnPlayControlButtonClicked_PlayingState_PausesPlayback)
{
    stub.set_lamda(&CusMediaPlayer::state, [](const CusMediaPlayer *) -> QMediaPlayer::PlaybackState {
        __DBG_STUB_INVOKE__
        return QMediaPlayer::PlayingState;
    });
    QSignalSpy pauseSpy(CusMediaPlayer::instance(), &CusMediaPlayer::sigPause);
    QSignalSpy playSpy(CusMediaPlayer::instance(), &CusMediaPlayer::sigPlay);

    frame->onPlayControlButtonClicked();
    EXPECT_EQ(pauseSpy.count(), 1);
    EXPECT_EQ(playSpy.count(), 0);
    EXPECT_EQ(frame->curState, QMediaPlayer::PausedState);
    EXPECT_FALSE(frame->controlButtonShowsPause);
}

TEST_F(UT_ToolBarFrame, OnPlayControlButtonClicked_StoppedState_PlaysFromStart)
{
    stub.set_lamda(&CusMediaPlayer::state, [](const CusMediaPlayer *) -> QMediaPlayer::PlaybackState {
        __DBG_STUB_INVOKE__
        return QMediaPlayer::StoppedState;
    });
    frame->onPlayDurationChanged(1000);
    frame->progressSlider->setValue(500);
    QSignalSpy playSpy(CusMediaPlayer::instance(), &CusMediaPlayer::sigPlay);

    frame->onPlayControlButtonClicked();
    EXPECT_EQ(playSpy.count(), 1);
    EXPECT_EQ(frame->progressSlider->value(), 0);
    EXPECT_EQ(frame->curState, QMediaPlayer::PlayingState);
}

TEST_F(UT_ToolBarFrame, OnPlayControlButtonClicked_PausedState_ResumesPlayback)
{
    stub.set_lamda(&CusMediaPlayer::state, [](const CusMediaPlayer *) -> QMediaPlayer::PlaybackState {
        __DBG_STUB_INVOKE__
        return QMediaPlayer::PausedState;
    });
    QSignalSpy playSpy(CusMediaPlayer::instance(), &CusMediaPlayer::sigPlay);

    frame->onPlayControlButtonClicked();
    EXPECT_EQ(playSpy.count(), 1);
    EXPECT_EQ(frame->curState, QMediaPlayer::PlayingState);
}

TEST_F(UT_ToolBarFrame, Play_EmitsSignalAndSwitchesToPlayingState)
{
    QSignalSpy spy(CusMediaPlayer::instance(), &CusMediaPlayer::sigPlay);
    frame->play();
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(frame->curState, QMediaPlayer::PlayingState);
    EXPECT_TRUE(frame->controlButtonShowsPause);
}

TEST_F(UT_ToolBarFrame, Pause_EmitsSignalAndSwitchesToPausedState)
{
    QSignalSpy spy(CusMediaPlayer::instance(), &CusMediaPlayer::sigPause);
    frame->pause();
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(frame->curState, QMediaPlayer::PausedState);
    EXPECT_FALSE(frame->controlButtonShowsPause);
}

TEST_F(UT_ToolBarFrame, Stop_EmitsSignalResetsSliderAndState)
{
    frame->onPlayDurationChanged(1000);
    frame->progressSlider->setValue(500);
    QSignalSpy spy(CusMediaPlayer::instance(), &CusMediaPlayer::sigStop);

    frame->stop();
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(frame->progressSlider->value(), 0);
    EXPECT_EQ(frame->curState, QMediaPlayer::StoppedState);
    EXPECT_FALSE(frame->controlButtonShowsPause);
}

TEST_F(UT_ToolBarFrame, SeekPosition_BeyondThreshold_EmitsSetPosition)
{
    stub.set_lamda(&CusMediaPlayer::position, [](const CusMediaPlayer *) -> qint64 {
        __DBG_STUB_INVOKE__
        return 0;
    });
    QSignalSpy spy(CusMediaPlayer::instance(), &CusMediaPlayer::sigSetPosition);

    frame->seekPosition(50);
    EXPECT_EQ(spy.count(), 1);
    ASSERT_EQ(spy.size(), 1);
    EXPECT_EQ(spy.at(0).at(0).toInt(), 50);
}

TEST_F(UT_ToolBarFrame, SeekPosition_WithinThreshold_DoesNotEmit)
{
    stub.set_lamda(&CusMediaPlayer::position, [](const CusMediaPlayer *) -> qint64 {
        __DBG_STUB_INVOKE__
        return 0;
    });
    QSignalSpy spy(CusMediaPlayer::instance(), &CusMediaPlayer::sigSetPosition);

    frame->seekPosition(2);
    frame->seekPosition(3);
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(UT_ToolBarFrame, SliderValueChanged_ConnectedToSeekPosition)
{
    stub.set_lamda(&CusMediaPlayer::position, [](const CusMediaPlayer *) -> qint64 {
        __DBG_STUB_INVOKE__
        return 0;
    });
    frame->onPlayDurationChanged(1000);
    QSignalSpy spy(CusMediaPlayer::instance(), &CusMediaPlayer::sigSetPosition);

    frame->progressSlider->setValue(80);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_ToolBarFrame, EventFilter_EnterOnPlayButton_SetsHovered)
{
    QEvent event(QEvent::Enter);
    frame->eventFilter(frame->playControlButton, &event);
    EXPECT_TRUE(frame->playControlButtonHovered);
}

TEST_F(UT_ToolBarFrame, EventFilter_LeaveOnPlayButton_ClearsHovered)
{
    frame->playControlButtonHovered = true;
    QEvent event(QEvent::Leave);
    frame->eventFilter(frame->playControlButton, &event);
    EXPECT_FALSE(frame->playControlButtonHovered);
}

TEST_F(UT_ToolBarFrame, EventFilter_StyleChange_RefreshesIconWithoutCrash)
{
    QEvent styleEvent(QEvent::StyleChange);
    frame->eventFilter(frame->playControlButton, &styleEvent);
    QEvent paletteEvent(QEvent::PaletteChange);
    frame->eventFilter(frame->playControlButton, &paletteEvent);
    QEvent appPaletteEvent(QEvent::ApplicationPaletteChange);
    frame->eventFilter(frame->playControlButton, &appPaletteEvent);
    SUCCEED();
}

TEST_F(UT_ToolBarFrame, EventFilter_OtherObject_DoesNotAffectHoverState)
{
    QWidget other;
    QEvent event(QEvent::Enter);
    frame->eventFilter(&other, &event);
    EXPECT_FALSE(frame->playControlButtonHovered);
}

TEST_F(UT_ToolBarFrame, UpdatePlayButtonIcon_KeepsIconSizeConfigured)
{
    frame->controlButtonShowsPause = true;
    frame->updatePlayButtonIcon();
    EXPECT_EQ(frame->playControlButton->iconSize(), QSize(16, 16));
}

class UT_Cover : public testing::Test
{
protected:
    void SetUp() override
    {
        cover = new Cover();
        cover->resize(40, 40);
    }

    void TearDown() override
    {
        delete cover;
        cover = nullptr;
    }

    QImage grabCover()
    {
        QPixmap shot(cover->size());
        shot.fill(Qt::transparent);
        QPainter painter(&shot);
        cover->render(&painter, QPoint(), QRegion(), QWidget::RenderFlags(QWidget::DrawChildren));
        return shot.toImage();
    }

protected:
    Cover *cover { nullptr };
};

TEST_F(UT_Cover, Constructor_TranslucentBackgroundEnabled)
{
    EXPECT_TRUE(cover->testAttribute(Qt::WA_TranslucentBackground));
}

TEST_F(UT_Cover, PaintEvent_NoBackground_FillsWhite)
{
    const QImage &shot = grabCover();
    const QColor &center = shot.pixelColor(shot.width() / 2, shot.height() / 2);
    EXPECT_EQ(center.rgb(), qRgb(255, 255, 255));
    EXPECT_EQ(center.alpha(), 255);
}

TEST_F(UT_Cover, SetCoverPixmap_WithPixmap_DrawsScaledPixmap)
{
    QPixmap pixmap(8, 8);
    pixmap.fill(Qt::red);
    cover->setCoverPixmap(pixmap);

    const QImage &shot = grabCover();
    EXPECT_EQ(shot.pixelColor(shot.width() / 2, shot.height() / 2).rgb(), qRgb(255, 0, 0));
}

TEST_F(UT_Cover, PaintEvent_RoundedCorners_CornerOutsideClipIsTransparent)
{
    QPixmap pixmap(8, 8);
    pixmap.fill(Qt::red);
    cover->setCoverPixmap(pixmap);

    const QImage &shot = grabCover();
    EXPECT_EQ(shot.pixelColor(0, 0).alpha(), 0);
    EXPECT_EQ(shot.pixelColor(shot.width() - 1, 0).alpha(), 0);
    EXPECT_EQ(shot.pixelColor(0, shot.height() - 1).alpha(), 0);
    EXPECT_EQ(shot.pixelColor(shot.width() - 1, shot.height() - 1).alpha(), 0);
}

class UT_CusMediaPlayer : public testing::Test
{
protected:
    void SetUp() override
    {
        g_mediaWorkPlayCount = 0;
        stub.set_lamda(&MediaWork::createMediaPlayer, [](MediaWork *) { __DBG_STUB_INVOKE__ });
        stub.set_lamda(&MediaWork::setMedia, [](MediaWork *, const QUrl &) { __DBG_STUB_INVOKE__ });
        stub.set_lamda(&MediaWork::setPosition, [](MediaWork *, qint64) { __DBG_STUB_INVOKE__ });
        stub.set_lamda(&MediaWork::pause, [](MediaWork *) { __DBG_STUB_INVOKE__ });
        stub.set_lamda(&MediaWork::stop, [](MediaWork *) { __DBG_STUB_INVOKE__ });
        stub.set_lamda(&MediaWork::play, [](MediaWork *) { __DBG_STUB_INVOKE__ ++g_mediaWorkPlayCount; });
    }

    void TearDown() override
    {
        qApp->processEvents();
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
};

TEST_F(UT_CusMediaPlayer, Instance_ReturnsSameSingleton)
{
    EXPECT_NE(CusMediaPlayer::instance(), nullptr);
    EXPECT_EQ(CusMediaPlayer::instance(), CusMediaPlayer::instance());
}

TEST_F(UT_CusMediaPlayer, CreateMediaPlayer_EmitsCreateSignalAtMostOnce)
{
    CusMediaPlayer *player = CusMediaPlayer::instance();
    QSignalSpy spy(player, &CusMediaPlayer::sigCreateMediaPlayer);
    player->createMediaPlayer();
    player->createMediaPlayer();
    player->createMediaPlayer();
    EXPECT_LE(spy.count(), 1);
}

TEST_F(UT_CusMediaPlayer, SetMedia_EmitsStopAndSetMediaSignals)
{
    CusMediaPlayer *player = CusMediaPlayer::instance();
    QSignalSpy stopSpy(player, &CusMediaPlayer::sigStop);
    QSignalSpy setSpy(player, &CusMediaPlayer::sigSetMedia);
    const QUrl url = QUrl::fromLocalFile("/tmp/song.mp3");

    player->setMedia(url);
    EXPECT_EQ(stopSpy.count(), 1);
    EXPECT_EQ(setSpy.count(), 1);
    ASSERT_EQ(setSpy.size(), 1);
    EXPECT_EQ(setSpy.at(0).at(0).toUrl(), url);
}

TEST_F(UT_CusMediaPlayer, Duration_ForwardsToWorker)
{
    stub.set_lamda(&MediaWork::duration, [](const MediaWork *) -> qint64 {
        __DBG_STUB_INVOKE__
        return 4242;
    });
    EXPECT_EQ(CusMediaPlayer::instance()->duration(), 4242);
}

TEST_F(UT_CusMediaPlayer, State_ForwardsToWorker)
{
    stub.set_lamda(&MediaWork::state, [](const MediaWork *) -> QMediaPlayer::PlaybackState {
        __DBG_STUB_INVOKE__
        return QMediaPlayer::PlayingState;
    });
    EXPECT_EQ(CusMediaPlayer::instance()->state(), QMediaPlayer::PlayingState);
}

TEST_F(UT_CusMediaPlayer, Position_ForwardsToWorker)
{
    stub.set_lamda(&MediaWork::position, [](const MediaWork *) -> qint64 {
        __DBG_STUB_INVOKE__
        return 777;
    });
    EXPECT_EQ(CusMediaPlayer::instance()->position(), 777);
}

TEST_F(UT_CusMediaPlayer, SignalPlay_IsDeliveredToWorkerThread)
{
    CusMediaPlayer *player = CusMediaPlayer::instance();
    emit player->sigPlay();

    bool delivered = false;
    for (int i = 0; i < 100 && !delivered; ++i) {
        QThread::msleep(20);
        delivered = g_mediaWorkPlayCount.load() >= 1;
    }
    EXPECT_TRUE(delivered);
}

static QUrl g_capturedSourceUrl;
static qint64 g_capturedPosition { -1 };
static std::atomic<int> g_playerPlayCount { 0 };
static std::atomic<int> g_playerPauseCount { 0 };
static std::atomic<int> g_playerStopCount { 0 };

class UT_MediaWork : public testing::Test
{
protected:
    void SetUp() override
    {
        g_capturedSourceUrl = QUrl();
        g_capturedPosition = -1;
        g_playerPlayCount = 0;
        g_playerPauseCount = 0;
        g_playerStopCount = 0;
        work = new MediaWork();
    }

    void TearDown() override
    {
        delete work;
        work = nullptr;
        stub.clear();
    }

protected:
    MediaWork *work { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_MediaWork, QueryMethods_BeforePlayerCreated_ReturnDefaults)
{
    EXPECT_EQ(work->duration(), -1);
    EXPECT_EQ(work->position(), -1);
    EXPECT_EQ(work->state(), QMediaPlayer::StoppedState);
}

TEST_F(UT_MediaWork, ControlSlots_BeforePlayerCreated_AreSafeNoOps)
{
    work->setMedia(QUrl::fromLocalFile("/tmp/song.mp3"));
    work->setPosition(1000);
    work->play();
    work->pause();
    work->stop();
    EXPECT_EQ(work->state(), QMediaPlayer::StoppedState);
    EXPECT_EQ(work->duration(), -1);
    EXPECT_EQ(work->position(), -1);
}

TEST_F(UT_MediaWork, CreateMediaPlayer_CreatesPlayerWithInitialValues)
{
    work->createMediaPlayer();
    ASSERT_NE(work->mediaPlayer, nullptr);
    EXPECT_EQ(work->state(), QMediaPlayer::StoppedState);
    EXPECT_EQ(work->position(), 0);
    EXPECT_EQ(work->duration(), 0);
}

TEST_F(UT_MediaWork, CreateMediaPlayer_ConnectsPositionSignal)
{
    work->createMediaPlayer();
    QSignalSpy spy(work, &MediaWork::playerPositionChanged);

    const bool invoked = QMetaObject::invokeMethod(work->mediaPlayer, "positionChanged", Q_ARG(qint64, 7));
    EXPECT_TRUE(invoked);
    EXPECT_EQ(spy.count(), 1);
    ASSERT_EQ(spy.size(), 1);
    EXPECT_EQ(spy.at(0).at(0).toLongLong(), qint64(7));
}

TEST_F(UT_MediaWork, CreateMediaPlayer_ConnectsDurationSignal)
{
    work->createMediaPlayer();
    QSignalSpy spy(work, &MediaWork::playerDurationChanged);

    const bool invoked = QMetaObject::invokeMethod(work->mediaPlayer, "durationChanged", Q_ARG(qint64, 9));
    EXPECT_TRUE(invoked);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_MediaWork, CreateMediaPlayer_ConnectsStatusSignal)
{
    work->createMediaPlayer();
    QSignalSpy spy(work, &MediaWork::playerStatusChanged);

    const bool invoked = QMetaObject::invokeMethod(
            work->mediaPlayer, "mediaStatusChanged",
            Q_ARG(QMediaPlayer::MediaStatus, QMediaPlayer::LoadedMedia));
    EXPECT_TRUE(invoked);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_MediaWork, CreateMediaPlayer_ConnectsStateSignal)
{
    work->createMediaPlayer();
    QSignalSpy spy(work, &MediaWork::playerStateChanged);

    const bool invoked = QMetaObject::invokeMethod(
            work->mediaPlayer, "playbackStateChanged",
            Q_ARG(QMediaPlayer::PlaybackState, QMediaPlayer::PlayingState));
    EXPECT_TRUE(invoked);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_MediaWork, SetMedia_AfterPlayerCreated_ForwardsSourceToPlayer)
{
    work->createMediaPlayer();
    stub.set_lamda(&QMediaPlayer::setSource, [](QMediaPlayer *, const QUrl &url) {
        __DBG_STUB_INVOKE__
        g_capturedSourceUrl = url;
    });
    const QUrl url = QUrl::fromLocalFile("/tmp/forwarded.mp3");

    work->setMedia(url);
    EXPECT_EQ(g_capturedSourceUrl, url);
}

TEST_F(UT_MediaWork, SetPosition_AfterPlayerCreated_ForwardsPositionToPlayer)
{
    work->createMediaPlayer();
    stub.set_lamda(&QMediaPlayer::setPosition, [](QMediaPlayer *, qint64 pos) {
        __DBG_STUB_INVOKE__
        g_capturedPosition = pos;
    });

    work->setPosition(1234);
    EXPECT_EQ(g_capturedPosition, 1234);
}

TEST_F(UT_MediaWork, Play_AfterPlayerCreated_ForwardsToPlayer)
{
    work->createMediaPlayer();
    stub.set_lamda(&QMediaPlayer::play, [](QMediaPlayer *) {
        __DBG_STUB_INVOKE__
        ++g_playerPlayCount;
    });

    work->play();
    EXPECT_EQ(g_playerPlayCount.load(), 1);
}

TEST_F(UT_MediaWork, Pause_AfterPlayerCreated_ForwardsToPlayer)
{
    work->createMediaPlayer();
    stub.set_lamda(&QMediaPlayer::pause, [](QMediaPlayer *) {
        __DBG_STUB_INVOKE__
        ++g_playerPauseCount;
    });

    work->pause();
    EXPECT_EQ(g_playerPauseCount.load(), 1);
}

TEST_F(UT_MediaWork, Stop_AfterPlayerCreated_ForwardsToPlayer)
{
    work->createMediaPlayer();
    stub.set_lamda(&QMediaPlayer::stop, [](QMediaPlayer *) {
        __DBG_STUB_INVOKE__
        ++g_playerStopCount;
    });

    work->stop();
    EXPECT_EQ(g_playerStopCount.load(), 1);
}
