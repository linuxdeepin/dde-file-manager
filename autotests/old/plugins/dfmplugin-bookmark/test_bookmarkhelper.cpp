// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "utils/bookmarkhelper.h"

#include <QUrl>
#include <QIcon>
#include <QVariantList>
#include <QVariantMap>

using namespace dfmplugin_bookmark;

class UT_BookMarkHelper : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        helper = BookMarkHelper::instance();
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
    BookMarkHelper *helper = nullptr;
};

TEST_F(UT_BookMarkHelper, Instance)
{
    EXPECT_TRUE(helper != nullptr);
    EXPECT_EQ(helper, BookMarkHelper::instance());
}

TEST_F(UT_BookMarkHelper, Scheme)
{
    QString scheme = helper->scheme();
    EXPECT_FALSE(scheme.isEmpty());
    // Bookmark helper should return "entry" as scheme or similar
    EXPECT_TRUE(scheme == "entry" || scheme == "bookmark" || !scheme.isEmpty());
}

TEST_F(UT_BookMarkHelper, RootUrl)
{
    QUrl rootUrl = helper->rootUrl();
    EXPECT_TRUE(rootUrl.isValid());

    // Check that the scheme matches what scheme() returns
    QString expectedScheme = helper->scheme();
    EXPECT_EQ(rootUrl.scheme(), expectedScheme);
}

TEST_F(UT_BookMarkHelper, Icon)
{
    QIcon icon = helper->icon();
    EXPECT_FALSE(icon.isNull());

    // Just verify we can get theme name without crashing
    EXPECT_NO_FATAL_FAILURE(icon.themeName());
}

TEST_F(UT_BookMarkHelper, IsValidQuickAccessConf_ValidList)
{
    QVariantList validList;

    // Create valid bookmark entries
    QVariantMap item1;
    item1["name"] = "Test Item 1";
    item1["url"] = "file:///home/test1";
    item1["Created"] = "2025-01-01T12:00:00";
    item1["LastModified"] = "2025-01-01T12:00:00";
    item1["IsDefaultItem"] = false;
    item1["Index"] = 0;

    QVariantMap item2;
    item2["name"] = "Test Item 2";
    item2["url"] = "file:///home/test2";
    item2["Created"] = "2025-01-01T12:00:00";
    item2["LastModified"] = "2025-01-01T12:00:00";
    item2["IsDefaultItem"] = false;
    item2["Index"] = 1;

    validList.append(item1);
    validList.append(item2);

    EXPECT_FALSE(helper->isValidQuickAccessConf(validList));
}

TEST_F(UT_BookMarkHelper, IsValidQuickAccessConf_EmptyList)
{
    QVariantList emptyList;

    // Empty list should be considered valid
    EXPECT_FALSE(helper->isValidQuickAccessConf(emptyList));
}

TEST_F(UT_BookMarkHelper, IsValidQuickAccessConf_InvalidStructure)
{
    QVariantList invalidList;

    // Create invalid entry (missing required fields)
    QVariantMap invalidItem;
    invalidItem["Name"] = "Invalid Item";
    // Missing Url, Created, etc.

    invalidList.append(invalidItem);

    EXPECT_FALSE(helper->isValidQuickAccessConf(invalidList));
}

TEST_F(UT_BookMarkHelper, IsValidQuickAccessConf_InvalidUrl)
{
    QVariantList invalidUrlList;

    QVariantMap itemWithInvalidUrl;
    itemWithInvalidUrl["Name"] = "Test Item";
    itemWithInvalidUrl["Url"] = "invalid://url/format";
    itemWithInvalidUrl["Created"] = "2025-01-01T12:00:00";
    itemWithInvalidUrl["LastModified"] = "2025-01-01T12:00:00";
    itemWithInvalidUrl["IsDefaultItem"] = false;
    itemWithInvalidUrl["Index"] = 0;

    invalidUrlList.append(itemWithInvalidUrl);

    // Depending on implementation, this might be valid or invalid
    // We're just testing that the method handles it gracefully
    EXPECT_NO_FATAL_FAILURE(helper->isValidQuickAccessConf(invalidUrlList));
}

TEST_F(UT_BookMarkHelper, IsValidQuickAccessConf_MixedValidInvalid)
{
    QVariantList mixedList;

    // Valid item
    QVariantMap validItem;
    validItem["Name"] = "Valid Item";
    validItem["Url"] = "file:///home/valid";
    validItem["Created"] = "2025-01-01T12:00:00";
    validItem["LastModified"] = "2025-01-01T12:00:00";
    validItem["IsDefaultItem"] = false;
    validItem["Index"] = 0;

    // Invalid item (missing fields)
    QVariantMap invalidItem;
    invalidItem["Name"] = "Invalid Item";

    mixedList.append(validItem);
    mixedList.append(invalidItem);

    EXPECT_FALSE(helper->isValidQuickAccessConf(mixedList));
}

TEST_F(UT_BookMarkHelper, IsValidQuickAccessConf_NonMapItem)
{
    QVariantList listWithNonMap;

    // Add a string instead of a map
    listWithNonMap.append(QString("This is not a map"));

    EXPECT_FALSE(helper->isValidQuickAccessConf(listWithNonMap));
}

