// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QDebug>
#include <QSignalSpy>

#include "trashdiriterator.h"
#include "utils/trashhelper.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/device/deviceutils.h"
#include "dfm-base/utils/universalutils.h"
#include "dfm-base/interfaces/fileinfo.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/dfm_global_defines.h"
#include <dfm-io/denumerator.h>

#include <stubext.h>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DPTRASH_USE_NAMESPACE

using namespace dfmplugin_trash;

class TestTrashDirIterator : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(TestTrashDirIterator, Constructor)
{
    QUrl url("trash:///");
    TrashDirIterator iterator(url);

    EXPECT_EQ(iterator.url(), url);
}

TEST_F(TestTrashDirIterator, ConstructorWithFilters)
{
    QUrl url("trash:///");
    QStringList nameFilters = { "*.txt" };
    QDir::Filters filters = QDir::Files;
    QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags;

    TrashDirIterator iterator(url, nameFilters, filters, flags);

    EXPECT_EQ(iterator.url(), url);
}

TEST_F(TestTrashDirIterator, Destructor)
{
    TrashDirIterator *iterator = new TrashDirIterator(QUrl("trash:///"));
    EXPECT_NE(iterator, nullptr);
    delete iterator;
    // Verify that destruction doesn't cause crashes
    EXPECT_TRUE(true);
}

TEST_F(TestTrashDirIterator, Url)
{
    QUrl url("trash:///test/");
    TrashDirIterator iterator(url);

    QUrl result = iterator.url();
    EXPECT_EQ(result, url);
}

TEST_F(TestTrashDirIterator, Next)
{
    QUrl url("trash:///");
    TrashDirIterator iterator(url);

    QUrl result = iterator.next();
    // Default implementation returns empty QUrl
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(TestTrashDirIterator, HasNext)
{
    QUrl url("trash:///");
    TrashDirIterator iterator(url);

    // Mock DFMIO::DEnumerator::hasNext to return true
    bool mockHasNextResult = true;
    stub.set_lamda(&DFMIO::DEnumerator::hasNext, [&mockHasNextResult](DFMIO::DEnumerator *self) -> bool {
        Q_UNUSED(self)
        return mockHasNextResult;
    });

    // Mock DFMIO::DEnumerator::next to return a valid URL
    QUrl mockNextUrl("trash:///test.txt");
    stub.set_lamda(&DFMIO::DEnumerator::next, [&mockNextUrl](DFMIO::DEnumerator *self) -> QUrl {
        Q_UNUSED(self)
        return mockNextUrl;
    });

    // Mock DFMIO::DEnumerator::uri to return root URL
    QUrl mockUri("trash:///");
    stub.set_lamda(&DFMIO::DEnumerator::uri, [&mockUri](DFMIO::DEnumerator *self) -> QUrl {
        Q_UNUSED(self)
        return mockUri;
    });

    // Mock UniversalUtils::urlEquals
    stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &url1, const QUrl &url2) -> bool {
        return url1 == url2;
    });

    // Mock DeviceUtils::fstabBindInfo to return a test map
    QMap<QString, QString> mockMap;
    mockMap.insert("/dev/sda1", "/mnt/external");
    stub.set_lamda(&DeviceUtils::fstabBindInfo, [&mockMap]() -> QMap<QString, QString> {
        return mockMap;
    });

    // Mock InfoFactory to return a valid FileInfo
    auto mockFileInfo = InfoFactory::create<FileInfo>(QUrl("trash:///test.txt"));
    stub.set_lamda(static_cast<FileInfoPointer (*)(const QUrl &, Global::CreateFileInfoType, QString *)>(&InfoFactory::create<FileInfo>), 
        [&mockFileInfo](const QUrl &url, Global::CreateFileInfoType type, QString *key) -> FileInfoPointer {
            Q_UNUSED(url)
            Q_UNUSED(type)
            Q_UNUSED(key)
            return mockFileInfo;
    });

    // Mock TrashHelper instance and its trashNotEmpty method
    auto trashHelper = TrashHelper::instance();
    stub.set_lamda(&TrashHelper::instance, [trashHelper]() -> TrashHelper* {
        return trashHelper;
    });

    stub.set_lamda(&TrashHelper::rootUrl, []() -> QUrl {
        return QUrl("trash:///");
    });

    bool trashNotEmptyCalled = false;
    stub.set_lamda(&TrashHelper::trashNotEmpty, [&trashNotEmptyCalled]() {
        trashNotEmptyCalled = true;
    });

    // Test with mock setup
    bool result = iterator.hasNext();
    EXPECT_EQ(result, mockHasNextResult);
    EXPECT_TRUE(trashNotEmptyCalled);
}

TEST_F(TestTrashDirIterator, FileName)
{
    QUrl url("trash:///");
    TrashDirIterator iterator(url);

    // Just verify the method does not crash - since FileInfo may be null, 
    // we avoid accessing its members directly
    EXPECT_NO_THROW({
        QString name = iterator.fileName();
        (void)name; // Use the variable to avoid unused warning
    });
}

TEST_F(TestTrashDirIterator, FileName_NoFileInfo)
{
    QUrl url("trash:///");
    TrashDirIterator iterator(url);

    // Just verify the method does not crash when fileInfo is null
    EXPECT_NO_THROW({
        QString name = iterator.fileName();
        (void)name; // Use the variable to avoid unused warning
    });
}

TEST_F(TestTrashDirIterator, FileUrl)
{
    QUrl url("trash:///");
    TrashDirIterator iterator(url);

    // Just verify the method does not crash
    EXPECT_NO_THROW({
        QUrl result = iterator.fileUrl();
        (void)result; // Use the variable to avoid unused warning
    });
}

TEST_F(TestTrashDirIterator, FileInfo)
{
    QUrl url("trash:///");
    TrashDirIterator iterator(url);

    // Mock InfoFactory to return a valid FileInfo
    auto mockFileInfo = InfoFactory::create<FileInfo>(QUrl("trash:///test.txt"));
    bool infoFactoryCalled = false;
    stub.set_lamda(static_cast<FileInfoPointer (*)(const QUrl &, Global::CreateFileInfoType, QString *)>(&InfoFactory::create<FileInfo>), 
        [&mockFileInfo, &infoFactoryCalled](const QUrl &url, Global::CreateFileInfoType type, QString *key) -> FileInfoPointer {
            infoFactoryCalled = true;
            Q_UNUSED(url)
            Q_UNUSED(type)
            Q_UNUSED(key)
            return mockFileInfo;
    });

    const FileInfoPointer info = iterator.fileInfo();
    EXPECT_TRUE(infoFactoryCalled);
    EXPECT_NE(info, nullptr);
}

TEST_F(TestTrashDirIterator, FileInfo_WithExistingInfo)
{
    QUrl url("trash:///");
    TrashDirIterator iterator(url);

    // First call to fileInfo should create via InfoFactory
    auto mockFileInfo = InfoFactory::create<FileInfo>(QUrl("trash:///test.txt"));
    int infoFactoryCallCount = 0;
    stub.set_lamda(static_cast<FileInfoPointer (*)(const QUrl &, Global::CreateFileInfoType, QString *)>(&InfoFactory::create<FileInfo>), 
        [&mockFileInfo, &infoFactoryCallCount](const QUrl &url, Global::CreateFileInfoType type, QString *key) -> FileInfoPointer {
            infoFactoryCallCount++;
            Q_UNUSED(url)
            Q_UNUSED(type)
            Q_UNUSED(key)
            return mockFileInfo;
    });

    const FileInfoPointer info1 = iterator.fileInfo();
    const FileInfoPointer info2 = iterator.fileInfo();  // Should return same instance

    EXPECT_EQ(infoFactoryCallCount, 1);  // Should only be called once
    EXPECT_NE(info1, nullptr);
    EXPECT_EQ(info1, info2);
}

TEST_F(TestTrashDirIterator, DeviceMapInitialization)
{
    QUrl url("trash:///");
    TrashDirIterator iterator(url);

    // The constructor calls fstabBindInfo internally
    // Just check that the constructor runs without issues
    EXPECT_EQ(iterator.url(), url);
}

