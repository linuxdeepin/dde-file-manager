#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "dfmevent.h"
#include "dfilemenu.h"
#include "stub.h"
#include "stubext.h"

#define private public
#include "controllers/fileeventprocessor.cpp"
#include <QDialog>

using DFM_NAMESPACE::FileEventProcessor;

namespace  {
    class FileEventProcessorTest : public testing::Test
    {
    public:
        static void SetUpTestCase()
        {
            system("mkdir /tmp/test_file_processor/");
        }

        static void TearDownTestCase()
        {
            system("rm -rf /tmp/test_file_processor/");
        }

        virtual void SetUp() override
        {
            processer = new FileEventProcessor();
            system("touch /tmp/test_file_processor/test_a");
            system("touch /tmp/test_file_processor/test_b");
        }

        virtual void TearDown() override
        {
            delete processer;
            processer = nullptr;
        }

        QString getTestPath()
        {
            return "/tmp/test_file_processor/";
        }

        QSharedPointer<DFMMenuActionEvent> makeTestMenuAction(DFMGlobal::MenuAction action = DFMGlobal::MenuAction::Unknow)
        {
            const QObject *sender = nullptr;
            const DFileMenu *menu = new DFileMenu(nullptr);
            DUrl url = DUrl::fromLocalFile(getTestPath());
            DUrlList selectedList;
            selectedList.append(url);
            auto event = dMakeEventPointer<DFMMenuActionEvent>(sender,
                                                               menu,
                                                               url,
                                                               selectedList,
                                                               action
                                                               );
            return event;
        }

        static bool processMenuEvent(const QSharedPointer<DFMMenuActionEvent> &event)
        {
            bool ret = dde_file_manager::processMenuEvent(event);
            if (event->menu())
                delete event->menu();

            return ret;
        }

        FileEventProcessor *processer;
    };
}

TEST_F(FileEventProcessorTest, tst_is_avfs_mounted)
{
    bool mounted = isAvfsMounted();
    EXPECT_FALSE(mounted);
}

TEST_F(FileEventProcessorTest, tst_menu_event_rename_tag)
{
    auto event = makeTestMenuAction(DFMGlobal::MenuAction::RenameTag);
    bool result = processMenuEvent(event);
    EXPECT_TRUE(result);
}

TEST_F(FileEventProcessorTest, tst_menu_event_change_tag_color)
{
    DFMGlobal::MenuAction action = DFMGlobal::MenuAction::ChangeTagColor;
    const QObject *sender = nullptr;

    DFileMenu *menu = new DFileMenu(nullptr);
    DTagActionWidget *tagWidget = new DTagActionWidget;
    QWidgetAction *tagAction = new QWidgetAction(menu);
    tagAction->setDefaultWidget(tagWidget);
    tagAction->setText("Change color of present tag");

    menu->addAction(tagAction);

    tagWidget->setCheckedColorList({Qt::red});
    tagWidget->setExclusive(true);
    tagWidget->setToolTipVisible(false);

    DUrl url = DUrl::fromLocalFile(getTestPath());
    DUrlList selectedList;
    selectedList.append(url);
    auto event = dMakeEventPointer<DFMMenuActionEvent>(sender,
                                                       menu,
                                                       url,
                                                       selectedList,
                                                       action
                                                       );
    bool result = processMenuEvent(event);
    EXPECT_TRUE(result);
}

TEST_F(FileEventProcessorTest, tst_menu_event_tag_files_use_color)
{
    stub_ext::StubExt stext;
    stext.set_lamda(VADDR(QDialog, exec), [](){ return (int)QDialog::Rejected; });
    stext.set_lamda(VADDR(DFileService, makeTagsOfFiles), [](){ return true; });

    DFMGlobal::MenuAction action = DFMGlobal::MenuAction::TagFilesUseColor;
    const QObject *sender = nullptr;

    DFileMenu *menu = new DFileMenu(nullptr);
    DTagActionWidget *tagWidget = new DTagActionWidget;
    QWidgetAction *tagAction = new QWidgetAction(menu);
    tagAction->setDefaultWidget(tagWidget);
    tagAction->setText("Add color tags");

    menu->addAction(tagAction);

    tagWidget->setCheckedColorList({Qt::red});
    tagWidget->setExclusive(true);
    tagWidget->setToolTipVisible(false);

    DUrl url = DUrl::fromLocalFile(getTestPath());
    DUrlList selectedList;
    selectedList.append(url);
    auto event = dMakeEventPointer<DFMMenuActionEvent>(sender,
                                                       menu,
                                                       url,
                                                       selectedList,
                                                       action
                                                       );
    bool result = processMenuEvent(event);
    EXPECT_TRUE(result);
}

TEST_F(FileEventProcessorTest, tst_menu_delete_tag)
{
    stub_ext::StubExt stext;
    stext.set_lamda(&DFileService::deleteFiles, [](){ return true; });

    auto menuEvent = makeTestMenuAction(DFMGlobal::MenuAction::DeleteTags);
    bool ret = processMenuEvent(menuEvent);
    EXPECT_TRUE(ret);
}

TEST_F(FileEventProcessorTest, tst_menu_open_functions)
{
    stub_ext::StubExt stext;
    stext.set_lamda(ADDR(AppController, actionOpen), [](){});
    stext.set_lamda(ADDR(AppController, actionOpenDisk), [](){});
    stext.set_lamda(ADDR(AppController, actionOpenInNewWindow), [](){});
    stext.set_lamda(ADDR(AppController, actionOpenInNewTab), [](){});
    stext.set_lamda(ADDR(AppController, actionOpenDiskInNewTab), [](){});
    stext.set_lamda(ADDR(AppController, actionOpenAsAdmin), [](){});
    stext.set_lamda(ADDR(AppController, actionOpenDiskInNewWindow), [](){});
    stext.set_lamda(ADDR(AppController, actionOpenWithCustom), [](){});
    stext.set_lamda(ADDR(AppController, actionOpenFileLocation), [](){});

    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::Open);
    actions.append(DFMGlobal::MenuAction::OpenDisk);
    actions.append(DFMGlobal::MenuAction::OpenInNewWindow);
    actions.append(DFMGlobal::MenuAction::OpenInNewTab);
    actions.append(DFMGlobal::MenuAction::OpenDiskInNewWindow);
    actions.append(DFMGlobal::MenuAction::OpenDiskInNewTab);
    actions.append(DFMGlobal::MenuAction::OpenAsAdmin);
    actions.append(DFMGlobal::MenuAction::OpenWithCustom);
    actions.append(DFMGlobal::MenuAction::OpenFileLocation);

    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, tst_menu_compress_functions)
{
    stub_ext::StubExt stext;
    stext.set_lamda(VADDR(QDialog, exec), [](){ return (int)QDialog::Rejected; });

    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::Compress);
    actions.append(DFMGlobal::MenuAction::Decompress);
    actions.append(DFMGlobal::MenuAction::DecompressHere);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, tst_menu_paste_functions)
{
    Stub stub;
    QVariant (*ut_processEvent)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = []
            (const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant();
    };
    stub.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))\
            ADDR(DFMEventDispatcher, processEvent), ut_processEvent);

    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::Cut);
    actions.append(DFMGlobal::MenuAction::Copy);
    actions.append(DFMGlobal::MenuAction::Paste);
    actions.append(DFMGlobal::MenuAction::Rename);
    actions.append(DFMGlobal::MenuAction::Delete);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);

        qApp->processEvents();
    }
}

TEST_F(FileEventProcessorTest, tst_menu_bookmark_functions)
{
    stub_ext::StubExt stext;
    stext.set_lamda(ADDR(AppController, actionBookmarkRemove), [](){});
    stext.set_lamda(ADDR(AppController, actionBookmarkRename), [](){});
    stext.set_lamda(ADDR(AppController, actionAddToBookMark), [](){});

    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::BookmarkRemove);
    actions.append(DFMGlobal::MenuAction::BookmarkRename);
    actions.append(DFMGlobal::MenuAction::AddToBookMark);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, tst_menu_create_symlink)
{
    stub_ext::StubExt stext;
    stext.set_lamda(ADDR(AppController, actionCreateSymlink), [](){});

    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::CreateSymlink);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, tst_menu_send_to_desktop)
{
    stub_ext::StubExt stext;
    stext.set_lamda(ADDR(AppController, actionSendToDesktop), [](){});

    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::SendToDesktop);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, tst_menu_property)
{
    stub_ext::StubExt stext;
    stext.set_lamda(ADDR(AppController, actionProperty), [](){});

    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::Property);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, tst_menu_new_folder)
{
    stub_ext::StubExt stext;
    stext.set_lamda(ADDR(AppController, actionNewFolder), [](){});

    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::NewFolder);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, tst_menu_new_window)
{
    stub_ext::StubExt stext;
    stext.set_lamda(ADDR(AppController, actionOpenInNewWindow), [](){});

    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::NewWindow);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, tst_menu_select_all)
{
    stub_ext::StubExt stext;
    stext.set_lamda(ADDR(AppController, actionSelectAll), [](){});

    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::SelectAll);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, tst_menu_clear_recent_trash)
{
    Stub stub;
    void (*ut_actionClearRecent)() = [](){};
    void (*ut_actionClearTrash)() = [](){};
    typedef void (AppController::*ActionClearRecent)(const QSharedPointer<DFMMenuActionEvent> &event);
    stub.set((ActionClearRecent)&AppController::actionClearRecent, ut_actionClearRecent);
    stub.set(ADDR(AppController, actionClearTrash), ut_actionClearTrash);

    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::ClearRecent);
    actions.append(DFMGlobal::MenuAction::ClearTrash);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, tst_menu_new_office)
{
    stub_ext::StubExt stext;
    stext.set_lamda(ADDR(AppController, actionNewWord), [](){});
    stext.set_lamda(ADDR(AppController, actionNewExcel), [](){});
    stext.set_lamda(ADDR(AppController, actionNewPowerpoint), [](){});
    stext.set_lamda(ADDR(AppController, actionNewText), [](){});

    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::NewWord);
    actions.append(DFMGlobal::MenuAction::NewExcel);
    actions.append(DFMGlobal::MenuAction::NewPowerpoint);
    actions.append(DFMGlobal::MenuAction::NewText);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, tst_menu_open_in_terminal)
{
    stub_ext::StubExt stext;
    stext.set_lamda(ADDR(AppController, actionOpenInTerminal), [](){});

    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::OpenInTerminal);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, tst_menu_restore)
{
    stub_ext::StubExt stext;
    stext.set_lamda(ADDR(AppController, actionRestore), [](){});
    stext.set_lamda(ADDR(AppController, actionRestoreAll), [](){});

    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::Restore);
    actions.append(DFMGlobal::MenuAction::RestoreAll);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, tst_menu_complete_deletion)
{
    stub_ext::StubExt stext;
    stext.set_lamda(ADDR(AppController, actionCompleteDeletion), [](){});
    stext.set_lamda(ADDR(AppController, actionDelete), [](){});

    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::CompleteDeletion);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, tst_menu_disk)
{
    stub_ext::StubExt stext;
    stext.set_lamda(ADDR(AppController, actionMount), [](){});
    stext.set_lamda(ADDR(AppController, actionMountImage), [](){});
    stext.set_lamda(ADDR(AppController, actionUnmount), [](){});
    stext.set_lamda(ADDR(AppController, actionEject), [](){});

    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::Mount);
    actions.append(DFMGlobal::MenuAction::MountImage);
    actions.append(DFMGlobal::MenuAction::Unmount);
    actions.append(DFMGlobal::MenuAction::Eject);
    actions.append(DFMGlobal::MenuAction::SafelyRemoveDrive);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, tst_menu_setting)
{
    stub_ext::StubExt stext;
    stext.set_lamda(ADDR(AppController, actionSettings), [](){});

    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::Settings);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

static void actionExitStub(quint64 winId)
{
    Q_UNUSED(winId);
};

TEST_F(FileEventProcessorTest, tst_menu_exit)
{
    stub_ext::StubExt stext;
    stext.set_lamda(ADDR(AppController, actionExit), [](){});

    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::Exit);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, tst_menu_set_as_wallpaper)
{
    stub_ext::StubExt stext;
    stext.set_lamda(ADDR(AppController, actionSetAsWallpaper), [](){});

    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::SetAsWallpaper);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, tst_menu_forget_password)
{
    stub_ext::StubExt stext;
    stext.set_lamda(ADDR(AppController, actionForgetPassword), [](){});

    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::ForgetPassword);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, tst_menu_share)
{
    stub_ext::StubExt stubex;
    stubex.set_lamda(ADDR(AppController, actionShare), [](){});
    stubex.set_lamda(ADDR(AppController, actionUnShare), [](){});
    stubex.set_lamda(ADDR(AppController, actionConnectToServer), [](){});
    stubex.set_lamda(ADDR(AppController, actionSetUserSharePassword), [](){});

    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::Share);
    actions.append(DFMGlobal::MenuAction::UnShare);
    actions.append(DFMGlobal::MenuAction::ConnectToServer);
    actions.append(DFMGlobal::MenuAction::SetUserSharePassword);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, tst_menu_format_device)
{
    stub_ext::StubExt stubex;
    stubex.set_lamda(ADDR(AppController, actionFormatDevice), [](){});

    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::FormatDevice);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, tst_menu_optical_blank)
{
    stub_ext::StubExt stubex;
    stubex.set_lamda(ADDR(AppController, actionOpticalBlank), [](){});

    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::OpticalBlank);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, tst_menu_remove_from_recent)
{
    stub_ext::StubExt stubex;
    stubex.set_lamda(ADDR(DFileService, deleteFiles), [](){ return true; });

    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::RemoveFromRecent);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, tst_open_window_event)
{
    stub_ext::StubExt stext;
    stext.set_lamda(ADDR(WindowManager, showNewWindow), [](){});

    DUrl url = DUrl::fromLocalFile(getTestPath());
    DUrlList list;
    list.append(url);
    bool force = false;
    auto event = dMakeEventPointer<DFMOpenNewWindowEvent>(nullptr, list, force);
    QVariant data;
    bool ret = processer->fmEvent(event, &data);
    EXPECT_TRUE(ret);
}

TEST_F(FileEventProcessorTest, tst_change_current_url)
{
    stub_ext::StubExt stext;
    stext.set_lamda(ADDR(DFileManagerWindow, cd), [](){ return true; });

    // 目录处理
    DUrl url = DUrl::fromLocalFile(getTestPath());
    auto event = dMakeEventPointer<DFMChangeCurrentUrlEvent>(nullptr, url, nullptr);
    QVariant data;
    bool ret = processer->fmEvent(event, &data);
    EXPECT_TRUE(ret);

    // 文件处理
    url = DUrl::fromLocalFile(getTestPath() + "test_a");
    ret = processer->fmEvent(event, &data);
    EXPECT_TRUE(ret);
}

TEST_F(FileEventProcessorTest, tst_change_open_url)
{
    stub_ext::StubExt stext;
    stext.set_lamda(ADDR(WindowManager, showNewWindow), [](){});

    DUrl url = DUrl::fromLocalFile(getTestPath());
    DUrlList list;
    list.append(url);
    DFMOpenUrlEvent::DirOpenMode mode = DFMOpenUrlEvent::DirOpenMode::OpenInCurrentWindow;
    bool isEnter = false;
    auto event = dMakeEventPointer<DFMOpenUrlEvent>(nullptr, list, mode, isEnter);
    QVariant data;
    bool ret = processer->fmEvent(event, &data);
    EXPECT_TRUE(ret);

    DUrl url2 = DUrl::fromLocalFile(getTestPath() + "test_a");
    list.append(url2);
    mode = DFMOpenUrlEvent::DirOpenMode::OpenNewWindow;
    ret = processer->fmEvent(event, &data);
    EXPECT_TRUE(ret);
}
