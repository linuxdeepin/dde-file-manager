// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_LOCALFILEWATCHER
#define UT_LOCALFILEWATCHER

#include <stubext.h>
#include <dfm-base/file/local/localdiriterator.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/private/localdiriterator_p.h>

#include <dfm-io/dfileinfo.h>

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
    stub_ext::StubExt stub;
    stub.set_lamda(&dfmio::DFileInfo::attribute, []{ __DBG_STUB_INVOKE__ return ".testOOOOOO.txt";});
    stub.set_lamda(&FileUtils::isLocalDevice, []{ __DBG_STUB_INVOKE__ return false;});
    typedef FileInfoPointer (*TransfromInfo)(const QString &, FileInfoPointer);
    stub.set_lamda(static_cast<TransfromInfo>(&dfmbase::InfoFactory::transfromInfo), []{ __DBG_STUB_INVOKE__ return nullptr;});
    stub.set_lamda(&AsyncFileInfo::cacheAsyncAttributes, []{ __DBG_STUB_INVOKE__ });
    EXPECT_TRUE(iterator->fileInfo().isNull());

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

TEST_F(UT_LocalFileDirIterator, testLocalFileIteratorOther)
{
    LocalDirIterator iterator(QUrl("recent:///ddd.txt"));

    EXPECT_FALSE(iterator.initIterator());

    iterator.d->dfmioDirIterator = nullptr;
    EXPECT_FALSE(iterator.hasNext());
    EXPECT_FALSE(iterator.url().isValid());
    QVariantMap args;
    iterator.setArguments(args);
    EXPECT_TRUE(iterator.sortFileInfoList().isEmpty());
    EXPECT_TRUE(iterator.oneByOne());
    EXPECT_FALSE(iterator.initIterator());
    EXPECT_TRUE(iterator.asyncIterator() == nullptr);
    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(LocalDirIterator, fileUrl), []{ __DBG_STUB_INVOKE__ return QUrl();});
    EXPECT_TRUE(iterator.fileName().isEmpty());
    stub.set_lamda(VADDR(LocalDirIterator, fileUrl), []{ __DBG_STUB_INVOKE__ return QUrl("file:////");});
    EXPECT_TRUE(iterator.fileName().isEmpty());
    stub.set_lamda(VADDR(LocalDirIterator, fileUrl), []{ __DBG_STUB_INVOKE__ return QUrl("file:///tttt/");});
    EXPECT_TRUE(iterator.fileName() == QString("tttt"));
}

#endif
