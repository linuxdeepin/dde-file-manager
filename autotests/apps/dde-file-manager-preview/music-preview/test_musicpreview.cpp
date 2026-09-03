// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "musicpreview.h"
#include "musicpreviewplugin.h"
#include "musicmessageview.h"
#include "toolbarframe.h"
#include "cusmediaplayer.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/interfaces/fileinfo.h>

#include <gtest/gtest.h>

#include <QApplication>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUrl>
#include <QWidget>

#include <mutex>

DFMBASE_USE_NAMESPACE
using namespace plugin_filepreview;

class UT_MusicPreview : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        std::call_once(flag, [] {
            UrlRoute::regScheme(Global::Scheme::kFile, QDir::homePath(), QIcon(), false, "file");
            InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        });
    }

    void SetUp() override
    {
        preview = new MusicPreview();
    }

    void TearDown() override
    {
        delete preview;
        preview = nullptr;
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        qApp->processEvents();
        stub.clear();
    }

    QString makeMp3File(const QString &name)
    {
        const QString &path = dir.filePath(name);
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly))
            return QString();
        QByteArray frame(4 + 417, '\0');
        frame[0] = char(0xFF);
        frame[1] = char(0xFB);
        frame[2] = char(0x90);
        frame[3] = '\0';
        file.write(frame);
        file.close();
        return path;
    }

    QString makeTextFile(const QString &name)
    {
        const QString &path = dir.filePath(name);
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly))
            return QString();
        file.write("plain text");
        file.close();
        return path;
    }

protected:
    MusicPreview *preview { nullptr };
    QTemporaryDir dir;
    stub_ext::StubExt stub;
    static std::once_flag flag;
};

std::once_flag UT_MusicPreview::flag;

TEST_F(UT_MusicPreview, CanPreview_LocalMp3File_ReturnsTrue)
{
    const QString &path = makeMp3File("song.mp3");
    ASSERT_FALSE(path.isEmpty());
    EXPECT_TRUE(preview->canPreview(QUrl::fromLocalFile(path)));
}

TEST_F(UT_MusicPreview, CanPreview_TextFile_ReturnsFalse)
{
    const QString &path = makeTextFile("note.txt");
    ASSERT_FALSE(path.isEmpty());
    EXPECT_FALSE(preview->canPreview(QUrl::fromLocalFile(path)));
}

TEST_F(UT_MusicPreview, CanPreview_UnregisteredScheme_ReturnsFalse)
{
    EXPECT_FALSE(preview->canPreview(QUrl("bogus:///tmp/song.mp3")));
}

TEST_F(UT_MusicPreview, SetFileUrl_RemoteHttpUrl_ReturnsFalse)
{
    EXPECT_FALSE(preview->setFileUrl(QUrl("http://example.com/song.mp3")));
    EXPECT_TRUE(preview->fileUrl().isEmpty());
    EXPECT_EQ(preview->contentWidget(), nullptr);
}

TEST_F(UT_MusicPreview, SetFileUrl_NonAudioFile_ReturnsFalse)
{
    const QString &path = makeTextFile("note.txt");
    ASSERT_FALSE(path.isEmpty());
    EXPECT_FALSE(preview->setFileUrl(QUrl::fromLocalFile(path)));
    EXPECT_EQ(preview->contentWidget(), nullptr);
    EXPECT_EQ(preview->statusBarWidget(), nullptr);
}

TEST_F(UT_MusicPreview, SetFileUrl_ValidMp3_CreatesWidgetsAndReturnsTrue)
{
    const QString &path = makeMp3File("song.mp3");
    ASSERT_FALSE(path.isEmpty());
    const QUrl url = QUrl::fromLocalFile(path);

    EXPECT_TRUE(preview->setFileUrl(url));
    EXPECT_EQ(preview->fileUrl(), url);
    EXPECT_NE(preview->contentWidget(), nullptr);
    EXPECT_NE(preview->statusBarWidget(), nullptr);
    EXPECT_EQ(static_cast<int>(preview->statusBarWidgetAlignment()), 0);
    EXPECT_TRUE(preview->contentWidget()->size() == QSize(600, 336));
}

TEST_F(UT_MusicPreview, SetFileUrl_SameUrlTwice_ReturnsTrueWithoutRecreating)
{
    const QString &path = makeMp3File("song.mp3");
    const QUrl url = QUrl::fromLocalFile(path);
    ASSERT_TRUE(preview->setFileUrl(url));
    QWidget *view = preview->contentWidget();
    QWidget *bar = preview->statusBarWidget();

    EXPECT_TRUE(preview->setFileUrl(url));
    EXPECT_EQ(preview->contentWidget(), view);
    EXPECT_EQ(preview->statusBarWidget(), bar);
}

TEST_F(UT_MusicPreview, SetFileUrl_DifferentUrlAfterWidgetsCreated_ReturnsFalse)
{
    const QString &pathA = makeMp3File("a.mp3");
    const QString &pathB = makeMp3File("b.mp3");
    ASSERT_TRUE(preview->setFileUrl(QUrl::fromLocalFile(pathA)));

    EXPECT_FALSE(preview->setFileUrl(QUrl::fromLocalFile(pathB)));
    EXPECT_EQ(preview->fileUrl(), QUrl::fromLocalFile(pathA));
}

TEST_F(UT_MusicPreview, SetFileUrl_FailureThenSuccess_StillSucceeds)
{
    const QString &textPath = makeTextFile("note.txt");
    const QString &mp3Path = makeMp3File("song.mp3");
    ASSERT_FALSE(preview->setFileUrl(QUrl::fromLocalFile(textPath)));

    EXPECT_TRUE(preview->setFileUrl(QUrl::fromLocalFile(mp3Path)));
    EXPECT_EQ(preview->fileUrl(), QUrl::fromLocalFile(mp3Path));
}

TEST_F(UT_MusicPreview, Play_WithWidgets_EmitsSingletonPlaySignal)
{
    const QString &path = makeMp3File("song.mp3");
    ASSERT_TRUE(preview->setFileUrl(QUrl::fromLocalFile(path)));
    QSignalSpy spy(CusMediaPlayer::instance(), &CusMediaPlayer::sigPlay);

    preview->play();
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_MusicPreview, Pause_WithWidgets_EmitsSingletonPauseSignal)
{
    const QString &path = makeMp3File("song.mp3");
    ASSERT_TRUE(preview->setFileUrl(QUrl::fromLocalFile(path)));
    QSignalSpy spy(CusMediaPlayer::instance(), &CusMediaPlayer::sigPause);

    preview->pause();
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_MusicPreview, Stop_WithWidgets_EmitsSingletonStopSignal)
{
    const QString &path = makeMp3File("song.mp3");
    ASSERT_TRUE(preview->setFileUrl(QUrl::fromLocalFile(path)));
    QSignalSpy spy(CusMediaPlayer::instance(), &CusMediaPlayer::sigStop);

    preview->stop();
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_MusicPreview, HandleBeforDestroy_EmitsSingletonStopSignal)
{
    const QString &path = makeMp3File("song.mp3");
    ASSERT_TRUE(preview->setFileUrl(QUrl::fromLocalFile(path)));
    QSignalSpy spy(CusMediaPlayer::instance(), &CusMediaPlayer::sigStop);

    preview->handleBeforDestroy();
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_MusicPreview, Destroy_WidgetsDeletedViaDeferredDelete)
{
    const QString &path = makeMp3File("song.mp3");
    ASSERT_TRUE(preview->setFileUrl(QUrl::fromLocalFile(path)));
    QWidget *view = preview->contentWidget();
    QWidget *bar = preview->statusBarWidget();
    bool viewDestroyed = false;
    bool barDestroyed = false;
    QObject::connect(view, &QObject::destroyed, [&viewDestroyed](QObject *) { viewDestroyed = true; });
    QObject::connect(bar, &QObject::destroyed, [&barDestroyed](QObject *) { barDestroyed = true; });

    delete preview;
    preview = nullptr;
    EXPECT_FALSE(viewDestroyed);
    EXPECT_FALSE(barDestroyed);

    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    EXPECT_TRUE(viewDestroyed);
    EXPECT_TRUE(barDestroyed);
}

class UT_MusicPreviewPlugin : public testing::Test
{
};

TEST_F(UT_MusicPreviewPlugin, Create_AnyKey_ReturnsMusicPreviewInstance)
{
    MusicPreviewPlugin plugin;
    AbstractBasePreview *obj = plugin.create("music");
    ASSERT_NE(obj, nullptr);
    EXPECT_NE(qobject_cast<MusicPreview *>(obj), nullptr);
    delete obj;
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}
