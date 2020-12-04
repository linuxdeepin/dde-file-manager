#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "dfmevent.h"
#include "dfilemenu.h"

#define private public
#include "controllers/fileeventprocessor.cpp"
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

        FileEventProcessor *processer;
    };
}

TEST_F(FileEventProcessorTest, is_avfs_mounted)
{
    bool mounted = isAvfsMounted();
    EXPECT_FALSE(mounted);
}

TEST_F(FileEventProcessorTest, menu_event_rename_tag)
{
    auto event = makeTestMenuAction(DFMGlobal::MenuAction::RenameTag);
    bool result = processMenuEvent(event);
    EXPECT_TRUE(result);
}

TEST_F(FileEventProcessorTest, menu_event_change_tag_color)
{
    DFMGlobal::MenuAction action = DFMGlobal::MenuAction::ChangeTagColor;
    const QObject *sender = nullptr;

    DTagActionWidget* tagWidget = new DTagActionWidget;
    QWidgetAction* tagAction = new QWidgetAction(nullptr);
    tagAction->setDefaultWidget(tagWidget);
    tagAction->setText("Change color of present tag");

    DFileMenu *menu = new DFileMenu(nullptr);
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

TEST_F(FileEventProcessorTest, menu_event_tag_files_use_color)
{
    DFMGlobal::MenuAction action = DFMGlobal::MenuAction::TagFilesUseColor;
    const QObject *sender = nullptr;

    DTagActionWidget* tagWidget = new DTagActionWidget;
    QWidgetAction* tagAction = new QWidgetAction(nullptr);
    tagAction->setDefaultWidget(tagWidget);
    tagAction->setText("Add color tags");

    DFileMenu *menu = new DFileMenu(nullptr);
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

TEST_F(FileEventProcessorTest, menu_delete_tag)
{
    auto menuEvent = makeTestMenuAction(DFMGlobal::MenuAction::DeleteTags);
    QVariant data;
    bool ret = processMenuEvent(menuEvent);
    EXPECT_FALSE(ret);
}

TEST_F(FileEventProcessorTest, menu_open_functions)
{
    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::Open);
    actions.append(DFMGlobal::MenuAction::OpenDisk);
    actions.append(DFMGlobal::MenuAction::OpenInNewWindow);
    actions.append(DFMGlobal::MenuAction::OpenInNewTab);
    actions.append(DFMGlobal::MenuAction::OpenDiskInNewWindow);
    actions.append(DFMGlobal::MenuAction::OpenDiskInNewTab);
    // actions.append(DFMGlobal::MenuAction::OpenAsAdmin);
    actions.append(DFMGlobal::MenuAction::OpenWithCustom);
    actions.append(DFMGlobal::MenuAction::OpenFileLocation);

    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, menu_compress_functions)
{
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

TEST_F(FileEventProcessorTest, menu_paste_functions)
{
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

TEST_F(FileEventProcessorTest, menu_bookmark_functions)
{
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

TEST_F(FileEventProcessorTest, menu_create_symlink)
{
//    QList<DFMGlobal::MenuAction> actions;
//    actions.append(DFMGlobal::MenuAction::CreateSymlink);
//    foreach (DFMGlobal::MenuAction action, actions) {
//        auto menuEvent = makeTestMenuAction(action);
//        bool ret = processMenuEvent(menuEvent);
//        EXPECT_TRUE(ret);
//    }
}

TEST_F(FileEventProcessorTest, menu_send_to_desktop)
{
    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::SendToDesktop);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, menu_property)
{
    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::Property);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, menu_new_folder)
{
    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::NewFolder);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, menu_new_window)
{
    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::NewWindow);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, menu_select_all)
{
    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::SelectAll);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, menu_clear_recent_trash)
{
    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::ClearRecent);
    actions.append(DFMGlobal::MenuAction::ClearTrash);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, menu_new_office)
{
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

TEST_F(FileEventProcessorTest, menu_open_in_terminal)
{
    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::OpenInTerminal);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, menu_restore)
{
    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::Restore);
    actions.append(DFMGlobal::MenuAction::RestoreAll);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, menu_complete_deletion)
{
    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::CompleteDeletion);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, menu_disk)
{
    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::Mount);
    actions.append(DFMGlobal::MenuAction::Unmount);
    actions.append(DFMGlobal::MenuAction::Eject);
    actions.append(DFMGlobal::MenuAction::SafelyRemoveDrive);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, menu_setting)
{
    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::Settings);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, menu_exit)
{
    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::Exit);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, menu_set_as_wallpaper)
{
    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::SetAsWallpaper);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, menu_forget_password)
{
    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::ForgetPassword);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, menu_share)
{
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

TEST_F(FileEventProcessorTest, menu_format_device)
{
    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::FormatDevice);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, menu_optical_blank)
{
    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::OpticalBlank);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, menu_remove_from_recent)
{
    QList<DFMGlobal::MenuAction> actions;
    actions.append(DFMGlobal::MenuAction::RemoveFromRecent);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        bool ret = processMenuEvent(menuEvent);
        EXPECT_TRUE(ret);
    }
}

TEST_F(FileEventProcessorTest, open_window_event)
{
    DUrl url = DUrl::fromLocalFile(getTestPath());
    DUrlList list;
    list.append(url);
    bool force = false;
    auto event = dMakeEventPointer<DFMOpenNewWindowEvent>(nullptr, list, force);
    QVariant data;
    bool ret = processer->fmEvent(event, &data);
    EXPECT_TRUE(ret);
}

TEST_F(FileEventProcessorTest, change_current_url)
{
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

TEST_F(FileEventProcessorTest, change_open_url)
{
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
