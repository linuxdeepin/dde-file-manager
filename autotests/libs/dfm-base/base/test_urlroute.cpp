// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>
#include <QIcon>
#include <QTemporaryDir>

#include "stubext.h"

#include <dfm-base/base/urlroute.h>

DFMBASE_USE_NAMESPACE

class TestUrlRoute : public testing::Test
{
public:
    void SetUp() override
    {
        // Register a test scheme for testing
        testScheme = "test_scheme";
        testRoot = "/test/root";
        testDisplayName = "Test Display";

        UrlRoute::regScheme(testScheme, testRoot, QIcon(), false, testDisplayName);
    }

    void TearDown() override
    {
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    QString testScheme;
    QString testRoot;
    QString testDisplayName;
};

// ========== SchemeNode Tests ==========

TEST_F(TestUrlRoute, SchemeNode_Construction)
{
    // Test SchemeNode construction
    SchemeNode node("/path/to/root", QIcon(), false, "Test Node");

    EXPECT_EQ(node.rootPath(), QString("/path/to/root"));
    EXPECT_FALSE(node.isVirtual());
    EXPECT_EQ(node.displayName(), QString("Test Node"));
}

TEST_F(TestUrlRoute, SchemeNode_VirtualNode)
{
    // Test virtual node
    SchemeNode node("/virtual/path", QIcon(), true, "Virtual");

    EXPECT_TRUE(node.isVirtual());
}

TEST_F(TestUrlRoute, SchemeNode_EmptyCheck)
{
    // Test isEmpty for default node
    SchemeNode emptyNode;

    EXPECT_TRUE(emptyNode.isEmpty());
}

TEST_F(TestUrlRoute, SchemeNode_AssignmentOperator)
{
    // Test assignment operator
    SchemeNode node1("/path1", QIcon(), false, "Node1");
    SchemeNode node2;

    node2 = node1;

    EXPECT_EQ(node2.rootPath(), node1.rootPath());
    EXPECT_EQ(node2.displayName(), node1.displayName());
    EXPECT_EQ(node2.isVirtual(), node1.isVirtual());
}

// ========== regScheme() Tests ==========

TEST_F(TestUrlRoute, regScheme_BasicRegistration)
{
    // Test basic scheme registration
    QString scheme = "custom_scheme";
    QString root = "/custom/root";

    bool result = UrlRoute::regScheme(scheme, root);

    EXPECT_TRUE(result);
    EXPECT_TRUE(UrlRoute::hasScheme(scheme));
}

TEST_F(TestUrlRoute, regScheme_WithIconAndDisplayName)
{
    // Test registration with icon and display name
    QString scheme = "full_scheme";
    QString root = "/full/root";
    QIcon icon(":/icon/test");
    QString displayName = "Full Scheme";

    bool result = UrlRoute::regScheme(scheme, root, icon, false, displayName);

    EXPECT_TRUE(result);
    EXPECT_EQ(UrlRoute::rootDisplayName(scheme), displayName);
}

TEST_F(TestUrlRoute, regScheme_VirtualScheme)
{
    // Test registering virtual scheme
    QString scheme = "virtual_scheme";
    QString root = "/virtual/root";

    bool result = UrlRoute::regScheme(scheme, root, QIcon(), true);

    EXPECT_TRUE(result);
    EXPECT_TRUE(UrlRoute::isVirtual(scheme));
}

TEST_F(TestUrlRoute, regScheme_DuplicateScheme)
{
    // Test registering duplicate scheme should fail
    QString scheme = "duplicate_scheme";
    QString root1 = "/root1";
    QString root2 = "/root2";

    bool result1 = UrlRoute::regScheme(scheme, root1);
    EXPECT_TRUE(result1);

    QString errorString;
    bool result2 = UrlRoute::regScheme(scheme, root2, QIcon(), false, QString(), &errorString);

    EXPECT_FALSE(result2);
    EXPECT_FALSE(errorString.isEmpty());
}

// ========== hasScheme() Tests ==========

TEST_F(TestUrlRoute, hasScheme_ExistingScheme)
{
    // Test hasScheme for existing scheme
    EXPECT_TRUE(UrlRoute::hasScheme(testScheme));
}

TEST_F(TestUrlRoute, hasScheme_NonExistingScheme)
{
    // Test hasScheme for non-existing scheme
    EXPECT_FALSE(UrlRoute::hasScheme("nonexistent_scheme"));
}

// ========== rootPath() Tests ==========

TEST_F(TestUrlRoute, rootPath_RegisteredScheme)
{
    // Test getting root path for registered scheme
    QString root = UrlRoute::rootPath(testScheme);

    EXPECT_EQ(root, testRoot);
}

TEST_F(TestUrlRoute, rootPath_NonExistingScheme)
{
    // Test getting root path for non-existing scheme
    QString root = UrlRoute::rootPath("nonexistent");

    EXPECT_TRUE(root.isEmpty());
}

// ========== rootUrl() Tests ==========

TEST_F(TestUrlRoute, rootUrl_RegisteredScheme)
{
    // Test getting root URL
    QUrl url = UrlRoute::rootUrl(testScheme);

    EXPECT_TRUE(url.isValid());
    EXPECT_EQ(url.scheme(), testScheme);
}

TEST_F(TestUrlRoute, rootUrl_ValidScheme)
{
    // Test root URL has correct scheme
    QString scheme = "test_root_url";
    UrlRoute::regScheme(scheme, "/test");

    QUrl url = UrlRoute::rootUrl(scheme);

    EXPECT_EQ(url.scheme(), scheme);
}

// ========== icon() Tests ==========

TEST_F(TestUrlRoute, icon_RegisteredScheme)
{
    // Test getting icon for registered scheme
    QIcon icon = UrlRoute::icon(testScheme);

    // Icon may be null if not set, but call should not crash
    EXPECT_TRUE(true);
}

TEST_F(TestUrlRoute, icon_WithCustomIcon)
{
    // Test scheme with custom icon
    QString scheme = "icon_scheme";
    QIcon customIcon(":/test/icon");

    UrlRoute::regScheme(scheme, "/test", customIcon);

    QIcon retrievedIcon = UrlRoute::icon(scheme);
    EXPECT_TRUE(true);  // Just verify no crash
}

// ========== rootDisplayName() Tests ==========

TEST_F(TestUrlRoute, rootDisplayName_RegisteredScheme)
{
    // Test getting display name
    QString displayName = UrlRoute::rootDisplayName(testScheme);

    EXPECT_EQ(displayName, testDisplayName);
}

TEST_F(TestUrlRoute, rootDisplayName_NonExistingScheme)
{
    // Test display name for non-existing scheme
    QString displayName = UrlRoute::rootDisplayName("nonexistent");

    EXPECT_TRUE(displayName.isEmpty());
}

// ========== isVirtual() Tests ==========

TEST_F(TestUrlRoute, isVirtual_ByScheme_Virtual)
{
    // Test virtual scheme
    QString virtualScheme = "virtual_test";
    UrlRoute::regScheme(virtualScheme, "/virtual", QIcon(), true);

    EXPECT_TRUE(UrlRoute::isVirtual(virtualScheme));
}

TEST_F(TestUrlRoute, isVirtual_ByScheme_NonVirtual)
{
    // Test non-virtual scheme
    EXPECT_FALSE(UrlRoute::isVirtual(testScheme));
}

TEST_F(TestUrlRoute, isVirtual_ByUrl_Virtual)
{
    // Test virtual URL
    QString virtualScheme = "virtual_url_test";
    UrlRoute::regScheme(virtualScheme, "/virtual", QIcon(), true);

    QUrl url;
    url.setScheme(virtualScheme);
    url.setPath("/test");

    EXPECT_TRUE(UrlRoute::isVirtual(url));
}

TEST_F(TestUrlRoute, isVirtual_ByUrl_NonVirtual)
{
    // Test non-virtual URL
    QUrl url;
    url.setScheme(testScheme);
    url.setPath("/test");

    EXPECT_FALSE(UrlRoute::isVirtual(url));
}

// ========== toString() Tests ==========

TEST_F(TestUrlRoute, toString_BasicUrl)
{
    // Test URL to string conversion
    QUrl url("file:///home/user/test.txt");

    QString str = UrlRoute::toString(url);

    EXPECT_FALSE(str.isEmpty());
}

TEST_F(TestUrlRoute, toString_WithOptions)
{
    // Test with formatting options
    QUrl url("file:///home/user/test file.txt");

    QString str = UrlRoute::toString(url, QUrl::PrettyDecoded);

    EXPECT_FALSE(str.isEmpty());
}

// ========== urlToPath() Tests ==========

TEST_F(TestUrlRoute, urlToPath_FileUrl)
{
    // Test converting file URL to path
    QUrl url = QUrl::fromLocalFile("/home/user/test.txt");

    QString path = UrlRoute::urlToPath(url);

    EXPECT_EQ(path, QString("/home/user/test.txt"));
}

TEST_F(TestUrlRoute, urlToPath_CustomScheme)
{
    // Test custom scheme URL to path
    QUrl url;
    url.setScheme(testScheme);
    url.setPath("/test/file.txt");

    QString path = UrlRoute::urlToPath(url);

    EXPECT_FALSE(path.isEmpty());
}

// ========== urlToLocalPath() Tests ==========

TEST_F(TestUrlRoute, urlToLocalPath_FileUrl)
{
    // Test converting file URL to local path
    QUrl url = QUrl::fromLocalFile("/tmp/test.txt");

    QString path = UrlRoute::urlToLocalPath(url);

    EXPECT_EQ(path, QString("/tmp/test.txt"));
}

TEST_F(TestUrlRoute, urlToLocalPath_NonFileUrl)
{
    // Test non-file URL
    QUrl url("http://example.com/path");

    QString path = UrlRoute::urlToLocalPath(url);

    // Should handle gracefully
    EXPECT_TRUE(true);
}

// ========== fromLocalFile() Tests ==========

TEST_F(TestUrlRoute, fromLocalFile_AbsolutePath)
{
    // Test converting absolute path to URL
    QString path = "/home/user/document.txt";

    QUrl url = UrlRoute::fromLocalFile(path);

    EXPECT_TRUE(url.isValid());
    EXPECT_EQ(url.scheme(), QString("file"));
    EXPECT_EQ(url.toLocalFile(), path);
}

TEST_F(TestUrlRoute, fromLocalFile_EmptyPath)
{
    // Test empty path
    QUrl url = UrlRoute::fromLocalFile("");

    EXPECT_TRUE(url.isEmpty() || !url.isValid());
}

// ========== pathToReal() Tests ==========

TEST_F(TestUrlRoute, pathToReal_StandardPath)
{
    // Test converting path to real URL
    QString path = "/home/user/file.txt";

    QUrl url = UrlRoute::pathToReal(path);

    EXPECT_TRUE(url.isValid());
}

// ========== pathToUrl() Tests ==========

TEST_F(TestUrlRoute, pathToUrl_WithScheme)
{
    // Test converting path to URL with specific scheme
    QString path = "/test/file.txt";

    QUrl url = UrlRoute::pathToUrl(path, testScheme);

    EXPECT_TRUE(url.isValid());
    EXPECT_EQ(url.scheme(), testScheme);
}

TEST_F(TestUrlRoute, pathToUrl_FileScheme)
{
    // Test with file scheme
    QString path = "/home/user/document.txt";

    QUrl url = UrlRoute::pathToUrl(path, "file");

    EXPECT_EQ(url.scheme(), QString("file"));
    EXPECT_EQ(url.toLocalFile(), path);
}

// ========== isRootUrl() Tests ==========

TEST_F(TestUrlRoute, isRootUrl_RootUrl)
{
    // Test root URL detection
    QUrl url = UrlRoute::rootUrl(testScheme);

    EXPECT_TRUE(UrlRoute::isRootUrl(url));
}

TEST_F(TestUrlRoute, isRootUrl_NonRootUrl)
{
    // Test non-root URL
    QUrl url;
    url.setScheme(testScheme);
    url.setPath("/subpath/file.txt");

    EXPECT_FALSE(UrlRoute::isRootUrl(url));
}

// ========== isAncestorsUrl() Tests ==========

TEST_F(TestUrlRoute, isAncestorsUrl_DirectParent)
{
    // Test direct parent relationship
    QUrl child("file:///home/user/documents/file.txt");
    QUrl parent("file:///home/user/documents");

    QList<QUrl> list;
    bool result = UrlRoute::isAncestorsUrl(child, parent, &list);

    EXPECT_TRUE(result);
    EXPECT_FALSE(list.isEmpty());
}

TEST_F(TestUrlRoute, isAncestorsUrl_IndirectAncestor)
{
    // Test indirect ancestor relationship
    QUrl descendant("file:///home/user/documents/subdir/file.txt");
    QUrl ancestor("file:///home/user");

    QList<QUrl> list;
    bool result = UrlRoute::isAncestorsUrl(descendant, ancestor, &list);

    EXPECT_TRUE(result);
}

TEST_F(TestUrlRoute, isAncestorsUrl_NotAncestor)
{
    // Test non-ancestor relationship
    QUrl url1("file:///home/user1/file.txt");
    QUrl url2("file:///home/user2");

    QList<QUrl> list;
    bool result = UrlRoute::isAncestorsUrl(url1, url2, &list);

    EXPECT_FALSE(result);
}

// ========== urlParentList() Tests ==========

TEST_F(TestUrlRoute, urlParentList_NestedPath)
{
    // Test getting parent list for nested path
    QUrl url("file:///home/user/documents/subdir/file.txt");

    QList<QUrl> list;
    UrlRoute::urlParentList(url, &list);

    EXPECT_GT(list.size(), 0);
}

TEST_F(TestUrlRoute, urlParentList_RootPath)
{
    // Test parent list for root path
    QUrl url("file:///");

    QList<QUrl> list;
    UrlRoute::urlParentList(url, &list);

    // Root has no parents
    EXPECT_TRUE(list.isEmpty() || list.size() == 0);
}

// ========== fromUserInput() Tests ==========

TEST_F(TestUrlRoute, fromUserInput_FilePath)
{
    // Test user input as file path
    QString input = "/home/user/test.txt";

    QUrl url = UrlRoute::fromUserInput(input);

    EXPECT_TRUE(url.isValid());
}

TEST_F(TestUrlRoute, fromUserInput_HttpUrl)
{
    // Test user input as HTTP URL
    QString input = "http://example.com";

    QUrl url = UrlRoute::fromUserInput(input);

    EXPECT_TRUE(url.isValid());
    EXPECT_EQ(url.scheme(), QString("http"));
}

TEST_F(TestUrlRoute, fromUserInput_RelativePath)
{
    // Test user input as relative path
    QString input = "documents/file.txt";

    QUrl url = UrlRoute::fromUserInput(input);

    EXPECT_TRUE(url.isValid());
}

TEST_F(TestUrlRoute, fromUserInput_WithWorkingDirectory)
{
    // Test with working directory
    QString input = "test.txt";
    QString workingDir = "/home/user";

    QUrl url = UrlRoute::fromUserInput(input, workingDir);

    EXPECT_TRUE(url.isValid());
}

// ========== fromStringList() Tests ==========

TEST_F(TestUrlRoute, fromStringList_ValidUrls)
{
    // Test converting string list to URLs
    QStringList strList = {
        "file:///home/user/file1.txt",
        "file:///home/user/file2.txt",
        "http://example.com"
    };

    QList<QUrl> urls = UrlRoute::fromStringList(strList);

    EXPECT_EQ(urls.size(), 3);

    for (const QUrl &url : urls) {
        EXPECT_TRUE(url.isValid());
    }
}

TEST_F(TestUrlRoute, fromStringList_EmptyList)
{
    // Test empty string list
    QStringList strList;

    QList<QUrl> urls = UrlRoute::fromStringList(strList);

    EXPECT_TRUE(urls.isEmpty());
}

TEST_F(TestUrlRoute, fromStringList_MixedPaths)
{
    // Test mixed paths and URLs
    QStringList strList = {
        "/home/user/file.txt",
        "http://example.com",
        "relative/path.txt"
    };

    QList<QUrl> urls = UrlRoute::fromStringList(strList);

    EXPECT_EQ(urls.size(), 3);
}

// ========== urlsToByteArray() and byteArrayToUrls() Tests ==========

TEST_F(TestUrlRoute, urlsToByteArray_ValidUrls)
{
    // Test converting URLs to byte array
    QList<QUrl> urls = {
        QUrl("file:///home/user/file1.txt"),
        QUrl("file:///home/user/file2.txt")
    };

    QByteArray bytes = UrlRoute::urlsToByteArray(urls);

    EXPECT_FALSE(bytes.isEmpty());
}

TEST_F(TestUrlRoute, byteArrayToUrls_ValidByteArray)
{
    // Test converting byte array to URLs
    QList<QUrl> originalUrls = {
        QUrl("file:///home/user/file1.txt"),
        QUrl("file:///home/user/file2.txt")
    };

    QByteArray bytes = UrlRoute::urlsToByteArray(originalUrls);
    QList<QUrl> restoredUrls = UrlRoute::byteArrayToUrls(bytes);

    EXPECT_EQ(restoredUrls.size(), originalUrls.size());
}

TEST_F(TestUrlRoute, urlsToByteArray_EmptyList)
{
    // Test empty URL list
    QList<QUrl> urls;

    QByteArray bytes = UrlRoute::urlsToByteArray(urls);

    EXPECT_TRUE(bytes.isEmpty() || bytes.size() == 0);
}

TEST_F(TestUrlRoute, byteArrayToUrls_EmptyByteArray)
{
    // Test empty byte array
    QByteArray bytes;

    QList<QUrl> urls = UrlRoute::byteArrayToUrls(bytes);

    EXPECT_TRUE(urls.isEmpty());
}

// ========== Integration Tests ==========

TEST_F(TestUrlRoute, Integration_RoundTripUrlSerialization)
{
    // Test round trip URL serialization
    QList<QUrl> originalUrls = {
        QUrl("file:///home/user/doc1.txt"),
        QUrl("file:///home/user/doc2.txt"),
        QUrl("http://example.com/page.html")
    };

    QByteArray bytes = UrlRoute::urlsToByteArray(originalUrls);
    QList<QUrl> restoredUrls = UrlRoute::byteArrayToUrls(bytes);

    EXPECT_EQ(restoredUrls.size(), originalUrls.size());

    for (int i = 0; i < originalUrls.size(); ++i) {
        EXPECT_EQ(restoredUrls[i], originalUrls[i]);
    }
}

TEST_F(TestUrlRoute, Integration_CompleteSchemeWorkflow)
{
    // Test complete scheme workflow
    QString scheme = "workflow_test";
    QString root = "/workflow/root";
    QString displayName = "Workflow Test";

    // Register
    bool regResult = UrlRoute::regScheme(scheme, root, QIcon(), false, displayName);
    EXPECT_TRUE(regResult);

    // Check existence
    EXPECT_TRUE(UrlRoute::hasScheme(scheme));

    // Get properties
    EXPECT_EQ(UrlRoute::rootPath(scheme), root);
    EXPECT_EQ(UrlRoute::rootDisplayName(scheme), displayName);

    // Create URL
    QUrl url = UrlRoute::rootUrl(scheme);
    EXPECT_EQ(url.scheme(), scheme);

    // Check root
    EXPECT_TRUE(UrlRoute::isRootUrl(url));
}

TEST_F(TestUrlRoute, Integration_PathConversions)
{
    // Test path conversion workflow
    QString originalPath = "/home/user/document.txt";

    // Path to URL
    QUrl url = UrlRoute::fromLocalFile(originalPath);
    EXPECT_TRUE(url.isValid());

    // URL to path
    QString convertedPath = UrlRoute::urlToPath(url);
    EXPECT_EQ(convertedPath, originalPath);
}
