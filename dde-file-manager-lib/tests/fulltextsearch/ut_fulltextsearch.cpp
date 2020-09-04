#include "fulltextsearch.h"

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QProcess>
#include <QStandardPaths>
#include <QFile>

#define filetextSearch DFMFullTextSearchManager::getInstance()
DFM_USE_NAMESPACE
namespace  {
class TestFullTextSearch: public testing::Test
{
public:
    virtual void SetUp() override
    {
        std::cout << "start TestVaultHelper" << std::endl;
        searchPath = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first() + "/fulltextsearch";
        filePath = searchPath + "/1.txt";
        QProcess::execute("mkdir " + searchPath);
        QFile file(filePath);
        if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {
            file.write("你好謝謝hello123");
            file.close();
        }
    }

    virtual void TearDown() override
    {
        std::cout << "end TestVaultHelper" << std::endl;
        QProcess::execute("rm -rf " + searchPath);
    }

public:
    QString searchPath;
    QString filePath;
};

TEST_F(TestFullTextSearch, fulltextIndex)
{
    EXPECT_EQ(1, filetextSearch->fulltextIndex(searchPath));
    EXPECT_EQ(0, filetextSearch->fulltextIndex(searchPath));
}

TEST_F(TestFullTextSearch, fullTextSearch)
{
    QThread::sleep(1);
    QStringList searchResult = filetextSearch->fullTextSearch("你好");
    EXPECT_TRUE(searchResult.contains(filePath));
    searchResult = filetextSearch->fullTextSearch("謝謝");
    EXPECT_TRUE(searchResult.contains(filePath));
    searchResult = filetextSearch->fullTextSearch("hello");
    EXPECT_TRUE(searchResult.contains(filePath));
    searchResult = filetextSearch->fullTextSearch("123");
    EXPECT_TRUE(searchResult.contains(filePath));
}

TEST_F(TestFullTextSearch, updateIndex)
{
    QThread::sleep(2);
    QFile file(filePath);
    if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        file.write("知道");
        file.close();

        filetextSearch->setSearchState(JobController::Started);
        filetextSearch->updateIndex(searchPath);
        QStringList searchResult = filetextSearch->fullTextSearch("知道");
        EXPECT_TRUE(searchResult.contains(filePath));
    }
}
}
