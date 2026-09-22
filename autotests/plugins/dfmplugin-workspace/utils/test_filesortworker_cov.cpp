// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage targets (uncovered functions in utils/filesortworker.cpp), reached through the
// private-access helper from 3rdparty/testutils/cpp-stub/addr_pri.h:
//   ctor(full args)             -> SetUp (fixture constructs worker with callback+filters)
//   handleGroupingChanged       -> HandleGroupingChanged_EmptyState_EarlyReturn
//   handleGroupingRemove        -> HandleGroupingRemove_EmptyState_NoCrash
//   handleGroupingUpdate        -> HandleGroupingUpdate_EmptyState_NoCrash
//   handleGroupingInsert        -> HandleGroupingInsert_CoverageRange_AppliesEmptyGrouping
//   handleToggleGroupTruncation -> HandleToggleGroupTruncation_GuardsRejected
//   checkNameFilters            -> CheckNameFilters_MatchAndDirBranches
//   sortInfoUpdateByFileInfo    -> SortInfoUpdateByFileInfo_UnknownChildReturnsFalse
//   switchListView              -> SwitchListView_ClearsTreeState
//   sortAllTreeFilesByParent    -> SortAllTreeFilesByParent_EmptyWorkerReturnsEmpty
//   removeChildrenByParents     -> RemoveChildrenByParents_UnknownDirReturnsEmpty
//   removeVisibleTreeChildren   -> RemoveVisibleTreeChildren_EmptyDepthMapReturnsEmpty
//   removeSubDir                -> RemoveSubDir_UnknownDir_NoCrash
//   removeFileItems             -> RemoveFileItems_RemovesUnknownUrlsQuietly
//   findEndPos                  -> FindEndPos_EmptyWorkerBoundaryValues
//   findStartPos                -> FindStartPos_PositionAndMissBranches
//   removeVisibleChildren       -> RemoveVisibleChildren_NonPositiveSizeIgnored
//   insertSortList              -> InsertSortList_EmptyAndSingleEntryBranches
//   isDefaultHiddenFile         -> IsDefaultHiddenFile_OrdinaryFilesNotHidden
//   findRealShowIndex           -> FindRealShowIndex_UnknownItemReturnsMinusOne... (index+1 = 0)
//   indexOfVisibleChild         -> IndexOfVisibleChild_NotPresentReturnsMinusOne
//   checkAndSortBytMimeType     -> CheckAndSortBytMimeType_WorkerThreadIdlePath
//   doCompleteFileInfo          -> DoCompleteFileInfo_RealFileAndMissingFile
//   getAllFiles                 -> GetAllFiles_EmptyWorkerReturnsEmpty
//   applyGrouping               -> ApplyGrouping_EmptyFileListEarlyReturn
//   updateSorterContext         -> InsertSortList test reaches it (also direct call)
// Branch notes are inline in each test.

#include <gtest/gtest.h>

#include "addr_pri.h"
#include "stubext.h"

#include "utils/filesortworker.h"
#include "utils/workspacehelper.h"
#include "models/fileitemdata.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/file/local/localdiriterator.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/interfaces/sortfileinfo.h>
#include <dfm-base/dfm_global_defines.h>

#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QUrl>
#include <QVariant>
#include <QTest>

#include <thread>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
using namespace dfmplugin_workspace;

// --- private member access (works for private/protected members alike) ---
ACCESS_PRIVATE_FUN(FileSortWorker, void(), handleGroupingChanged)
ACCESS_PRIVATE_FUN(FileSortWorker, void(), handleGroupingInsert)
ACCESS_PRIVATE_FUN(FileSortWorker, void(), handleGroupingRemove)
ACCESS_PRIVATE_FUN(FileSortWorker, void(), handleGroupingUpdate)
ACCESS_PRIVATE_FUN(FileSortWorker, void(const QString &, const QString &), handleToggleGroupTruncation)
ACCESS_PRIVATE_FUN(FileSortWorker, void(FileItemDataPointer), checkNameFilters)
ACCESS_PRIVATE_FUN(FileSortWorker, bool(FileInfoPointer), sortInfoUpdateByFileInfo)
ACCESS_PRIVATE_FUN(FileSortWorker, void(), switchListView)
ACCESS_PRIVATE_FUN(FileSortWorker, QList<QUrl>(const QUrl &, bool), sortAllTreeFilesByParent)
ACCESS_PRIVATE_FUN(FileSortWorker, QList<QUrl>(const QList<QUrl> &), removeChildrenByParents)
ACCESS_PRIVATE_FUN(FileSortWorker, QList<QUrl>(const QUrl &), removeVisibleTreeChildren)
ACCESS_PRIVATE_FUN(FileSortWorker, void(const QUrl &), removeSubDir)
ACCESS_PRIVATE_FUN(FileSortWorker, void(const QList<QUrl> &), removeFileItems)
ACCESS_PRIVATE_FUN(FileSortWorker, int(const QUrl &), findEndPos)
ACCESS_PRIVATE_FUN(FileSortWorker, void(int, int), removeVisibleChildren)
ACCESS_PRIVATE_FUN(FileSortWorker, int(const QUrl &, const QList<QUrl> &), insertSortList)
ACCESS_PRIVATE_FUN(FileSortWorker, bool(const QUrl &), isDefaultHiddenFile)
ACCESS_PRIVATE_FUN(FileSortWorker, int(const QUrl &), findRealShowIndex)
ACCESS_PRIVATE_FUN(FileSortWorker, int(const QUrl &), indexOfVisibleChild)
ACCESS_PRIVATE_FUN(FileSortWorker, void(const QUrl &), checkAndSortBytMimeType)
ACCESS_PRIVATE_FUN(FileSortWorker, void(SortInfoPointer), doCompleteFileInfo)
ACCESS_PRIVATE_FUN(FileSortWorker, QList<FileItemDataPointer>() const, getAllFiles)
ACCESS_PRIVATE_FUN(FileSortWorker, void(const QList<FileItemDataPointer> &), applyGrouping)
ACCESS_PRIVATE_FUN(FileSortWorker, void(), updateSorterContext)

// findStartPos is overloaded (list+parent and single-url variants), so the plain macro
// cannot resolve &FileSortWorker::findStartPos. Use an explicitly typed instantiation of
// the same private-access template to pick the two-argument overload.
namespace {
namespace private_access_detail {
struct TagFindStartPosList final {};
template struct private_access<int (FileSortWorker::*)(const QList<QUrl> &, const QUrl &),
                                &FileSortWorker::findStartPos, TagFindStartPosList>;
using PtrType_TagFindStartPosList = int (FileSortWorker::*)(const QList<QUrl> &, const QUrl &);
PtrType_TagFindStartPosList get(TagFindStartPosList);
}   // namespace private_access_detail

inline int findStartPosInList(FileSortWorker &w, const QList<QUrl> &list, const QUrl &parent)
{
    return (w.*private_access_detail::get(private_access_detail::TagFindStartPosList {}))(list, parent);
}
}   // namespace

class UT_FileSortWorkerCov : public ::testing::Test
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

        realFile = tempDir->path() + "/data.bin";
        {
            QFile f(realFile);
            ASSERT_TRUE(f.open(QIODevice::WriteOnly));
            ASSERT_EQ(f.write("0123456789A", 11), 11);
        }

        stub.set_lamda(&Application::appAttribute, []() {
            return QVariant(false);
        });
        stub.set_lamda(&WorkspaceHelper::instance, []() {
            static WorkspaceHelper helper;
            return &helper;
        });
        stub.set_lamda(&WorkspaceHelper::isViewModeSupported, []() {
            return true;
        });
        stub.set_lamda(&DeviceProxyManager::instance, []() {
            static DeviceProxyManager manager;
            return &manager;
        });

        // full-argument constructor (uncovered overload from the gap list)
        worker = new FileSortWorker(rootUrl, testKey,
                                    [](const void *, const QVariant &) { return true; },
                                    QStringList { "*.txt" },
                                    QDir::Filters(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot));
    }

    void TearDown() override
    {
        delete worker;
        stub.clear();
        tempDir.reset();
    }

    std::unique_ptr<QTemporaryDir> tempDir;
    QUrl rootUrl;
    QString realFile;
    QString testKey = "ut-sort-cov-key";
    FileSortWorker *worker = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_FileSortWorkerCov, Ctor_FullArguments_BuildsConsistentInitialState)
{
    // Arrange (worker built in SetUp with callback + name filters + dir filters)

    // Act
    const int count = worker->childrenCount();
    const auto urls = worker->getChildrenUrls();

    // Assert
    EXPECT_EQ(count, 0);
    EXPECT_TRUE(urls.isEmpty());
    EXPECT_EQ(worker->getSortOrder(), Qt::AscendingOrder);
}

TEST_F(UT_FileSortWorkerCov, HandleGroupingChanged_EmptyState_EarlyReturn)
{
    // Arrange: no grouping data at all (groupedModelData and visibleChildren empty)

    // Act
    call_private_fun::FileSortWorkerhandleGroupingChanged(*worker);

    // Assert: state untouched -> children count still 0, grouping flag still off
    EXPECT_EQ(worker->childrenCount(), 0);
    EXPECT_FALSE(worker->currentIsGroupingMode());
}

TEST_F(UT_FileSortWorkerCov, HandleToggleGroupTruncation_GuardsRejected)
{
    // Arrange: grouping disabled, unknown group key

    // Act
    call_private_fun::FileSortWorkerhandleToggleGroupTruncation(*worker, testKey, "grp-1");
    call_private_fun::FileSortWorkerhandleToggleGroupTruncation(*worker, testKey, "");

    // Assert: guard branches hit, nothing changed
    EXPECT_EQ(worker->childrenCount(), 0);
    EXPECT_EQ(worker->getGroupItemCount(), 0);
}

TEST_F(UT_FileSortWorkerCov, CheckNameFilters_MatchAndDirBranches)
{
    // Arrange
    auto makeItem = [this](const QString &name, bool isDir) {
        auto info = QSharedPointer<SortFileInfo>::create();
        info->setUrl(QUrl::fromLocalFile(tempDir->path() + "/" + name));
        info->setFile(!isDir);
        info->setDir(isDir);
        return FileItemDataPointer(new FileItemData(SortInfoPointer(info), nullptr));
    };
    FileItemDataPointer fileItem = makeItem("notes.txt", false);
    FileItemDataPointer dirItem = makeItem("somedir", true);
    ASSERT_NE(fileItem, nullptr);

    // Act: "*.txt" matches notes.txt; dir items return early
    call_private_fun::FileSortWorkercheckNameFilters(*worker, fileItem);
    call_private_fun::FileSortWorkercheckNameFilters(*worker, dirItem);
    call_private_fun::FileSortWorkercheckNameFilters(*worker, FileItemDataPointer());

    // Assert
    EXPECT_NO_FATAL_FAILURE(call_private_fun::FileSortWorkercheckNameFilters(*worker, fileItem));
    EXPECT_NE(fileItem, nullptr);
    EXPECT_EQ(worker->childrenCount(), 0);
}

TEST_F(UT_FileSortWorkerCov, SortInfoUpdateByFileInfo_UnknownChildReturnsFalse)
{
    // Arrange: real file info exists, but it was never added as a child of the worker
    FileInfoPointer info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(realFile));
    ASSERT_NE(info, nullptr);

    // Act
    const bool nullInfo = call_private_fun::FileSortWorkersortInfoUpdateByFileInfo(*worker, FileInfoPointer());
    const bool unknownChild = call_private_fun::FileSortWorkersortInfoUpdateByFileInfo(*worker, info);

    // Assert: both guarded branches return false
    EXPECT_EQ(nullInfo ? 1 : 0, 0);
    EXPECT_FALSE(nullInfo);
    EXPECT_FALSE(unknownChild);
}

TEST_F(UT_FileSortWorkerCov, SwitchListView_ClearsTreeState)
{
    // Arrange: switch to tree then back to list so both internal maps get touched
    worker->handleSwitchTreeView(true);

    // Act
    call_private_fun::FileSortWorkerswitchListView(*worker);

    // Assert
    EXPECT_EQ(worker->childrenCount(), 0);
    EXPECT_TRUE(worker->getChildrenUrls().isEmpty());
}

TEST_F(UT_FileSortWorkerCov, SortAllTreeFilesByParent_EmptyWorkerReturnsEmpty)
{
    // Arrange: empty visible children and empty depth map

    // Act
    const QList<QUrl> result = call_private_fun::FileSortWorkersortAllTreeFilesByParent(*worker, rootUrl, false);

    // Assert
    EXPECT_TRUE(result.isEmpty());
    EXPECT_EQ(worker->childrenCount(), 0);
}

TEST_F(UT_FileSortWorkerCov, RemoveHelpers_EmptyState_AllBranchesSafe)
{
    // Arrange
    const QUrl unknownDir = QUrl::fromLocalFile(tempDir->path() + "/no-such-dir");

    // Act
    const QList<QUrl> unknownDirs { unknownDir };
    const QList<QUrl> byParents = call_private_fun::FileSortWorkerremoveChildrenByParents(*worker, unknownDirs);
    const QList<QUrl> visibleTree = call_private_fun::FileSortWorkerremoveVisibleTreeChildren(*worker, unknownDir);
    call_private_fun::FileSortWorkerremoveSubDir(*worker, unknownDir);
    call_private_fun::FileSortWorkerremoveFileItems(*worker, unknownDirs);

    // Assert
    EXPECT_TRUE(byParents.isEmpty());
    EXPECT_TRUE(visibleTree.isEmpty());
    EXPECT_EQ(worker->childrenCount(), 0);
}

TEST_F(UT_FileSortWorkerCov, FindEndPos_EmptyWorkerBoundaryValues)
{
    // Arrange: empty worker, root url and one unknown sibling

    // Act
    const int rootEnd = call_private_fun::FileSortWorkerfindEndPos(*worker, rootUrl);
    const int unknownEnd = call_private_fun::FileSortWorkerfindEndPos(*worker, QUrl::fromLocalFile(tempDir->path() + "/other"));

    // Assert: root maps to childrenCountInternal() == 0; unknown -> not in tree -> -1
    EXPECT_EQ(rootEnd, 0);
    EXPECT_EQ(unknownEnd, -1);
}

TEST_F(UT_FileSortWorkerCov, FindStartPos_PositionAndMissBranches)
{
    // Arrange
    const QUrl a = QUrl::fromLocalFile(tempDir->path() + "/a");
    const QUrl b = QUrl::fromLocalFile(tempDir->path() + "/b");
    const QUrl c = QUrl::fromLocalFile(tempDir->path() + "/c");
    const QList<QUrl> list { a, b };

    // Act
    const int asRoot = findStartPosInList(*worker, list, rootUrl);
    const int afterB = findStartPosInList(*worker, list, b);
    const int miss = findStartPosInList(*worker, list, c);

    // Assert
    EXPECT_EQ(asRoot, 0);
    EXPECT_EQ(afterB, 2);   // insert position is one past the parent
    EXPECT_EQ(miss, -1);
}

TEST_F(UT_FileSortWorkerCov, IndexHelpers_UnknownItemsReportAbsent)
{
    // Arrange
    const QUrl unknown = QUrl::fromLocalFile(tempDir->path() + "/vanish.txt");

    // Act
    const int visibleIdx = call_private_fun::FileSortWorkerindexOfVisibleChild(*worker, unknown);
    const int realShow = call_private_fun::FileSortWorkerfindRealShowIndex(*worker, unknown);
    const int childShow = worker->getChildShowIndex(unknown);

    // Assert: indexOfVisibleChild -> -1; findRealShowIndex -> indexOfVisibleChild()+1 == 0
    EXPECT_EQ(visibleIdx, -1);
    EXPECT_EQ(realShow, 0);
    EXPECT_EQ(childShow, -1);
}

TEST_F(UT_FileSortWorkerCov, RemoveVisibleChildren_NonPositiveSizeIgnored)
{
    // Arrange: visible children list is empty

    // Act
    call_private_fun::FileSortWorkerremoveVisibleChildren(*worker, 0, 0);
    call_private_fun::FileSortWorkerremoveVisibleChildren(*worker, 0, -3);

    // Assert
    EXPECT_EQ(worker->childrenCount(), 0);
    EXPECT_TRUE(worker->getChildrenUrls().isEmpty());
}

TEST_F(UT_FileSortWorkerCov, InsertSortList_EmptyAndSingleEntryBranches)
{
    // Arrange
    const QUrl node = QUrl::fromLocalFile(tempDir->path() + "/n.txt");
    const QList<QUrl> one { node };

    // Act: empty list early-returns 0; single entry runs updateSorterContext + binary search
    const QList<QUrl> noEntries;
    const int emptyPos = call_private_fun::FileSortWorkerinsertSortList(*worker, node, noEntries);
    const int singlePos = call_private_fun::FileSortWorkerinsertSortList(*worker, node, one);
    call_private_fun::FileSortWorkerupdateSorterContext(*worker);

    // Assert
    EXPECT_EQ(emptyPos, 0);
    EXPECT_GE(singlePos, 0);
    EXPECT_LE(singlePos, 1);
}

TEST_F(UT_FileSortWorkerCov, IsDefaultHiddenFile_OrdinaryFilesNotHidden)
{
    // Arrange
    const QUrl normal = QUrl::fromLocalFile(realFile);
    const QUrl other = QUrl::fromLocalFile(tempDir->path() + "/plain.log");

    // Act
    const bool normalHidden = call_private_fun::FileSortWorkerisDefaultHiddenFile(*worker, normal);
    const bool otherHidden = call_private_fun::FileSortWorkerisDefaultHiddenFile(*worker, other);

    // Assert: default hidden set only contains <mountpoint>/root and lost+found entries
    EXPECT_EQ(normalHidden, false);
    EXPECT_FALSE(normalHidden);
    EXPECT_FALSE(otherHidden);
}

TEST_F(UT_FileSortWorkerCov, CheckAndSortBytMimeType_WorkerThreadIdlePath)
{
    // Arrange: function asserts it is not called on the gui thread; mimeSorting is false
    const QUrl someUrl = QUrl::fromLocalFile(realFile);
    std::string failure;

    // Act
    std::thread workerThread([&] {
      try {
        call_private_fun::FileSortWorkercheckAndSortBytMimeType(*worker, someUrl);
      } catch (const std::exception &e) {
        failure = e.what();
      }
    });
    workerThread.join();

    // Assert: idle path returns immediately, state unchanged
    EXPECT_TRUE(failure.empty());
    EXPECT_EQ(worker->childrenCount(), 0);
}

TEST_F(UT_FileSortWorkerCov, DoCompleteFileInfo_RealFileAndMissingFile)
{
    // Arrange
    auto goodInfo = QSharedPointer<SortFileInfo>::create();
    goodInfo->setUrl(QUrl::fromLocalFile(realFile));
    auto badInfo = QSharedPointer<SortFileInfo>::create();
    badInfo->setUrl(QUrl::fromLocalFile(tempDir->path() + "/ghost.bin"));
    ASSERT_FALSE(goodInfo->isInfoCompleted());

    // Act
    call_private_fun::FileSortWorkerdoCompleteFileInfo(*worker, SortInfoPointer(goodInfo));
    call_private_fun::FileSortWorkerdoCompleteFileInfo(*worker, SortInfoPointer(badInfo));

    // Assert: statx succeeded for the real file, failed for the ghost file
    EXPECT_EQ(goodInfo->isInfoCompleted(), true);
    EXPECT_TRUE(goodInfo->isInfoCompleted());
    EXPECT_FALSE(badInfo->isInfoCompleted());
}

TEST_F(UT_FileSortWorkerCov, GetAllFiles_EmptyWorkerReturnsEmpty)
{
    // Arrange: empty children data map

    // Act
    const QList<FileItemDataPointer> files = call_private_fun::FileSortWorkergetAllFiles(*worker);

    // Assert
    EXPECT_TRUE(files.isEmpty());
    EXPECT_EQ(static_cast<int>(files.size()), worker->childrenCount());
}

TEST_F(UT_FileSortWorkerCov, HandleGroupingRemoveAndUpdate_EmptyState_NoCrash)
{
    // Arrange: install the fallback (NoGroup) strategy so engine state is initialized
    worker->handleReGrouping(Qt::AscendingOrder, GroupStrategy::kNoGroup, QVariantHash());

    // Act
    call_private_fun::FileSortWorkerhandleGroupingRemove(*worker);
    call_private_fun::FileSortWorkerhandleGroupingUpdate(*worker);

    // Assert
    EXPECT_EQ(worker->childrenCount(), 0);
    EXPECT_EQ(worker->getGroupItemCount(), 0);
}

TEST_F(UT_FileSortWorkerCov, HandleGroupingInsert_CoverageRange_AppliesEmptyGrouping)
{
    // Arrange: NoGroup strategy installed so applyGrouping's Q_ASSERT(currentStrategy) holds
    worker->handleReGrouping(Qt::AscendingOrder, GroupStrategy::kNoGroup, QVariantHash());

    // Act: range (0,0) with 0 visible children counts as full coverage -> applyGrouping(getAllFiles())
    call_private_fun::FileSortWorkerhandleGroupingInsert(*worker);

    // Assert
    EXPECT_EQ(worker->childrenCount(), 0);
    EXPECT_TRUE(worker->getChildrenUrls().isEmpty());
}

TEST_F(UT_FileSortWorkerCov, ApplyGrouping_EmptyFileListEarlyReturn)
{
    // Arrange: NoGroup strategy installed; empty file list
    worker->handleReGrouping(Qt::AscendingOrder, GroupStrategy::kNoGroup, QVariantHash());

    // Act
    call_private_fun::FileSortWorkerapplyGrouping(*worker, QList<FileItemDataPointer>());

    // Assert
    EXPECT_EQ(worker->getGroupItemCount(), 0);
    EXPECT_FALSE(worker->currentIsGroupingMode());
}
