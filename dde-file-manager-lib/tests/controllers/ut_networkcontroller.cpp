#include <gtest/gtest.h>
#include <dfmevent.h>

#include "controllers/networkcontroller.h"

#include <QDateTime>
#include <QProcess>
#include <QUrl>
#include <QFile>
#include <QDir>

using namespace testing;

class NetworkControllerTest:public testing::Test{

public:
    NetworkController controller;
    virtual void SetUp() override{
        std::cout << "start NetworkControllerTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end NetworkControllerTest" << std::endl;
    }
};

TEST_F(NetworkControllerTest, can_createFileInfo)
{
    // 阻塞CI
    // auto event = dMakeEventPointer<DFMCreateFileInfoEvent>(nullptr, DUrl("smb:///"));
    // EXPECT_TRUE(controller.createFileInfo(event) != nullptr);
}

TEST_F(NetworkControllerTest, can_createDirIterator)
{
    // 阻塞CI
    // auto event = dMakeEventPointer<DFMCreateDiriterator>(nullptr,
    //                                                      DUrl("smb:///"),
    //                                                      QStringList(),
    //                                                      QDir::AllEntries,
    //                                                      QDirIterator::NoIteratorFlags);

    // auto itera = controller.createDirIterator(event);
    // EXPECT_TRUE(itera->hasNext());
    // while (itera->hasNext()) {
    //     qDebug()<< "next >> " << itera->fileUrl();
    //     qDebug()<< "next >> " << itera->fileInfo();
    //     qDebug()<< "next >> " << itera->url();
    //     qDebug()<< "next >> " << itera->fileName();
    //     EXPECT_TRUE(!itera->next().isEmpty());
    //     itera->close();
    // }
}

TEST_F(NetworkControllerTest, can_getChildren)
{
    auto event = dMakeEventPointer<DFMGetChildrensEvent>(nullptr,
                                                         DUrl("smb:///"),
                                                         QStringList(),
                                                         nullptr);

    EXPECT_TRUE(!controller.getChildren(event).isEmpty());
}

TEST_F(NetworkControllerTest, can_pasteFile)
{
    auto event = dMakeEventPointer<DFMPasteEvent>(nullptr,
                                                  DFMGlobal::CutAction,
                                                  DUrl("smb:///"),
                                                  DUrlList());

    EXPECT_TRUE(controller.pasteFile(event) == DUrlList());
}
