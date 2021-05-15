#include <gtest/gtest.h>
#include <QMutex>
#include <QWaitCondition>
#include "models/dfmrootfileinfo.h"
#include "interfaces/dabstractfilewatcher.h"

#include "testhelper.h"

#define private public
#include "drootfilemanager.h"

DFM_USE_NAMESPACE

#define rootfileManager DRootFileManager::instance()
namespace {

class TestDRootFileManager : public DRootFileManager, public testing::Test
{

public:
    void SetUp() override
    {
        std::cout << "start TestDFMSettings";
    }

    void TearDown() override
    {
        std::cout << "end TestDFMSettings";
    }

    DUrl getDfmRootPath(const QString &name)
    {
        return DUrl(DFMROOT_ROOT + name + "." SUFFIX_USRDIR);
    }

    DAbstractFileInfoPointer getOneFilePointer(const QString &name)
    {
        return QExplicitlySharedDataPointer<DAbstractFileInfo> (new DFMRootFileInfo(getDfmRootPath(name)));
    }
};
}

TEST_F(TestDRootFileManager, while_first_start_without_thread_fetch_process_no_items)
{
    if (rootfileManager->isRootFileInited()) {
        EXPECT_TRUE(!rootfileManager->getRootFile().isEmpty());
    }
    // 非isRootFileInited情况下 会有其他函数调用到插入RootFile, 所以这里不做else检查
    /*else {
        EXPECT_TRUE(rootfileManager->getRootFile().isEmpty());
    }*/
}
#if 0
TEST_F(TestDRootFileManager, start_trigger_and_waiting_for_results)
{
    TestHelper::runInLoop([ = ] {
        startQuryRootFile();
    }, 2000);

    TestHelper::runInLoop([ = ] {
        Q_EMIT rootFileWather()->fileDeleted(getDfmRootPath("Vvideos"));
        Q_EMIT rootFileWather()->subfileCreated(getDfmRootPath("Vvideos"));
    }, 1000);

    EXPECT_TRUE(getRootFile().size() >= 6);
    EXPECT_TRUE(isRootFileInited());
}

TEST_F(TestDRootFileManager, get_results_find_nothide_dfmrootitem)
{
    TestHelper::runInLoop([ = ] {
        startQuryRootFile();
    }, 2000);

    hideSystemPartition();
    EXPECT_TRUE(getRootFile().size() >= 6);
    EXPECT_TRUE(isRootFileContain(getDfmRootPath("videos")));
    EXPECT_TRUE(getFileInfo(getDfmRootPath("videos")));
}
#endif
// temp fix, max.lv
//TEST_F(TestDRootFileManager, start_trigger_will_not_do_it_as_twice)
//{
//    TestHelper::runInLoop([=]{
//        startQuryRootFile();
//    },1000);

//    DAbstractFileWatcher* olderWatcher = rootFileWather();
//    EXPECT_TRUE(isRootFileInited());

//    TestHelper::runInLoop([=]{
//        startQuryRootFile();
//    });

//    EXPECT_EQ(rootFileWather(), olderWatcher);
//}
