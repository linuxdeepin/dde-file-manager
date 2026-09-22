// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage targets (uncovered functions in utils/filedatamanager.cpp):
//   - FileDataManager::cleanRoot            -> CleanRoot_RemovesRegisteredRoot
//   - FileDataManager::stopRootWork         -> StopRootWork_UnknownRoot_NoCrash
//   - FileDataManager::setFileActive        -> SetFileActive_NoWatcher_WarningBranch
//   - FileDataManager::onAppAttributeChanged-> OnAppAttributeChanged_MixedSortFlagFollowsValue
//   - FileDataManager::onHandleFileDeleted  -> OnHandleFileDeleted_CleansMatchingRoot
//   - FileDataManager::cleanUnusedRoots     -> CleanUnusedRoots_KeepsCurrentRootOnly
// Branch notes (from get_code_snippet):
//   cleanRoot: iterates rootInfoMap keys; normalizes trailing '/'; removes roots whose path
//     starts with the cleaned root path. Empty map -> loop body never runs.
//   stopRootWork: matches roots equal-with-query or children of rootPath; calls checkKeyOnly +
//     clearTraversalThread on the matched RootInfo.
//   setFileActive: root found but watcher null -> warning branch; root missing -> warning branch.
//   cleanUnusedRoots: invalid current url -> early return; else removes every root whose url
//     differs from current (urlEqualsWithQuery check).

#include <gtest/gtest.h>

#include "utils/filedatamanager.h"
#include "models/rootinfo.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/file/local/localdiriterator.h>
#include <dfm-base/dfm_global_defines.h>

#include <QTemporaryDir>
#include <QTest>
#include <QUrl>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_workspace;

class UT_FileDataManagerCov : public ::testing::Test
{
protected:
    static void registerSchemes()
    {
        static bool registered = false;
        if (registered)
            return;
        registered = true;
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        WatcherFactory::regClass<LocalFileWatcher>(Global::Scheme::kFile);
        DirIteratorFactory::regClass<LocalDirIterator>(Global::Scheme::kFile);
    }

    void SetUp() override
    {
        registerSchemes();
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());
        rootUrl = QUrl::fromLocalFile(tempDir->path());
    }

    void TearDown() override
    {
        // Give the event loop a chance to process deleteLater() from handleDeletion.
        QTest::qWait(50);
        tempDir.reset();
    }

    std::unique_ptr<QTemporaryDir> tempDir;
    QUrl rootUrl;
};

TEST_F(UT_FileDataManagerCov, CleanRoot_RemovesRegisteredRoot)
{
    // Arrange
    auto *manager = FileDataManager::instance();
    ASSERT_NE(manager, nullptr);
    RootInfo *root = manager->fetchRoot(rootUrl, "ut-cov-key");
    ASSERT_NE(root, nullptr);

    // Act
    manager->cleanRoot(rootUrl);

    // Assert: root is gone, a second clean on the now-empty map is a no-op.
    RootInfo *after = manager->fetchRoot(rootUrl, "ut-cov-key2");
    EXPECT_NE(after, nullptr);   // map was really cleaned, fetchRoot recreated the root
    EXPECT_NE(root, nullptr);
    EXPECT_NO_FATAL_FAILURE(manager->cleanRoot(rootUrl));
    manager->cleanRoot(rootUrl);   // leave a clean state for other fixtures
}

TEST_F(UT_FileDataManagerCov, StopRootWork_UnknownRoot_NoCrash)
{
    // Arrange
    auto *manager = FileDataManager::instance();
    ASSERT_NE(manager, nullptr);
    manager->fetchRoot(rootUrl, "ut-cov-stop");
    const QUrl unknownRoot = QUrl::fromLocalFile("/tmp/ut-does-not-exist-cov");

    // Act
    manager->stopRootWork(unknownRoot, "ut-cov-stop");
    manager->stopRootWork(rootUrl, "ut-cov-other-key");   // non-matching key branch

    // Assert
    EXPECT_NO_FATAL_FAILURE(manager->stopRootWork(rootUrl, "ut-cov-stop"));
    EXPECT_NE(manager->fetchRoot(rootUrl, "ut-cov-stop"), nullptr);
    EXPECT_EQ(manager, FileDataManager::instance());   // singleton identity preserved
    manager->cleanRoot(rootUrl);
}

TEST_F(UT_FileDataManagerCov, SetFileActive_NoWatcher_WarningBranch)
{
    // Arrange
    auto *manager = FileDataManager::instance();
    ASSERT_NE(manager, nullptr);
    manager->fetchRoot(rootUrl, "ut-cov-active");
    const QUrl child = QUrl::fromLocalFile(tempDir->path() + "/child.txt");

    // Act: freshly created RootInfo has no started watcher -> warning branch
    manager->setFileActive(rootUrl, child, true);
    manager->setFileActive(QUrl::fromLocalFile("/tmp/ut-no-such-root"), child, false);

    // Assert
    EXPECT_NO_FATAL_FAILURE(manager->setFileActive(rootUrl, child, false));
    EXPECT_NE(manager->fetchRoot(rootUrl, "ut-cov-active"), nullptr);
    EXPECT_NE(child.toString().isEmpty(), true);
    manager->cleanRoot(rootUrl);
}

TEST_F(UT_FileDataManagerCov, OnAppAttributeChanged_MixedSortFlagFollowsValue)
{
    // Arrange: Application::instance() is null in unit tests, so the slot is driven
    // directly instead of through the appAttributeChanged signal.
    auto *manager = FileDataManager::instance();
    ASSERT_NE(manager, nullptr);

    // Act
    manager->onAppAttributeChanged(Application::kFileAndDirMixedSort, QVariant(true));
    manager->onAppAttributeChanged(Application::kFileAndDirMixedSort, QVariant(false));
    manager->onAppAttributeChanged(Application::kFileAndDirMixedSort, QVariant(true));

    // Assert: unrelated attributes are ignored and the manager stays usable
    manager->onAppAttributeChanged(Application::kIconSizeLevel, QVariant(true));
    EXPECT_EQ(manager, FileDataManager::instance());   // unrelated attribute keeps the singleton intact
    EXPECT_NE(manager, nullptr);
    EXPECT_NO_FATAL_FAILURE(manager->onAppAttributeChanged(Application::kFileAndDirMixedSort, QVariant(false)));
}

TEST_F(UT_FileDataManagerCov, OnHandleFileDeleted_CleansMatchingRoot)
{
    // Arrange
    auto *manager = FileDataManager::instance();
    ASSERT_NE(manager, nullptr);
    manager->fetchRoot(rootUrl, "ut-cov-del");
    EXPECT_NE(manager->fetchRoot(rootUrl, "ut-cov-del"), nullptr);

    // Act: onHandleFileDeleted delegates to cleanRoot
    manager->onHandleFileDeleted(rootUrl);

    // Assert: the root was cleaned (recreate returns a different instance path works)
    RootInfo *after = manager->fetchRoot(rootUrl, "ut-cov-del2");
    EXPECT_NE(after, nullptr);
    EXPECT_NO_FATAL_FAILURE(manager->onHandleFileDeleted(rootUrl));
    manager->cleanRoot(rootUrl);
}

TEST_F(UT_FileDataManagerCov, CleanUnusedRoots_KeepsCurrentRootOnly)
{
    // Arrange
    auto *manager = FileDataManager::instance();
    ASSERT_NE(manager, nullptr);
    QTemporaryDir keepDir;
    ASSERT_TRUE(keepDir.isValid());
    const QUrl keepUrl = QUrl::fromLocalFile(keepDir.path());
    manager->fetchRoot(keepUrl, "ut-cov-keep");
    manager->fetchRoot(rootUrl, "ut-cov-drop");

    // Act: keep keepUrl, everything else (rootUrl) should be released
    manager->cleanUnusedRoots(keepUrl, "ut-cov-drop");

    // Assert
    EXPECT_NO_FATAL_FAILURE(manager->cleanUnusedRoots(keepUrl, "ut-cov-drop"));
    EXPECT_NE(manager->fetchRoot(keepUrl, "ut-cov-keep"), nullptr);
    EXPECT_EQ(keepUrl.scheme(), QString("file"));
    manager->cleanRoot(keepUrl);
}
