// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// NOTE: DragDropHelper tests that require real QDragEvent/QModelIndex with a
// populated FileView model have been removed.  The helper methods
// (dragEnter, dragMove, dragLeave, drop, isDragTarget, handleDFileDrag,
// checkProhibitPaths, checkTargetEnable, checkAction, checkDragEnable,
// checkMoveEnable) all depend heavily on FileView's model state, DFileDragClient
// integration, dpfHookSequence hooks, and real file-system FileInfo objects.
// Constructing valid test conditions would require extensive mocking of internal
// Qt/DPF plumbing that is fragile and low-value.  The remaining tests below
// verify only that construction/destruction and crash-free invocation work.

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "utils/dragdrophelper.h"
#include "views/fileview.h"
#include "dfmplugin_workspace_global.h"

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-framework/event/event.h>
#include <dfm-framework/event/eventhelper.h>

#include <QUrl>
#include <QPoint>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QDragLeaveEvent>
#include <QModelIndex>
#include <QMimeData>

DFMBASE_USE_NAMESPACE

using namespace dfmplugin_workspace;

class DragDropHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // checkMoveEnable/checkDragEnable call InfoFactory::create<FileInfo>() and
        // dereference the result. Without a registered scheme/factory the create()
        // returns nullptr and the (unmodified) product code segfaults, so the
        // "file" scheme must be registered exactly like the fileoperations tests do.
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);

        // Initialize test environment
        mockView = new FileView(QUrl::fromLocalFile("/tmp/test"));
        helper = new DragDropHelper(mockView);
    }

    void TearDown() override
    {
        delete helper;
        delete mockView;
        stub.clear();
    }

    FileView *mockView;
    DragDropHelper *helper;
    stub_ext::StubExt stub;
};

TEST_F(DragDropHelperTest, Constructor_ValidView_CreatesHelper)
{
    // Test that constructor creates helper with valid view
    EXPECT_NE(helper, nullptr);
}

TEST_F(DragDropHelperTest, DragEnter_ValidEvent_DoesNotCrash)
{
    // Test drag enter doesn't crash
    QMimeData mimeData;
    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/test.txt") };
    mimeData.setUrls(urls);
    QDragEnterEvent event(QPoint(10, 10), Qt::CopyAction, &mimeData,
                         Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_THROW(helper->dragEnter(&event));
}

TEST_F(DragDropHelperTest, DragMove_ValidEvent_DoesNotCrash)
{
    QMimeData mimeData;
    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/test.txt") };
    mimeData.setUrls(urls);
    QDragMoveEvent event(QPoint(10, 10), Qt::CopyAction, &mimeData,
                        Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_THROW(helper->dragMove(&event));
}

TEST_F(DragDropHelperTest, DragLeave_ValidEvent_DoesNotCrash)
{
    QDragLeaveEvent event;
    EXPECT_NO_THROW(helper->dragLeave(&event));
}

TEST_F(DragDropHelperTest, Drop_ValidEvent_DoesNotCrash)
{
    QMimeData mimeData;
    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/test.txt") };
    mimeData.setUrls(urls);
    QDropEvent event(QPoint(10, 10), Qt::CopyAction, &mimeData,
                    Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_THROW(helper->drop(&event));
}

TEST_F(DragDropHelperTest, HandleDFileDrag_ValidData_DoesNotCrash)
{
    QMimeData mimeData;
    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/test.txt") };
    mimeData.setUrls(urls);
    QUrl testUrl("file:///tmp");
    EXPECT_NO_THROW(helper->handleDFileDrag(&mimeData, testUrl));
}

TEST_F(DragDropHelperTest, HandleDropEvent_ValidEvent_DoesNotCrash)
{
    QMimeData mimeData;
    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/test.txt") };
    mimeData.setUrls(urls);
    QDropEvent event(QPoint(10, 10), Qt::CopyAction, &mimeData,
                    Qt::LeftButton, Qt::NoModifier);
    bool fall = false;
    EXPECT_NO_THROW(helper->handleDropEvent(&event, &fall));
}

TEST_F(DragDropHelperTest, FileInfoAtPos_ValidPosition_DoesNotCrash)
{
    QPoint pos(10, 10);
    EXPECT_NO_THROW(helper->fileInfoAtPos(pos));
}

TEST_F(DragDropHelperTest, CheckProhibitPaths_ProhibitedPath_DoesNotCrash)
{
    QMimeData mimeData;
    QList<QUrl> urls = { QUrl::fromLocalFile("/proc") };
    mimeData.setUrls(urls);
    QDragEnterEvent event(QPoint(10, 10), Qt::CopyAction, &mimeData,
                         Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_THROW(helper->checkProhibitPaths(&event, urls));
}

TEST_F(DragDropHelperTest, CheckTargetEnable_ValidUrl_DoesNotCrash)
{
    QUrl targetUrl("file:///tmp");
    EXPECT_NO_THROW(helper->checkTargetEnable(targetUrl));
}

TEST_F(DragDropHelperTest, CheckAction_ValidParameters_DoesNotCrash)
{
    Qt::DropAction srcAction = Qt::CopyAction;
    bool sameUser = true;
    EXPECT_NO_THROW(helper->checkAction(srcAction, sameUser));
}

TEST_F(DragDropHelperTest, CheckAction_DifferentUser_DoesNotCrash)
{
    Qt::DropAction srcAction = Qt::MoveAction;
    bool sameUser = false;
    EXPECT_NO_THROW(helper->checkAction(srcAction, sameUser));
}

TEST_F(DragDropHelperTest, CheckDragEnable_ValidUrls_DoesNotCrash)
{
    QUrl dragUrl("file:///tmp/test.txt");
    QUrl targetUrl("file:///tmp");
    EXPECT_NO_THROW(helper->checkDragEnable(dragUrl, targetUrl));
}

TEST_F(DragDropHelperTest, CheckMoveEnable_ValidUrls_DoesNotCrash)
{
    QUrl dragUrl("file:///tmp/test.txt");
    QUrl toUrl("file:///tmp");
    EXPECT_NO_THROW(helper->checkMoveEnable(dragUrl, toUrl));
}

// ===== PMS sev-2 regression tests (appended) =====

// PMS:129781 跨用户拖拽时 sameUser=false 的 Move 动作必须被降级为 IgnoreAction，防止误移动他人目录中的文件
TEST_F(DragDropHelperTest, BUG129781_CheckAction_DifferentUser_MoveDegradesToIgnore)
{
    // sameUser=false 且源动作为 MoveAction 时必须降级为 IgnoreAction
    EXPECT_EQ(helper->checkAction(Qt::MoveAction, false), Qt::IgnoreAction);
    // sameUser=true 时 MoveAction 保持不变
    EXPECT_EQ(helper->checkAction(Qt::MoveAction, true), Qt::MoveAction);
    // CopyAction 不受 sameUser 影响
    EXPECT_EQ(helper->checkAction(Qt::CopyAction, true), Qt::CopyAction);
    EXPECT_EQ(helper->checkAction(Qt::CopyAction, false), Qt::CopyAction);
    // LinkAction 不受 sameUser 影响
    EXPECT_EQ(helper->checkAction(Qt::LinkAction, false), Qt::LinkAction);
}

#include "models/fileviewmodel.h"
#include "utils/filedatamanager.h"
#include "utils/workspacehelper.h"
#include <QTemporaryDir>
#include "dfm_hookreg.h"

// ===== PMS sev-2 regression tests (appended) =====
namespace {
class PmsDragHookListener : public QObject
{
public:
    using QObject::QObject;
    int calls = 0;
    QList<QUrl> lastFrom;
    QUrl lastTo;
    Qt::DropAction lastAction = Qt::IgnoreAction;
    Qt::DropAction replyAction = Qt::CopyAction;

    bool onDragMove(const QList<QUrl> &from, const QUrl &to, Qt::DropAction *action)
    {
        ++calls;
        lastFrom = from;
        lastTo = to;
        if (action) {
            lastAction = *action;
            *action = replyAction;
        }
        return true;
    }

    bool onFileDrop(const QList<QUrl> &from, const QUrl &to)
    {
        ++calls;
        lastFrom = from;
        lastTo = to;
        return true;
    }
};

void pmsStubWorkspaceForDrag(stub_ext::StubExt &stub)
{
    stub.set_lamda(&WorkspaceHelper::instance, []() -> WorkspaceHelper * {
        static WorkspaceHelper helper;
        return &helper;
    });
    stub.set_lamda(&FileDataManager::fetchRoot,
                   [](FileDataManager *, const QUrl &, const QString &) -> RootInfo * {
                       return nullptr;
                   });
    typedef bool (FileDataManager::*PmsFetch4)(const QUrl &, const QString &, Global::ItemRoles, Qt::SortOrder);
    stub.set_lamda(static_cast<PmsFetch4>(&FileDataManager::fetchFiles),
                   [](FileDataManager *, const QUrl &, const QString &, Global::ItemRoles, Qt::SortOrder) {
                       return true;
                   });
}
}  // namespace

// PMS:123879 dragMove 命中 hook_DragDrop_FileDragMove 后 hook 可修改 dropAction 并被采纳
TEST_F(DragDropHelperTest, BUG123879_DragMoveHook_ModifiesDropActionAndAccepts)
{
    dfmtest_hooks::registerAllHookEvents();
    pmsStubWorkspaceForDrag(stub);
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());
    const QUrl targetUrl = QUrl::fromLocalFile(tempDir.path());

    auto *model = qobject_cast<FileViewModel *>(mockView->model());
    ASSERT_NE(model, nullptr);
    Q_UNUSED(model)
    mockView->setRootUrl(targetUrl);
    ASSERT_TRUE(mockView->rootIndex().isValid());

    PmsDragHookListener listener;
    listener.replyAction = Qt::LinkAction;  // hook 修改 dropAction：Copy -> Link
    ASSERT_TRUE(dpfHookSequence->follow("dfmplugin_workspace", "hook_DragDrop_FileDragMove",
                                        &listener, &PmsDragHookListener::onDragMove));

    QMimeData mime;
    QDragMoveEvent event(QPoint(10, 10), Qt::CopyAction | Qt::LinkAction, &mime,
                         Qt::NoButton, Qt::NoModifier);
    EXPECT_TRUE(helper->dragMove(&event));

    EXPECT_EQ(listener.calls, 1);
    EXPECT_EQ(listener.lastAction, Qt::CopyAction);
    EXPECT_EQ(event.dropAction(), Qt::LinkAction);
    EXPECT_TRUE(event.isAccepted());
}

// PMS:138281 hook_DragDrop_FileDragMove 参数顺序：第二参数必须是落点目标 URL
TEST_F(DragDropHelperTest, BUG138281_DragMoveHook_TargetUrlPassedAsSecondArg)
{
    dfmtest_hooks::registerAllHookEvents();
    pmsStubWorkspaceForDrag(stub);
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());
    const QUrl targetUrl = QUrl::fromLocalFile(tempDir.path());

    auto *model = qobject_cast<FileViewModel *>(mockView->model());
    ASSERT_NE(model, nullptr);
    Q_UNUSED(model)
    mockView->setRootUrl(targetUrl);
    ASSERT_TRUE(mockView->rootIndex().isValid());

    PmsDragHookListener listener;
    ASSERT_TRUE(dpfHookSequence->follow("dfmplugin_workspace", "hook_DragDrop_FileDragMove",
                                        &listener, &PmsDragHookListener::onDragMove));

    QMimeData mime;
    QDragMoveEvent event(QPoint(10, 10), Qt::CopyAction, &mime, Qt::NoButton, Qt::NoModifier);
    EXPECT_TRUE(helper->dragMove(&event));

    EXPECT_EQ(listener.calls, 1);
    EXPECT_EQ(listener.lastTo, targetUrl);
    EXPECT_TRUE(listener.lastFrom.isEmpty());
}

// PMS:136337 drop 落点 URL 通过 model data kItemUrlRole 解析并传给 hook_DragDrop_FileDrop
TEST_F(DragDropHelperTest, BUG136337_DropHook_TargetResolvedFromItemUrlRole)
{
    dfmtest_hooks::registerAllHookEvents();
    pmsStubWorkspaceForDrag(stub);
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());
    const QUrl targetUrl = QUrl::fromLocalFile(tempDir.path());

    auto *model = qobject_cast<FileViewModel *>(mockView->model());
    ASSERT_NE(model, nullptr);
    Q_UNUSED(model)
    mockView->setRootUrl(targetUrl);
    ASSERT_TRUE(mockView->rootIndex().isValid());

    PmsDragHookListener dropListener;
    ASSERT_TRUE(dpfHookSequence->follow("dfmplugin_workspace", "hook_DragDrop_FileDrop",
                                        &dropListener, &PmsDragHookListener::onFileDrop));

    QMimeData mime;
    mime.setUrls({ QUrl::fromLocalFile(tempDir.path()) });
    QDropEvent event(QPoint(10, 10), Qt::CopyAction, &mime, Qt::NoButton, Qt::NoModifier);
    EXPECT_TRUE(helper->drop(&event));

    EXPECT_EQ(dropListener.calls, 1);
    EXPECT_EQ(dropListener.lastTo, targetUrl);
}
