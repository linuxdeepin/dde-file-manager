#include <gtest/gtest.h>

#include "controllers/trashmanager.h"
#include "dfmstandardpaths.h"
#include "dfmevent.h"

#include <QProcess>

namespace  {
class TestTrashManager: public testing::Test
{
public:
    virtual void SetUp() override
    {
        std::cout << "start TestTrashManager" << std::endl;
        m_trash = new TrashManager();
        trashFileUrl = DUrl::fromTrashFile("/test.txt");
        QProcess::execute("touch " + trashFileUrl.toLocalFile());
    }

    virtual void TearDown() override
    {
        std::cout << "end TestTrashManager" << std::endl;
        QProcess::execute("rm -f " + trashFileUrl.toLocalFile());
        if (m_trash) {
            delete m_trash;
            m_trash = nullptr;
        }
    }

    void DumpDirector(DDirIteratorPointer director)
    {
        auto fileName = director->fileName();
        auto fileUrl = director->fileUrl();
        auto fileInfo = director->fileInfo();
        auto durl = director->url();

        while (director->hasNext()) {
            director->next();
        }
    }

public:
    TrashManager *m_trash;
    DUrl trashFileUrl;
};
}

TEST_F(TestTrashManager, createFileInfo)
{
    auto pfile = m_trash->createFileInfo(dMakeEventPointer<DFMCreateFileInfoEvent>(nullptr, trashFileUrl));
    EXPECT_TRUE(pfile != nullptr);
}

TEST_F(TestTrashManager, openFile)
{
    QProcess::execute("mkdir " + DUrl::fromTrashFile("/test").toLocalFile());
    auto res = m_trash->openFile(dMakeEventPointer<DFMOpenFileEvent>(nullptr, DUrl::fromTrashFile("/test")));
    QProcess::execute("rm -r " + DUrl::fromTrashFile("/test").toLocalFile());
    EXPECT_TRUE(res);
}

TEST_F(TestTrashManager, openFiles)
{
//    QProcess::execute("mkdir " + DUrl::fromTrashFile("/test").toLocalFile());
//    auto res = m_trash->openFiles(dMakeEventPointer<DFMOpenFilesEvent>(nullptr, DUrlList() << DUrl::fromTrashFile("/test")));
//    QProcess::execute("rm -r " + DUrl::fromTrashFile("/test").toLocalFile());
//    EXPECT_TRUE(res);
}

TEST_F(TestTrashManager, moveToTrash)
{
    auto res = m_trash->moveToTrash(dMakeEventPointer<DFMMoveToTrashEvent>(nullptr, DUrlList()));
    EXPECT_TRUE(res.isEmpty());
}

TEST_F(TestTrashManager, writeFilesToClipboard)
{
    auto res = m_trash->writeFilesToClipboard(dMakeEventPointer<DFMWriteUrlsToClipboardEvent>(nullptr, DFMGlobal::CutAction, DUrlList() << trashFileUrl));
    EXPECT_TRUE(res);
}

TEST_F(TestTrashManager, pasteFile)
{
    QProcess::execute("touch /tmp/1.txt");
    auto res = m_trash->pasteFile(dMakeEventPointer<DFMPasteEvent>(nullptr, DFMGlobal::CutAction, DUrl::fromTrashFile("/"), DUrlList() << DUrl("file:///tmp/1.txt")));
    EXPECT_TRUE(!res.isEmpty());
}

TEST_F(TestTrashManager, restoreFile)
{
    auto res = m_trash->restoreFile(dMakeEventPointer<DFMRestoreFromTrashEvent>(nullptr, DUrlList() << DUrl::fromTrashFile("/1.txt")));
    QProcess::execute("rm -f 1.txt");
    EXPECT_TRUE(res);
}

TEST_F(TestTrashManager, deleteFiles)
{
    auto res = m_trash->deleteFiles(dMakeEventPointer<DFMDeleteEvent>(nullptr, DUrlList() << trashFileUrl, true, true));
    EXPECT_TRUE(res);
}

TEST_F(TestTrashManager, createDirIterator)
{
    QDirIterator::IteratorFlags flags = static_cast<QDirIterator::IteratorFlags>(DDirIterator::SortINode);
    auto res = m_trash->createDirIterator(dMakeEventPointer<DFMCreateDiriterator>(nullptr, DUrl::fromTrashFile("/"), QStringList(), QDir::AllEntries, flags, false, false));
    EXPECT_TRUE(res != nullptr);
    DumpDirector(res);
}

TEST_F(TestTrashManager, createFileWatcher)
{
    auto res = m_trash->createFileWatcher(dMakeEventPointer<DFMCreateFileWatcherEvent>(nullptr, DUrl::fromTrashFile("/")));
    EXPECT_TRUE(res != nullptr);
}

TEST_F(TestTrashManager, restoreTrashFile)
{
    QProcess::execute("touch /tmp/1.txt");
    m_trash->pasteFile(dMakeEventPointer<DFMPasteEvent>(nullptr, DFMGlobal::CutAction, DUrl::fromTrashFile("/"), DUrlList() << DUrl("file:///tmp/1.txt")));

    auto res = m_trash->restoreTrashFile(DUrlList() << DUrl::fromTrashFile("/1.txt"));
    QProcess::execute("rm -f /tmp/1.txt");
    EXPECT_TRUE(res);
}

TEST_F(TestTrashManager, isEmpty)
{
    auto res = m_trash->isEmpty();
    EXPECT_FALSE(res);
}

TEST_F(TestTrashManager, isWorking)
{
    auto res = m_trash->isWorking();
    EXPECT_FALSE(res);
}
