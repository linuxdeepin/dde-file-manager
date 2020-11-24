#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "dfmevent.h"
#include "dfilemenu.h"

#define private public
#include "controllers/fileeventprocessor.h"
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

TEST_F(FileEventProcessorTest, open_window_event)
{
    DUrl url = DUrl::fromLocalFile(getTestPath());
    DUrlList list;
    list.append(url);
    bool force = false;
    auto event = dMakeEventPointer<DFMOpenNewWindowEvent>(nullptr, list, force);
    QVariant data;
    bool ret = processer->fmEvent(event, &data);
    EXPECT_EQ(ret, true);
}

TEST_F(FileEventProcessorTest, change_current_url)
{
    // 目录处理
    DUrl url = DUrl::fromLocalFile(getTestPath());
    auto event = dMakeEventPointer<DFMChangeCurrentUrlEvent>(nullptr, url, nullptr);
    QVariant data;
    bool ret = processer->fmEvent(event, &data);
    EXPECT_EQ(ret, true);

    // 文件处理
    url = DUrl::fromLocalFile(getTestPath() + "test_a");
    ret = processer->fmEvent(event, &data);
    EXPECT_EQ(ret, true);
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
    EXPECT_EQ(ret, true);

    DUrl url2 = DUrl::fromLocalFile(getTestPath() + "test_a");
    list.append(url2);
    mode = DFMOpenUrlEvent::DirOpenMode::OpenNewWindow;
    ret = processer->fmEvent(event, &data);
    EXPECT_EQ(ret, true);
}

TEST_F(FileEventProcessorTest, menu_event_rename_tag)
{
    auto menuEvent = makeTestMenuAction(DFMGlobal::MenuAction::RenameTag);
    QVariant data;
    bool ret = processer->fmEvent(menuEvent, &data);
    EXPECT_EQ(ret, true);
}

TEST_F(FileEventProcessorTest, menu_event_change_tag_color)
{
    auto menuEvent = makeTestMenuAction(DFMGlobal::MenuAction::ChangeTagColor);
    QVariant data;
    bool ret = processer->fmEvent(menuEvent, &data);
    EXPECT_EQ(ret, true);
}

TEST_F(FileEventProcessorTest, menu_event_tag_files_use_color)
{
//    auto menuEvent = makeTestMenuAction(DFMGlobal::MenuAction::TagFilesUseColor);
//    QVariant data;
//    bool ret = processer->fmEvent(menuEvent, &data);
//    EXPECT_EQ(ret, true);
}

TEST_F(FileEventProcessorTest, menu_delete_tag)
{
//    auto menuEvent = makeTestMenuAction(DFMGlobal::MenuAction::DeleteTags);
//    QVariant data;
//    bool ret = processer->fmEvent(menuEvent, &data);
//    EXPECT_EQ(ret, true);
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
        QVariant data;
        bool ret = processer->fmEvent(menuEvent, &data);
        EXPECT_EQ(ret, true);
    }
}

TEST_F(FileEventProcessorTest, menu_compress_functions)
{
    QList<DFMGlobal::MenuAction> actions;
//    actions.append(DFMGlobal::MenuAction::Compress);
//    actions.append(DFMGlobal::MenuAction::Decompress);
//    actions.append(DFMGlobal::MenuAction::DecompressHere);
    foreach (DFMGlobal::MenuAction action, actions) {
        auto menuEvent = makeTestMenuAction(action);
        QVariant data;
        bool ret = processer->fmEvent(menuEvent, &data);
        EXPECT_EQ(ret, true);
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
        QVariant data;
        bool ret = processer->fmEvent(menuEvent, &data);
        EXPECT_EQ(ret, true);

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
        QVariant data;
        bool ret = processer->fmEvent(menuEvent, &data);
        EXPECT_EQ(ret, true);
    }
}
