// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>
#include <QVariantMap>

#include "utils/custommanager.h"
#include "utils/searchhelper.h"
#include "dfmplugin_search_global.h"

#include "stubext.h"

DPSEARCH_USE_NAMESPACE

class TestCustomManager : public testing::Test
{
public:
    void SetUp() override
    {
        manager = CustomManager::instance();
    }

    void TearDown() override
    {
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    CustomManager *manager = nullptr;
};

TEST_F(TestCustomManager, Instance_ReturnsSameInstance)
{
    auto manager1 = CustomManager::instance();
    auto manager2 = CustomManager::instance();

    EXPECT_NE(manager1, nullptr);
    EXPECT_EQ(manager1, manager2);
}

TEST_F(TestCustomManager, RegisterCustomInfo_WithNewScheme_ReturnsTrue)
{
    QString scheme = "test_scheme";
    QVariantMap properties;
    properties[CustomKey::kDisableSearch] = true;
    properties[CustomKey::kRedirectedPath] = "/test/path";

    bool result = manager->registerCustomInfo(scheme, properties);

    EXPECT_TRUE(result);
}

TEST_F(TestCustomManager, RegisterCustomInfo_WithExistingScheme_ReturnsFalse)
{
    QString scheme = "existing_scheme";
    QVariantMap properties1;
    properties1[CustomKey::kDisableSearch] = true;

    QVariantMap properties2;
    properties2[CustomKey::kDisableSearch] = false;

    // First registration should succeed
    bool result1 = manager->registerCustomInfo(scheme, properties1);
    EXPECT_TRUE(result1);

    // Second registration with same scheme should fail
    bool result2 = manager->registerCustomInfo(scheme, properties2);
    EXPECT_FALSE(result2);
}

TEST_F(TestCustomManager, IsRegisted_WithRegisteredScheme_ReturnsTrue)
{
    QString scheme = "registered_scheme";
    QVariantMap properties;
    properties[CustomKey::kDisableSearch] = true;

    manager->registerCustomInfo(scheme, properties);

    bool result = manager->isRegisted(scheme);

    EXPECT_TRUE(result);
}

TEST_F(TestCustomManager, IsRegisted_WithUnregisteredScheme_ReturnsFalse)
{
    QString scheme = "unregistered_scheme";

    bool result = manager->isRegisted(scheme);

    EXPECT_FALSE(result);
}

TEST_F(TestCustomManager, IsDisableSearch_WithNonSearchUrl_ChecksDirectScheme)
{
    QString scheme = "file";
    QUrl url("file:///home/test");
    QVariantMap properties;
    properties[CustomKey::kDisableSearch] = true;

    manager->registerCustomInfo(scheme, properties);

    bool result = manager->isDisableSearch(url);

    EXPECT_TRUE(result);
}

TEST_F(TestCustomManager, IsDisableSearch_WithSearchUrl_ChecksTargetScheme)
{
    QString targetScheme = "ftp";
    QUrl targetUrl("ftp://example.com/file");
    QUrl searchUrl = SearchHelper::fromSearchFile(targetUrl, "keyword", "123");

    QVariantMap properties;
    properties[CustomKey::kDisableSearch] = true;

    manager->registerCustomInfo(targetScheme, properties);

    // Mock SearchHelper::searchTargetUrl
    stub.set_lamda(&SearchHelper::searchTargetUrl, [targetUrl](const QUrl &searchUrl) -> QUrl {
        __DBG_STUB_INVOKE__
        return targetUrl;
    });

    bool result = manager->isDisableSearch(searchUrl);

    EXPECT_TRUE(result);
}

TEST_F(TestCustomManager, IsDisableSearch_WithUnregisteredScheme_ReturnsFalse)
{
    QUrl url("unknown://test");

    bool result = manager->isDisableSearch(url);

    EXPECT_FALSE(result);
}

TEST_F(TestCustomManager, IsDisableSearch_WithRegisteredSchemeButNoDisableFlag_ReturnsFalse)
{
    QString scheme = "http";
    QUrl url("http://example.com");
    QVariantMap properties;
    // No kDisableSearch property set
    properties["other_property"] = "value";

    manager->registerCustomInfo(scheme, properties);

    bool result = manager->isDisableSearch(url);

    EXPECT_FALSE(result);
}

TEST_F(TestCustomManager, RedirectedPath_WithNonSearchUrl_ReturnsRedirectedPath)
{
    QString scheme = "smb";
    QUrl url("smb://server/share/folder");
    QString redirectPath = "/mnt/smb";

    QVariantMap properties;
    properties[CustomKey::kRedirectedPath] = redirectPath;

    manager->registerCustomInfo(scheme, properties);

    QString result = manager->redirectedPath(url);

    QString expectedPath = redirectPath + url.path();
    EXPECT_EQ(result, expectedPath);
}

TEST_F(TestCustomManager, RedirectedPath_WithSearchUrl_ChecksTargetUrl)
{
    QVariantMap data;
    data.insert("Property_Key_RedirectedPath", "/home");
    manager->registerCustomInfo("test", data);

    QUrl url = SearchHelper::fromSearchFile(QUrl::fromUserInput("test:///home"), "test", "123");
    auto path = manager->redirectedPath(url);
    EXPECT_TRUE(!path.isEmpty());
}

TEST_F(TestCustomManager, RedirectedPath_WithUnregisteredScheme_ReturnsEmptyString)
{
    QUrl url("unknown://test/path");

    QString result = manager->redirectedPath(url);

    EXPECT_TRUE(result.isEmpty());
}

TEST_F(TestCustomManager, RedirectedPath_WithEmptyRedirectPath_ReturnsEmptyString)
{
    QString scheme = "nfs";
    QUrl url("nfs://server/export");

    QVariantMap properties;
    properties[CustomKey::kRedirectedPath] = QString(); // Empty redirect path

    manager->registerCustomInfo(scheme, properties);

    QString result = manager->redirectedPath(url);

    EXPECT_TRUE(result.isEmpty());
}

TEST_F(TestCustomManager, IsUseNormalMenu_WithNormalMenuEnabled_ReturnsTrue)
{
    QString scheme = "sftp";

    QVariantMap properties;
    properties[CustomKey::kUseNormalMenu] = true;

    manager->registerCustomInfo(scheme, properties);

    bool result = manager->isUseNormalMenu(scheme);

    EXPECT_TRUE(result);
}

TEST_F(TestCustomManager, IsUseNormalMenu_WithNormalMenuDisabled_ReturnsFalse)
{
    QString scheme = "mtp";

    QVariantMap properties;
    properties[CustomKey::kUseNormalMenu] = false;

    manager->registerCustomInfo(scheme, properties);

    bool result = manager->isUseNormalMenu(scheme);

    EXPECT_FALSE(result);
}

TEST_F(TestCustomManager, IsUseNormalMenu_WithUnregisteredScheme_ReturnsFalse)
{
    QString scheme = "unregistered";

    bool result = manager->isUseNormalMenu(scheme);

    EXPECT_FALSE(result);
}

TEST_F(TestCustomManager, IsUseNormalMenu_WithoutNormalMenuProperty_ReturnsFalse)
{
    QString scheme = "gphoto2";

    QVariantMap properties;
    properties[CustomKey::kDisableSearch] = true; // Different property

    manager->registerCustomInfo(scheme, properties);

    bool result = manager->isUseNormalMenu(scheme);

    EXPECT_FALSE(result);
}

TEST_F(TestCustomManager, MultipleSchemes_WithDifferentProperties_WorksCorrectly)
{
    QString scheme1 = "scheme1";
    QString scheme2 = "scheme2";
    QString scheme3 = "scheme3";

    QVariantMap properties1;
    properties1[CustomKey::kDisableSearch] = true;
    properties1[CustomKey::kRedirectedPath] = "/path1";

    QVariantMap properties2;
    properties2[CustomKey::kDisableSearch] = false;
    properties2[CustomKey::kUseNormalMenu] = true;

    QVariantMap properties3;
    properties3[CustomKey::kRedirectedPath] = "/path3";
    properties3[CustomKey::kUseNormalMenu] = false;

    // Register all schemes
    EXPECT_TRUE(manager->registerCustomInfo(scheme1, properties1));
    EXPECT_TRUE(manager->registerCustomInfo(scheme2, properties2));
    EXPECT_TRUE(manager->registerCustomInfo(scheme3, properties3));

    // Test all are registered
    EXPECT_TRUE(manager->isRegisted(scheme1));
    EXPECT_TRUE(manager->isRegisted(scheme2));
    EXPECT_TRUE(manager->isRegisted(scheme3));

    // Test disable search
    QUrl url1(scheme1 + "://test");
    QUrl url2(scheme2 + "://test");
    EXPECT_TRUE(manager->isDisableSearch(url1));
    EXPECT_FALSE(manager->isDisableSearch(url2));

    // Test normal menu
    EXPECT_FALSE(manager->isUseNormalMenu(scheme1));
    EXPECT_TRUE(manager->isUseNormalMenu(scheme2));
    EXPECT_FALSE(manager->isUseNormalMenu(scheme3));

    // Test redirected path
    QUrl url3(scheme3 + "://test/subpath");
    QString redirected = manager->redirectedPath(url3);
    EXPECT_EQ(redirected, QString("/path3/subpath"));
}
