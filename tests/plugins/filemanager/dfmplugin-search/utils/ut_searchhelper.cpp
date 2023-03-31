// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils/searchhelper.h"
#include "topwidget/advancesearchbar.h"
#include "stubext.h"

#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/base/schemefactory.h>

#include <dfm-framework/event/event.h>

#include <gtest/gtest.h>

Q_DECLARE_METATYPE(QVariant *)
Q_DECLARE_METATYPE(QString *)

DPSEARCH_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DPF_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

TEST(SearchHelperTest, ut_scheme)
{
    EXPECT_EQ(SearchHelper::scheme(), "search");
}

TEST(SearchHelperTest, ut_rootUrl)
{
    const auto &rootUrl = SearchHelper::rootUrl();
    EXPECT_TRUE(SearchHelper::isRootUrl(rootUrl));
}

TEST(SearchHelperTest, ut_isSearchFile)
{
    QUrl url;
    url.setScheme("search");
    url.setPath("/");

    EXPECT_TRUE(SearchHelper::isSearchFile(url));
}

TEST(SearchHelperTest, ut_searchTargetUrl)
{
    QUrl targetUrl = QUrl::fromLocalFile("/home");
    QUrl searchUrl = SearchHelper::rootUrl();
    searchUrl = SearchHelper::setSearchTargetUrl(searchUrl, targetUrl);

    QUrl resultUrl = SearchHelper::searchTargetUrl(searchUrl);
    EXPECT_EQ(targetUrl.path(), resultUrl.path());
}

TEST(SearchHelperTest, ut_searchKeyword)
{
    QUrl searchUrl = SearchHelper::rootUrl();
    searchUrl = SearchHelper::setSearchKeyword(searchUrl, "test");

    QString keyword = SearchHelper::searchKeyword(searchUrl);
    EXPECT_EQ(keyword, "test");
}

TEST(SearchHelperTest, ut_searchWinId)
{
    QUrl searchUrl = SearchHelper::rootUrl();
    searchUrl = SearchHelper::setSearchWinId(searchUrl, "123");

    QString winId = SearchHelper::searchWinId(searchUrl);
    EXPECT_EQ(winId, "123");
}

TEST(SearchHelperTest, ut_fromSearchFile_1)
{
    QUrl url = SearchHelper::fromSearchFile("/home");

    EXPECT_TRUE(SearchHelper::isSearchFile(url));
    EXPECT_EQ(url.path(), "/home");
}

TEST(SearchHelperTest, ut_fromSearchFile_2)
{
    QUrl targetUrl = QUrl::fromLocalFile("/home");
    QUrl url = SearchHelper::fromSearchFile(targetUrl, "test", "123");

    EXPECT_TRUE(SearchHelper::isSearchFile(url));

    QUrl resultUrl = SearchHelper::searchTargetUrl(url);
    EXPECT_EQ(targetUrl.path(), resultUrl.path());

    QString keyword = SearchHelper::searchKeyword(url);
    EXPECT_EQ(keyword, "test");

    QString winId = SearchHelper::searchWinId(url);
    EXPECT_EQ(winId, "123");
}

TEST(SearchHelperTest, ut_showTopWidget)
{
    bool ret = SearchHelper::showTopWidget(nullptr, QUrl());
    EXPECT_FALSE(ret);

    stub_ext::StubExt st;
    st.set_lamda(&AdvanceSearchBar::isVisible, []() { return true; });
    st.set_lamda(&AdvanceSearchBar::refreshOptions, []() { return; });

    AdvanceSearchBar bar;
    ret = SearchHelper::showTopWidget(&bar, QUrl());
    EXPECT_TRUE(ret);
}

TEST(SearchHelperTest, ut_customColumnRole)
{
    bool ret = SearchHelper::instance()->customColumnRole(QUrl::fromLocalFile("/home"), nullptr);
    EXPECT_FALSE(ret);

    stub_ext::StubExt st;
    typedef bool (EventSequenceManager::*RunFunc)(const QString &, const QString &, QUrl, QList<ItemRoles> *&);
    auto runFunc = static_cast<RunFunc>(&EventSequenceManager::run);
    st.set_lamda(runFunc, [] { return false; });

    QUrl url = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/home"), "test", "123");
    QList<ItemRoles> roleList;
    ret = SearchHelper::instance()->customColumnRole(url, &roleList);

    EXPECT_TRUE(ret);
    EXPECT_TRUE(!roleList.isEmpty());
}

TEST(SearchHelperTest, ut_customRoleDisplayName)
{
    bool ret = SearchHelper::instance()->customRoleDisplayName(QUrl::fromLocalFile("/home"),
                                                               kItemFilePathRole,
                                                               nullptr);
    EXPECT_FALSE(ret);

    stub_ext::StubExt st;
    typedef bool (EventSequenceManager::*RunFunc)(const QString &, const QString &, QUrl,
                                                  const ItemRoles &, QString *&);
    auto runFunc = static_cast<RunFunc>(&EventSequenceManager::run);
    st.set_lamda(runFunc, [] { return false; });

    QUrl url = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/home"), "test", "123");
    QString name;
    ret = SearchHelper::instance()->customRoleDisplayName(url,
                                                          kItemFilePathRole,
                                                          &name);
    EXPECT_TRUE(ret);
    EXPECT_EQ(name, "Path");
}

TEST(SearchHelperTest, ut_blockPaste)
{
    bool ret = SearchHelper::instance()->blockPaste(123, {}, QUrl::fromLocalFile("/home"));
    EXPECT_FALSE(ret);

    QUrl url = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/"), "test", "123");
    ret = SearchHelper::instance()->blockPaste(123, {}, url);
    EXPECT_TRUE(ret);
}

TEST(SearchHelperTest, ut_checkWildcardAndToRegularExpression)
{
    QString ret = SearchHelper::instance()->checkWildcardAndToRegularExpression("123");
    EXPECT_TRUE(!ret.isEmpty());
}

TEST(SearchHelperTest, ut_isHiddenFile)
{
    QString path = QDir::homePath();
    QHash<QString, QSet<QString>> filters;
    EXPECT_NO_FATAL_FAILURE(SearchHelper::instance()->isHiddenFile(path, filters, "/"));
}
