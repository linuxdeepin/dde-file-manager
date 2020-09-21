#include <gtest/gtest.h>
#include <QDateTime>
#include <dfmevent.h>

#include "controllers/networkcontroller.h"
#include <QProcess>
#include <QDebug>

using namespace testing;

class NetworkControllerTest:public testing::Test{

public:
    NetworkController controller;
    virtual void SetUp() override{
        std::cout << "start DFileCopyQueueTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end DFileCopyQueueTest" << std::endl;
    }
};

TEST_F(NetworkControllerTest, can_createFileInfo)
{
    auto event = dMakeEventPointer<DFMCreateFileInfoEvent>(nullptr, DUrl("smb:///"));
    EXPECT_TRUE(controller.createFileInfo(event) != nullptr);
}

TEST_F(NetworkControllerTest, can_createDirIterator)
{
    auto event = dMakeEventPointer<DFMCreateDiriterator>(nullptr, DUrl("smb:///"),QStringList(),QDir::AllEntries,QDirIterator::NoIteratorFlags);
    auto reslut = controller.createDirIterator(event);
    EXPECT_TRUE(reslut != nullptr);
}

TEST_F(NetworkControllerTest, can_getChildren)
{
    auto event = dMakeEventPointer<DFMGetChildrensEvent>(nullptr, DUrl("smb:///"),QStringList(), nullptr);
    EXPECT_TRUE(!controller.getChildren(event).isEmpty());
}

TEST_F(NetworkControllerTest, can_pasteFile)
{
    auto event = dMakeEventPointer<DFMPasteEvent>(nullptr, DFMGlobal::CutAction, DUrl(), DUrlList());
    EXPECT_TRUE(!controller.pasteFile(event).isEmpty());
}
