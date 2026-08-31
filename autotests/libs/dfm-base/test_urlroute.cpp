// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_urlroute.cpp
 * @brief Unit tests for UrlRoute methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/base/urlroute.h"

#include <QTest>

using namespace src;

class UrlRouteTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new UrlRoute();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    UrlRoute *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UrlRouteTest, icon)
{
    // Test method: QIcon icon((const QString &scheme))
    QString _arg0{};
    auto result = obj->icon(_arg0);
    EXPECT_TRUE(result.isNull());

}

TEST_F(UrlRouteTest, schemeInfos)
{
    // Test getter: QHash<QString, SchemeNode> schemeInfos()
    auto result = obj->schemeInfos();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(UrlRouteTest, regScheme)
{
    // Test method: bool regScheme((const QString &scheme,
                         const QString &root,
                         const QIcon &icon,
                         const bool isVirtual,
                         const QString &displayName,
                         QString *errorString))
    QString _arg0{};
    QString _arg1{};
    QIcon _arg2{};
    QString _arg4{};
    auto result = obj->regScheme(_arg0, _arg1, _arg2, false, _arg4, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(UrlRouteTest, toString)
{
    // Test method: QString toString((const QUrl &url, QUrl::FormattingOptions options))
    QUrl _arg0{};
    auto result = obj->toString(_arg0, QUrl::FormattingOptions());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(UrlRouteTest, hasScheme)
{
    // Test method: bool hasScheme((const QString &scheme))
    QString _arg0{};
    auto result = obj->hasScheme(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(UrlRouteTest, isVirtual)
{
    // Test method: bool isVirtual((const QString &scheme))
    QString _arg0{};
    auto result = obj->isVirtual(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(UrlRouteTest, urlParent)
{
    // Test method: QUrl urlParent((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->urlParent(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(UrlRouteTest, rootPath)
{
    // Test method: QString rootPath((const QString &scheme))
    QString _arg0{};
    auto result = obj->rootPath(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(UrlRouteTest, rootUrl)
{
    // Test method: QUrl rootUrl((const QString &scheme))
    QString _arg0{};
    auto result = obj->rootUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(UrlRouteTest, schemeRealTree)
{
    // Test getter: QMultiMap<int, QString> schemeRealTree()
    auto result = obj->schemeRealTree();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(UrlRouteTest, isRootUrl)
{
    // Test method: bool isRootUrl((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->isRootUrl(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(UrlRouteTest, isAncestorsUrl)
{
    // Test method: bool isAncestorsUrl((QUrl url, const QUrl &ancestorsUrl, QList<QUrl> *list))
    QUrl _arg1{};
    auto result = obj->isAncestorsUrl(QUrl(), _arg1, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(UrlRouteTest, urlParentList)
{
    // Test method: void urlParentList((QUrl url, QList<QUrl> *list))
    EXPECT_NO_FATAL_FAILURE(obj->urlParentList(QUrl(), nullptr));
}

TEST_F(UrlRouteTest, rootDisplayName)
{
    // Test method: QString rootDisplayName((const QString &scheme))
    QString _arg0{};
    auto result = obj->rootDisplayName(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(UrlRouteTest, fromUserInput)
{
    // Test method: QUrl fromUserInput((const QString &userInput, QString workingDirectory, bool preferredLocalPath, QUrl::UserInputResolutionOptions options))
    QString _arg0{};
    auto result = obj->fromUserInput(_arg0, QString(), false, QUrl::UserInputResolutionOptions());
    EXPECT_FALSE(result.isValid());

}

TEST_F(UrlRouteTest, fromStringList)
{
    // Test method: QList<QUrl> fromStringList((const QStringList &strList))
    QStringList _arg0{};
    auto result = obj->fromStringList(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(UrlRouteTest, pathToReal)
{
    // Test method: QUrl pathToReal((const QString &path))
    QString _arg0{};
    auto result = obj->pathToReal(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(UrlRouteTest, fromLocalFile)
{
    // Test method: QUrl fromLocalFile((const QString &path))
    QString _arg0{};
    auto result = obj->fromLocalFile(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(UrlRouteTest, pathToUrl)
{
    // Test method: QUrl pathToUrl((const QString &path, const QString &scheme))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->pathToUrl(_arg0, _arg1);
    EXPECT_FALSE(result.isValid());

}

TEST_F(UrlRouteTest, urlToPath)
{
    // Test method: QString urlToPath((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->urlToPath(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(UrlRouteTest, urlsToByteArray)
{
    // Test method: QByteArray urlsToByteArray((const QList<QUrl> &list))
    QList<QUrl> _arg0{};
    auto result = obj->urlsToByteArray(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(UrlRouteTest, byteArrayToUrls)
{
    // Test method: QList<QUrl> byteArrayToUrls((const QByteArray &arr))
    QByteArray _arg0{};
    auto result = obj->byteArrayToUrls(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(UrlRouteTest, urlToLocalPath)
{
    // Test method: QString urlToLocalPath((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->urlToLocalPath(_arg0);
    EXPECT_TRUE(result.isEmpty());

}
