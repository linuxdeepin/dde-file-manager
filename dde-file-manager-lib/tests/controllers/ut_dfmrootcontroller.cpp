#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#define private public
#include "controllers/dfmrootcontroller.h"
#include "dfmevent.h"

namespace  {
    class DFMRootControllerTest : public testing::Test
    {
    public:
        virtual void SetUp() override
        {
            controller = new DFMRootController();
        }

        virtual void TearDown() override
        {
            delete controller;
            controller = nullptr;
        }

        DFMRootController *controller;
    };
}

TEST_F(DFMRootControllerTest, rename_file)
{
    DUrl from;
    from.setScheme(DFMROOT_SCHEME);
    from.setPath("/tmp/file_A");

    DUrl to;
    from.setPath("/tmp/file_B");
    to.setScheme(DFMROOT_SCHEME);

    system("touch /tmp/file_A");
    bool success = controller->renameFile(dMakeEventPointer<DFMRenameEvent>(nullptr, from, to));
    // 创建的文件都尝试删除, 不依赖重命名的结果
    system("rm /tmp/file_B");
    system("rm /tmp/file_A");
    EXPECT_TRUE(success);
}

TEST_F(DFMRootControllerTest, get_children)
{
    DUrl url;
    url.setScheme(DFMROOT_SCHEME);
    url.setPath("/");

    QStringList nameFilters;
    QDir::Filters filters;
    auto event = dMakeEventPointer<DFMGetChildrensEvent>(nullptr,
                                                         url,
                                                         nameFilters,
                                                         filters);
    QList<DAbstractFileInfoPointer> list = controller->getChildren(event);
    EXPECT_TRUE(!list.empty());
}

TEST_F(DFMRootControllerTest, create_file_info)
{
    DUrl url;
    url.setScheme(DFMROOT_SCHEME);
    url.setPath("/");
    DAbstractFileInfoPointer fi = controller->createFileInfo(dMakeEventPointer<DFMCreateFileInfoEvent>(nullptr, url));
    EXPECT_TRUE(fi != nullptr);
}

TEST_F(DFMRootControllerTest, create_file_watcher)
{
    DUrl url;
    url.setScheme(DFMROOT_SCHEME);
    url.setPath("/");
    DAbstractFileWatcher *watcher = controller->createFileWatcher(dMakeEventPointer<DFMCreateFileWatcherEvent>(nullptr, url));
    EXPECT_TRUE(watcher != nullptr);

    watcher->startWatcher();
    system("touch /tmp/newfile");
    system("echo 'test' >> /tmp/newfile");
    system("mv /tmp/newfile /tmp/newfile2");
    system("rm /tmp/newfile2");
    for (int i=0; i<4; i++) {
        qApp->processEvents();
        sleep(1);
    }
    watcher->stopWatcher();
}

TEST_F(DFMRootControllerTest, load_disk_info)
{
    QString json;
    controller->loadDiskInfo(json);
}
