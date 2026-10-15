// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "stubext.h"

#include "views/filepreviewdialog.h"
#include "views/filepreviewdialogstatusbar.h"
#include "views/unknowfilepreview.h"
#include "utils/previewfileoperation.h"
#include "test_fakepreviewplugin.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/fileinfohelper.h>
#include <dfm-base/dfm_global_defines.h>

#include <DGuiApplicationHelper>

#include <QApplication>
#include <QCloseEvent>
#include <QChildEvent>
#include <QResizeEvent>
#include <QShowEvent>
#include <QKeyEvent>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QFile>
#include <QLabel>
#include <QPushButton>
#include <QTest>
#include <QElapsedTimer>
#include <QProcess>

using namespace dfmplugin_filepreview;
using namespace covplugin;
DFMBASE_USE_NAMESPACE
DGUI_USE_NAMESPACE

namespace {
// a preview whose metaObject className is exactly "VideoPreview" so the
// dialog takes the video cooldown path in playCurrentPreviewFile()
class VideoPreview final : public DFMBASE_NAMESPACE::AbstractBasePreview
{
    Q_OBJECT
public:
    explicit VideoPreview(QObject *parent = nullptr)
        : DFMBASE_NAMESPACE::AbstractBasePreview(parent) {}
    bool setFileUrl(const QUrl &url) override { m_url = url; return true; }
    QUrl fileUrl() const override { return m_url; }
    QWidget *contentWidget() const override
    {
        if (!w)
            w = new QWidget;
        return w;
    }
    void play() override { ++playCount; }
    void stop() override { ++stopCount; }
    QUrl m_url;
    int playCount = 0;
    int stopCount = 0;
    mutable QWidget *w = nullptr;
};

// create a temp file helper
QString makeFile(QTemporaryDir *dir, const QString &name, const QByteArray &content = "hello cov")
{
    const QString path = dir->filePath(name);
    QFile f(path);
    f.open(QIODevice::WriteOnly);
    f.write(content);
    f.close();
    return path;
}
}   // namespace

class FilePreviewDialogTest : public testing::Test
{
protected:
    void SetUp() override
    {
        dir.reset(new QTemporaryDir());
        ASSERT_TRUE(dir->isValid());
        txtUrl = QUrl::fromLocalFile(makeFile(dir.get(), "a.txt"));
        txtUrl2 = QUrl::fromLocalFile(makeFile(dir.get(), "b.txt"));
    }

    void TearDown() override
    {
        // dialogs delete their previews via deleteLater
        qApp->processEvents();
        qApp->sendPostedEvents(nullptr, QEvent::DeferredDelete);
        stub.clear();
        dir.reset();
    }

    std::unique_ptr<QTemporaryDir> dir;
    QUrl txtUrl;
    QUrl txtUrl2;
    stub_ext::StubExt stub;
};

TEST_F(FilePreviewDialogTest, Construct_SingleTextFile_PluginPreviewShown)
{
    // Arrange / Act
    FilePreviewDialog dialog({ txtUrl });

    // Assert: plugin preview selected for text/plain, nav buttons hidden.
    // The preview instance lives inside the plugin DSO which owns its own
    // staticMetaObject, so identify it by class name and observe it through
    // the abstract interface / Qt properties instead of qobject_cast.
    AbstractBasePreview *cov = dialog.preview;
    ASSERT_NE(cov, nullptr);
    EXPECT_EQ(QString(cov->metaObject()->className()), QString("covplugin::CovPreview"));
    EXPECT_EQ(cov->fileUrl(), txtUrl);
    EXPECT_EQ(cov->title(), QString("cov: a.txt"));
    EXPECT_TRUE(dialog.statusBar->preButton()->isHidden());
    EXPECT_TRUE(dialog.statusBar->nextButton()->isHidden());
    EXPECT_TRUE(dialog.statusBar->title()->text().contains(QString("cov: a.txt")));
}

TEST_F(FilePreviewDialogTest, Construct_UnknownContent_FallsBackToUnknowFilePreview)
{
    // Arrange: random binary content has no preview plugin
    const QUrl binUrl = QUrl::fromLocalFile(
                makeFile(dir.get(), "blob.bin", "\x00\x01\x02\xff\xfe"));
    QMimeDatabase mdb;

    // Act
    FilePreviewDialog dialog({ binUrl });

    // Assert
    EXPECT_TRUE(dialog.hasValidPreview());
    EXPECT_NE(qobject_cast<UnknowFilePreview *>(dialog.preview), nullptr);
    EXPECT_EQ(dialog.currentPageIndex, 0);
}

TEST_F(FilePreviewDialogTest, Construct_TwoFiles_NavigationButtonStates)
{
    // Arrange / Act
    FilePreviewDialog dialog({ txtUrl, txtUrl2 });

    // Assert
    EXPECT_FALSE(dialog.statusBar->preButton()->isHidden());
    EXPECT_FALSE(dialog.statusBar->nextButton()->isHidden());
    EXPECT_FALSE(dialog.statusBar->preButton()->isEnabled());
    EXPECT_TRUE(dialog.statusBar->nextButton()->isEnabled());
}

TEST_F(FilePreviewDialogTest, SwitchToPage_InvalidIndex_NoStateChange)
{
    // Arrange
    FilePreviewDialog dialog({ txtUrl, txtUrl2 });
    const int index = dialog.currentPageIndex;

    // Act
    dialog.switchToPage(-1);
    dialog.switchToPage(99);

    // Assert
    EXPECT_EQ(dialog.currentPageIndex, index);
}

TEST_F(FilePreviewDialogTest, PreviousAndNextPage_RespectBoundsAndVideoLock)
{
    // Arrange
    FilePreviewDialog dialog({ txtUrl, txtUrl2 });
    ASSERT_EQ(dialog.currentPageIndex, 0);

    // Act: at the first page going back is a no-op
    dialog.previousPage();
    EXPECT_EQ(dialog.currentPageIndex, 0);

    // Act: forward works, backward works
    dialog.nextPage();
    EXPECT_EQ(dialog.currentPageIndex, 1);
    dialog.previousPage();
    EXPECT_EQ(dialog.currentPageIndex, 0);

    // Arrange: while a video is playing navigation is suppressed
    dialog.playingVideo = true;
    dialog.nextPage();

    // Assert
    EXPECT_EQ(dialog.currentPageIndex, 0);
}

TEST_F(FilePreviewDialogTest, UpdatePreviewList_UpdatesFilesAndButtons)
{
    // Arrange
    FilePreviewDialog dialog({ txtUrl });
    ASSERT_TRUE(dialog.statusBar->preButton()->isHidden());

    // Act
    dialog.updatePreviewList({ txtUrl, txtUrl2 });

    // Assert
    EXPECT_FALSE(dialog.statusBar->preButton()->isHidden());
    EXPECT_EQ(dialog.currentPageIndex, 0);
    EXPECT_TRUE(dialog.hasValidPreview());

    // Arrange: locked while playing video
    dialog.playingVideo = true;

    // Act
    dialog.updatePreviewList({ txtUrl });

    // Assert: list untouched
    EXPECT_FALSE(dialog.statusBar->preButton()->isHidden());
}

TEST_F(FilePreviewDialogTest, SetEntryUrlList_EmptyOrForeignList_Ignored)
{
    // Arrange
    FilePreviewDialog dialog({ txtUrl, txtUrl2 });
    dialog.nextPage();
    ASSERT_EQ(dialog.currentPageIndex, 1);

    // Act
    dialog.setEntryUrlList({});
    dialog.setEntryUrlList({ QUrl::fromLocalFile("/other/file.txt") });

    // Assert
    EXPECT_FALSE(dialog.previewDir);
    EXPECT_EQ(dialog.currentPageIndex, 1);

    // Act: a list that contains the current file switches to dir mode
    dialog.setEntryUrlList({ txtUrl, txtUrl2 });

    // Assert
    EXPECT_TRUE(dialog.previewDir);
    EXPECT_EQ(dialog.currentPageIndex, 1);
}

TEST_F(FilePreviewDialogTest, Done_StopsAndDropsPreview)
{
    // Arrange
    FilePreviewDialog dialog({ txtUrl });
    ASSERT_TRUE(dialog.hasValidPreview());

    // Act
    dialog.done(0);
    qApp->processEvents();
    qApp->sendPostedEvents(nullptr, QEvent::DeferredDelete);

    // Assert
    EXPECT_FALSE(dialog.hasValidPreview());

    // Act: calling done again with no preview must not crash
    dialog.done(0);

    // Assert
    SUCCEED();
}

TEST_F(FilePreviewDialogTest, SetCurrentWinID_StoresTheId)
{
    // Arrange
    FilePreviewDialog dialog({ txtUrl });

    // Act
    dialog.setCurrentWinID(0x12345);

    // Assert
    EXPECT_EQ(dialog.currentWinID, quint64(0x12345));
}

TEST_F(FilePreviewDialogTest, PlayCurrentPreviewFile_PluginPreview_PlaysImmediately)
{
    // Arrange
    FilePreviewDialog dialog({ txtUrl });
    AbstractBasePreview *cov = dialog.preview;
    ASSERT_EQ(QString(cov->metaObject()->className()), QString("covplugin::CovPreview"));
    const int before = cov->property("playCount").toInt();

    // Act
    dialog.playCurrentPreviewFile();

    // Assert: generic preview plays with no cooldown flag
    EXPECT_EQ(cov->property("playCount").toInt(), before + 1);
    EXPECT_FALSE(dialog.playingVideo);
}

TEST_F(FilePreviewDialogTest, PlayCurrentPreviewFile_VideoPreview_EntersCooldown)
{
    // Arrange: swap in a preview whose class name is VideoPreview
    FilePreviewDialog dialog({ txtUrl });
    auto *video = new VideoPreview(&dialog);
    auto *old = dialog.preview;
    dialog.preview = video;
    old->deleteLater();
    qApp->processEvents();
    qApp->sendPostedEvents(nullptr, QEvent::DeferredDelete);

    // Act
    dialog.playCurrentPreviewFile();

    // Assert: cooldown armed, the video played
    EXPECT_TRUE(dialog.playingVideo);
    EXPECT_EQ(video->playCount, 1);

    // Act: wait past the 1s cooldown (lambda resets the flag)
    QTest::qWait(1200);

    // Assert
    EXPECT_FALSE(dialog.playingVideo);
}

TEST_F(FilePreviewDialogTest, OpenFile_SuccessClosesDialog_FailureKeepsIt)
{
    // Arrange: opening is delegated to PreviewFileOperation
    FilePreviewDialog dialog({ txtUrl });
    dialog.show();
    int openCalls = 0;
    bool openResult = true;
    stub.set_lamda(ADDR(PreviewFileOperation, openFileHandle),
                   [&openCalls, &openResult](quint64, const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       openCalls++;
                       return openResult;
                   });
    QSignalSpy closeSpy(&dialog, &FilePreviewDialog::signalCloseEvent);

    // Act
    dialog.openFile();

    // Assert
    EXPECT_EQ(openCalls, 1);
    EXPECT_FALSE(dialog.isVisible());
    EXPECT_EQ(closeSpy.count(), 1);

    // Arrange: failed open keeps the dialog alive
    FilePreviewDialog staying({ txtUrl });
    staying.show();
    openResult = false;
    openCalls = 0;

    // Act
    staying.openFile();

    // Assert
    EXPECT_EQ(openCalls, 1);
    EXPECT_TRUE(staying.isVisible());
}

TEST_F(FilePreviewDialogTest, HandleFileInfoRefreshFinished_CurrentUrlRefreshesPreview)
{
    // Arrange
    FilePreviewDialog dialog({ txtUrl });
    AbstractBasePreview *cov = dialog.preview;
    ASSERT_EQ(QString(cov->metaObject()->className()), QString("covplugin::CovPreview"));
    const QUrl firstUrl = cov->fileUrl();

    // Act: refresh notice for a foreign url is ignored
    dialog.handleFileInfoRefreshFinished(QUrl::fromLocalFile("/elsewhere"), QString(), false);

    // Assert
    EXPECT_EQ(cov->fileUrl(), firstUrl);

    // Act: refresh notice for the current url reloads the preview
    dialog.handleFileInfoRefreshFinished(txtUrl, QString(), false);

    // Assert
    EXPECT_EQ(cov->fileUrl(), txtUrl);
    EXPECT_TRUE(dialog.hasValidPreview());
}

TEST_F(FilePreviewDialogTest, CloseEvent_EmitsSignalAndStopsPreview)
{
    // Arrange
    FilePreviewDialog dialog({ txtUrl });
    QSignalSpy spy(&dialog, &FilePreviewDialog::signalCloseEvent);
    QCloseEvent event;

    // Act
    QCoreApplication::sendEvent(&dialog, &event);
    qApp->processEvents();
    qApp->sendPostedEvents(nullptr, QEvent::DeferredDelete);

    // Assert
    EXPECT_EQ(spy.count(), 1);
    EXPECT_FALSE(dialog.hasValidPreview());
}

TEST_F(FilePreviewDialogTest, EventFilter_ArrowKeysNavigate_EscapeCloses)
{
    // Arrange
    FilePreviewDialog dialog({ txtUrl, txtUrl2 });
    ASSERT_EQ(dialog.currentPageIndex, 0);

    // Act: Right moves to the next page
    QKeyEvent right(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    EXPECT_TRUE(dialog.eventFilter(&dialog, &right));

    // Assert
    EXPECT_EQ(dialog.currentPageIndex, 1);

    // Act: Left moves back
    QKeyEvent left(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    EXPECT_TRUE(dialog.eventFilter(&dialog, &left));
    EXPECT_EQ(dialog.currentPageIndex, 0);

    // Act: unknown key falls through to the base class
    QKeyEvent other(QEvent::KeyPress, Qt::Key_F5, Qt::NoModifier);
    EXPECT_FALSE(dialog.eventFilter(&dialog, &other));

    // Act: Escape closes the dialog
    QSignalSpy spy(&dialog, &FilePreviewDialog::signalCloseEvent);
    QKeyEvent escape(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    EXPECT_TRUE(dialog.eventFilter(&dialog, &escape));

    // Assert
    EXPECT_EQ(spy.count(), 1);

    // Arrange: while a video is playing Space is not consumed: the switch
    // breaks out and the event falls through to the base class handler
    FilePreviewDialog videoDialog({ txtUrl });
    videoDialog.playingVideo = true;
    QSignalSpy videoSpy(&videoDialog, &FilePreviewDialog::signalCloseEvent);
    QKeyEvent escape2(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);

    // Act
    EXPECT_FALSE(videoDialog.eventFilter(&videoDialog, &escape2));

    // Assert: close request ignored during the cooldown
    EXPECT_EQ(videoSpy.count(), 0);
}

TEST_F(FilePreviewDialogTest, ShowResizeChildEvents_PassThroughSafely)
{
    // Arrange
    FilePreviewDialog dialog({ txtUrl });
    dialog.show();

    // Act
    QShowEvent showEvent;
    EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(&dialog, &showEvent));
    QResizeEvent resize(dialog.size() + QSize(10, 10), dialog.size());
    EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(&dialog, &resize));
    QWidget child(&dialog);
    QChildEvent childAdded(QEvent::ChildAdded, &child);
    EXPECT_NO_FATAL_FAILURE(QCoreApplication::sendEvent(&dialog, &childAdded));
    QTest::qWait(80);   // let the resize repaint timer fire

    // Assert
    EXPECT_TRUE(dialog.isVisible());
}

TEST_F(FilePreviewDialogTest, GeneralKey_MimeKeysGeneralized)
{
    // Arrange
    FilePreviewDialog dialog({ txtUrl });

    // Act / Assert
    EXPECT_EQ(dialog.generalKey("text/plain"), QString("text/*"));
    EXPECT_EQ(dialog.generalKey("application/x-thing"), QString("application/*"));
    EXPECT_EQ(dialog.generalKey("inode-directory"), QString("inode-directory"));
}

TEST_F(FilePreviewDialogTest, UpdateTitle_WithoutPreview_EarlyReturn)
{
    // Arrange: done() dropped the preview
    FilePreviewDialog dialog({ txtUrl });
    dialog.done(0);
    qApp->processEvents();

    // Act / Assert: the null-preview guard must not crash
    EXPECT_NO_FATAL_FAILURE(dialog.updateTitle());
}

TEST_F(FilePreviewDialogTest, CenterPos_SavedWhileVisibleAndRestored)
{
    // Arrange
    FilePreviewDialog dialog({ txtUrl });

    // Act: while hidden nothing is recorded
    dialog.saveCenterPos();
    EXPECT_TRUE(dialog.previousCenter.isNull());

    // Arrange: shown dialog records its center
    dialog.show();
    dialog.move(100, 100);
    dialog.saveCenterPos();

    // Act: resize (preview switch) then restore keeps the old center
    const QPointF saved = dialog.previousCenter;
    dialog.resize(dialog.size() + QSize(40, 40));
    dialog.restoreCenterPos();

    // Assert: the window moved back so its center matches the saved one
    EXPECT_NEAR(QPointF(dialog.geometry().center()).x(), saved.x(), 30.0);
    EXPECT_NEAR(QPointF(dialog.geometry().center()).y(), saved.y(), 30.0);

    // Act: restoring without a recorded center is a no-op
    dialog.previousCenter = QPointF();
    EXPECT_NO_FATAL_FAILURE(dialog.restoreCenterPos());
}

TEST_F(FilePreviewDialogTest, Destructor_DisposesPreviewSafely)
{
    // Arrange
    auto *dialog = new FilePreviewDialog({ txtUrl });
    ASSERT_TRUE(dialog->hasValidPreview());

    // Act / Assert
    EXPECT_NO_FATAL_FAILURE(delete dialog);
    qApp->processEvents();
    qApp->sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

// ---------------------------------------------------------------------------
// FilePreviewDialogStatusBar
// ---------------------------------------------------------------------------
class FilePreviewDialogStatusBarTest : public testing::Test
{
protected:
    void SetUp() override { bar = new FilePreviewDialogStatusBar(); }
    void TearDown() override
    {
        delete bar;
        qApp->processEvents();
    }
    FilePreviewDialogStatusBar *bar { nullptr };
};

TEST_F(FilePreviewDialogStatusBarTest, Constructor_CreatesAllControls)
{
    // Arrange / Act & Assert
    ASSERT_NE(bar->preButton(), nullptr);
    ASSERT_NE(bar->nextButton(), nullptr);
    ASSERT_NE(bar->openButton(), nullptr);
    ASSERT_NE(bar->title(), nullptr);
    EXPECT_EQ(bar->preButton()->objectName(), QString("PreButton"));
    EXPECT_EQ(bar->nextButton()->objectName(), QString("NextButton"));
    EXPECT_EQ(bar->openButton()->objectName(), QString("OpenButton"));
    EXPECT_EQ(bar->title()->objectName(), QString("TitleLabel"));
    EXPECT_TRUE(bar->title()->isHidden());
    EXPECT_EQ(bar->preButton()->shortcut(), QKeySequence(QKeySequence::Back));
    EXPECT_EQ(bar->openButton()->text(), FilePreviewDialogStatusBar::tr("Open", "button"));
}

TEST_F(FilePreviewDialogStatusBarTest, EventFilter_EnterLeaveTogglesHoverState)
{
    // Arrange
    ASSERT_FALSE(bar->preBtnHovered);
    QEvent enter(QEvent::Enter);
    QEvent leave(QEvent::Leave);

    // Act
    QCoreApplication::sendEvent(bar->preButton(), &enter);

    // Assert: the status bar is installed as filter on the buttons
    EXPECT_TRUE(bar->preBtnHovered);
    EXPECT_FALSE(bar->nextBtnHovered);

    // Act
    QCoreApplication::sendEvent(bar->preButton(), &leave);

    // Assert
    EXPECT_FALSE(bar->preBtnHovered);

    // Act: the next button toggles its own flag
    QCoreApplication::sendEvent(bar->nextButton(), &enter);
    EXPECT_TRUE(bar->nextBtnHovered);
    QCoreApplication::sendEvent(bar->nextButton(), &leave);
    EXPECT_FALSE(bar->nextBtnHovered);
}

TEST_F(FilePreviewDialogStatusBarTest, EventFilter_IconRefreshEvents_PassThrough)
{
    // Arrange
    QEvent enabledChange(QEvent::EnabledChange);
    QEvent styleChange(QEvent::StyleChange);
    QEvent paletteChange(QEvent::PaletteChange);

    // Act / Assert: handled internally, never crashes, base class runs
    EXPECT_FALSE(bar->eventFilter(bar->preButton(), &enabledChange));
    EXPECT_FALSE(bar->eventFilter(bar->nextButton(), &styleChange));
    EXPECT_FALSE(bar->eventFilter(bar->preButton(), &paletteChange));
    // events on unrelated objects are untouched
    EXPECT_FALSE(bar->eventFilter(bar->title(), &paletteChange));
}

TEST_F(FilePreviewDialogStatusBarTest, ThemeChange_RefreshesNavButtonIcons)
{
    // Arrange: the bar subscribes to DTK theme changes in its constructor
    auto *helper = DGuiApplicationHelper::instance();

    // Act
    emit helper->themeTypeChanged(DGuiApplicationHelper::LightType);

    // Assert: the connected lambda refreshed the icons without crashing
    EXPECT_NO_FATAL_FAILURE(bar->updateNavButtonIcons());
    EXPECT_FALSE(bar->preBtnHovered);
}

// ---------------------------------------------------------------------------
// UnknowFilePreview
// ---------------------------------------------------------------------------
class UnknowFilePreviewTest : public testing::Test
{
protected:
    void SetUp() override
    {
        dir.reset(new QTemporaryDir());
        ASSERT_TRUE(dir->isValid());
        preview = new UnknowFilePreview();
    }

    void TearDown() override
    {
        delete preview;
        qApp->processEvents();
        qApp->sendPostedEvents(nullptr, QEvent::DeferredDelete);
        dir.reset();
    }

    std::unique_ptr<QTemporaryDir> dir;
    UnknowFilePreview *preview { nullptr };
};

TEST_F(UnknowFilePreviewTest, Constructor_ContentWidgetSized)
{
    // Arrange / Act
    QWidget *content = preview->contentWidget();

    // Assert
    ASSERT_NE(content, nullptr);
    EXPECT_EQ(content->size(), QSize(590, 260));
}

TEST_F(UnknowFilePreviewTest, SetFileUrl_ExistingLocalFile_Succeeds)
{
    // Arrange
    const QUrl url = QUrl::fromLocalFile(makeFile(dir.get(), "unknown.xyz"));

    // Act
    const bool ok = preview->setFileUrl(url);

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(preview->fileUrl(), url);
}

TEST_F(UnknowFilePreviewTest, SetFileUrl_UnregisteredScheme_Fails)
{
    // Arrange: no info class registered for this scheme
    const QUrl url("cov-unknown://host/path");

    // Act
    const bool ok = preview->setFileUrl(url);

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_EQ(preview->fileUrl(), url);
}

TEST_F(UnknowFilePreviewTest, SetFileInfo_NullInfo_EarlyReturn)
{
    // Arrange / Act / Assert: must not touch any label nor crash
    EXPECT_NO_FATAL_FAILURE(preview->setFileInfo(FileInfoPointer()));
    EXPECT_TRUE(preview->nameLabel->text().isEmpty());
}

TEST_F(UnknowFilePreviewTest, SetFileInfo_RegularFile_FillsLabels)
{
    // Arrange
    const QUrl url = QUrl::fromLocalFile(makeFile(dir.get(), "notes.txt", "12345"));
    FileInfoPointer info = InfoFactory::create<FileInfo>(url);
    ASSERT_FALSE(info.isNull());

    // Act
    preview->setFileUrl(url);
    preview->setFileInfo(info);

    // Assert
    EXPECT_EQ(preview->nameLabel->text(), QString("notes.txt"));
    EXPECT_TRUE(preview->sizeLabel->text().startsWith(QString("Size:")));
    EXPECT_TRUE(preview->typeLabel->text().startsWith(QString("Type:")));
}

TEST_F(UnknowFilePreviewTest, UpdateFolderSizeCount_DirectCall_FormatsResult)
{
    // Arrange: 2560 bytes = 2.5 KB (a value whose formatted form has no
    // trailing zero, so the full formatting pipeline is asserted)
    DFMBASE_NAMESPACE::FileScanner::ScanResult result;
    result.totalSize = 2560;
    result.fileCount = 2;
    result.directoryCount = 1;

    // Act
    preview->updateFolderSizeCount(result);

    // Assert
    EXPECT_TRUE(preview->sizeLabel->text().contains(QString("2.5 KB")));
    EXPECT_EQ(preview->typeLabel->text(), QString("Items: 3"));
}

TEST_F(UnknowFilePreviewTest, SetFileInfo_Directory_StartsScanAndDiscardsIt)
{
    // Arrange: a small directory with two files
    QDir sub(dir->filePath("somedir"));
    ASSERT_TRUE(sub.mkpath("."));
    makeFile(dir.get(), "somedir/one.txt", "aaa");
    makeFile(dir.get(), "somedir/two.txt", "bbbb");
    const QUrl dirUrl = QUrl::fromLocalFile(sub.absolutePath());
    FileInfoPointer info = InfoFactory::create<FileInfo>(dirUrl);
    ASSERT_FALSE(info.isNull());

    // Act: setting a directory starts an async size scan
    preview->setFileUrl(dirUrl);
    preview->setFileInfo(info);

    // Assert: placeholder while scanning, a scanner object exists
    EXPECT_EQ(preview->sizeLabel->text(), QString("Size: 0"));
    ASSERT_FALSE(preview->fileCalculationUtils.isNull());

    // Act: pump the loop until progress arrives (small dir -> fast)
    QElapsedTimer timer;
    timer.start();
    while (preview->sizeLabel->text() == QString("Size: 0") && timer.elapsed() < 5000)
        qApp->processEvents(QEventLoop::AllEvents, 50);

    // Assert: the scan updated the labels through the progress lambda
    EXPECT_NE(preview->sizeLabel->text(), QString("Size: 0"));
    EXPECT_TRUE(preview->typeLabel->text().startsWith(QString("Items:")));

    // Act: switching to another file retires the running scanner
    const QUrl fileUrl = QUrl::fromLocalFile(makeFile(dir.get(), "plain.txt", "x"));
    FileInfoPointer fileInfo = InfoFactory::create<FileInfo>(fileUrl);
    preview->setFileInfo(fileInfo);

    // Assert
    EXPECT_TRUE(preview->fileCalculationUtils.isNull());
}

TEST_F(UnknowFilePreviewTest, DiscardCurrentScanner_WithoutScanner_NoCrash)
{
    // Arrange / Act / Assert
    EXPECT_NO_FATAL_FAILURE(preview->discardCurrentScanner());
}

TEST_F(UnknowFilePreviewTest, Destructor_CleansUpSafely)
{
    // Arrange
    auto *doomed = new UnknowFilePreview();
    const QUrl url = QUrl::fromLocalFile(makeFile(dir.get(), "gone.bin", "\x01"));
    doomed->setFileUrl(url);

    // Act / Assert
    EXPECT_NO_FATAL_FAILURE(delete doomed);
    qApp->processEvents();
    qApp->sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

// ---------------------------------------------------------------------------
// PreviewFileOperation
// ---------------------------------------------------------------------------
class PreviewFileOperationTest : public testing::Test
{
protected:
    void TearDown() override { stub.clear(); }
    stub_ext::StubExt stub;
};

TEST_F(PreviewFileOperationTest, Constructor_PlainQObject)
{
    // Arrange / Act
    PreviewFileOperation op;

    // Assert
    EXPECT_TRUE(op.parent() == nullptr);
}

TEST_F(PreviewFileOperationTest, OpenFileHandle_InvalidOrMissingUrl_False)
{
    // Arrange / Act / Assert
    EXPECT_FALSE(PreviewFileOperation::openFileHandle(0, QUrl()));
    EXPECT_FALSE(PreviewFileOperation::openFileHandle(0, QUrl("http://remote/file")));

    QTemporaryDir dir;
    const QUrl missing = QUrl::fromLocalFile(dir.filePath("not-exist.txt"));
    EXPECT_FALSE(PreviewFileOperation::openFileHandle(0, missing));
}

TEST_F(PreviewFileOperationTest, OpenFileHandle_LocalHandlerSucceeds_True)
{
    // Arrange
    QTemporaryDir dir;
    QTemporaryFile file;
    ASSERT_TRUE(file.open());
    const QUrl url = QUrl::fromLocalFile(file.fileName());
    int openCalls = 0;
    stub.set_lamda(static_cast<bool (LocalFileHandler::*)(const QList<QUrl> &)>(&LocalFileHandler::openFiles),
                   [&openCalls](LocalFileHandler *, const QList<QUrl> &urls) -> bool {
                       __DBG_STUB_INVOKE__
                       openCalls++;
                       return urls.size() == 1;
                   });

    // Act
    const bool ok = PreviewFileOperation::openFileHandle(42, url);

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(openCalls, 1);
}

TEST_F(PreviewFileOperationTest, OpenFileHandle_FallbackLaunchesFileManager)
{
    // Arrange: the local handler fails with no event recorded -> fallback
    QTemporaryFile file;
    ASSERT_TRUE(file.open());
    const QUrl url = QUrl::fromLocalFile(file.fileName());
    stub.set_lamda(static_cast<bool (LocalFileHandler::*)(const QList<QUrl> &)>(&LocalFileHandler::openFiles),
                   [](LocalFileHandler *, const QList<QUrl> &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });
    stub.set_lamda(ADDR(LocalFileHandler, lastEventType),
                   [](LocalFileHandler *) -> DFMBASE_NAMESPACE::GlobalEventType {
                       __DBG_STUB_INVOKE__
                       return DFMBASE_NAMESPACE::GlobalEventType::kUnknowType;
                   });
    QStringList capturedArgs;
    stub.set_lamda(static_cast<bool (*)(const QString &, const QStringList &, const QString &, qint64 *)>(&QProcess::startDetached),
                   [&capturedArgs](const QString &, const QStringList &args, const QString &, qint64 *) -> bool {
                       __DBG_STUB_INVOKE__
                       capturedArgs = args;
                       return true;
                   });

    // Act
    const bool ok = PreviewFileOperation::openFileHandle(0, url);

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_TRUE(capturedArgs.contains(url.path()));
}

TEST_F(PreviewFileOperationTest, OpenFileHandle_HandlerFailedWithEvent_False)
{
    // Arrange: lastEventType set -> no fallback, plain failure
    QTemporaryFile file;
    ASSERT_TRUE(file.open());
    const QUrl url = QUrl::fromLocalFile(file.fileName());
    stub.set_lamda(static_cast<bool (LocalFileHandler::*)(const QList<QUrl> &)>(&LocalFileHandler::openFiles),
                   [](LocalFileHandler *, const QList<QUrl> &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });
    stub.set_lamda(ADDR(LocalFileHandler, lastEventType),
                   [](LocalFileHandler *) -> DFMBASE_NAMESPACE::GlobalEventType {
                       __DBG_STUB_INVOKE__
                       return DFMBASE_NAMESPACE::kOpenFiles;
                   });

    // Act
    const bool ok = PreviewFileOperation::openFileHandle(0, url);

    // Assert
    EXPECT_FALSE(ok);
}

#include "test_filepreviewdialog_cov.moc"
