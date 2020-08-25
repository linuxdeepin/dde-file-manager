#include "fulltextsearch.h"

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QFileInfo>
#include <QDir>

#define filetextSearch DFMFullTextSearchManager::getInstance()
DFM_USE_NAMESPACE
namespace  {
class TestFullTextSearch: public testing::Test
{
public:
    QString searchPath = QDir::currentPath() + "/TestData";
    virtual void SetUp() override
    {
        QDir dir;
        if (!dir.exists(searchPath)) {
            dir.mkpath(searchPath);
        }
        QFile file(searchPath + "/test.txt");
        if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {
            file.write("你好謝謝hello123");
            file.close();
        }

        std::cout << "start TestVaultHelper" << std::endl;
    }

    virtual void TearDown() override
    {
        QDir dir(searchPath);
        QFile file(searchPath + "/test.txt");
        file.remove();
        dir.rmpath(searchPath);
        std::cout << "end TestVaultHelper" << std::endl;
    }
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
    EXPECT_TRUE(searchResult.contains(searchPath + "/test.txt"));
    searchResult = filetextSearch->fullTextSearch("謝謝");
    EXPECT_TRUE(searchResult.contains(searchPath + "/test.txt"));
    searchResult = filetextSearch->fullTextSearch("hello");
    EXPECT_TRUE(searchResult.contains(searchPath + "/test.txt"));
    searchResult = filetextSearch->fullTextSearch("123");
    EXPECT_TRUE(searchResult.contains(searchPath + "/test.txt"));
}

TEST_F(TestFullTextSearch, updateIndex)
{
    QFile file(searchPath + "/test1.txt");
    // Add
    if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        file.write("测试数据");
        file.close();

        //filetextSearch->updateIndex(searchPath + "/test1.txt", DFMFullTextSearchManager::Add);
        QStringList searchResult = filetextSearch->fullTextSearch("测试");
        EXPECT_TRUE(searchResult.contains(searchPath + "/test1.txt"));
    }

    // Modify
    if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        file.write("你好");
        file.close();

        //filetextSearch->updateIndex(searchPath + "/test1.txt", DFMFullTextSearchManager::Modify);
        QStringList searchResult = filetextSearch->fullTextSearch("你好");
        EXPECT_TRUE(searchResult.contains(searchPath + "/test1.txt"));
    }

    // Delete
    if (file.remove()) {
       // filetextSearch->updateIndex(searchPath + "/test1.txt", DFMFullTextSearchManager::Delete);
        QStringList searchResult = filetextSearch->fullTextSearch("你好");
        EXPECT_FALSE(searchResult.contains(searchPath + "/test1.txt"));
    }
}
}
