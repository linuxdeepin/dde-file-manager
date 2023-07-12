// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_ABSTRACTFILEWATCHER
#define UT_ABSTRACTFILEWATCHER

#include <dfm-base/file/local/localdiriterator.h>

#include <QDir>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE

class UT_LocalFileDirIterator : public testing::Test
{
public:
    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
    }

    ~UT_LocalFileDirIterator() override;

};

UT_LocalFileDirIterator::~UT_LocalFileDirIterator() {

}

TEST_F(UT_LocalFileDirIterator, testLocalFileIterator)
{
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    url.setPath(url.path() + QDir::separator() + "testAsyncFileInfo");
    LocalDirIterator * iterator = new LocalDirIterator(url);
    EXPECT_NO_FATAL_FAILURE(iterator->cacheBlockIOAttribute());
    QProcess::execute("mkdir ./testAsyncFileInfo");
    QProcess::execute("touch ./testAsyncFileInfo/testAsyncFileInfo.txt");
    EXPECT_TRUE(iterator->hasNext());
    auto fileUrl = QUrl::fromLocalFile(url.path() + QDir::separator() + "testAsyncFileInfo.txt");
    EXPECT_EQ(fileUrl, iterator->next());
    EXPECT_TRUE(!iterator->fileInfo().isNull());
    EXPECT_EQ(fileUrl, iterator->fileUrl());
    EXPECT_EQ("testAsyncFileInfo.txt", iterator->fileName());
    EXPECT_EQ(url, iterator->url());
    EXPECT_FALSE(iterator->enableIteratorByKeyword("test"));
    iterator->close();
    iterator->deleteLater();

    iterator = new LocalDirIterator(url);
    QVariantMap args;
    args.insert("sortRole",
                QVariant::fromValue(dfmio::DEnumerator::SortRoleCompareFlag::kSortRoleCompareFileName));
    args.insert("mixFileAndDir", false);
    args.insert("sortOrder", Qt::DescendingOrder);
    iterator->setArguments(args);
    auto infolist = iterator->sortFileInfoList();
    EXPECT_EQ(1, infolist.count());
    EXPECT_EQ(fileUrl, infolist.first()->fileUrl());
    iterator->close();
    iterator->deleteLater();

    QSharedPointer<LocalDirIterator> iteratorPointer {new LocalDirIterator(url)};
    auto future = iteratorPointer->asyncIterator();
    EXPECT_TRUE(0 == future->fileCount());
    future->startAsyncIterator();
    iteratorPointer->close();
    future->deleteLater();

    QProcess::execute("fm -rf ./testAsyncFileInfo");
}

#endif
