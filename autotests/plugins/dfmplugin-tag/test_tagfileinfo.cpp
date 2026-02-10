// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-tag/files/tagfileinfo.h"
#include "plugins/common/dfmplugin-tag/files/tagfileinfo_p.h"
#include "plugins/common/dfmplugin-tag/utils/tagmanager.h"

#include <dfm-base/base/schemefactory.h>

#include <gtest/gtest.h>

#include <QUrl>
#include <QColor>
#include <QIcon>
#include <QFile>

using namespace dfmplugin_tag;
DFMBASE_USE_NAMESPACE

class UT_TagFileInfo : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        testUrl.setScheme("tag");
        testUrl.setPath("/TestTag");
        fileInfo = new TagFileInfo(testUrl);
    }

    virtual void TearDown() override
    {
        delete fileInfo;
        fileInfo = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    TagFileInfo *fileInfo = nullptr;
    QUrl testUrl;
};

TEST_F(UT_TagFileInfo, constructor)
{
    // Test constructor
    EXPECT_NE(fileInfo, nullptr);
}

TEST_F(UT_TagFileInfo, exists_RootUrl)
{
    // Test exists for root URL
    QUrl rootUrl;
    rootUrl.setScheme("tag");

    TagFileInfo rootInfo(rootUrl);

    stub.set_lamda(VADDR(FileInfo, urlOf), [](const FileInfo*, UrlInfoType) -> QUrl {
        __DBG_STUB_INVOKE__
        QUrl url;
        url.setScheme("tag");
        return url;
    });

    bool result = rootInfo.exists();

    EXPECT_TRUE(result);
}

TEST_F(UT_TagFileInfo, exists_TagExists)
{
    // Test exists when tag exists in system
    stub.set_lamda(&TagManager::instance, []() -> TagManager* {
        __DBG_STUB_INVOKE__
        static TagManager manager;
        return &manager;
    });

    stub.set_lamda(&TagManager::getAllTags, [](TagManager*) -> TagManager::TagColorMap {
        __DBG_STUB_INVOKE__
        TagManager::TagColorMap map;
        map["TestTag"] = QColor("#ffa503");
        return map;
    });

    stub.set_lamda(&TagFileInfo::tagName, [](const TagFileInfo*) -> QString {
        __DBG_STUB_INVOKE__
        return "TestTag";
    });

    bool result = fileInfo->exists();

    EXPECT_TRUE(result);
}

TEST_F(UT_TagFileInfo, exists_TagNotExists)
{
    // Test exists when tag doesn't exist
    stub.set_lamda(&TagManager::instance, []() -> TagManager* {
        __DBG_STUB_INVOKE__
        static TagManager manager;
        return &manager;
    });

    stub.set_lamda(&TagManager::getAllTags, [](TagManager*) -> TagManager::TagColorMap {
        __DBG_STUB_INVOKE__
        return TagManager::TagColorMap(); // Empty map
    });

    stub.set_lamda(&TagFileInfo::tagName, [](const TagFileInfo*) -> QString {
        __DBG_STUB_INVOKE__
        return "NonExistentTag";
    });

    bool result = fileInfo->exists();

    EXPECT_FALSE(result);
}

TEST_F(UT_TagFileInfo, permissions)
{
    // Test permissions
    QFile::Permissions perms = fileInfo->permissions();

    // Should have read and write permissions
    EXPECT_TRUE(perms & QFile::ReadOwner);
    EXPECT_TRUE(perms & QFile::WriteOwner);
    EXPECT_TRUE(perms & QFile::ReadUser);
    EXPECT_TRUE(perms & QFile::WriteUser);
}

TEST_F(UT_TagFileInfo, isAttributes_Readable)
{
    // Test is readable attribute
    bool result = fileInfo->isAttributes(OptInfoType::kIsReadable);

    EXPECT_TRUE(result);
}

TEST_F(UT_TagFileInfo, isAttributes_Writable)
{
    // Test is writable attribute
    bool result = fileInfo->isAttributes(OptInfoType::kIsWritable);

    EXPECT_TRUE(result);
}

TEST_F(UT_TagFileInfo, isAttributes_Executable)
{
    // Test is executable attribute
    bool result = fileInfo->isAttributes(OptInfoType::kIsExecutable);

    EXPECT_TRUE(result);
}

TEST_F(UT_TagFileInfo, isAttributes_Default)
{
    // Test other attributes fallback to parent
    stub.set_lamda(VADDR(ProxyFileInfo, isAttributes), [](const ProxyFileInfo*, OptInfoType) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = fileInfo->isAttributes(OptInfoType::kIsHidden);

    EXPECT_FALSE(result);
}

TEST_F(UT_TagFileInfo, canAttributes_CanDrop)
{
    // Test can drop attribute
    bool result = fileInfo->canAttributes(CanableInfoType::kCanDrop);

    EXPECT_TRUE(result);
}

TEST_F(UT_TagFileInfo, canAttributes_Default)
{
    // Test other can attributes fallback to parent
    stub.set_lamda(VADDR(ProxyFileInfo, canAttributes), [](const ProxyFileInfo*, CanableInfoType) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = fileInfo->canAttributes(CanableInfoType::kCanDelete);

    EXPECT_FALSE(result);
}

TEST_F(UT_TagFileInfo, nameOf_FileName)
{
    // Test getting file name
    stub.set_lamda(&TagFileInfoPrivate::fileName, [](const TagFileInfoPrivate*) -> QString {
        __DBG_STUB_INVOKE__
        return "TestTag";
    });

    QString name = fileInfo->nameOf(NameInfoType::kFileName);

    EXPECT_EQ(name, "TestTag");
}

TEST_F(UT_TagFileInfo, nameOf_FileCopyName)
{
    // Test getting file copy name
    stub.set_lamda(&TagFileInfoPrivate::fileName, [](const TagFileInfoPrivate*) -> QString {
        __DBG_STUB_INVOKE__
        return "TestTag";
    });

    QString name = fileInfo->nameOf(NameInfoType::kFileCopyName);

    EXPECT_EQ(name, "TestTag");
}

TEST_F(UT_TagFileInfo, nameOf_Default)
{
    // Test other name types fallback to parent
    stub.set_lamda(VADDR(ProxyFileInfo, nameOf), [](const ProxyFileInfo*, NameInfoType) -> QString {
        __DBG_STUB_INVOKE__
        return "DefaultName";
    });

    QString name = fileInfo->nameOf(NameInfoType::kCompleteBaseName);

    EXPECT_EQ(name, "DefaultName");
}

TEST_F(UT_TagFileInfo, displayOf_FileDisplayName)
{
    // Test getting file display name
    stub.set_lamda(&TagFileInfoPrivate::fileName, [](const TagFileInfoPrivate*) -> QString {
        __DBG_STUB_INVOKE__
        return "TestTag";
    });

    QString display = fileInfo->displayOf(DisPlayInfoType::kFileDisplayName);

    EXPECT_EQ(display, "TestTag");
}

TEST_F(UT_TagFileInfo, displayOf_Default)
{
    // Test other display types fallback to parent
    stub.set_lamda(VADDR(ProxyFileInfo, displayOf), [](const ProxyFileInfo*, DisPlayInfoType) -> QString {
        __DBG_STUB_INVOKE__
        return "DefaultDisplay";
    });

    QString display = fileInfo->displayOf(DisPlayInfoType::kSizeDisplayName);

    EXPECT_EQ(display, "DefaultDisplay");
}

TEST_F(UT_TagFileInfo, fileType)
{
    // Test file type
    FileInfo::FileType type = fileInfo->fileType();

    EXPECT_EQ(type, FileInfo::FileType::kDirectory);
}

TEST_F(UT_TagFileInfo, fileIcon)
{
    // Test file icon
    QIcon icon = fileInfo->fileIcon();

    EXPECT_FALSE(icon.isNull());
}

TEST_F(UT_TagFileInfo, tagName)
{
    // Test getting tag name
    stub.set_lamda(&TagFileInfoPrivate::fileName, [](const TagFileInfoPrivate*) -> QString {
        __DBG_STUB_INVOKE__
        return "TestTag";
    });

    QString name = fileInfo->tagName();

    EXPECT_EQ(name, "TestTag");
}

TEST_F(UT_TagFileInfo, multipleInstances)
{
    // Test creating multiple instances
    QUrl url1;
    url1.setScheme("tag");
    url1.setPath("/Tag1");

    QUrl url2;
    url2.setScheme("tag");
    url2.setPath("/Tag2");

    TagFileInfo info1(url1);
    TagFileInfo info2(url2);

    stub.set_lamda(&TagFileInfoPrivate::fileName, [](const TagFileInfoPrivate* d) -> QString {
        __DBG_STUB_INVOKE__
        // Return different names based on the object
        return "Tag1"; // Simplified for testing
    });

    EXPECT_NO_THROW(info1.tagName());
    EXPECT_NO_THROW(info2.tagName());
}
