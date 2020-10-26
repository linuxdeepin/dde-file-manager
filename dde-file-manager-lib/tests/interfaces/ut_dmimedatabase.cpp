#include <gtest/gtest.h>

#include "dmimedatabase.h"

using namespace testing;
DFM_USE_NAMESPACE

class DMimeDatabaseTest:public testing::Test{

public:
    DMimeDatabase db;
    virtual void SetUp() override{
        std::cout << "start DFileCopyQueueTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end DFileCopyQueueTest" << std::endl;
    }
};

TEST_F(DMimeDatabaseTest, can_get_mimeTypeForUrl)
{
    DUrl url;
    url.setScheme(FILE_SCHEME);
    url.setPath("/etc/presage.xml");
    EXPECT_TRUE(db.mimeTypeForUrl(url).name().contains("xml"));
}

TEST_F(DMimeDatabaseTest, can_get_mimeTypeForFile)
{
    DUrl url;
    url.setScheme(FILE_SCHEME);
    url.setPath("/etc/presage.xml");
    EXPECT_TRUE(db.mimeTypeForFile(url.toLocalFile()).name().contains("xml"));
    EXPECT_FALSE(db.mimeTypeForFile(QString("/proc/kmsg"),QMimeDatabase::MatchExtension).name().contains("xml"));
}

TEST_F(DMimeDatabaseTest, can_get_mimeTypeForFile_othertype)
{
    DUrl url;
    url.setScheme(FILE_SCHEME);
    url.setPath("/etc/presage.xml");
    EXPECT_TRUE(db.mimeTypeForFile(url.toLocalFile(),QMimeDatabase::MatchExtension).name().contains("xml"));
}

TEST_F(DMimeDatabaseTest, can_get_mimeTypeForFile_gvfs)
{
    DUrl url;
    url.setScheme(FILE_SCHEME);
    url.setPath("/etc/presage.xml");
    EXPECT_TRUE(db.mimeTypeForFile(url.toLocalFile(),QMimeDatabase::MatchExtension, QString("qwerty"),false).name().contains("xml"));
    EXPECT_FALSE(db.mimeTypeForFile(QString("/etc"),QMimeDatabase::MatchExtension, QString("asdasda"),false).name().contains("xml"));
    EXPECT_TRUE(db.mimeTypeForFile(url.toLocalFile(),QMimeDatabase::MatchExtension, QString("qwerty"),false).name().contains("xml"));
    EXPECT_TRUE(db.mimeTypeForFile(url.toLocalFile(),QMimeDatabase::MatchDefault, QString("fdsafd"),false).name().contains("xml"));
    EXPECT_TRUE(db.mimeTypeForFile(url.toLocalFile(),QMimeDatabase::MatchDefault, QString("fdsafed"),true).name().contains("xml"));
}
