// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QApplication>
#include <QSignalSpy>
#include <QDBusVariant>

#include "stubext.h"

#include "tagmanagerdbus.h"
#include "tagdbhandler.h"
#include "daemonplugin_tag_global.h"

DAEMONPTAG_USE_NAMESPACE

class TestTagManagerDBus : public testing::Test
{
public:
    void SetUp() override
    {
        manager = new TagManagerDBus;
    }

    void TearDown() override
    {
        delete manager;
        manager = nullptr;
        stub.clear();
    }

protected:
    TagManagerDBus *manager = nullptr;
    stub_ext::StubExt stub;
};

// Test Query method with kTags option
TEST_F(TestTagManagerDBus, Query_WithTagsOption_ShouldReturnAllTags)
{
    QVariantMap mockTags;
    mockTags["tag1"] = "red";
    mockTags["tag2"] = "blue";
    
    stub.set_lamda(&TagDbHandler::getAllTags, [&mockTags](TagDbHandler *) {
        __DBG_STUB_INVOKE__
        return mockTags;
    });
    
    QDBusVariant result = manager->Query(static_cast<int>(QueryOpts::kTags));
    
    EXPECT_EQ(result.variant().toMap(), mockTags);
}

// Test Query method with kFilesWithTags option
TEST_F(TestTagManagerDBus, Query_WithFilesWithTagsOption_ShouldReturnAllFileWithTags)
{
    QVariantHash mockFilesTags;
    mockFilesTags["/path/file1"] = QStringList{"tag1", "tag2"};
    mockFilesTags["/path/file2"] = QStringList{"tag3"};
    
    stub.set_lamda(&TagDbHandler::getAllFileWithTags, [&mockFilesTags](TagDbHandler *) {
        __DBG_STUB_INVOKE__
        return mockFilesTags;
    });
    
    QDBusVariant result = manager->Query(static_cast<int>(QueryOpts::kFilesWithTags));
    
    EXPECT_EQ(result.variant().toHash(), mockFilesTags);
}

// Test Query method with kTagsOfFile option
TEST_F(TestTagManagerDBus, Query_WithTagsOfFileOption_ShouldReturnTagsByUrls)
{
    QStringList urls = {"/path/file1", "/path/file2"};
    QVariantMap mockResult;
    mockResult["/path/file1"] = QStringList{"tag1", "tag2"};
    
    stub.set_lamda(&TagDbHandler::getTagsByUrls, [&mockResult](TagDbHandler *, const QStringList &urlList) {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(urlList.size(), 2);
        return mockResult;
    });
    
    QDBusVariant result = manager->Query(static_cast<int>(QueryOpts::kTagsOfFile), urls);
    
    EXPECT_EQ(result.variant().toMap(), mockResult);
}

// Test Query method with kFilesOfTag option
TEST_F(TestTagManagerDBus, Query_WithFilesOfTagOption_ShouldReturnFilesByTag)
{
    QStringList tags = {"tag1", "tag2"};
    QVariantMap mockResult;
    mockResult["tag1"] = QStringList{"/path/file1", "/path/file2"};
    
    stub.set_lamda(&TagDbHandler::getFilesByTag, [&mockResult](TagDbHandler *, const QStringList &tagList) {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(tagList.size(), 2);
        return mockResult;
    });
    
    QDBusVariant result = manager->Query(static_cast<int>(QueryOpts::kFilesOfTag), tags);
    
    EXPECT_EQ(result.variant().toMap(), mockResult);
}

// Test Query method with kColorOfTags option
TEST_F(TestTagManagerDBus, Query_WithColorOfTagsOption_ShouldReturnTagsColor)
{
    QStringList tags = {"tag1", "tag2"};
    QVariantMap mockResult;
    mockResult["tag1"] = "red";
    mockResult["tag2"] = "blue";
    
    stub.set_lamda(&TagDbHandler::getTagsColor, [&mockResult](TagDbHandler *, const QStringList &tagList) {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(tagList.size(), 2);
        return mockResult;
    });
    
    QDBusVariant result = manager->Query(static_cast<int>(QueryOpts::kColorOfTags), tags);
    
    EXPECT_EQ(result.variant().toMap(), mockResult);
}

// Test Query method with kTagIntersectionOfFiles option
TEST_F(TestTagManagerDBus, Query_WithTagIntersectionOption_ShouldReturnSameTagsOfDiffUrls)
{
    QStringList urls = {"/path/file1", "/path/file2"};
    QStringList mockResult = {"commonTag1", "commonTag2"};
    
    stub.set_lamda(&TagDbHandler::getSameTagsOfDiffUrls, [&mockResult](TagDbHandler *, const QStringList &urlList) {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(urlList.size(), 2);
        return QVariant(mockResult);
    });
    
    QDBusVariant result = manager->Query(static_cast<int>(QueryOpts::kTagIntersectionOfFiles), urls);
    
    EXPECT_EQ(result.variant().toStringList(), mockResult);
}

// Test Insert method with kTags option
TEST_F(TestTagManagerDBus, Insert_WithTagsOption_ShouldAddTagProperty)
{
    QVariantMap tagData;
    tagData["newTag"] = "green";
    
    stub.set_lamda(&TagDbHandler::addTagProperty, [&tagData](TagDbHandler *, const QVariantMap &data) {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(data, tagData);
        return true;
    });
    
    bool result = manager->Insert(static_cast<int>(InsertOpts::kTags), tagData);
    
    EXPECT_TRUE(result);
}

// Test Insert method with kTagOfFiles option
TEST_F(TestTagManagerDBus, Insert_WithTagOfFilesOption_ShouldAddTagsForFiles)
{
    QVariantMap fileTagData;
    fileTagData["/path/file1"] = QStringList{"tag1", "tag2"};
    
    stub.set_lamda(&TagDbHandler::addTagsForFiles, [&fileTagData](TagDbHandler *, const QVariantMap &data) {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(data, fileTagData);
        return true;
    });
    
    bool result = manager->Insert(static_cast<int>(InsertOpts::kTagOfFiles), fileTagData);
    
    EXPECT_TRUE(result);
}

// Test Insert method with invalid option
TEST_F(TestTagManagerDBus, Insert_WithInvalidOption_ShouldReturnFalse)
{
    QVariantMap data;
    data["test"] = "value";
    
    bool result = manager->Insert(999, data); // Invalid option
    
    EXPECT_FALSE(result);
}

// Test Delete method with kTags option
TEST_F(TestTagManagerDBus, Delete_WithTagsOption_ShouldDeleteTags)
{
    QVariantMap deleteData;
    deleteData["tags"] = QStringList{"tag1", "tag2"};
    
    stub.set_lamda(&TagDbHandler::deleteTags, [](TagDbHandler *, const QStringList &tags) {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(tags.size(), 2);
        EXPECT_TRUE(tags.contains("tag1"));
        EXPECT_TRUE(tags.contains("tag2"));
        return true;
    });
    
    bool result = manager->Delete(static_cast<int>(DeleteOpts::kTags), deleteData);
    
    EXPECT_TRUE(result);
}

// Test Delete method with kFiles option
TEST_F(TestTagManagerDBus, Delete_WithFilesOption_ShouldDeleteFiles)
{
    QVariantMap deleteData;
    deleteData["/path/file1"] = "dummy";
    deleteData["/path/file2"] = "dummy";
    
    stub.set_lamda(&TagDbHandler::deleteFiles, [](TagDbHandler *, const QStringList &files) {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(files.size(), 2);
        EXPECT_TRUE(files.contains("/path/file1"));
        EXPECT_TRUE(files.contains("/path/file2"));
        return true;
    });
    
    bool result = manager->Delete(static_cast<int>(DeleteOpts::kFiles), deleteData);
    
    EXPECT_TRUE(result);
}

// Test Delete method with kTagOfFiles option
TEST_F(TestTagManagerDBus, Delete_WithTagOfFilesOption_ShouldRemoveTagsOfFiles)
{
    QVariantMap deleteData;
    deleteData["/path/file1"] = QStringList{"tag1"};
    
    stub.set_lamda(&TagDbHandler::removeTagsOfFiles, [&deleteData](TagDbHandler *, const QVariantMap &data) {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(data, deleteData);
        return true;
    });
    
    bool result = manager->Delete(static_cast<int>(DeleteOpts::kTagOfFiles), deleteData);
    
    EXPECT_TRUE(result);
}

// Test Delete method with invalid option
TEST_F(TestTagManagerDBus, Delete_WithInvalidOption_ShouldReturnFalse)
{
    QVariantMap data;
    data["test"] = "value";
    
    bool result = manager->Delete(999, data); // Invalid option
    
    EXPECT_FALSE(result);
}

// Test Update method with kColors option
TEST_F(TestTagManagerDBus, Update_WithColorsOption_ShouldChangeTagColors)
{
    QVariantMap colorData;
    colorData["tag1"] = "newRed";
    
    stub.set_lamda(&TagDbHandler::changeTagColors, [&colorData](TagDbHandler *, const QVariantMap &data) {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(data, colorData);
        return true;
    });
    
    bool result = manager->Update(static_cast<int>(UpdateOpts::kColors), colorData);
    
    EXPECT_TRUE(result);
}

// Test Update method with kTagsNameWithFiles option
TEST_F(TestTagManagerDBus, Update_WithTagsNameWithFilesOption_ShouldChangeTagNamesWithFiles)
{
    QVariantMap nameData;
    nameData["oldTag"] = "newTag";
    
    stub.set_lamda(&TagDbHandler::changeTagNamesWithFiles, [&nameData](TagDbHandler *, const QVariantMap &data) {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(data, nameData);
        return true;
    });
    
    bool result = manager->Update(static_cast<int>(UpdateOpts::kTagsNameWithFiles), nameData);
    
    EXPECT_TRUE(result);
}

// Test Update method with kFilesPaths option
TEST_F(TestTagManagerDBus, Update_WithFilesPathsOption_ShouldChangeFilePaths)
{
    QVariantMap pathData;
    pathData["/old/path"] = "/new/path";
    
    stub.set_lamda(&TagDbHandler::changeFilePaths, [&pathData](TagDbHandler *, const QVariantMap &data) {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(data, pathData);
        return true;
    });
    
    bool result = manager->Update(static_cast<int>(UpdateOpts::kFilesPaths), pathData);
    
    EXPECT_TRUE(result);
}

// Test Update method with invalid option
TEST_F(TestTagManagerDBus, Update_WithInvalidOption_ShouldReturnFalse)
{
    QVariantMap data;
    data["test"] = "value";
    
    bool result = manager->Update(999, data); // Invalid option
    
    EXPECT_FALSE(result);
}

// Test signal connections during initialization
TEST_F(TestTagManagerDBus, InitConnect_ShouldConnectAllSignals)
{
    // Create signal spies for all signals
    QSignalSpy newTagsAddedSpy(manager, &TagManagerDBus::NewTagsAdded);
    QSignalSpy tagsDeletedSpy(manager, &TagManagerDBus::TagsDeleted);
    QSignalSpy tagsColorChangedSpy(manager, &TagManagerDBus::TagsColorChanged);
    QSignalSpy tagsNameChangedSpy(manager, &TagManagerDBus::TagsNameChanged);
    QSignalSpy filesTaggedSpy(manager, &TagManagerDBus::FilesTagged);
    QSignalSpy filesUntaggedSpy(manager, &TagManagerDBus::FilesUntagged);
    
    // Simulate signals from TagDbHandler
    QVariantMap testData;
    testData["test"] = "value";
    QStringList testList = {"test"};
    
    // Emit signals from TagDbHandler instance to verify connections
    emit TagDbHandler::instance()->newTagsAdded(testData);
    emit TagDbHandler::instance()->tagsDeleted(testList);
    emit TagDbHandler::instance()->tagsColorChanged(testData);
    emit TagDbHandler::instance()->tagsNameChanged(testData);
    emit TagDbHandler::instance()->filesWereTagged(testData);
    emit TagDbHandler::instance()->filesUntagged(testData);
    
    // Verify all signals were properly forwarded
    EXPECT_EQ(newTagsAddedSpy.count(), 1);
    EXPECT_EQ(tagsDeletedSpy.count(), 1);
    EXPECT_EQ(tagsColorChangedSpy.count(), 1);
    EXPECT_EQ(tagsNameChangedSpy.count(), 1);
    EXPECT_EQ(filesTaggedSpy.count(), 1);
    EXPECT_EQ(filesUntaggedSpy.count(), 1);
    
    // Verify signal parameters
    EXPECT_EQ(newTagsAddedSpy.at(0).at(0).toMap(), testData);
    EXPECT_EQ(tagsDeletedSpy.at(0).at(0).toStringList(), testList);
    EXPECT_EQ(tagsColorChangedSpy.at(0).at(0).toMap(), testData);
    EXPECT_EQ(tagsNameChangedSpy.at(0).at(0).toMap(), testData);
    EXPECT_EQ(filesTaggedSpy.at(0).at(0).toMap(), testData);
    EXPECT_EQ(filesUntaggedSpy.at(0).at(0).toMap(), testData);
} 