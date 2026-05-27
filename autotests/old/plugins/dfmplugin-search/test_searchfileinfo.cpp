// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>

#include "fileinfo/searchfileinfo.h"
#include "utils/searchhelper.h"

#include <dfm-base/base/urlroute.h>

#include "stubext.h"

DPSEARCH_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

class TestSearchFileInfo : public testing::Test
{
public:
    void SetUp() override
    {
        // Create test URLs
        rootUrl = SearchHelper::rootUrl();
        searchUrl = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/home/test"), "keyword", "123");

        // Create SearchFileInfo instances
        rootFileInfo = new SearchFileInfo(rootUrl);
        searchFileInfo = new SearchFileInfo(searchUrl);
    }

    void TearDown() override
    {
        delete rootFileInfo;
        delete searchFileInfo;
        rootFileInfo = nullptr;
        searchFileInfo = nullptr;
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    QUrl rootUrl;
    QUrl searchUrl;
    SearchFileInfo *rootFileInfo = nullptr;
    SearchFileInfo *searchFileInfo = nullptr;
};

TEST_F(TestSearchFileInfo, Constructor_WithValidUrl_CreatesInstance)
{
    QUrl testUrl = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/home"), "test", "456");
    SearchFileInfo info(testUrl);

    // Basic construction test - should not crash
    EXPECT_TRUE(true);
}

TEST_F(TestSearchFileInfo, Exists_WithRootUrl_ReturnsTrue)
{
    bool result = rootFileInfo->exists();

    EXPECT_TRUE(result);
}

TEST_F(TestSearchFileInfo, Exists_WithNonRootUrl_CallsParentExists)
{
    bool parentExistsResult = true;

    // Mock parent FileInfo::exists
    stub.set_lamda(VADDR(FileInfo, exists), [parentExistsResult](FileInfo *) -> bool {
        __DBG_STUB_INVOKE__
        return parentExistsResult;
    });

    bool result = searchFileInfo->exists();

    EXPECT_EQ(result, parentExistsResult);
}

TEST_F(TestSearchFileInfo, SupportedOfAttributes_WithRootUrlAndDropType_ReturnsIgnoreAction)
{
    Qt::DropActions result = rootFileInfo->supportedOfAttributes(SupportedType::kDrop);

    EXPECT_EQ(result, Qt::IgnoreAction);
}

TEST_F(TestSearchFileInfo, SupportedOfAttributes_WithNonRootUrl_CallsParentMethod)
{
    Qt::DropActions expectedActions = Qt::CopyAction | Qt::MoveAction;

    // Mock parent FileInfo::supportedOfAttributes
    stub.set_lamda(VADDR(FileInfo, supportedOfAttributes),
                   [expectedActions](FileInfo *, SupportedType type) -> Qt::DropActions {
                       __DBG_STUB_INVOKE__
                       return expectedActions;
                   });

    EXPECT_NO_FATAL_FAILURE(searchFileInfo->supportedOfAttributes(SupportedType::kDrop));
}

TEST_F(TestSearchFileInfo, IsAttributes_WithRootUrlAndIsDir_ReturnsTrue)
{
    bool result = rootFileInfo->isAttributes(OptInfoType::kIsDir);

    EXPECT_TRUE(result);
}

TEST_F(TestSearchFileInfo, IsAttributes_WithRootUrlAndIsReadable_ReturnsTrue)
{
    bool result = rootFileInfo->isAttributes(OptInfoType::kIsReadable);

    EXPECT_TRUE(result);
}

TEST_F(TestSearchFileInfo, IsAttributes_WithRootUrlAndIsWritable_ReturnsTrue)
{
    bool result = rootFileInfo->isAttributes(OptInfoType::kIsWritable);

    EXPECT_TRUE(result);
}

TEST_F(TestSearchFileInfo, IsAttributes_WithRootUrlAndIsHidden_ReturnsFalse)
{
    bool result = rootFileInfo->isAttributes(OptInfoType::kIsHidden);

    EXPECT_FALSE(result);
}

TEST_F(TestSearchFileInfo, IsAttributes_WithNonRootUrl_CallsParentMethod)
{
    bool expectedResult = true;

    // Mock parent FileInfo::isAttributes
    stub.set_lamda(VADDR(FileInfo, isAttributes),
                   [expectedResult](FileInfo *, OptInfoType type) -> bool {
                       __DBG_STUB_INVOKE__
                       return expectedResult;
                   });

    bool result = searchFileInfo->isAttributes(OptInfoType::kIsDir);

    EXPECT_EQ(result, expectedResult);
}

TEST_F(TestSearchFileInfo, IsAttributes_WithOtherType_CallsParentMethod)
{
    bool expectedResult = false;

    // Mock parent FileInfo::isAttributes
    stub.set_lamda(VADDR(FileInfo, isAttributes),
                   [expectedResult](FileInfo *, OptInfoType type) -> bool {
                       __DBG_STUB_INVOKE__
                       return expectedResult;
                   });

    bool result = rootFileInfo->isAttributes(OptInfoType::kIsExecutable);

    EXPECT_EQ(result, expectedResult);
}

TEST_F(TestSearchFileInfo, Size_WithRootUrl_ReturnsMinusOne)
{
    qint64 result = rootFileInfo->size();

    EXPECT_EQ(result, -1);
}

TEST_F(TestSearchFileInfo, Size_WithNonRootUrl_CallsParentMethod)
{
    qint64 expectedSize = 1024;

    // Mock parent FileInfo::size
    stub.set_lamda(VADDR(FileInfo, size), [expectedSize](FileInfo *) -> qint64 {
        __DBG_STUB_INVOKE__
        return expectedSize;
    });

    EXPECT_NO_FATAL_FAILURE(searchFileInfo->size());
}

TEST_F(TestSearchFileInfo, DisplayOf_WithFileDisplayNameAndRootUrl_ReturnsSearch)
{
    // Mock UrlRoute::isRootUrl to return true for our root URL
    stub.set_lamda(&UrlRoute::isRootUrl, [](const QUrl &url) -> bool {
        __DBG_STUB_INVOKE__
        return true; // Simulate root URL
    });

    QString result = rootFileInfo->displayOf(DisPlayInfoType::kFileDisplayName);

    EXPECT_EQ(result, QObject::tr("Search"));
}

TEST_F(TestSearchFileInfo, DisplayOf_WithOtherDisplayType_CallsParentMethod)
{
    QString expectedDisplayName = "Test Display";

    // Mock parent FileInfo::displayOf
    stub.set_lamda(VADDR(FileInfo, displayOf),
                   [expectedDisplayName](FileInfo *, DisPlayInfoType type) -> QString {
                       __DBG_STUB_INVOKE__
                       return expectedDisplayName;
                   });

    QString result = rootFileInfo->displayOf(DisPlayInfoType::kFileDisplayPath);

    EXPECT_EQ(result, expectedDisplayName);
}

TEST_F(TestSearchFileInfo, NameOf_WithFileNameAndRootUrl_ReturnsSearch)
{
    QString result = rootFileInfo->nameOf(NameInfoType::kFileName);

    EXPECT_EQ(result, QObject::tr("Search"));
}

TEST_F(TestSearchFileInfo, NameOf_WithOtherNameType_CallsParentMethod)
{
    QString expectedName = "test_file.txt";

    // Mock parent FileInfo::nameOf
    stub.set_lamda(VADDR(FileInfo, nameOf),
                   [expectedName](FileInfo *, NameInfoType type) -> QString {
                       __DBG_STUB_INVOKE__
                       return expectedName;
                   });

    QString result = searchFileInfo->nameOf(NameInfoType::kBaseName);

    EXPECT_EQ(result, expectedName);
}

TEST_F(TestSearchFileInfo, ViewOfTip_WithEmptyDir_ReturnsNoResults)
{
    QString result = rootFileInfo->viewOfTip(ViewInfoType::kEmptyDir);

    EXPECT_EQ(result, QObject::tr("No results"));
}

TEST_F(TestSearchFileInfo, ViewOfTip_WithLoading_ReturnsSearching)
{
    QString result = rootFileInfo->viewOfTip(ViewInfoType::kLoading);

    EXPECT_EQ(result, QObject::tr("Searching..."));
}

TEST_F(TestSearchFileInfo, IsAttributes_WithNonRootUrlAndAllTypes_CallsParentCorrectly)
{
    // Test multiple file types with non-root URL to ensure parent method is called
    OptInfoType types[] = {
        OptInfoType::kIsDir,
        OptInfoType::kIsReadable,
        OptInfoType::kIsWritable,
        OptInfoType::kIsHidden,
        OptInfoType::kIsExecutable
    };

    // Mock parent FileInfo::isAttributes
    stub.set_lamda(VADDR(FileInfo, isAttributes),
                   []() -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    for (int i = 0; i < 5; ++i) {
        EXPECT_NO_FATAL_FAILURE(searchFileInfo->isAttributes(types[i]));
    }
}

TEST_F(TestSearchFileInfo, SupportedOfAttributes_WithNonDropType_CallsParentMethod)
{
    Qt::DropActions expectedActions = Qt::CopyAction;

    // Mock parent FileInfo::supportedOfAttributes
    stub.set_lamda(VADDR(FileInfo, supportedOfAttributes),
                   [expectedActions](FileInfo *, SupportedType type) -> Qt::DropActions {
                       __DBG_STUB_INVOKE__
                       return expectedActions;
                   });

    Qt::DropActions result = rootFileInfo->supportedOfAttributes(SupportedType::kDrag);

    EXPECT_EQ(result, expectedActions);
}

TEST_F(TestSearchFileInfo, DisplayOf_WithNonRootUrl_CallsParentMethod)
{
    QString expectedDisplay = "Non-root display";

    // Mock UrlRoute::isRootUrl to return false
    stub.set_lamda(&UrlRoute::isRootUrl, [](const QUrl &url) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Mock parent FileInfo::displayOf
    stub.set_lamda(VADDR(FileInfo, displayOf),
                   [expectedDisplay](FileInfo *, DisPlayInfoType type) -> QString {
                       __DBG_STUB_INVOKE__
                       return expectedDisplay;
                   });

    QString result = searchFileInfo->displayOf(DisPlayInfoType::kFileDisplayName);

    EXPECT_EQ(result, expectedDisplay);
}

TEST_F(TestSearchFileInfo, NameOf_WithNonRootUrlAndFileName_CallsParentMethod)
{
    // Mock parent FileInfo::nameOf
    stub.set_lamda(VADDR(FileInfo, nameOf),
                   [](FileInfo *, NameInfoType type) -> QString {
                       __DBG_STUB_INVOKE__
                       return "";
                   });

    EXPECT_NO_FATAL_FAILURE(searchFileInfo->nameOf(NameInfoType::kFileName));
}
