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

class TagDbHandlerReal2 : public testing::Test
{
public:
    void SetUp() override
    {
        tempDir.reset(new QTemporaryDir);
        ASSERT_TRUE(tempDir->isValid());

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
    std::unique_ptr<QTemporaryDir> tempDir;
};

TEST_F(TagDbHandlerReal2, AddTagProperty_InvalidTagName_ReturnsFalse)
{
    QVariantMap data;
    data[""] = "#ff0000";

    EXPECT_FALSE(handler->addTagProperty(data));
    // insertTagProperty() failure only logs; lastErr is not set on this path.
    EXPECT_TRUE(handler->lastError().isEmpty());
}

TEST_F(TagDbHandlerReal2, AddTagProperty_NullColor_ReturnsFalse)
{
    QVariantMap data;
    data["Real2NullColor"] = QVariant();

    EXPECT_FALSE(handler->addTagProperty(data));
    // insertTagProperty() failure only logs; lastErr is not set on this path.
    EXPECT_TRUE(handler->lastError().isEmpty());
}

TEST_F(TagDbHandlerReal2, AddTagsForFiles_DeduplicatesExistingTags)
{
    handler->addTagProperty({ { "Real2DupTag", "#ff0000" } });

    QVariantMap first;
    first["/tmp/real2/file.txt"] = QStringList { "Real2DupTag" };
    EXPECT_TRUE(handler->addTagsForFiles(first));

    QVariantMap second;
    second["/tmp/real2/file.txt"] = QStringList { "Real2DupTag", "Real2DupTag" };
    EXPECT_TRUE(handler->addTagsForFiles(second));

    auto result = handler->getTagsByUrls({ "/tmp/real2/file.txt" });
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result["/tmp/real2/file.txt"].toStringList().size(), 1);
}

TEST_F(TagDbHandlerReal2, RemoveTagsOfFiles_RemovesOnlySpecifiedTags)
{
    handler->addTagProperty({ { "Real2KeepTag", "#00ff00" }, { "Real2RemoveTag", "#ff0000" } });
    handler->addTagsForFiles({
        { "/tmp/real2/multi.txt", QStringList { "Real2KeepTag", "Real2RemoveTag" } }
    });

    QVariantMap removeData;
    removeData["/tmp/real2/multi.txt"] = QStringList { "Real2RemoveTag" };
    EXPECT_TRUE(handler->removeTagsOfFiles(removeData));

    auto tags = handler->getTagsByUrls({ "/tmp/real2/multi.txt" });
    EXPECT_EQ(tags["/tmp/real2/multi.txt"].toStringList(), QStringList { "Real2KeepTag" });
}

TEST_F(TagDbHandlerReal2, DeleteTags_RemovesTagPropertyAndFileTagInfo)
{
    handler->addTagProperty({ { "Real2DeleteTag", "#ff0000" } });
    handler->addTagsForFiles({ { "/tmp/real2/delete.txt", QStringList { "Real2DeleteTag" } } });

    EXPECT_TRUE(handler->deleteTags({ "Real2DeleteTag" }));

    EXPECT_TRUE(handler->getAllTags().isEmpty());
    EXPECT_TRUE(handler->getTagsByUrls({ "/tmp/real2/delete.txt" }).isEmpty());
}

TEST_F(TagDbHandlerReal2, ChangeTagNamesWithFiles_EmptyNewName_ReturnsFalse)
{
    handler->addTagProperty({ { "Real2OldName", "#ff0000" } });

    QVariantMap data;
    data["Real2OldName"] = "";

    EXPECT_FALSE(handler->changeTagNamesWithFiles(data));
    // This failure path only logs; lastErr is not set by the product code.
    EXPECT_TRUE(handler->lastError().isEmpty());
}

TEST_F(TagDbHandlerReal2, ChangeFilePaths_EmptyPath_ReturnsFalse)
{
    QVariantMap data;
    data["/tmp/real2/old.txt"] = "";

    EXPECT_FALSE(handler->changeFilePaths(data));
    // This failure path only logs; lastErr is not set by the product code.
    EXPECT_TRUE(handler->lastError().isEmpty());
}

TEST_F(TagDbHandlerReal2, GetTagsColor_NonExistentTags_ReturnsEmpty)
{
    auto colors = handler->getTagsColor({ "Real2Missing1", "Real2Missing2" });
    EXPECT_TRUE(colors.isEmpty());
    EXPECT_TRUE(handler->lastError().isEmpty());
}

TEST_F(TagDbHandlerReal2, GetTagsByUrls_FileWithoutTags_ReturnsEmpty)
{
    auto tags = handler->getTagsByUrls({ "/tmp/real2/notagged.txt" });
    EXPECT_TRUE(tags.isEmpty());
    EXPECT_TRUE(handler->lastError().isEmpty());
}

TEST_F(TagDbHandlerReal2, GetSameTagsOfDiffUrls_SingleUrl_ReturnsAllTags)
{
    handler->addTagProperty({ { "Real2CommonA", "#ff0000" }, { "Real2CommonB", "#00ff00" } });
    handler->addTagsForFiles({
        { "/tmp/real2/single.txt", QStringList { "Real2CommonA", "Real2CommonB" } }
    });

    QVariant result = handler->getSameTagsOfDiffUrls({ "/tmp/real2/single.txt" });
    QStringList common = result.toStringList();
    EXPECT_EQ(common.size(), 2);
}

TEST_F(TagDbHandlerReal2, GetFilesByTag_NoFiles_ReturnsEmpty)
{
    handler->addTagProperty({ { "Real2EmptyTag", "#ff0000" } });

    // getFilesByTag() always inserts an entry per requested tag; a tag
    // without files yields an empty file list, not an empty map.
    auto files = handler->getFilesByTag({ "Real2EmptyTag" });
    EXPECT_EQ(files.size(), 1);
    EXPECT_TRUE(files["Real2EmptyTag"].toStringList().isEmpty());
    EXPECT_TRUE(handler->lastError().isEmpty());
}

TEST_F(TagDbHandlerReal2, SaveTrashFileTags_InvalidParams_ReturnsFalse)
{
    EXPECT_FALSE(handler->saveTrashFileTags("", 1, { "tag" }));
    EXPECT_FALSE(handler->saveTrashFileTags("/tmp/real2/trash", 0, { "tag" }));
    EXPECT_FALSE(handler->saveTrashFileTags("/tmp/real2/trash", 1, {}));
}

TEST_F(TagDbHandlerReal2, GetTrashFileTags_InvalidParams_ReturnsEmpty)
{
    EXPECT_TRUE(handler->getTrashFileTags("", 1).isEmpty());
    EXPECT_TRUE(handler->getTrashFileTags("/tmp/real2/trash", 0).isEmpty());
}

TEST_F(TagDbHandlerReal2, GetTrashFileTags_QueryParams_Invalid_ReturnsEmpty)
{
    EXPECT_TRUE(handler->getTrashFileTags({}).isEmpty());
    EXPECT_TRUE(handler->getTrashFileTags({ "originalPath:/tmp/real2/trash" }).isEmpty());
}

TEST_F(TagDbHandlerReal2, RemoveTrashFileTags_InvalidParams_ReturnsFalse)
{
    EXPECT_FALSE(handler->removeTrashFileTags("", 1));
    EXPECT_FALSE(handler->removeTrashFileTags("/tmp/real2/trash", 0));
}

TEST_F(TagDbHandlerReal2, HasTrashFileTags_InvalidParams_ReturnsFalse)
{
    EXPECT_FALSE(handler->hasTrashFileTags("", 1));
    EXPECT_FALSE(handler->hasTrashFileTags("/tmp/real2/trash", 0));
}

TEST_F(TagDbHandlerReal2, ClearAllTrashTags_WhenEmpty_ReturnsTrue)
{
    EXPECT_TRUE(handler->clearAllTrashTags());
    EXPECT_TRUE(handler->getAllTrashFileTags().isEmpty());
}

TEST_F(TagDbHandlerReal2, GetAllFileWithTags_WhenEmpty_ReturnsEmpty)
{
    EXPECT_TRUE(handler->getAllFileWithTags().isEmpty());
}
