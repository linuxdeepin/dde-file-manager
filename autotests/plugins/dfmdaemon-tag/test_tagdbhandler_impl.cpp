// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QStringList>
#include <QVariantMap>

#include "stubext.h"

#include "tagdbhandler.h"
#include "beans/filetaginfo.h"
#include "beans/tagproperty.h"
#include "beans/trashfiletaginfo.h"

#include <dfm-base/base/standardpaths.h>

DAEMONPTAG_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

class TagDBHandlerImpl : public testing::Test
{
public:
    void SetUp() override
    {
        if (!tempDir) {
            tempDir.reset(new QTemporaryDir);
            ASSERT_TRUE(tempDir->isValid());
        }

        stub.set_lamda(static_cast<QString (*)(StandardPaths::StandardLocation)>(&StandardPaths::location),
                       [this](StandardPaths::StandardLocation type) -> QString {
            __DBG_STUB_INVOKE__
            if (type == StandardPaths::kApplicationConfigPath)
                return tempDir->path();
            return QString("/tmp/test");
        });

        handler = TagDbHandler::instance();
        cleanDatabase();
    }

    void TearDown() override
    {
        cleanDatabase();
        stub.clear();
    }

protected:
    void cleanDatabase()
    {
        if (!handler)
            return;

        const QStringList tags = handler->getAllTags().keys();
        if (!tags.isEmpty())
            handler->deleteTags(tags);

        handler->clearAllTrashTags();

        const QStringList files = handler->getAllFileWithTags().keys();
        if (!files.isEmpty())
            handler->deleteFiles(files);
    }

    TagDbHandler *handler = nullptr;
    stub_ext::StubExt stub;
    static std::unique_ptr<QTemporaryDir> tempDir;
};

std::unique_ptr<QTemporaryDir> TagDBHandlerImpl::tempDir;

TEST_F(TagDBHandlerImpl, Instance_ReturnsSameSingleton)
{
    TagDbHandler *first = TagDbHandler::instance();
    TagDbHandler *second = TagDbHandler::instance();
    EXPECT_EQ(first, second);
    EXPECT_NE(first, nullptr);
}

TEST_F(TagDBHandlerImpl, GetAllTags_EmptyDatabase_ReturnsEmptyMap)
{
    QVariantMap tags = handler->getAllTags();
    EXPECT_TRUE(tags.isEmpty());
    EXPECT_TRUE(handler->lastError().isEmpty());
}

TEST_F(TagDBHandlerImpl, AddTagProperty_ValidData_InsertsTagsAndEmitsSignal)
{
    QVariantMap data;
    data["redTag"] = "#ff0000";
    data["blueTag"] = "#0000ff";

    QSignalSpy spy(handler, &TagDbHandler::newTagsAdded);
    EXPECT_TRUE(handler->addTagProperty(data));

    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toMap(), data);

    QVariantMap tags = handler->getAllTags();
    EXPECT_EQ(tags.size(), 2);
    EXPECT_EQ(tags["redTag"].toString(), "#ff0000");
    EXPECT_EQ(tags["blueTag"].toString(), "#0000ff");
}

TEST_F(TagDBHandlerImpl, AddTagProperty_EmptyData_ReturnsFalseAndSetsError)
{
    QSignalSpy spy(handler, &TagDbHandler::newTagsAdded);
    EXPECT_FALSE(handler->addTagProperty({}));
    EXPECT_EQ(spy.count(), 0);
    EXPECT_FALSE(handler->lastError().isEmpty());
}

TEST_F(TagDBHandlerImpl, AddTagProperty_DuplicateTag_SkipsInsertButStillEmitsSignal)
{
    QVariantMap data;
    data["uniqueTag"] = "#123456";
    EXPECT_TRUE(handler->addTagProperty(data));

    QSignalSpy spy(handler, &TagDbHandler::newTagsAdded);
    EXPECT_TRUE(handler->addTagProperty(data));
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toMap(), data);

    EXPECT_EQ(handler->getAllTags().size(), 1);
}

TEST_F(TagDBHandlerImpl, GetTagsColor_WithExistingTags_ReturnsMatchingColors)
{
    QVariantMap data;
    data["colorTag1"] = "#111111";
    data["colorTag2"] = "#222222";
    data["colorTag3"] = "#333333";
    handler->addTagProperty(data);

    QVariantMap colors = handler->getTagsColor({ "colorTag1", "colorTag3", "missingTag" });
    EXPECT_EQ(colors.size(), 2);
    EXPECT_EQ(colors["colorTag1"].toString(), "#111111");
    EXPECT_EQ(colors["colorTag3"].toString(), "#333333");
    EXPECT_FALSE(colors.contains("missingTag"));
}

TEST_F(TagDBHandlerImpl, GetTagsColor_EmptyInput_ReturnsEmptyAndSetsError)
{
    QVariantMap colors = handler->getTagsColor({});
    EXPECT_TRUE(colors.isEmpty());
    EXPECT_FALSE(handler->lastError().isEmpty());
}

TEST_F(TagDBHandlerImpl, AddTagsForFiles_ValidData_TagsFilesAndEmitsSignal)
{
    handler->addTagProperty({ { "tagA", "#aaaaaa" }, { "tagB", "#bbbbbb" } });

    QVariantMap fileData;
    fileData["/test/file1"] = QStringList { "tagA", "tagB" };
    fileData["/test/file2"] = QStringList { "tagA" };

    QSignalSpy spy(handler, &TagDbHandler::filesWereTagged);
    EXPECT_TRUE(handler->addTagsForFiles(fileData));
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toMap(), fileData);

    QVariantMap result = handler->getTagsByUrls({ "/test/file1", "/test/file2" });
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result["/test/file1"].toStringList().size(), 2);
    EXPECT_EQ(result["/test/file2"].toStringList().size(), 1);
}

TEST_F(TagDBHandlerImpl, AddTagsForFiles_EmptyInput_ReturnsFalseAndSetsError)
{
    EXPECT_FALSE(handler->addTagsForFiles({}));
    EXPECT_FALSE(handler->lastError().isEmpty());
}

TEST_F(TagDBHandlerImpl, GetTagsByUrls_WithExistingFiles_ReturnsTags)
{
    handler->addTagProperty({ { "urlTag", "#cccccc" } });
    handler->addTagsForFiles({ { "/test/urlFile", QStringList { "urlTag" } } });

    QVariantMap result = handler->getTagsByUrls({ "/test/urlFile", "/test/missingFile" });
    EXPECT_EQ(result.size(), 1);
    EXPECT_TRUE(result.contains("/test/urlFile"));
}

TEST_F(TagDBHandlerImpl, GetTagsByUrls_EmptyInput_ReturnsEmptyAndSetsError)
{
    QVariantMap result = handler->getTagsByUrls({});
    EXPECT_TRUE(result.isEmpty());
    EXPECT_FALSE(handler->lastError().isEmpty());
}

TEST_F(TagDBHandlerImpl, RemoveTagsOfFiles_ValidData_RemovesTagsAndEmitsSignal)
{
    handler->addTagProperty({ { "removeTag", "#dddddd" } });
    handler->addTagsForFiles({ { "/test/removeFile", QStringList { "removeTag" } } });
    EXPECT_FALSE(handler->getTagsByUrls({ "/test/removeFile" }).isEmpty());

    QVariantMap removeData;
    removeData["/test/removeFile"] = QStringList { "removeTag" };

    QSignalSpy spy(handler, &TagDbHandler::filesUntagged);
    EXPECT_TRUE(handler->removeTagsOfFiles(removeData));
    EXPECT_EQ(spy.count(), 1);
    EXPECT_TRUE(handler->getTagsByUrls({ "/test/removeFile" }).isEmpty());
}

TEST_F(TagDBHandlerImpl, RemoveTagsOfFiles_EmptyInput_ReturnsFalseAndSetsError)
{
    EXPECT_FALSE(handler->removeTagsOfFiles({}));
    EXPECT_FALSE(handler->lastError().isEmpty());
}

TEST_F(TagDBHandlerImpl, DeleteTags_ExistingTags_DeletesAndEmitsSignal)
{
    handler->addTagProperty({ { "delTag", "#eeeeee" } });
    EXPECT_TRUE(handler->getAllTags().contains("delTag"));

    QSignalSpy spy(handler, &TagDbHandler::tagsDeleted);
    EXPECT_TRUE(handler->deleteTags({ "delTag" }));
    EXPECT_EQ(spy.count(), 1);
    EXPECT_FALSE(handler->getAllTags().contains("delTag"));
}

TEST_F(TagDBHandlerImpl, DeleteTags_EmptyInput_ReturnsFalseAndSetsError)
{
    EXPECT_FALSE(handler->deleteTags({}));
    EXPECT_FALSE(handler->lastError().isEmpty());
}

TEST_F(TagDBHandlerImpl, DeleteFiles_ExistingFiles_RemovesFileTagEntries)
{
    handler->addTagProperty({ { "fileDelTag", "#ffffff" } });
    handler->addTagsForFiles({ { "/test/delFile", QStringList { "fileDelTag" } } });
    EXPECT_FALSE(handler->getAllFileWithTags().isEmpty());

    EXPECT_TRUE(handler->deleteFiles({ "/test/delFile" }));
    EXPECT_TRUE(handler->getTagsByUrls({ "/test/delFile" }).isEmpty());
}

TEST_F(TagDBHandlerImpl, DeleteFiles_EmptyInput_ReturnsFalseAndSetsError)
{
    EXPECT_FALSE(handler->deleteFiles({}));
    EXPECT_FALSE(handler->lastError().isEmpty());
}

TEST_F(TagDBHandlerImpl, ChangeTagColors_ExistingTag_UpdatesColorAndEmitsSignal)
{
    handler->addTagProperty({ { "changeColorTag", "#oldcolor" } });

    QVariantMap colorData;
    colorData["changeColorTag"] = "#newcolor";

    QSignalSpy spy(handler, &TagDbHandler::tagsColorChanged);
    EXPECT_TRUE(handler->changeTagColors(colorData));
    EXPECT_EQ(spy.count(), 1);

    QVariantMap colors = handler->getTagsColor({ "changeColorTag" });
    EXPECT_EQ(colors["changeColorTag"].toString(), "#newcolor");
}

TEST_F(TagDBHandlerImpl, ChangeTagColors_EmptyInput_ReturnsFalseAndSetsError)
{
    EXPECT_FALSE(handler->changeTagColors({}));
    EXPECT_FALSE(handler->lastError().isEmpty());
}

TEST_F(TagDBHandlerImpl, ChangeTagNamesWithFiles_ExistingTag_RenamesAndEmitsSignal)
{
    handler->addTagProperty({ { "oldName", "#abcdef" } });
    handler->addTagsForFiles({ { "/test/nameFile", QStringList { "oldName" } } });

    QVariantMap nameData;
    nameData["oldName"] = "newName";

    QSignalSpy spy(handler, &TagDbHandler::tagsNameChanged);
    EXPECT_TRUE(handler->changeTagNamesWithFiles(nameData));
    EXPECT_EQ(spy.count(), 1);

    QVariantMap tags = handler->getTagsByUrls({ "/test/nameFile" });
    QStringList fileTags = tags["/test/nameFile"].toStringList();
    EXPECT_TRUE(fileTags.contains("newName"));
    EXPECT_FALSE(fileTags.contains("oldName"));
}

TEST_F(TagDBHandlerImpl, ChangeTagNamesWithFiles_EmptyInput_ReturnsFalseAndSetsError)
{
    EXPECT_FALSE(handler->changeTagNamesWithFiles({}));
    EXPECT_FALSE(handler->lastError().isEmpty());
}

TEST_F(TagDBHandlerImpl, ChangeFilePaths_ExistingFile_UpdatesPath)
{
    handler->addTagProperty({ { "pathTag", "#aabbcc" } });
    handler->addTagsForFiles({ { "/test/oldPath", QStringList { "pathTag" } } });

    QVariantMap pathData;
    pathData["/test/oldPath"] = "/test/newPath";

    EXPECT_TRUE(handler->changeFilePaths(pathData));
    EXPECT_TRUE(handler->getTagsByUrls({ "/test/oldPath" }).isEmpty());
    EXPECT_FALSE(handler->getTagsByUrls({ "/test/newPath" }).isEmpty());
}

TEST_F(TagDBHandlerImpl, ChangeFilePaths_EmptyInput_ReturnsFalseAndSetsError)
{
    EXPECT_FALSE(handler->changeFilePaths({}));
    EXPECT_FALSE(handler->lastError().isEmpty());
}

TEST_F(TagDBHandlerImpl, GetSameTagsOfDiffUrls_CommonTags_ReturnsIntersection)
{
    handler->addTagProperty({ { "common", "#000000" }, { "only1", "#111111" }, { "only2", "#222222" } });
    handler->addTagsForFiles({
        { "/test/same1", QStringList { "common", "only1" } },
        { "/test/same2", QStringList { "common", "only2" } }
    });

    QVariant result = handler->getSameTagsOfDiffUrls({ "/test/same1", "/test/same2" });
    QStringList common = result.toStringList();
    EXPECT_EQ(common.size(), 1);
    EXPECT_EQ(common.first(), "common");
}

TEST_F(TagDBHandlerImpl, GetSameTagsOfDiffUrls_EmptyInput_ReturnsEmptyAndSetsError)
{
    QVariant result = handler->getSameTagsOfDiffUrls({});
    EXPECT_TRUE(result.toStringList().isEmpty());
    EXPECT_FALSE(handler->lastError().isEmpty());
}

TEST_F(TagDBHandlerImpl, GetFilesByTag_ExistingTag_ReturnsFiles)
{
    handler->addTagProperty({ { "searchTag", "#999999" } });
    handler->addTagsForFiles({
        { "/test/fileA", QStringList { "searchTag" } },
        { "/test/fileB", QStringList { "searchTag" } }
    });

    QVariantMap result = handler->getFilesByTag({ "searchTag" });
    EXPECT_EQ(result.size(), 1);
    QStringList files = result["searchTag"].toStringList();
    EXPECT_EQ(files.size(), 2);
}

TEST_F(TagDBHandlerImpl, GetFilesByTag_EmptyInput_ReturnsEmptyAndSetsError)
{
    QVariantMap result = handler->getFilesByTag({});
    EXPECT_TRUE(result.isEmpty());
    EXPECT_FALSE(handler->lastError().isEmpty());
}

TEST_F(TagDBHandlerImpl, GetAllFileWithTags_WithData_ReturnsHash)
{
    handler->addTagProperty({ { "allTag", "#888888" } });
    handler->addTagsForFiles({
        { "/test/all1", QStringList { "allTag" } },
        { "/test/all2", QStringList { "allTag" } }
    });

    QVariantHash result = handler->getAllFileWithTags();
    EXPECT_EQ(result.size(), 2);
    EXPECT_TRUE(result.contains("/test/all1"));
    EXPECT_TRUE(result.contains("/test/all2"));
}

TEST_F(TagDBHandlerImpl, SaveAndGetTrashFileTags_ValidData_SavesAndRetrieves)
{
    handler->addTagProperty({ { "trashTag", "#777777" } });

    QSignalSpy spy(handler, &TagDbHandler::trashFileTagsChanged);
    EXPECT_TRUE(handler->saveTrashFileTags("/test/trashFile", 12345, { "trashTag" }));
    EXPECT_EQ(spy.count(), 1);

    QStringList tags = handler->getTrashFileTags("/test/trashFile", 12345);
    EXPECT_EQ(tags.size(), 1);
    EXPECT_EQ(tags.first(), "trashTag");
}

TEST_F(TagDBHandlerImpl, GetTrashFileTags_StringListParams_ReturnsTagsMap)
{
    handler->addTagProperty({ { "trashTag2", "#666666" } });
    handler->saveTrashFileTags("/test/trashFile2", 42, { "trashTag2" });

    QVariantMap result = handler->getTrashFileTags({ "originalPath:/test/trashFile2", "inode:42" });
    EXPECT_EQ(result["tags"].toStringList(), QStringList { "trashTag2" });
}

TEST_F(TagDBHandlerImpl, RemoveTrashFileTags_Existing_RemovesAndEmitsSignal)
{
    handler->addTagProperty({ { "trashTag3", "#555555" } });
    handler->saveTrashFileTags("/test/trashFile3", 7, { "trashTag3" });
    EXPECT_TRUE(handler->hasTrashFileTags("/test/trashFile3", 7));

    QSignalSpy spy(handler, &TagDbHandler::trashFileTagsChanged);
    EXPECT_TRUE(handler->removeTrashFileTags("/test/trashFile3", 7));
    EXPECT_EQ(spy.count(), 1);
    EXPECT_FALSE(handler->hasTrashFileTags("/test/trashFile3", 7));
}

TEST_F(TagDBHandlerImpl, ClearAllTrashTags_WithData_ClearsAll)
{
    handler->addTagProperty({ { "trashTag4", "#444444" } });
    handler->saveTrashFileTags("/test/trashFile4", 8, { "trashTag4" });
    EXPECT_FALSE(handler->getAllTrashFileTags().isEmpty());

    QSignalSpy spy(handler, &TagDbHandler::trashFileTagsChanged);
    EXPECT_TRUE(handler->clearAllTrashTags());
    EXPECT_EQ(spy.count(), 1);
    EXPECT_TRUE(handler->getAllTrashFileTags().isEmpty());
}

TEST_F(TagDBHandlerImpl, HasTrashFileTags_VariousStates_ReturnsCorrectly)
{
    handler->addTagProperty({ { "trashTag5", "#333333" } });
    handler->saveTrashFileTags("/test/trashFile5", 9, { "trashTag5" });

    EXPECT_TRUE(handler->hasTrashFileTags("/test/trashFile5", 9));
    EXPECT_FALSE(handler->hasTrashFileTags("/test/trashFile5", 99));
    EXPECT_FALSE(handler->hasTrashFileTags("", 0));
}

TEST_F(TagDBHandlerImpl, GetAllTrashFileTags_WithData_ReturnsHash)
{
    handler->addTagProperty({ { "trashTag6", "#222222" } });
    handler->saveTrashFileTags("/test/trashFile6", 10, { "trashTag6" });

    QVariantHash result = handler->getAllTrashFileTags();
    EXPECT_EQ(result.size(), 1);

    QString key = QString("%1:%2").arg("/test/trashFile6").arg(10);
    EXPECT_TRUE(result.contains(key));
}
