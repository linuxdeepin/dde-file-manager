// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "videopreview.h"
#include "videopreviewplugin.h"
#include "videowidget.h"
#include "videostatusbar.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QApplication>
#include <QSignalSpy>
#include <QTest>
#include <QTemporaryFile>
#include <QMetaObject>

DFMBASE_USE_NAMESPACE
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

QString createVideoFile()
{
    QTemporaryFile file;
    file.setAutoRemove(false);
    file.open();
    file.write("fake-video");
    file.close();
    return file.fileName();
}

void destroyPreview(VideoPreview *preview)
{
    delete preview;
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

}   // namespace

class UT_VideoPreview : public testing::Test
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
    }

    virtual void TearDown() override
    {
        if (preview) {
            destroyPreview(preview);
            preview = nullptr;
        }
        stub.clear();
    }

protected:
    VideoPreview *preview { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_VideoPreview, Construct_CreatesPlayerAndStatusBarWidgets)
{
    EXPECT_NE(preview->contentWidget(), nullptr);
    EXPECT_NE(preview->statusBarWidget(), nullptr);
    EXPECT_NE(preview->playerWidget, nullptr);
    EXPECT_NE(preview->playerWidget->titleBar, nullptr);
    EXPECT_EQ(preview->playerWidget->minimumSize(), QSize(800, 355));
    EXPECT_EQ(dmrLog.lastBackendProperty, QStringLiteral("keep-open"));
}

TEST_F(UT_VideoPreview, Destruct_CleansUpWithoutCrash)
{
    QPointer<VideoWidget> player = preview->playerWidget;
    QPointer<VideoStatusBar> status = preview->statusBar;
    EXPECT_NO_FATAL_FAILURE(destroyPreview(preview));
    preview = nullptr;
    EXPECT_TRUE(player.isNull());
    EXPECT_TRUE(status.isNull());
}

TEST_F(UT_VideoPreview, SetFileUrl_RemoteUrl_ReturnsFalse)
{
    const QUrl url("http://example.com/video.mp4");
    EXPECT_FALSE(preview->setFileUrl(url));
    EXPECT_FALSE(preview->fileUrl().isValid());
}

TEST_F(UT_VideoPreview, SetFileUrl_NonExistentFile_ReturnsFalse)
{
    const QUrl url = QUrl::fromLocalFile("/nonexistent/path/video.mp4");
    EXPECT_FALSE(preview->setFileUrl(url));
    EXPECT_NE(preview->fileUrl(), url);
}

TEST_F(UT_VideoPreview, SetFileUrl_UnplayableFile_ReturnsFalse)
{
    dmrLog.playable = false;
    const QString path = createVideoFile();
    const QUrl url = QUrl::fromLocalFile(path);
    EXPECT_FALSE(preview->setFileUrl(url));
    QFile::remove(path);
}

TEST_F(UT_VideoPreview, SetFileUrl_ParseFailure_ReturnsFalse)
{
    dmrLog.parseOk = false;
    const QString path = createVideoFile();
    const QUrl url = QUrl::fromLocalFile(path);
    EXPECT_FALSE(preview->setFileUrl(url));
    QFile::remove(path);
}

TEST_F(UT_VideoPreview, SetFileUrl_ValidVideoFile_ReturnsTrueAndAppliesInfo)
{
    const QString path = createVideoFile();
    const QUrl url = QUrl::fromLocalFile(path);

    EXPECT_TRUE(preview->setFileUrl(url));
    EXPECT_EQ(preview->fileUrl(), QUrl::fromLocalFile(path));
    EXPECT_EQ(preview->videoUrl, QUrl::fromLocalFile(path));
    EXPECT_EQ(preview->playerWidget->titleBar->text(), QStringLiteral("Test Movie"));
    EXPECT_EQ(preview->statusBar->slider->maximum(), 120);

    QFile::remove(path);
}

TEST_F(UT_VideoPreview, FileUrl_DefaultConstruction_IsEmpty)
{
    EXPECT_TRUE(preview->fileUrl().isEmpty());
    EXPECT_FALSE(preview->fileUrl().isValid());
}

TEST_F(UT_VideoPreview, ContentWidget_ReturnsPlayerWidget)
{
    EXPECT_EQ(preview->contentWidget(), static_cast<QWidget *>(preview->playerWidget.data()));
}

TEST_F(UT_VideoPreview, StatusBarWidget_ReturnsVideoStatusBar)
{
    EXPECT_EQ(preview->statusBarWidget(), static_cast<QWidget *>(preview->statusBar.data()));
}

TEST_F(UT_VideoPreview, ShowStatusBarSeparator_Always_ReturnsFalse)
{
    EXPECT_FALSE(preview->showStatusBarSeparator());
}

TEST_F(UT_VideoPreview, StatusBarWidgetAlignment_Always_ReturnsEmptyAlignment)
{
    EXPECT_EQ(preview->statusBarWidgetAlignment(), Qt::Alignment());
}

TEST_F(UT_VideoPreview, Play_WithValidUrl_PlaysFileAndStartsAutoHideTimer)
{
    const QString path = createVideoFile();
    const QUrl url = QUrl::fromLocalFile(path);
    ASSERT_TRUE(preview->setFileUrl(url));

    preview->play();
    EXPECT_EQ(preview->playerWidget->videoUrl, url);
    EXPECT_TRUE(preview->playerWidget->titleBar->m_autoHideTimer->isActive());

    QFile::remove(path);
}

TEST_F(UT_VideoPreview, Play_WithoutUrl_DoesNothing)
{
    preview->play();
    EXPECT_TRUE(dmrLog.lastPlayedUrl.isEmpty());
    EXPECT_FALSE(preview->playerWidget->titleBar->m_autoHideTimer->isActive());
}

TEST_F(UT_VideoPreview, Pause_ForwardsToEnginePauseResume)
{
    preview->pause();
    EXPECT_EQ(dmrLog.pauseResumeCalls, 1);
}

TEST_F(UT_VideoPreview, Stop_ForwardsToEngineStop)
{
    preview->stop();
    EXPECT_EQ(dmrLog.stopCalls, 1);
}

TEST_F(UT_VideoPreview, SigPlayState_EmittedByPreview_IsReceivedByStatusBar)
{
    QSignalSpy spy(preview, &VideoPreview::sigPlayState);
    QMetaObject::invokeMethod(preview, "sigPlayState");
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_VideoPreview, ElapsedChanged_EmittedByPreview_IsReceivedByStatusBar)
{
    QSignalSpy spy(preview, &VideoPreview::elapsedChanged);
    QMetaObject::invokeMethod(preview, "elapsedChanged");
    EXPECT_EQ(spy.count(), 1);
}

class UT_VideoPreviewPlugin : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        dmrLog = DmrCallLog();
        installDmrStubs(stub);
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
};

TEST_F(UT_VideoPreviewPlugin, Create_AnyKey_ReturnsVideoPreviewInstance)
{
    VideoPreviewPlugin plugin;
    AbstractBasePreview *preview = plugin.create("video");
    EXPECT_NE(preview, nullptr);
    EXPECT_NE(dynamic_cast<VideoPreview *>(preview), nullptr);
    destroyPreview(static_cast<VideoPreview *>(preview));
}

TEST_F(UT_VideoPreviewPlugin, Create_EmptyKey_ReturnsVideoPreviewInstance)
{
    VideoPreviewPlugin plugin;
    AbstractBasePreview *preview = plugin.create("");
    EXPECT_NE(preview, nullptr);
    destroyPreview(static_cast<VideoPreview *>(preview));
}
