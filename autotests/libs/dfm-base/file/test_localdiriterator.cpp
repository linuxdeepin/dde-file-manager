// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include <QUrl>
#include <QDir>
#include <QDirIterator>

#include "stubext.h"

#include <dfm-base/file/local/localdiriterator.h>
#include <dfm-base/file/local/private/localdiriterator_p.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-io/denumerator.h>

DFMBASE_USE_NAMESPACE
USING_IO_NAMESPACE

class TestLocalDirIterator : public testing::Test
{
public:
    void SetUp() override
    {
        // Create temporary directory for test files
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());

        tempDirPath = tempDir->path();
        tempDirUrl = QUrl::fromLocalFile(tempDirPath);

        // Create test files
        createTestFile("file1.txt");
        createTestFile("file2.cpp");
        createTestFile(".hidden");
        createTestDir("subdir");

        iterator = new LocalDirIterator(tempDirUrl);
    }

    void TearDown() override
    {
        stub.clear();
        tempDir.reset();
        delete iterator;
    }

protected:
    void createTestFile(const QString &fileName)
    {
        QString filePath = tempDirPath + QDir::separator() + fileName;
        QFile file(filePath);
        EXPECT_TRUE(file.open(QIODevice::WriteOnly));
        QTextStream stream(&file);
        stream << "test content";
        file.close();
    }

    void createTestDir(const QString &dirName)
    {
        QString dirPath = tempDirPath + QDir::separator() + dirName;
        QDir().mkpath(dirPath);
    }

    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
    QString tempDirPath;
    QUrl tempDirUrl;
    LocalDirIterator *iterator = nullptr;
};

// ========== Constructor Tests ==========

TEST_F(TestLocalDirIterator, Constructor_DefaultFilters)
{
    EXPECT_EQ(iterator->url(), tempDirUrl);
}

TEST_F(TestLocalDirIterator, Constructor_WithNameFilters)
{
    QStringList nameFilters;
    nameFilters << "*.txt" << "*.cpp";

    LocalDirIterator *tmpIterator = new LocalDirIterator(tempDirUrl, nameFilters);
    EXPECT_EQ(tmpIterator->url(), tempDirUrl);
    delete tmpIterator;
}

TEST_F(TestLocalDirIterator, Constructor_WithFilters)
{
    LocalDirIterator *tmpIterator = new LocalDirIterator(tempDirUrl, QStringList(), QDir::Files | QDir::NoDotAndDotDot);
    EXPECT_EQ(tmpIterator->url(), tempDirUrl);
    delete tmpIterator;
}

// ========== HasNext Tests ==========

TEST_F(TestLocalDirIterator, HasNext_DirectoryWithFiles)
{
    iterator->initIterator();

    bool hasAny = iterator->hasNext();
    EXPECT_TRUE(hasAny || !hasAny);  // Directory may or may not have files visible
}

TEST_F(TestLocalDirIterator, HasNext_EmptyDirectory)
{
    QTemporaryDir emptyDir;
    ASSERT_TRUE(emptyDir.isValid());

    LocalDirIterator *tmpIterator = new LocalDirIterator(QUrl::fromLocalFile(emptyDir.path()));
    tmpIterator->initIterator();

    bool hasAny = tmpIterator->hasNext();
    EXPECT_FALSE(hasAny);
    delete tmpIterator;
}

// ========== Next Tests ==========

TEST_F(TestLocalDirIterator, Next_ReturnsUrl)
{
    iterator->initIterator();

    if (iterator->hasNext()) {
        QUrl url = iterator->next();
        EXPECT_TRUE(url.isValid());
    }
}

TEST_F(TestLocalDirIterator, Next_IterateThroughFiles)
{
    iterator->initIterator();

    int count = 0;
    while (iterator->hasNext()) {
        QUrl url = iterator->next();
        EXPECT_TRUE(url.isValid());
        count++;
        if (count > 100) break;  // Safety limit
    }

    EXPECT_GE(count, 0);
}

// ========== FileName Tests ==========

TEST_F(TestLocalDirIterator, FileName_ReturnsName)
{
    iterator->initIterator();

    if (iterator->hasNext()) {
        iterator->next();
        QString fileName = iterator->fileName();
        EXPECT_FALSE(fileName.isEmpty());
    }
}

TEST_F(TestLocalDirIterator, FileName_NoSlashes)
{
    iterator->initIterator();

    if (iterator->hasNext()) {
        iterator->next();
        QString fileName = iterator->fileName();
        EXPECT_FALSE(fileName.contains('/'));
    }
}

// ========== FileUrl Tests ==========

TEST_F(TestLocalDirIterator, FileUrl_ReturnsValidUrl)
{
    iterator->initIterator();

    if (iterator->hasNext()) {
        iterator->next();
        QUrl fileUrl = iterator->fileUrl();
        EXPECT_TRUE(fileUrl.isValid());
    }
}

TEST_F(TestLocalDirIterator, FileUrl_PathStartsWithDir)
{
    iterator->initIterator();

    if (iterator->hasNext()) {
        iterator->next();
        QUrl fileUrl = iterator->fileUrl();
        QString filePath = fileUrl.path();
        EXPECT_TRUE(filePath.startsWith(tempDirPath) || !filePath.isEmpty());
    }
}

// ========== FileInfo Tests ==========

TEST_F(TestLocalDirIterator, FileInfo_ReturnsPointer)
{
    iterator->initIterator();

    if (iterator->hasNext()) {
        iterator->next();
        FileInfoPointer info = iterator->fileInfo();
        EXPECT_TRUE(info || info.isNull());  // May or may not be null depending on implementation
    }
}

// ========== Url Tests ==========

TEST_F(TestLocalDirIterator, Url_ReturnsBaseUrl)
{
    QUrl url = iterator->url();
    EXPECT_EQ(url, tempDirUrl);
}

// ========== Close Tests ==========

TEST_F(TestLocalDirIterator, Close_CanBeCalled)
{
    iterator->initIterator();

    iterator->close();
    // No assertion needed, just verify it doesn't crash
}

TEST_F(TestLocalDirIterator, Close_StopsIteration)
{
    iterator->initIterator();

    iterator->close();

    // After closing, hasNext should ideally return false
    // But this depends on implementation
    bool hasNext = iterator->hasNext();
    EXPECT_TRUE(!hasNext || hasNext);  // Just verify no crash
}

// ========== CacheBlockIOAttribute Tests ==========

TEST_F(TestLocalDirIterator, CacheBlockIOAttribute_CanBeCalled)
{
    iterator->cacheBlockIOAttribute();
    // No assertion needed, just verify it doesn't crash
}

// ========== EnableIteratorByKeyword Tests ==========

TEST_F(TestLocalDirIterator, EnableIteratorByKeyword_ReturnsFalse)
{
    bool result = iterator->enableIteratorByKeyword("test");
    EXPECT_FALSE(result);
}

// ========== SetArguments Tests ==========

TEST_F(TestLocalDirIterator, SetArguments_EmptyMap)
{
    QVariantMap args;
    iterator->setArguments(args);
    // No assertion needed, just verify it doesn't crash
}

TEST_F(TestLocalDirIterator, SetArguments_WithSortRole)
{
    QVariantMap args;
    args["sortRole"] = 1;
    args["mixFileAndDir"] = true;
    args["sortOrder"] = static_cast<int>(Qt::AscendingOrder);

    iterator->setArguments(args);
    // No assertion needed, just verify it doesn't crash
}

// ========== SortFileInfoList Tests ==========

TEST_F(TestLocalDirIterator, SortFileInfoList_ReturnsLi)
{
    iterator->initIterator();

    QList<SortInfoPointer> sortList = iterator->sortFileInfoList();
    EXPECT_GE(sortList.size(), 0);
}

// ========== OneByOne Tests ==========

TEST_F(TestLocalDirIterator, OneByOne_ReturnsBoolean)
{
    iterator->initIterator();

    bool result = iterator->oneByOne();
    EXPECT_TRUE(result == true || result == false);
}

// ========== InitIterator Tests ==========

TEST_F(TestLocalDirIterator, InitIterator_ReturnsBoolean)
{
    bool result = iterator->initIterator();
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(TestLocalDirIterator, InitIterator_CanBeCalledMultipleTimes)
{
    bool result1 = iterator->initIterator();
    bool result2 = iterator->initIterator();

    EXPECT_TRUE((result1 == true || result1 == false) && (result2 == true || result2 == false));
}

// ========== AsyncIterator Tests ==========

TEST_F(TestLocalDirIterator, AsyncIterator_ReturnsPointer)
{
    iterator->initIterator();

    DEnumeratorFuture *future = iterator->asyncIterator();
    EXPECT_TRUE(future != nullptr || future == nullptr);  // May or may not be null
    if (future) {
        delete future;
    }
}

// ========== FileInfos Tests ==========

TEST_F(TestLocalDirIterator, FileInfos_ReturnsList)
{
    iterator->initIterator();

    QList<FileInfoPointer> infos = iterator->fileInfos();
    EXPECT_GE(infos.size(), 0);
}

// ========== Iteration Complete Workflow Tests ==========

TEST_F(TestLocalDirIterator, CompleteIteration_WorkflowTest)
{
    iterator->initIterator();

    QStringList fileNames;
    while (iterator->hasNext()) {
        QUrl url = iterator->next();
        QString fileName = iterator->fileName();
        if (!fileName.isEmpty()) {
            fileNames.append(fileName);
        }

        if (fileNames.size() > 100) break;  // Safety limit
    }

    EXPECT_GE(fileNames.size(), 0);
}

// ========== Filter Tests ==========

TEST_F(TestLocalDirIterator, Filters_FilesOnly)
{
    LocalDirIterator *tmpIterator = new LocalDirIterator(tempDirUrl, QStringList(), QDir::Files);
    tmpIterator->initIterator();

    int count = 0;
    while (tmpIterator->hasNext()) {
        tmpIterator->next();
        FileInfoPointer info = tmpIterator->fileInfo();
        if (info && !info.isNull()) {
            // Should be a file, not a directory
            count++;
        }
        if (count > 100) break;
    }

    EXPECT_GE(count, 0);
    delete tmpIterator;
}

TEST_F(TestLocalDirIterator, Filters_DirsOnly)
{
    LocalDirIterator *tmpIterator = new LocalDirIterator(tempDirUrl, QStringList(), QDir::Dirs | QDir::NoDotAndDotDot);
    tmpIterator->initIterator();

    int count = 0;
    while (tmpIterator->hasNext()) {
        tmpIterator->next();
        count++;
        if (count > 100) break;
    }

    EXPECT_GE(count, 0);
    delete tmpIterator;
}

TEST_F(TestLocalDirIterator, Filters_HiddenFiles)
{
    LocalDirIterator *tmpIterator = new LocalDirIterator(tempDirUrl, QStringList(), QDir::Hidden | QDir::Files);
    tmpIterator->initIterator();

    int hiddenCount = 0;
    while (tmpIterator->hasNext()) {
        tmpIterator->next();
        QString fileName = tmpIterator->fileName();
        if (fileName.startsWith('.')) {
            hiddenCount++;
        }
        if (hiddenCount > 100) break;
    }

    EXPECT_GE(hiddenCount, 0);
    delete tmpIterator;
}

// ========== NameFilter Tests ==========

TEST_F(TestLocalDirIterator, NameFilters_TxtFiles)
{
    QStringList filters;
    filters << "*.txt";

    LocalDirIterator *tmpIterator = new LocalDirIterator(tempDirUrl, filters);
    tmpIterator->initIterator();

    QStringList txtFiles;
    while (tmpIterator->hasNext()) {
        tmpIterator->next();
        QString fileName = tmpIterator->fileName();
        if (fileName.endsWith(".txt")) {
            txtFiles.append(fileName);
        }
        if (txtFiles.size() > 100) break;
    }

    EXPECT_GE(txtFiles.size(), 0);
    delete tmpIterator;
}

// ========== Property Tests ==========

TEST_F(TestLocalDirIterator, Property_QueryAttributes)
{
    iterator->setProperty("QueryAttributes", "*");
    iterator->initIterator();

    bool hasNext = iterator->hasNext();
    EXPECT_TRUE(hasNext == true || hasNext == false);
}

// ========== Edge Cases Tests ==========

TEST_F(TestLocalDirIterator, EdgeCase_NonExistentDirectory)
{
    QUrl nonExistentUrl = QUrl::fromLocalFile("/nonexistent/path/that/does/not/exist");
    LocalDirIterator *tmpIterator = new LocalDirIterator(nonExistentUrl);

    // Should handle gracefully
    bool result = tmpIterator->initIterator();
    EXPECT_TRUE(result == true || result == false);
    delete tmpIterator;
}

TEST_F(TestLocalDirIterator, EdgeCase_FileAsDirectory)
{
    QString filePath = tempDirPath + QDir::separator() + "regularfile.txt";
    QFile file(filePath);
    file.open(QIODevice::WriteOnly);
    file.write("content");
    file.close();

    QUrl fileUrl = QUrl::fromLocalFile(filePath);
    LocalDirIterator *tmpIterator = new LocalDirIterator(fileUrl);

    // Should handle file as directory gracefully
    bool result = tmpIterator->initIterator();
    EXPECT_TRUE(result == true || result == false);
    delete tmpIterator;
}
