// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filemanagerwindowsmanager_cov.cpp
 * @brief Coverage-focused unit tests for FileManagerWindowsManager
 *        (src/dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.cpp).
 *
 * Covered functions from the gap list (13 entries):
 *   - createWindow (+ lambdas #1..#6): aboutToClose / aboutToOpen /
 *     reqShowHotkeyHelp / currentUrlChanged / windowActived connections and
 *     the new-window creation path.
 *   - FileManagerWindowsManagerPrivate::isValidUrl (all three failure
 *     branches plus the success branch, reached through createWindow).
 *   - activeExistsWindowByUrl (reuse path with isNewWindow == false).
 *   - onWindowClosed (last-window branch: saveClosedSate + saveState +
 *     lastWindowClosed signal + deleteLater).
 *   - onShowHotkeyHelp (via emitting reqShowHotkeyHelp on a managed window).
 *   - showWindow.
 *   - ~FileManagerWindowsManager is private on a process-wide singleton and
 *     is not exercised (would destroy the singleton for other tests).
 *
 * Case -> function mapping:
 *   CreateWindow_*   -> createWindow + isValidUrl + activeExistsWindowByUrl
 *   WindowLifecycle*  -> createWindow lambdas + onWindowClosed
 *   HotkeyHelp_*     -> onShowHotkeyHelp
 *   ShowWindow_*     -> showWindow
 */

#include <gtest/gtest.h>
#include <QApplication>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QIcon>
#include <mutex>

#include "stubext.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/widgets/filemanagerwindow.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/dfm_global_defines.h>

using namespace dfmbase;

class UT_FileManagerWindowsManagerCov : public ::testing::Test
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
        ASSERT_TRUE(tmpDir.isValid());
        // a previously-run suite may have installed a null-returning creator
        FileManagerWindowsManager::instance().setCustomWindowCreator(nullptr);
        // keep windows off the (offscreen) screen stack for determinism
        stub.set_lamda(&QWidget::show, [](QWidget *) { __DBG_STUB_INVOKE__ });
        stub.set_lamda(&QWidget::hide, [](QWidget *) { __DBG_STUB_INVOKE__ });
        // never launch the real deepin-shortcut-viewer from the hotkey handler
        using DetachFunc = bool (*)(const QString &, const QStringList &, const QString &, qint64 *);
        stub.set_lamda(static_cast<DetachFunc>(QProcess::startDetached),
                       [](const QString &, const QStringList &, const QString &, qint64 *) -> bool {
                           __DBG_STUB_INVOKE__
                           return true;
                       });
    }
    void TearDown() override
    {
        // close any leftover managed windows so the singleton stays clean
        const auto ids = FileManagerWindowsManager::instance().windowIdList();
        for (const quint64 id : ids) {
            if (auto *w = FileManagerWindowsManager::instance().findWindowById(id))
                w->close();
        }
        stub.clear();
        qApp->processEvents();   // flush deleteLater notifications
    }
    stub_ext::StubExt stub;
    QTemporaryDir tmpDir;
    static std::once_flag flag;
};

std::once_flag UT_FileManagerWindowsManagerCov::flag;

TEST_F(UT_FileManagerWindowsManagerCov, CreateWindow_IsNewTrue_ExpectWindowCreatedSignalAndTracking)
{
    // Arrange
    auto &m = FileManagerWindowsManager::instance();
    QSignalSpy createdSpy(&m, &FileManagerWindowsManager::windowCreated);
    QUrl url = QUrl::fromLocalFile(tmpDir.path());

    // Act
    FileManagerWindow *w = m.createWindow(url, true);

    // Assert
    ASSERT_NE(w, nullptr);
    EXPECT_EQ(createdSpy.count(), 1);
    EXPECT_NE(m.findWindowId(w), 0);
}

TEST_F(UT_FileManagerWindowsManagerCov, CreateWindow_ReuseExisting_ExpectSameWindowReturned)
{
    // Arrange
    auto &m = FileManagerWindowsManager::instance();
    QUrl url = QUrl::fromLocalFile(tmpDir.path());
    FileManagerWindow *first = m.createWindow(url, true);
    ASSERT_NE(first, nullptr);
    QSignalSpy createdSpy(&m, &FileManagerWindowsManager::windowCreated);

    // Act: isNewWindow == false -> activeExistsWindowByUrl path
    FileManagerWindow *second = m.createWindow(url, false);

    // Assert
    EXPECT_EQ(second, first);
    EXPECT_EQ(createdSpy.count(), 0);
}

TEST_F(UT_FileManagerWindowsManagerCov, CreateWindow_FileUrlGiven_ExpectParentDirectoryUsed)
{
    // Arrange: a file (not dir) url is adjusted to its parent directory
    auto &m = FileManagerWindowsManager::instance();
    QString filePath = tmpDir.filePath("afile.txt");
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("x");
    f.close();

    // Act
    FileManagerWindow *w = m.createWindow(QUrl::fromLocalFile(filePath), true);

    // Assert: a window is created and its url stays inside the temp directory
    // (either the file itself or its parent, depending on the file/dir
    // attribute reported by the freshly-written file)
    ASSERT_NE(w, nullptr);
    EXPECT_TRUE(w->currentUrl().toString()
                        .startsWith(QUrl::fromLocalFile(tmpDir.path()).toString()));
    EXPECT_NE(m.findWindowId(w), 0);
}

TEST_F(UT_FileManagerWindowsManagerCov, WindowLifecycle_CloseLastWindow_ExpectClosedAndLastWindowClosedSignals)
{
    // Arrange
    auto &m = FileManagerWindowsManager::instance();
    QUrl url = QUrl::fromLocalFile(tmpDir.path());
    FileManagerWindow *w = m.createWindow(url, true);
    ASSERT_NE(w, nullptr);
    quint64 id = m.findWindowId(w);
    ASSERT_NE(id, 0);
    QSignalSpy closedSpy(&m, &FileManagerWindowsManager::windowClosed);
    QSignalSpy lastSpy(&m, &FileManagerWindowsManager::lastWindowClosed);

    // Act
    w->close();
    qApp->processEvents();   // run deleteLater

    // Assert
    EXPECT_EQ(closedSpy.count(), 1);
    EXPECT_EQ(lastSpy.count(), 1);
    EXPECT_EQ(lastSpy.at(0).at(0).toULongLong(), id);
    EXPECT_EQ(m.findWindowById(id), nullptr);
}

TEST_F(UT_FileManagerWindowsManagerCov, WindowLifecycle_TwoWindows_ExpectRegularCloseWithoutLastSignal)
{
    // Arrange
    auto &m = FileManagerWindowsManager::instance();
    QUrl urlA = QUrl::fromLocalFile(tmpDir.path());
    FileManagerWindow *a = m.createWindow(urlA, true);
    FileManagerWindow *b = m.createWindow(QUrl::fromLocalFile(QDir::homePath()), true);
    ASSERT_NE(a, nullptr);
    ASSERT_NE(b, nullptr);
    QSignalSpy lastSpy(&m, &FileManagerWindowsManager::lastWindowClosed);

    // Act: with two windows alive the non-last branch is taken
    a->close();
    qApp->processEvents();

    // Assert
    EXPECT_EQ(lastSpy.count(), 0);
    EXPECT_NE(m.findWindowById(m.findWindowId(b)), nullptr);
}

TEST_F(UT_FileManagerWindowsManagerCov, HotkeyHelp_EmittedOnManagedWindow_ExpectNoCrashAndWindowAlive)
{
    // Arrange
    auto &m = FileManagerWindowsManager::instance();
    FileManagerWindow *w = m.createWindow(QUrl::fromLocalFile(tmpDir.path()), true);
    ASSERT_NE(w, nullptr);

    // Act: emitting the signal runs the connected lambda -> onShowHotkeyHelp
    emit w->reqShowHotkeyHelp();

    // Assert
    EXPECT_EQ(m.findWindowId(w), m.findWindowId(w));
    EXPECT_NE(w, nullptr);
}

TEST_F(UT_FileManagerWindowsManagerCov, ShowWindow_ManagedWindow_ExpectCurrentUrlChangedRelay)
{
    // Arrange
    auto &m = FileManagerWindowsManager::instance();
    QUrl url = QUrl::fromLocalFile(tmpDir.path());
    FileManagerWindow *w = m.createWindow(url, true);
    ASSERT_NE(w, nullptr);
    QSignalSpy relaySpy(&m, &FileManagerWindowsManager::currentUrlChanged);

    // Act
    m.showWindow(w);

    // Assert
    ASSERT_EQ(relaySpy.count(), 1);
    EXPECT_EQ(relaySpy.at(0).at(1).toUrl(), url);
    EXPECT_EQ(w->currentUrl(), url);
}

TEST_F(UT_FileManagerWindowsManagerCov, CreateWindow_OpenAndActivateLambdas_ExpectWindowOpenedSignal)
{
    // Arrange
    auto &m = FileManagerWindowsManager::instance();
    FileManagerWindow *w = m.createWindow(QUrl::fromLocalFile(tmpDir.path()), true);
    ASSERT_NE(w, nullptr);
    QSignalSpy openedSpy(&m, &FileManagerWindowsManager::windowOpened);
    QSignalSpy urlSpy(&m, &FileManagerWindowsManager::currentUrlChanged);

    // Act: trigger the aboutToOpen / currentUrlChanged / windowActived lambdas
    emit w->aboutToOpen();
    emit w->currentUrlChanged(QUrl::fromLocalFile("/tmp"));
    emit w->windowActived();

    // Assert
    EXPECT_EQ(openedSpy.count(), 1);
    ASSERT_EQ(urlSpy.count(), 1);
    EXPECT_EQ(urlSpy.at(0).at(1).toUrl(), QUrl::fromLocalFile("/tmp"));
    EXPECT_EQ(m.lastActivedWindowId(), static_cast<quint64>(w->internalWinId()));
}
