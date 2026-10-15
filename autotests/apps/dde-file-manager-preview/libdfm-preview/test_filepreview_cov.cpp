// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "stubext.h"

#include "filepreview.h"
#include "utils/previewdialogmanager.h"
#include "utils/previewfileoperation.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/utils/dialogmanager.h>

#include <QApplication>
#include <QTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTimer>
#include <QFile>
#include <QDir>

using namespace dfmplugin_filepreview;
DFMBASE_USE_NAMESPACE

// C entry points implemented in filepreview.cpp (no public header)
extern "C" {
int initFilePreview();
int showFilePreviewDialog(quint64 windowId, const QList<QUrl> &selecturls, const QList<QUrl> dirUrl);
}

namespace {
QString makeFile(QTemporaryDir *dir, const QString &name)
{
    const QString path = dir->filePath(name);
    QFile f(path);
    f.open(QIODevice::WriteOnly);
    f.write("cov preview service");
    f.close();
    return path;
}
}   // namespace

// ---------------------------------------------------------------------------
// FilePreview service (filepreview.cpp)
// ---------------------------------------------------------------------------
class FilePreviewServiceTest : public testing::Test
{
protected:
    void SetUp() override
    {
        dir.reset(new QTemporaryDir());
        ASSERT_TRUE(dir->isValid());
    }

    void TearDown() override
    {
        stub.clear();
        dir.reset();
    }

    // stub the dconfig backend used by start()/isPreviewEnabled()
    void installDConfigStubs(bool addConfigResult, const QVariant &previewEnable = QVariant(true))
    {
        stub.set_lamda(ADDR(DConfigManager, addConfig),
                       [addConfigResult](DConfigManager *, const QString &, QString *) -> bool {
                           __DBG_STUB_INVOKE__
                           return addConfigResult;
                       });
        stub.set_lamda(static_cast<QVariant (DConfigManager::*)(const QString &, const QString &, const QVariant &) const>(&DConfigManager::value),
                       [previewEnable](DConfigManager *, const QString &, const QString &key, const QVariant &) -> QVariant {
                           __DBG_STUB_INVOKE__
                           if (key == QString("previewEnable"))
                               return previewEnable;
                           return QVariant();
                       });
    }

    std::unique_ptr<QTemporaryDir> dir;
    stub_ext::StubExt stub;
};

TEST_F(FilePreviewServiceTest, Initialize_RegistersAsyncFileScheme)
{
    // Arrange
    FilePreview service;

    // Act
    service.initialize();

    // Assert: the asyncfile scheme is registered (file was done in main)
    EXPECT_TRUE(UrlRoute::hasScheme(Global::Scheme::kAsyncFile));
}

TEST_F(FilePreviewServiceTest, Start_DConfigAvailable_True)
{
    // Arrange
    FilePreview service;
    installDConfigStubs(true);

    // Act
    const bool started = service.start();

    // Assert
    EXPECT_TRUE(started);
}

TEST_F(FilePreviewServiceTest, Start_DConfigUnavailable_False)
{
    // Arrange
    FilePreview service;
    installDConfigStubs(false);

    // Act
    const bool started = service.start();

    // Assert
    EXPECT_FALSE(started);
}

TEST_F(FilePreviewServiceTest, IsPreviewEnabled_DependsOnDConfigValue)
{
    // Arrange
    FilePreview service;

    // Act: valid false value -> disabled
    installDConfigStubs(true, QVariant(false));
    const bool disabled = service.isPreviewEnabled();

    // Arrange: invalid value -> default enabled
    installDConfigStubs(true, QVariant());
    const bool defaulted = service.isPreviewEnabled();

    // Assert
    EXPECT_FALSE(disabled);
    EXPECT_TRUE(defaulted);
}

TEST_F(FilePreviewServiceTest, ShowFilePreview_Disabled_DoesNotDelegate)
{
    // Arrange
    FilePreview service;
    installDConfigStubs(true, QVariant(false));
    int managerCalls = 0;
    stub.set_lamda(&PreviewDialogManager::showPreviewDialog,
                   [&managerCalls](PreviewDialogManager *, quint64, const QList<QUrl> &, const QList<QUrl> &) {
                       __DBG_STUB_INVOKE__
                       managerCalls++;
                   });

    // Act
    service.showFilePreview(1, { QUrl::fromLocalFile(makeFile(dir.get(), "one.txt")) }, {});

    // Assert
    EXPECT_EQ(managerCalls, 0);
}

TEST_F(FilePreviewServiceTest, ShowFilePreview_Enabled_DelegatesToManager)
{
    // Arrange
    FilePreview service;
    installDConfigStubs(true, QVariant(true));
    int managerCalls = 0;
    quint64 capturedWinId = 0;
    QList<QUrl> capturedUrls;
    stub.set_lamda(&PreviewDialogManager::showPreviewDialog,
                   [&managerCalls, &capturedWinId, &capturedUrls](PreviewDialogManager *, quint64 winId,
                                                                  const QList<QUrl> &urls, const QList<QUrl> &) {
                       __DBG_STUB_INVOKE__
                       managerCalls++;
                       capturedWinId = winId;
                       capturedUrls = urls;
                   });
    const QUrl url = QUrl::fromLocalFile(makeFile(dir.get(), "two.txt"));

    // Act
    service.showFilePreview(77, { url }, {});

    // Assert
    EXPECT_EQ(managerCalls, 1);
    EXPECT_EQ(capturedWinId, quint64(77));
    EXPECT_EQ(capturedUrls, QList<QUrl> { url });
}

TEST_F(FilePreviewServiceTest, CEntryPoints_InitAndShowDialog)
{
    // Arrange: the C entry points drive the singleton service
    const QUrl url = QUrl::fromLocalFile(makeFile(dir.get(), "entry.txt"));
    QList<QUrl> select { url };

    // Act: showing before init only logs; after init with an empty selection
    // the manager early-returns without creating any dialog
    EXPECT_EQ(showFilePreviewDialog(1, {}, {}), 0);
    EXPECT_EQ(initFilePreview(), 0);
    EXPECT_EQ(showFilePreviewDialog(2, select, {}), 0);

    // Assert: a dialog got created for the valid file (unknown-content file
    // may still fall back to UnknowFilePreview which counts as valid)
    EXPECT_NE(PreviewDialogManager::instance()->filePreviewDialog, nullptr);

    // Cleanup: close the created dialog so later tests start clean
    PreviewDialogManager::instance()->onPreviewDialogClose();
    qApp->processEvents();
    qApp->sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

// ---------------------------------------------------------------------------
// PreviewDialogManager
// ---------------------------------------------------------------------------
class PreviewDialogManagerTest : public testing::Test
{
protected:
    void SetUp() override
    {
        dir.reset(new QTemporaryDir());
        ASSERT_TRUE(dir->isValid());
        // keep dconfig writes out of the sandbox
        stub.set_lamda(ADDR(DConfigManager, setValue),
                       [](DConfigManager *, const QString &, const QString &, const QVariant &) {
                           __DBG_STUB_INVOKE__
                       });
    }

    void TearDown() override
    {
        auto *manager = PreviewDialogManager::instance();
        if (manager->filePreviewDialog) {
            manager->filePreviewDialog->deleteLater();
            manager->filePreviewDialog = nullptr;
        }
        manager->exitTimer->stop();
        qApp->processEvents();
        qApp->sendPostedEvents(nullptr, QEvent::DeferredDelete);
        stub.clear();
        dir.reset();
    }

    std::unique_ptr<QTemporaryDir> dir;
    stub_ext::StubExt stub;
};

TEST_F(PreviewDialogManagerTest, Instance_ReturnsTheSameSingleton)
{
    // Arrange / Act
    PreviewDialogManager *first = PreviewDialogManager::instance();
    PreviewDialogManager *second = PreviewDialogManager::instance();

    // Assert
    EXPECT_EQ(first, second);
}

TEST_F(PreviewDialogManagerTest, ShowPreviewDialog_EmptySelection_CreatesNothing)
{
    // Arrange / Act
    PreviewDialogManager::instance()->showPreviewDialog(1, {}, {});

    // Assert
    EXPECT_EQ(PreviewDialogManager::instance()->filePreviewDialog, nullptr);
}

TEST_F(PreviewDialogManagerTest, ShowPreviewDialog_LocalFile_CreatesDialog)
{
    // Arrange
    const QUrl url = QUrl::fromLocalFile(makeFile(dir.get(), "shown.txt"));

    // Act
    PreviewDialogManager::instance()->showPreviewDialog(5, { url }, {});
    auto *dialog = PreviewDialogManager::instance()->filePreviewDialog;

    // Assert
    ASSERT_NE(dialog, nullptr);
    EXPECT_TRUE(dialog->hasValidPreview());
    EXPECT_EQ(dialog->currentWinID, quint64(5));
    EXPECT_TRUE(dialog->isVisible());

    // Act: a second call updates the existing dialog instead of recreating
    const QUrl other = QUrl::fromLocalFile(makeFile(dir.get(), "other.txt"));
    PreviewDialogManager::instance()->showPreviewDialog(6, { url, other }, {});
    EXPECT_EQ(PreviewDialogManager::instance()->filePreviewDialog, dialog);
    EXPECT_EQ(dialog->currentWinID, quint64(6));
}

TEST_F(PreviewDialogManagerTest, ShowPreviewDialog_InvalidSymlink_ShowsErrorAndCreatesNothing)
{
    // Arrange: symlink pointing at a missing target
    const QString linkPath = dir->filePath("dangling.lnk");
    QFile::link(dir->filePath("no-such-target"), linkPath);
    int errorShown = 0;
    stub.set_lamda(ADDR(DialogManager, showErrorDialog),
                   [&errorShown](DialogManager *, const QString &title, const QString &) {
                       __DBG_STUB_INVOKE__
                       errorShown++;
                       EXPECT_TRUE(title.contains(QString("original file")));
                   });

    // Act
    PreviewDialogManager::instance()->showPreviewDialog(1, { QUrl::fromLocalFile(linkPath) }, {});

    // Assert
    EXPECT_EQ(errorShown, 1);
    EXPECT_EQ(PreviewDialogManager::instance()->filePreviewDialog, nullptr);
}

TEST_F(PreviewDialogManagerTest, ShowPreviewDialog_UnopenableUrl_DialogDiscarded)
{
    // Arrange: no file-info class for this scheme -> preview cannot be built
    const QUrl url("cov-unknown-scheme://host/file");

    // Act
    PreviewDialogManager::instance()->showPreviewDialog(1, { url }, {});
    qApp->processEvents();
    qApp->sendPostedEvents(nullptr, QEvent::DeferredDelete);

    // Assert
    EXPECT_EQ(PreviewDialogManager::instance()->filePreviewDialog, nullptr);
}

TEST_F(PreviewDialogManagerTest, OnPreviewDialogClose_SchedulesDeleteAndStartsExitTimer)
{
    // Arrange
    const QUrl url = QUrl::fromLocalFile(makeFile(dir.get(), "closed.txt"));
    PreviewDialogManager::instance()->showPreviewDialog(1, { url }, {});
    auto *manager = PreviewDialogManager::instance();
    ASSERT_NE(manager->filePreviewDialog, nullptr);
    manager->exitTimer->stop();

    // Act
    manager->onPreviewDialogClose();
    qApp->processEvents();
    qApp->sendPostedEvents(nullptr, QEvent::DeferredDelete);

    // Assert: the dialog is gone and the 60s exit timer is running
    EXPECT_EQ(manager->filePreviewDialog, nullptr);
    EXPECT_TRUE(manager->exitTimer->isActive());
    manager->exitTimer->stop();

    // Act: closing with no dialog only restarts the timer
    EXPECT_NO_FATAL_FAILURE(manager->onPreviewDialogClose());

    // Assert
    EXPECT_TRUE(manager->exitTimer->isActive());
}

TEST_F(PreviewDialogManagerTest, ExitTimerTimeout_QuitsApplicationLambda)
{
    // Arrange: the manager subscribes a quit() lambda to the timer timeout
    auto *manager = PreviewDialogManager::instance();

    // Act: fire the timeout immediately (no event loop is running, so quit()
    // is a no-op for the test process)
    manager->exitTimer->start(0);
    QTest::qWait(20);

    // Assert
    EXPECT_FALSE(manager->exitTimer->isActive());
}

TEST_F(PreviewDialogManagerTest, Tr_TranslatesWithContext)
{
    // Arrange / Act
    const QString translated = PreviewDialogManager::tr("preview");

    // Assert: translation machinery initialized, text passes through
    EXPECT_EQ(translated, QString("preview"));
}
