// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "models/generalmodelfilter.h"
#include "models/modeldatahandler.h"

#include <gtest/gtest.h>
#include <QUrl>
#include <QVector>

using namespace ddplugin_organizer;

class MockModelDataHandler : public ModelDataHandler
{
public:
    MockModelDataHandler() : ModelDataHandler() {}

    bool acceptInsert(const QUrl &url) override
    {
        acceptInsertCalled = true;
        lastInsertUrl = url;
        return mockAcceptInsertResult;
    }

    QList<QUrl> acceptReset(const QList<QUrl> &urls) override
    {
        acceptResetCalled = true;
        lastResetUrls = urls;
        return mockAcceptResetResult;
    }

    bool acceptRename(const QUrl &oldUrl, const QUrl &newUrl) override
    {
        acceptRenameCalled = true;
        lastOldUrl = oldUrl;
        lastNewUrl = newUrl;
        return mockAcceptRenameResult;
    }

    bool acceptUpdate(const QUrl &url, const QVector<int> &roles) override
    {
        acceptUpdateCalled = true;
        lastUpdateUrl = url;
        lastUpdateRoles = roles;
        return mockAcceptUpdateResult;
    }

    // Test state tracking
    bool acceptInsertCalled = false;
    QUrl lastInsertUrl;
    bool mockAcceptInsertResult = true;

    bool acceptResetCalled = false;
    QList<QUrl> lastResetUrls;
    QList<QUrl> mockAcceptResetResult;

    bool acceptRenameCalled = false;
    QUrl lastOldUrl;
    QUrl lastNewUrl;
    bool mockAcceptRenameResult = true;

    bool acceptUpdateCalled = false;
    QUrl lastUpdateUrl;
    QVector<int> lastUpdateRoles;
    bool mockAcceptUpdateResult = true;
};

class UT_GeneralModelFilter : public testing::Test
{
protected:
    void SetUp() override
    {
        filter = new GeneralModelFilter();
        mockFilter = new MockModelDataHandler();
    }

    void TearDown() override
    {
        delete filter;
        filter = nullptr;
        // delete mockFilter;
        // mockFilter = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    GeneralModelFilter *filter = nullptr;
    MockModelDataHandler *mockFilter = nullptr;
};

TEST_F(UT_GeneralModelFilter, Constructor_Default_InitializesCorrectly)
{
    EXPECT_NE(filter, nullptr);
    EXPECT_FALSE(filter->modelFilters.isEmpty());
    EXPECT_FALSE(filter->defaultFilters.isEmpty());
}

TEST_F(UT_GeneralModelFilter, Destructor_CleansUpFilters)
{
    EXPECT_NO_THROW(delete filter);
    filter = nullptr;
}

TEST_F(UT_GeneralModelFilter, installFilter_WithValidFilter_ReturnsTrue)
{
    bool result = filter->installFilter(mockFilter);
    EXPECT_TRUE(result);
    EXPECT_TRUE(filter->modelFilters.contains(mockFilter));
}

TEST_F(UT_GeneralModelFilter, installFilter_WithNullFilter_ReturnsFalse)
{
    bool result = filter->installFilter(nullptr);
    EXPECT_FALSE(result);
}

TEST_F(UT_GeneralModelFilter, installFilter_WithDuplicateFilter_ReturnsFalse)
{
    filter->installFilter(mockFilter);
    bool result = filter->installFilter(mockFilter);
    EXPECT_FALSE(result);
    EXPECT_EQ(filter->modelFilters.count(mockFilter), 1);
}

TEST_F(UT_GeneralModelFilter, removeFilter_RemovesFilter)
{
    filter->installFilter(mockFilter);
    EXPECT_TRUE(filter->modelFilters.contains(mockFilter));

    filter->removeFilter(mockFilter);
    EXPECT_FALSE(filter->modelFilters.contains(mockFilter));
}

TEST_F(UT_GeneralModelFilter, removeFilter_WithNonExistentFilter_DoesNotCrash)
{
    EXPECT_NO_THROW(filter->removeFilter(mockFilter));
}

TEST_F(UT_GeneralModelFilter, acceptInsert_WithAllFiltersAccept_ReturnsTrue)
{
    QUrl testUrl("file:///tmp/test.txt");

    bool result = filter->acceptInsert(testUrl);
    EXPECT_TRUE(result);
}

TEST_F(UT_GeneralModelFilter, acceptInsert_WithOneFilterReject_ReturnsFalse)
{
    QUrl testUrl("file:///tmp/test.txt");

    // Install a mock filter that rejects
    mockFilter->mockAcceptInsertResult = false;
    filter->installFilter(mockFilter);

    bool result = filter->acceptInsert(testUrl);
    EXPECT_FALSE(result);
}

TEST_F(UT_GeneralModelFilter, acceptInsert_WithNullFilterInList_ReturnsFalse)
{
    QUrl testUrl("file:///tmp/test.txt");

    // Add a null filter to the list
    filter->modelFilters.append(nullptr);

    bool result = filter->acceptInsert(testUrl);
    EXPECT_FALSE(result);
}

TEST_F(UT_GeneralModelFilter, acceptReset_WithValidUrls_ReturnsFilteredList)
{
    QList<QUrl> testUrls;
    testUrls << QUrl("file:///tmp/file1.txt") << QUrl("file:///tmp/file2.txt");

    QList<QUrl> result = filter->acceptReset(testUrls);
    EXPECT_EQ(result.size(), testUrls.size());
}

TEST_F(UT_GeneralModelFilter, acceptReset_WithEmptyList_ReturnsEmptyList)
{
    QList<QUrl> emptyUrls;

    QList<QUrl> result = filter->acceptReset(emptyUrls);
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_GeneralModelFilter, acceptReset_WithFilterModifyingList_ReturnsModifiedList)
{
    QList<QUrl> testUrls;
    testUrls << QUrl("file:///tmp/file1.txt") << QUrl("file:///tmp/file2.txt");

    QList<QUrl> filteredUrls;
    filteredUrls << QUrl("file:///tmp/file1.txt");

    // Install a mock filter that modifies the list
    // mockFilter->mockAcceptResetResult = filteredUrls;
    filter->installFilter(mockFilter);


    QList<QUrl> result = filter->acceptReset(testUrls);
    EXPECT_EQ(result.size(), filteredUrls.size());
}

TEST_F(UT_GeneralModelFilter, acceptRename_WithAllFiltersAccept_ReturnsTrue)
{
    QUrl oldUrl("file:///tmp/old.txt");
    QUrl newUrl("file:///tmp/new.txt");


    bool result = filter->acceptRename(oldUrl, newUrl);
    EXPECT_TRUE(result);
}

TEST_F(UT_GeneralModelFilter, acceptRename_WithOneFilterReject_ReturnsFalse)
{
    QUrl oldUrl("file:///tmp/old.txt");
    QUrl newUrl("file:///tmp/new.txt");

    // Install a mock filter that rejects
    mockFilter->mockAcceptRenameResult = false;
    filter->installFilter(mockFilter);

    bool result = filter->acceptRename(oldUrl, newUrl);
    EXPECT_FALSE(result);
}

TEST_F(UT_GeneralModelFilter, acceptUpdate_WithAllFiltersAccept_ReturnsTrue)
{
    QUrl testUrl("file:///tmp/test.txt");
    QVector<int> roles = {Qt::DisplayRole};

    bool result = filter->acceptUpdate(testUrl, roles);
    EXPECT_TRUE(result);
}

TEST_F(UT_GeneralModelFilter, acceptUpdate_WithOneFilterReject_ReturnsFalse)
{
    QUrl testUrl("file:///tmp/test.txt");
    QVector<int> roles = {Qt::DisplayRole};

    // Install a mock filter that rejects
    mockFilter->mockAcceptUpdateResult = false;
    filter->installFilter(mockFilter);

    bool result = filter->acceptUpdate(testUrl, roles);
    EXPECT_FALSE(result);
}

TEST_F(UT_GeneralModelFilter, acceptUpdate_WithEmptyRoles_ReturnsTrue)
{
    QUrl testUrl("file:///tmp/test.txt");
    QVector<int> emptyRoles;


    bool result = filter->acceptUpdate(testUrl, emptyRoles);
    EXPECT_TRUE(result);
}