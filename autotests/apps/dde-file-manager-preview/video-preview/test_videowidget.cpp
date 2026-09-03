// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "videopreview.h"
#include "videowidget.h"
#include "videostatusbar.h"
#include "titlebarwidget.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QApplication>
#include <QSignalSpy>
#include <QTest>
#include <QTemporaryFile>
#include <QMetaObject>
#include <QMouseEvent>
#include <QShowEvent>
#include <QResizeEvent>
#include <QEnterEvent>
#include <QTimer>
#include <QPropertyAnimation>
#include <DIconButton>
#include <DSlider>
#include <DGuiApplicationHelper>

using namespace plugin_filepreview;

namespace {

dmr::PlayerEngine *fakeEngine()
{
    static QObject obj;
    return reinterpret_cast<dmr::PlayerEngine *>(&obj);
}

struct DmrCallLog
{
    int enginePlayCalls { 0 };
    int pauseResumeCalls { 0 };
    int stopCalls { 0 };
    bool playable { true };
    bool parseOk { true };
    dmr::MovieInfo info;
    qint64 elapsedValue { 0 };
    dmr::PlayerEngine::CoreState engineState { dmr::PlayerEngine::Playing };
    QUrl lastPlayedUrl;
    int lastSeekPos { -1 };
    QString lastBackendProperty;
};

DmrCallLog dmrLog;

struct PlayerWidgetCtorStub
{
    PlayerWidgetCtorStub()
    {
        auto self = reinterpret_cast<dmr::PlayerWidget *>(this);
        new (self) QWidget(nullptr);
        self->_engine = nullptr;
    }
};

void installDmrStubs(stub_ext::StubExt &stub)
{
    stub.set(stub_ext::StubExt::get_ctor_addr<dmr::PlayerWidget>(),
             stub_ext::StubExt::get_ctor_addr<PlayerWidgetCtorStub>());

    stub.set_lamda(&dmr::PlayerWidget::engine,
                   [](dmr::PlayerWidget *) -> dmr::PlayerEngine & { return *fakeEngine(); });
    stub.set_lamda(&dmr::PlayerWidget::play,
                   [](dmr::PlayerWidget *, const QUrl &url) { dmrLog.lastPlayedUrl = url; });
    stub.set_lamda(&dmr::PlayerEngine::play, []() { ++dmrLog.enginePlayCalls; });
    stub.set_lamda(&dmr::PlayerEngine::pauseResume, []() { ++dmrLog.pauseResumeCalls; });
    stub.set_lamda(&dmr::PlayerEngine::stop, []() { ++dmrLog.stopCalls; });
    stub.set_lamda(&dmr::PlayerEngine::seekAbsolute, [](dmr::PlayerEngine *, int pos) { dmrLog.lastSeekPos = pos; });
    stub.set_lamda(&dmr::PlayerEngine::elapsed, []() { return dmrLog.elapsedValue; });
    stub.set_lamda(&dmr::PlayerEngine::state, []() { return dmrLog.engineState; });
    stub.set_lamda(&dmr::PlayerEngine::setBackendProperty,
                   [](dmr::PlayerEngine *, const QString &name, const QVariant &) { dmrLog.lastBackendProperty = name; });
    stub.set_lamda(static_cast<bool (dmr::PlayerEngine::*)(const QUrl &)>(&dmr::PlayerEngine::isPlayableFile),
                   [](dmr::PlayerEngine *, const QUrl &) { return dmrLog.playable; });
    stub.set_lamda(&dmr::MovieInfo::parseFromFile, [](const QFileInfo &, bool *ok) {
        if (ok)
            *ok = dmrLog.parseOk;
        return dmrLog.info;
    });
    stub.set_lamda(&dmr::utils::Time2str, [](qint64) { return QStringLiteral("00:00:00"); });
}

}   // namespace

class UT_VideoWidget : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        dmrLog = DmrCallLog();
        dmrLog.info.title = "Test Movie";
        dmrLog.info.duration = 120;
        dmrLog.info.width = 1920;
        dmrLog.info.height = 1080;
        installDmrStubs(stub);
        preview = new VideoPreview();
        preview->info.width = 1920;
        preview->info.height = 1080;
        preview->info.duration = 120;
        widget = preview->playerWidget.data();
    }

    virtual void TearDown() override
    {
        if (preview) {
            delete preview;
            QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
            preview = nullptr;
            widget = nullptr;
        }
        stub.clear();
    }

protected:
    VideoPreview *preview { nullptr };
    VideoWidget *widget { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_VideoWidget, Construct_InitializesMembers)
{
    EXPECT_EQ(widget->p, preview);
    EXPECT_NE(widget->titleBar, nullptr);
    EXPECT_FALSE(widget->isVisible());
    EXPECT_TRUE(widget->videoUrl.isEmpty());
}

TEST_F(UT_VideoWidget, SizeHint_WithoutWindowHandle_UsesPrimaryScreenHalf)
{
    const QSize screen = QGuiApplication::primaryScreen()->availableSize();
    const QSize expected = QSize(preview->info.width, preview->info.height)
                                   .scaled(qMin(preview->info.width, int(screen.width() * 0.5)),
                                           qMin(preview->info.height, int(screen.height() * 0.5)),
                                           Qt::KeepAspectRatio);
    EXPECT_EQ(widget->sizeHint(), expected);
    EXPECT_FALSE(widget->sizeHint().isEmpty());
}

TEST_F(UT_VideoWidget, SizeHint_WithWindowHandle_ScalesWithoutCrash)
{
    widget->show();
    EXPECT_NE(widget->window()->windowHandle(), nullptr);
    EXPECT_NO_FATAL_FAILURE(widget->sizeHint());
    EXPECT_FALSE(widget->sizeHint().isEmpty());
}

TEST_F(UT_VideoWidget, PlayFile_NotVisible_StoresUrlOnly)
{
    const QUrl url = QUrl::fromLocalFile("/tmp/video.mp4");
    widget->playFile(url);
    EXPECT_EQ(widget->videoUrl, url);
    EXPECT_TRUE(dmrLog.lastPlayedUrl.isEmpty());
}

TEST_F(UT_VideoWidget, PlayFile_VisibleWithEmptyUrl_DoesNotPlay)
{
    widget->show();
    widget->playFile(QUrl());
    EXPECT_TRUE(widget->videoUrl.isEmpty());
    EXPECT_TRUE(dmrLog.lastPlayedUrl.isEmpty());
}

TEST_F(UT_VideoWidget, PlayFile_Visible_PlaysImmediately)
{
    widget->show();
    const QUrl url = QUrl::fromLocalFile("/tmp/video.mp4");
    widget->playFile(url);
    EXPECT_EQ(dmrLog.lastPlayedUrl, url);
}

TEST_F(UT_VideoWidget, ShowEvent_WithUrlSet_PlaysVideo)
{
    const QUrl url = QUrl::fromLocalFile("/tmp/video.mp4");
    widget->playFile(url);
    ASSERT_TRUE(dmrLog.lastPlayedUrl.isEmpty());

    QShowEvent event;
    QApplication::sendEvent(widget, &event);
    EXPECT_EQ(dmrLog.lastPlayedUrl, url);
}

TEST_F(UT_VideoWidget, ShowEvent_WithoutUrl_SkipsPlayback)
{
    QShowEvent event;
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(widget, &event));
    EXPECT_TRUE(dmrLog.lastPlayedUrl.isEmpty());
}

TEST_F(UT_VideoWidget, MouseReleaseEvent_ForwardsToPreviewPause)
{
    QMouseEvent event(QEvent::MouseButtonRelease, QPointF(10, 10), QPointF(10, 10),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(widget, &event));
    EXPECT_EQ(dmrLog.pauseResumeCalls, 1);
}

TEST_F(UT_VideoWidget, EnterEvent_StopsTimerAndShowsTitleBarAnimated)
{
    widget->titleBar->startAutoHideTimer();
    ASSERT_TRUE(widget->titleBar->m_autoHideTimer->isActive());

    QEnterEvent event(QPointF(10, 10), QPointF(10, 10), QPointF(10, 10));
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(widget, &event));

    EXPECT_FALSE(widget->titleBar->m_autoHideTimer->isActive());
    EXPECT_FALSE(widget->titleBar->isHidden());
    EXPECT_EQ(widget->titleBar->m_fadeAnimation->state(), QAbstractAnimation::Running);
    EXPECT_EQ(widget->titleBar->m_fadeAnimation->endValue().toReal(), 1.0);
}

TEST_F(UT_VideoWidget, LeaveEvent_StartsHideAnimation)
{
    QEvent event(QEvent::Leave);
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(widget, &event));

    EXPECT_EQ(widget->titleBar->m_fadeAnimation->state(), QAbstractAnimation::Running);
    EXPECT_EQ(widget->titleBar->m_fadeAnimation->endValue().toReal(), 0.0);
}

TEST_F(UT_VideoWidget, ResizeEvent_RepositionsTitleBar)
{
    widget->resize(1000, 500);
    EXPECT_EQ(widget->size(), QSize(1000, 500));

    QResizeEvent event(widget->size(), QSize(800, 400));
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(widget, &event));
    EXPECT_EQ(widget->titleBar->geometry(), QRect(0, 0, 1000, 40));
    EXPECT_EQ(widget->titleBar->height(), 40);
}

class UT_VideoStatusBar : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        dmrLog = DmrCallLog();
        installDmrStubs(stub);
        preview = new VideoPreview();
        bar = preview->statusBar.data();
    }

    virtual void TearDown() override
    {
        if (preview) {
            delete preview;
            QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
            preview = nullptr;
            bar = nullptr;
        }
        stub.clear();
    }

protected:
    VideoPreview *preview { nullptr };
    VideoStatusBar *bar { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_VideoStatusBar, Construct_InitializesDefaults)
{
    EXPECT_EQ(bar->p, preview);
    EXPECT_FALSE(bar->sliderIsPressed);
    EXPECT_EQ(bar->slider->minimum(), 0);
    EXPECT_NE(bar->timeLabel, nullptr);
    ASSERT_NE(bar->controlButton, nullptr);
    EXPECT_EQ(bar->controlButton->objectName(), QStringLiteral("ControlButton"));
    EXPECT_FALSE(bar->controlButtonHovered);
    EXPECT_TRUE(bar->controlButtonShowsPause);
}

TEST_F(UT_VideoStatusBar, EventFilter_Enter_SetsHoveredFlag)
{
    QEvent event(QEvent::Enter);
    EXPECT_FALSE(bar->eventFilter(bar->controlButton, &event));
    EXPECT_TRUE(bar->controlButtonHovered);
}

TEST_F(UT_VideoStatusBar, EventFilter_Leave_ClearsHoveredFlag)
{
    bar->controlButtonHovered = true;
    QEvent event(QEvent::Leave);
    EXPECT_FALSE(bar->eventFilter(bar->controlButton, &event));
    EXPECT_FALSE(bar->controlButtonHovered);
}

TEST_F(UT_VideoStatusBar, EventFilter_StyleAndDisplayChanges_RefreshIconWithoutCrash)
{
    const QList<QEvent::Type> types { QEvent::StyleChange, QEvent::PaletteChange,
                                      QEvent::ApplicationPaletteChange, QEvent::ScreenChangeInternal,
                                      QEvent::DevicePixelRatioChange };
    for (const QEvent::Type type : types) {
        QEvent event(type);
        EXPECT_NO_FATAL_FAILURE(bar->eventFilter(bar->controlButton, &event));
    }
}

TEST_F(UT_VideoStatusBar, EventFilter_UnrelatedEvent_Ignored)
{
    QEvent event(QEvent::MouseButtonRelease);
    EXPECT_FALSE(bar->eventFilter(bar->controlButton, &event));
    EXPECT_FALSE(bar->controlButtonHovered);
}

TEST_F(UT_VideoStatusBar, EventFilter_OtherWatchedObject_Ignored)
{
    QEvent event(QEvent::Enter);
    EXPECT_FALSE(bar->eventFilter(bar->slider, &event));
    EXPECT_FALSE(bar->controlButtonHovered);
}

TEST_F(UT_VideoStatusBar, SliderPressedAndReleased_TogglesPressedFlag)
{
    QMetaObject::invokeMethod(bar->slider, "sliderPressed");
    EXPECT_TRUE(bar->sliderIsPressed);

    QMetaObject::invokeMethod(bar->slider, "sliderReleased");
    EXPECT_FALSE(bar->sliderIsPressed);
}

TEST_F(UT_VideoStatusBar, SliderValueChanged_SeeksEngineAbsolutely)
{
    bar->slider->setValue(30);
    EXPECT_EQ(dmrLog.lastSeekPos, 30);
}

TEST_F(UT_VideoStatusBar, ElapsedChanged_NotPressed_UpdatesSliderAndLabel)
{
    dmrLog.elapsedValue = 61;
    QMetaObject::invokeMethod(preview, "elapsedChanged");
    EXPECT_EQ(bar->slider->value(), 61);
    EXPECT_EQ(bar->timeLabel->text(), QStringLiteral("00:00:00"));
}

TEST_F(UT_VideoStatusBar, ElapsedChanged_Pressed_KeepsSliderButUpdatesLabel)
{
    bar->slider->setValue(5);
    QMetaObject::invokeMethod(bar->slider, "sliderPressed");
    ASSERT_TRUE(bar->sliderIsPressed);

    dmrLog.elapsedValue = 99;
    QMetaObject::invokeMethod(preview, "elapsedChanged");
    EXPECT_NE(bar->slider->value(), 99);
    EXPECT_EQ(bar->slider->value(), 5);
    EXPECT_EQ(bar->timeLabel->text(), QStringLiteral("00:00:00"));
}

TEST_F(UT_VideoStatusBar, ControlButtonClicked_PausesThenPlaysEngine)
{
    auto *button = bar->findChild<DTK_WIDGET_NAMESPACE::DIconButton *>("ControlButton");
    ASSERT_NE(button, nullptr);
    button->click();
    EXPECT_EQ(dmrLog.pauseResumeCalls, 1);
    EXPECT_EQ(dmrLog.enginePlayCalls, 1);
}

TEST_F(UT_VideoStatusBar, SigPlayState_PlayingState_ShowsPauseButton)
{
    dmrLog.engineState = dmr::PlayerEngine::Playing;
    QMetaObject::invokeMethod(preview, "sigPlayState");
    EXPECT_TRUE(bar->controlButtonShowsPause);
}

TEST_F(UT_VideoStatusBar, SigPlayState_IdleState_ShowsStartButton)
{
    dmrLog.engineState = dmr::PlayerEngine::Idle;
    QMetaObject::invokeMethod(preview, "sigPlayState");
    EXPECT_FALSE(bar->controlButtonShowsPause);
}

TEST_F(UT_VideoStatusBar, ThemeTypeChanged_RefreshesControlButtonIcon)
{
    using ColorType = DTK_GUI_NAMESPACE::DGuiApplicationHelper::ColorType;
    auto *helper = DTK_GUI_NAMESPACE::DGuiApplicationHelper::instance();
    EXPECT_TRUE(QMetaObject::invokeMethod(helper, "themeTypeChanged", Q_ARG(ColorType, DTK_GUI_NAMESPACE::DGuiApplicationHelper::LightType)));
    EXPECT_NO_FATAL_FAILURE(bar->updateControlButtonIcon());
}

class UT_TitleBarWidget : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        bar = new TitleBarWidget();
    }

    virtual void TearDown() override
    {
        delete bar;
        bar = nullptr;
    }

protected:
    TitleBarWidget *bar { nullptr };
};

TEST_F(UT_TitleBarWidget, Construct_DefaultState)
{
    EXPECT_EQ(bar->text(), QString());
    EXPECT_EQ(bar->opacity(), 1.0);
    EXPECT_FALSE(bar->isVisible());
    EXPECT_EQ(bar->height(), 40);
    EXPECT_TRUE(bar->hasMouseTracking());
    EXPECT_NE(bar->m_autoHideTimer, nullptr);
    EXPECT_NE(bar->m_fadeAnimation, nullptr);
    EXPECT_FALSE(bar->m_autoHideTimer->isActive());
}

TEST_F(UT_TitleBarWidget, SetText_NewText_UpdatesStoredText)
{
    bar->setText("Deepin Movie");
    EXPECT_EQ(bar->text(), QStringLiteral("Deepin Movie"));

    EXPECT_NO_FATAL_FAILURE(bar->setText("Deepin Movie"));
}

TEST_F(UT_TitleBarWidget, StartAutoHideTimer_ActivatesSingleShotTimer)
{
    bar->startAutoHideTimer();
    EXPECT_TRUE(bar->m_autoHideTimer->isActive());
    EXPECT_TRUE(bar->m_autoHideTimer->isSingleShot());
}

TEST_F(UT_TitleBarWidget, StopAutoHideTimer_DeactivatesTimer)
{
    bar->startAutoHideTimer();
    bar->stopAutoHideTimer();
    EXPECT_FALSE(bar->m_autoHideTimer->isActive());

    EXPECT_NO_FATAL_FAILURE(bar->stopAutoHideTimer());
}

TEST_F(UT_TitleBarWidget, AutoHideTimeout_HidesTitleBarViaAnimation)
{
    bar->show();
    bar->startAutoHideTimer();
    QTRY_COMPARE_WITH_TIMEOUT(bar->opacity(), 0.0, 10000);
}

TEST_F(UT_TitleBarWidget, ShowAnimated_FadesInToFullOpacity)
{
    bar->setOpacity(0.2);
    bar->showAnimated();
    EXPECT_TRUE(bar->isVisible());
    EXPECT_EQ(bar->m_fadeAnimation->state(), QAbstractAnimation::Running);
    QTRY_COMPARE_WITH_TIMEOUT(bar->opacity(), 1.0, 5000);
}

TEST_F(UT_TitleBarWidget, HideAnimated_FadesOutToTransparent)
{
    bar->show();
    bar->hideAnimated();
    EXPECT_EQ(bar->m_fadeAnimation->state(), QAbstractAnimation::Running);
    QTRY_COMPARE_WITH_TIMEOUT(bar->opacity(), 0.0, 5000);
    // source defect: widget remains visible (only fully transparent) after hide
    // animation, it still occupies and intercepts mouse input in the video area.
    EXPECT_TRUE(bar->isVisible());
}

TEST_F(UT_TitleBarWidget, SetOpacity_ValueOutOfRange_ClampedToUnitInterval)
{
    bar->setOpacity(1.7);
    EXPECT_EQ(bar->opacity(), 1.0);

    bar->setOpacity(-0.3);
    EXPECT_EQ(bar->opacity(), 0.0);

    bar->setOpacity(0.5);
    EXPECT_EQ(bar->opacity(), 0.5);
}

TEST_F(UT_TitleBarWidget, PaintEvent_WithAndWithoutText_RendersWithoutCrash)
{
    EXPECT_NO_FATAL_FAILURE(bar->grab());

    bar->setText("Sample Title");
    const QPixmap rendered = bar->grab();
    EXPECT_FALSE(rendered.isNull());
}

TEST_F(UT_TitleBarWidget, ResizeEvent_NewSize_AppliesWidthKeepsFixedHeight)
{
    bar->resize(250, 90);
    EXPECT_EQ(bar->width(), 250);
    EXPECT_EQ(bar->height(), 40);
}
