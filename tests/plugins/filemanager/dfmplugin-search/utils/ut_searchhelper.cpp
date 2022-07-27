/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "utils/searchhelper.h"
#include "topwidget/advancesearchbar.h"
#include "stubext.h"

#include "dfm-base/file/local/localfileinfo.h"
#include "dfm-base/base/schemefactory.h"

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

TEST(SearchHelperTest, ut_searchTaskId)
{
    QUrl searchUrl = SearchHelper::rootUrl();
    searchUrl = SearchHelper::setSearchTaskId(searchUrl, "123");

    QString taskId = SearchHelper::searchTaskId(searchUrl);
    EXPECT_EQ(taskId, "123");
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

    QString taskId = SearchHelper::searchTaskId(url);
    EXPECT_EQ(taskId, "123");
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

TEST(SearchHelperTest, ut_customRoleData)
{
    bool ret = SearchHelper::instance()->customRoleData(QUrl::fromLocalFile("/"),
                                                        QUrl::fromLocalFile("/home"),
                                                        kItemFilePathRole,
                                                        nullptr);
    EXPECT_FALSE(ret);

    stub_ext::StubExt st;
    typedef bool (EventSequenceManager::*RunFunc)(const QString &, const QString &, QUrl,
                                                  const QUrl &, const ItemRoles &, QVariant *&);
    auto runFunc = static_cast<RunFunc>(&EventSequenceManager::run);
    st.set_lamda(runFunc, [] { return false; });

    QUrl url = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/"), "test", "123");
    QVariant data;
    ret = SearchHelper::instance()->customRoleData(url,
                                                   QUrl::fromLocalFile("/home"),
                                                   kItemFilePathRole,
                                                   &data);
    EXPECT_FALSE(ret);
}

TEST(SearchHelperTest, ut_blockPaste)
{
    bool ret = SearchHelper::instance()->blockPaste(123, QUrl::fromLocalFile("/home"));
    EXPECT_FALSE(ret);

    QUrl url = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/"), "test", "123");
    ret = SearchHelper::instance()->blockPaste(123, url);
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
