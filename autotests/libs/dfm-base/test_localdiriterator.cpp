// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_localdiriterator.cpp
 * @brief Unit tests for LocalDirIterator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/file/local/localdiriterator.h"

#include <QTest>

using namespace src;

class LocalDirIteratorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new LocalDirIterator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    LocalDirIterator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(LocalDirIteratorTest, LocalDirIterator)
{
    // Test constructor: LocalDirIterator((const QUrl &url,
                                   const QStringList &nameFilters,
                                   QDir::Filters filters,
                                   QDirIterator::IteratorFlags flags))
    ASSERT_NE(obj, nullptr);
}

TEST_F(LocalDirIteratorTest, M_~LocalDirIterator)
{
    // Test method:  ~LocalDirIterator(())
    EXPECT_NO_FATAL_FAILURE({ LocalDirIterator *tmp = new LocalDirIterator(); delete tmp; });
}

TEST_F(LocalDirIteratorTest, next)
{
    // Test getter: QUrl next()
    auto result = obj->next();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(LocalDirIteratorTest, close)
{
    // Test method: void close(())
    EXPECT_NO_FATAL_FAILURE(obj->close());
}

TEST_F(LocalDirIteratorTest, url)
{
    // Test getter: QUrl url()
    auto result = obj->url();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(LocalDirIteratorTest, hasNext)
{
    // Test bool getter: hasNext()
    bool result = obj->hasNext();
    EXPECT_FALSE(result);

}

TEST_F(LocalDirIteratorTest, fileName)
{
    // Test getter: QString fileName()
    auto result = obj->fileName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(LocalDirIteratorTest, fileUrl)
{
    // Test getter: QUrl fileUrl()
    auto result = obj->fileUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(LocalDirIteratorTest, fileInfo)
{
    // Test getter: FileInfoPointer fileInfo()
    auto result = obj->fileInfo();
    EXPECT_EQ(result.get(), nullptr);

}

TEST_F(LocalDirIteratorTest, setArguments)
{
    // Test setter: void setArguments((const QVariantMap &args))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setArguments(_arg0));
}

TEST_F(LocalDirIteratorTest, initIterator)
{
    // Test bool getter: initIterator()
    bool result = obj->initIterator();
    EXPECT_FALSE(result);

}

TEST_F(LocalDirIteratorTest, asyncIterator)
{
    // Test getter: DEnumeratorFuture asyncIterator()
    auto result = obj->asyncIterator();
    EXPECT_NO_FATAL_FAILURE({ obj->asyncIterator(); });

}

TEST_F(LocalDirIteratorTest, fileInfos)
{
    // Test getter: QList<FileInfoPointer> fileInfos()
    auto result = obj->fileInfos();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(LocalDirIteratorTest, oneByOne)
{
    // Test bool getter: oneByOne()
    bool result = obj->oneByOne();
    EXPECT_FALSE(result);

}

TEST_F(LocalDirIteratorTest, cacheBlockIOAttribute)
{
    // Test method: void cacheBlockIOAttribute(())
    EXPECT_NO_FATAL_FAILURE(obj->cacheBlockIOAttribute());
}

TEST_F(LocalDirIteratorTest, sortFileInfoList)
{
    // Test getter: QList<SortInfoPointer> sortFileInfoList()
    auto result = obj->sortFileInfoList();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(LocalDirIteratorTest, enableIteratorByKeyword)
{
    // Test bool getter: enableIteratorByKeyword()
    bool result = obj->enableIteratorByKeyword();
    EXPECT_FALSE(result);

}

TEST_F(LocalDirIteratorTest, d)
{
    // Test getter: QScopedPointer<LocalDirIteratorPrivate> d()
    auto result = obj->d();
    EXPECT_EQ(result.get(), nullptr);

}
