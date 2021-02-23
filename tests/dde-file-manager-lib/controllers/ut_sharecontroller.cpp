#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QProcess>
#include <QStandardPaths>
#include <QDateTime>
#include <QUrl>
#include <QFile>
#include "dfmevent.h"

#include "controllers/sharecontroler.h"
#include "dabstractfilewatcher.h"
#include "dfmglobal.h"

#include "testhelper.h"

namespace  {
class TestShareController : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestShareController" << std::endl;
        m_controller = QSharedPointer<ShareControler>::create();
        QString fileName = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
        QString tempFilePath = TestHelper::createTmpFile(".txt");
        QString tmpDirPath = TestHelper::createTmpDir();
        tmpFileUrl = DUrl::fromUserShareFile(tempFilePath);
        tmpDirUrl = DUrl::fromUserShareFile(tmpDirPath);
    }

    void TearDown() override
    {
        std::cout << "end TestShareController" << std::endl;
        QStringList tmpPaths;
        tmpPaths << tmpFileUrl.path() << tmpDirUrl.path();
        TestHelper::deleteTmpFiles(tmpPaths);
    }

public:
    QSharedPointer<ShareControler> m_controller;
    DUrl tmpFileUrl;
    DUrl tmpDirUrl;
};

TEST_F(TestShareController, test_test_createFileInfo)
{
    TestHelper::runInLoop([](){});
    auto event = dMakeEventPointer<DFMCreateFileInfoEvent>(nullptr, tmpFileUrl);
    EXPECT_TRUE(m_controller->createFileInfo(event) != nullptr);
}

TEST_F(TestShareController, test_createFileWatcher)
{
    DFMGlobal::initUserShareManager();
    QSharedPointer<DFMCreateFileWatcherEvent> event = dMakeEventPointer<DFMCreateFileWatcherEvent>(nullptr, DUrl::fromUserShareFile("/"));
    auto ret = m_controller->createFileWatcher(event);
    EXPECT_TRUE(ret->startWatcher());
    EXPECT_TRUE(ret->stopWatcher());
    EXPECT_NE(nullptr, ret);
}

TEST_F(TestShareController, test_openFile)
{
    auto event = dMakeEventPointer<DFMOpenFileEvent>(nullptr, tmpFileUrl);
    EXPECT_TRUE(m_controller->openFile(event));
    QProcess::execute("killall deepin-editor");
}

TEST_F(TestShareController, test_setPermissions)
{
    auto event = dMakeEventPointer<DFMSetPermissionEvent>(nullptr, tmpFileUrl, (QFileDevice::ReadOther | QFileDevice::ReadGroup | QFileDevice::ReadOwner));
    EXPECT_TRUE(m_controller->setPermissions(event));
}

TEST_F(TestShareController, test_shareFolder)
{
    // 阻塞CI
    // auto event = dMakeEventPointer<DFMFileShareEvent>(nullptr, tmpDirUrl, "testShare");
    // EXPECT_TRUE(m_controller->shareFolder(event));
}

TEST_F(TestShareController, test_unShareFolder)
{
    // 阻塞CI
    // auto event = dMakeEventPointer<DFMCancelFileShareEvent>(nullptr, tmpDirUrl);
    // EXPECT_TRUE(m_controller->unShareFolder(event));
}

TEST_F(TestShareController, test_getChildren)
{
    // 阻塞CI
    // auto shareEvent = dMakeEventPointer<DFMFileShareEvent>(nullptr, tmpDirUrl, "testShare");
    // EXPECT_TRUE(m_controller->shareFolder(shareEvent));
    // auto event = dMakeEventPointer<DFMGetChildrensEvent>(nullptr, DUrl::fromUserShareFile("/"),QStringList(), QDir::NoFilter, QDirIterator::NoIteratorFlags);
    // auto ret = m_controller->getChildren(event);
    // EXPECT_FALSE(ret.isEmpty());
    // auto unshareEvent = dMakeEventPointer<DFMCancelFileShareEvent>(nullptr, tmpDirUrl);
    // EXPECT_TRUE(m_controller->unShareFolder(unshareEvent));
    // ret = m_controller->getChildren(event);
    // EXPECT_TRUE(ret.isEmpty());
}

TEST_F(TestShareController, test_addToBookmark)
{
    auto event = dMakeEventPointer<DFMAddToBookmarkEvent>(nullptr, tmpDirUrl);
    EXPECT_TRUE(!m_controller->addToBookmark(event));
}

TEST_F(TestShareController, test_removeBookmark)
{
    auto event = dMakeEventPointer<DFMRemoveBookmarkEvent>(nullptr, tmpDirUrl);
    EXPECT_TRUE(!m_controller->removeBookmark(event));
}
}
