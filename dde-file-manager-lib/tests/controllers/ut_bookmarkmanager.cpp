#include <controllers/bookmarkmanager.h>
#include <dfmevent.h>

#include <QProcess>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QUrlQuery>

#include <gtest/gtest.h>

#define BOOKMARK_STR "TestBookMark"
#define BOOKMARK_NEW_STR "TestNewBookMark"
namespace  {
    class TestBookMarkManager : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new BookMarkManager();
            std::cout << "start TestBookMarkManager";
            tempDirPath =  QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first() + "/" + BOOKMARK_STR;
            QProcess::execute("mkdir " + tempDirPath);
        }
        void TearDown() override
        {
            QProcess::execute("rm -r " + tempDirPath);

            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestBookMarkManager";
        }
    public:
        BookMarkManager *m_pTester;
        QString tempDirPath;
    };
}

TEST_F(TestBookMarkManager, can_touch_a_new_bookmark)
{
    DUrl fileUrl = DUrl::fromLocalFile(tempDirPath);
    QStorageInfo si(tempDirPath);
    QList<QPair<QString, QString>> queryItems;
    queryItems.append(QPair<QString, QString>("mount_point", "device:" + QVariant(si.device()).toString()));
    queryItems.append(QPair<QString, QString>("locate_url", tempDirPath.replace("/home/", "/")));
    QUrlQuery query;
    query.setQueryItems(queryItems);

    DUrl bookMarkFrom = DUrl::fromBookMarkFile(fileUrl, fileUrl.fileName());
    bookMarkFrom.setQuery(query);
    QSharedPointer<DFMTouchFileEvent> event(new DFMTouchFileEvent(nullptr, bookMarkFrom));

    EXPECT_TRUE(m_pTester->touch(event));
}

TEST_F(TestBookMarkManager, can_rename_bookmark)
{
    DUrl fileUrl = DUrl::fromLocalFile(tempDirPath);
    DUrl bookMarkFrom = DUrl::fromBookMarkFile(fileUrl, BOOKMARK_STR);
    DUrl bookMarkTo = DUrl::fromBookMarkFile(fileUrl, BOOKMARK_NEW_STR);

    QSharedPointer<DFMRenameEvent> event(new DFMRenameEvent(nullptr, bookMarkFrom, bookMarkTo));

    EXPECT_TRUE(m_pTester->renameFile(event));
}

TEST_F(TestBookMarkManager, can_rename_bookmark_file)
{
    DUrl fromUrl = DUrl::fromLocalFile(tempDirPath);
    DUrl toUrl = DUrl::fromLocalFile(tempDirPath.replace(BOOKMARK_STR, BOOKMARK_NEW_STR));

    EXPECT_TRUE(m_pTester->onFileRenamed(fromUrl, toUrl));
}

TEST_F(TestBookMarkManager, can_remove_bookmark)
{
    DUrl fileUrl = DUrl::fromBookMarkFile(DUrl::fromLocalFile(tempDirPath.replace(BOOKMARK_STR, BOOKMARK_NEW_STR)), BOOKMARK_NEW_STR);

    QSharedPointer<DFMRemoveBookmarkEvent> event(new DFMRemoveBookmarkEvent(nullptr, fileUrl));

    EXPECT_TRUE(m_pTester->removeBookmark(event));
}
