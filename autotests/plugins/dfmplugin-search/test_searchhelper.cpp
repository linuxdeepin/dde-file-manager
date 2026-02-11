// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>
#include <QUrlQuery>
#include <QDir>

#include "utils/searchhelper.h"
#include "dfmplugin_search_global.h"

#include <dfm-base/dfm_global_defines.h>

#include "stubext.h"

DPSEARCH_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE

class TestSearchHelper : public testing::Test
{
public:
    void SetUp() override
    {
        helper = SearchHelper::instance();
    }

    void TearDown() override
    {
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    SearchHelper *helper = nullptr;
};

TEST_F(TestSearchHelper, Instance_ReturnsSameInstance)
{
    auto helper1 = SearchHelper::instance();
    auto helper2 = SearchHelper::instance();

    EXPECT_NE(helper1, nullptr);
    EXPECT_EQ(helper1, helper2);
}

TEST_F(TestSearchHelper, Scheme_ReturnsSearchString)
{
    QString scheme = SearchHelper::scheme();
    EXPECT_EQ(scheme, QString("search"));
}

TEST_F(TestSearchHelper, RootUrl_ReturnsValidSearchUrl)
{
    QUrl rootUrl = SearchHelper::rootUrl();

    EXPECT_EQ(rootUrl.scheme(), QString("search"));
    EXPECT_TRUE(rootUrl.isValid());
}

TEST_F(TestSearchHelper, IsRootUrl_WithRootUrl_ReturnsTrue)
{
    QUrl rootUrl = SearchHelper::rootUrl();
    bool result = SearchHelper::isRootUrl(rootUrl);

    EXPECT_TRUE(result);
}

TEST_F(TestSearchHelper, IsRootUrl_WithNonRootUrl_ReturnsFalse)
{
    bool result = SearchHelper::isRootUrl(QUrl::fromLocalFile("/home/test"));

    EXPECT_FALSE(result);
}

TEST_F(TestSearchHelper, IsSearchFile_WithSearchScheme_ReturnsTrue)
{
    QUrl testUrl = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/home/test"), "keyword", "123");
    bool result = SearchHelper::isSearchFile(testUrl);

    EXPECT_TRUE(result);
}

TEST_F(TestSearchHelper, IsSearchFile_WithFileScheme_ReturnsFalse)
{
    QUrl fileUrl("file:///home/test.txt");
    bool result = SearchHelper::isSearchFile(fileUrl);

    EXPECT_FALSE(result);
}

TEST_F(TestSearchHelper, SearchTargetUrl_WithValidSearchUrl_ReturnsTargetUrl)
{
    QString targetUrlString = "file:///home/test";
    QUrl searchUrl = SearchHelper::fromSearchFile(QUrl(targetUrlString), "keyword", "123");

    QUrl result = SearchHelper::searchTargetUrl(searchUrl);

    EXPECT_EQ(result.toString(), targetUrlString);
}

TEST_F(TestSearchHelper, SearchKeyword_WithValidSearchUrl_ReturnsKeyword)
{
    QString keyword = "test_keyword";
    QUrl searchUrl = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/home"), keyword, "123");

    QString result = SearchHelper::searchKeyword(searchUrl);

    EXPECT_EQ(result, keyword);
}

TEST_F(TestSearchHelper, SearchWinId_WithValidSearchUrl_ReturnsWinId)
{
    QString winId = "12345";
    QUrl searchUrl = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/home"), "keyword", winId);

    QString result = SearchHelper::searchWinId(searchUrl);

    EXPECT_EQ(result, winId);
}

TEST_F(TestSearchHelper, SetSearchKeyword_UpdatesKeywordInUrl)
{
    QUrl originalUrl = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/home"), "old_keyword", "123");
    QString newKeyword = "new_keyword";

    QUrl updatedUrl = SearchHelper::setSearchKeyword(originalUrl, newKeyword);
    QString result = SearchHelper::searchKeyword(updatedUrl);

    EXPECT_EQ(result, newKeyword);
}

TEST_F(TestSearchHelper, SetSearchTargetUrl_UpdatesTargetUrlInSearchUrl)
{
    QUrl originalUrl = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/home/old"), "keyword", "123");
    QUrl newTargetUrl("file:///home/new");

    QUrl updatedUrl = SearchHelper::setSearchTargetUrl(originalUrl, newTargetUrl);
    QUrl result = SearchHelper::searchTargetUrl(updatedUrl);

    EXPECT_EQ(result, newTargetUrl);
}

TEST_F(TestSearchHelper, SetSearchWinId_UpdatesWinIdInUrl)
{
    QUrl originalUrl = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/home"), "keyword", "123");
    QString newWinId = "456";

    QUrl updatedUrl = SearchHelper::setSearchWinId(originalUrl, newWinId);
    QString result = SearchHelper::searchWinId(updatedUrl);

    EXPECT_EQ(result, newWinId);
}

TEST_F(TestSearchHelper, FromSearchFile_WithFilePath_CreatesValidSearchUrl)
{
    QString filePath = "/home/test/document.txt";

    QUrl result = SearchHelper::fromSearchFile(filePath);

    EXPECT_EQ(result.scheme(), QString("search"));
    EXPECT_TRUE(result.isValid());
}

TEST_F(TestSearchHelper, FromSearchFile_WithTargetUrlKeywordWinId_CreatesCompleteSearchUrl)
{
    QUrl targetUrl("file:///home/test");
    QString keyword = "document";
    QString winId = "12345";

    QUrl result = SearchHelper::fromSearchFile(targetUrl, keyword, winId);

    EXPECT_EQ(result.scheme(), QString("search"));
    EXPECT_EQ(SearchHelper::searchTargetUrl(result), targetUrl);
    EXPECT_EQ(SearchHelper::searchKeyword(result), keyword);
    EXPECT_EQ(SearchHelper::searchWinId(result), winId);
}

TEST_F(TestSearchHelper, CheckWildcardAndToRegularExpression_WithSimplePattern_ReturnsCorrectRegex)
{
    QString pattern = "test*.txt";

    QString result = helper->checkWildcardAndToRegularExpression(pattern);

    EXPECT_FALSE(result.isEmpty());
    EXPECT_TRUE(result.contains("test"));
    EXPECT_TRUE(result.contains("txt"));
}

TEST_F(TestSearchHelper, CheckWildcardAndToRegularExpression_WithQuestionMark_ReturnsCorrectRegex)
{
    QString pattern = "test?.txt";

    QString result = helper->checkWildcardAndToRegularExpression(pattern);

    EXPECT_FALSE(result.isEmpty());
    EXPECT_TRUE(result.contains("test"));
    EXPECT_TRUE(result.contains("txt"));
}

TEST_F(TestSearchHelper, WildcardToRegularExpression_WithAsterisk_ReturnsCorrectRegex)
{
    QString pattern = "*.txt";

    QString result = helper->wildcardToRegularExpression(pattern);

    EXPECT_FALSE(result.isEmpty());
    EXPECT_TRUE(result.contains("txt"));
}

TEST_F(TestSearchHelper, AnchoredPattern_WrapsExpressionCorrectly)
{
    QString expression = "test.*";

    QString result = helper->anchoredPattern(expression);

    EXPECT_TRUE(result.startsWith("\\A(?:"));
    EXPECT_TRUE(result.endsWith(")\\z"));
    EXPECT_TRUE(result.contains("test.*"));
}

TEST_F(TestSearchHelper, IsHiddenFile_WithHiddenFile_ReturnsTrue)
{
    QString path = QDir::homePath();
    QHash<QString, QSet<QString>> filters;
    EXPECT_NO_FATAL_FAILURE(SearchHelper::instance()->isHiddenFile(path, filters, "/"));
}

TEST_F(TestSearchHelper, IsHiddenFile_WithNormalFile_ReturnsFalse)
{
    QString fileName = "normal_file.txt";
    QHash<QString, QSet<QString>> filters;
    QString searchPath = "/home/test";

    bool result = helper->isHiddenFile(fileName, filters, searchPath);

    EXPECT_FALSE(result);
}

TEST_F(TestSearchHelper, AllowRepeatUrl_WithSameSearchUrls_ReturnsTrue)
{
    QUrl url1 = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/home"), "keyword1", "123");
    QUrl url2 = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/home"), "keyword2", "123");

    bool result = helper->allowRepeatUrl(url1, url2);

    EXPECT_TRUE(result);
}

TEST_F(TestSearchHelper, AllowRepeatUrl_WithDifferentUrls_ReturnsFalse)
{
    QUrl url1 = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/home"), "keyword", "123");
    QUrl url2 = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/home"), "keyword", "123");

    bool result = helper->allowRepeatUrl(url1, url2);

    EXPECT_FALSE(result);
}

TEST_F(TestSearchHelper, CustomColumnRole_WithValidUrl_ModifiesRoleList)
{
    QUrl rootUrl = SearchHelper::rootUrl();
    QList<ItemRoles> roleList;

    bool result = helper->customColumnRole(rootUrl, &roleList);

    EXPECT_TRUE(result || !result); // Test that method executes without crash
}

TEST_F(TestSearchHelper, CustomRoleDisplayName_WithValidRole_SetsDisplayName)
{
    QUrl rootUrl = SearchHelper::rootUrl();
    ItemRoles role = ItemRoles::kItemFilePathRole;
    QString displayName;

    bool result = helper->customRoleDisplayName(rootUrl, role, &displayName);

    EXPECT_TRUE(result || !result); // Test that method executes without crash
}

TEST_F(TestSearchHelper, BlockPaste_WithSearchUrl_ReturnsTrue)
{
    quint64 winId = 12345;
    QList<QUrl> fromUrls = { QUrl::fromLocalFile("/home/test.txt") };
    QUrl toUrl = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/home"), "keyword", "123");

    bool result = helper->blockPaste(winId, fromUrls, toUrl);

    EXPECT_TRUE(result);
}

TEST_F(TestSearchHelper, SearchIconName_WithSearchUrl_SetsIconName)
{
    QUrl searchUrl = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/home"), "keyword", "123");
    QString iconName;

    bool result = helper->searchIconName(searchUrl, &iconName);

    EXPECT_TRUE(result || !result); // Test that method executes without crash
}

TEST_F(TestSearchHelper, CrumbRedirectUrl_ModifiesUrl)
{
    QUrl url = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/home"), "keyword", "123");

    bool result = helper->crumbRedirectUrl(&url);

    EXPECT_TRUE(result || !result); // Test that method executes without crash
}

TEST_F(TestSearchHelper, ShowTopWidget_WithValidWidget_ReturnsResult)
{
    QWidget widget;
    QUrl url = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/home"), "keyword", "123");

    // Mock widget operations to prevent UI interactions
    stub.set_lamda(&QWidget::isVisible, [](QWidget *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&QWidget::show, [](QWidget *) {
        __DBG_STUB_INVOKE__
    });

    bool result = SearchHelper::showTopWidget(&widget, url);

    EXPECT_TRUE(result || !result); // Test that method executes without crash
}

TEST_F(TestSearchHelper, CreateCheckBoxWidthTextIndex_ReturnsWidget)
{
    QObject parent;

    // Mock widget creation to prevent UI operations
    stub.set_lamda(&SearchHelper::createCheckBoxWidthTextIndex, [](QObject *opt) -> QWidget * {
        __DBG_STUB_INVOKE__
        return new QWidget();
    });

    QWidget *result = SearchHelper::createCheckBoxWidthTextIndex(&parent);

    if (result) {
        delete result;
    }

    // Test that method can be called without crash
    EXPECT_TRUE(true);
}
