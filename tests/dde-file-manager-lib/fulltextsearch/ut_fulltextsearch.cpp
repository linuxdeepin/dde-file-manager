#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QProcess>
#include <QStandardPaths>
#include <QFile>
#include "stub.h"

#define private public
#include "fulltextsearch.h"

#define filetextSearch DFMFullTextSearchManager::getInstance()
DFM_USE_NAMESPACE
namespace  {
class TestFullTextSearch: public testing::Test
{
public:
    virtual void SetUp() override
    {
        std::cout << "start TestFullTextSearch" << std::endl;
        searchPath = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first() + "/fulltextsearch";
        filePath = searchPath + "/1.txt";
        QProcess::execute("mkdir " + searchPath);
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write("你好謝謝hello123");
            file.close();
        }
    }

    virtual void TearDown() override
    {
        std::cout << "end TestFullTextSearch" << std::endl;
        QProcess::execute("rm -rf " + searchPath);
    }

public:
    QString searchPath;
    QString filePath;
};
}

TEST_F(TestFullTextSearch, fulltextIndex)
{
    EXPECT_EQ(1, filetextSearch->fulltextIndex(searchPath));
    EXPECT_EQ(0, filetextSearch->fulltextIndex(searchPath));
    QThread::sleep(2);
}

TEST_F(TestFullTextSearch, createFileIndex)
{
    QProcess::execute("rm -rf " + filetextSearch->indexStorePath);
    // Source directory doesn't exist
    EXPECT_FALSE(filetextSearch->createFileIndex("/123646"));
    // Unable to create index directory
    {
        bool (*stub_mkpath)(const QString &) = [](const QString &)->bool{
            return false;
        };
        Stub stub;
        stub.set(ADDR(QDir, mkpath), stub_mkpath);
        EXPECT_FALSE(filetextSearch->createFileIndex(searchPath));
    }
}

TEST_F(TestFullTextSearch, fullTextSearch)
{
    filetextSearch->createFileIndex(searchPath);
    QStringList searchResult = filetextSearch->fullTextSearch("你好謝謝hello123", searchPath);
    EXPECT_NO_FATAL_FAILURE(searchResult.contains(filePath));
}

TEST_F(TestFullTextSearch, updateIndex)
{
    QThread::sleep(2);
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        file.write("知道");
        file.close();
    }

    QFile file2("/tmp/fulltextsearch/2.txt");
    if (file2.open(QIODevice::ReadWrite)) {
        file2.write("知道123");
        file2.close();
    }

    filetextSearch->setSearchState(JobController::Started);
    filetextSearch->updateIndex(searchPath);
    QStringList searchResult = filetextSearch->fullTextSearch("知道", searchPath);
    EXPECT_NO_FATAL_FAILURE(searchResult.contains(filePath));
}

