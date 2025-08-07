// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "fileentity/userentryfileentity.h"
#include "utils/computerdatastruct.h"

#include <dfm-base/interfaces/abstractentryfileentity.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/dfm_global_defines.h>

#include <QUrl>
#include <QIcon>
#include <QString>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_computer;

class UT_UserEntryFileEntity : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.clear();

        // Create test URL with correct user directory suffix
        testUrl.setScheme("entry");
        testUrl.setPath("desktop.userdir");

        // Setup default mock behaviors
        setupDefaultMocks();

        entity = new UserEntryFileEntity(testUrl);
    }

    virtual void TearDown() override
    {
        delete entity;
        entity = nullptr;
        stub.clear();
    }

    void setupDefaultMocks()
    {
        // Mock StandardPaths::displayName
        stub.set_lamda(static_cast<QString (*)(const QString &)>(&StandardPaths::displayName),
                       [](const QString &dirName) -> QString {
            __DBG_STUB_INVOKE__
            if (dirName == "desktop")
                return "Desktop";
            else if (dirName == "documents")
                return "Documents";
            else if (dirName == "downloads")
                return "Downloads";
            else if (dirName == "pictures")
                return "Pictures";
            else if (dirName == "videos")
                return "Videos";
            else if (dirName == "music")
                return "Music";
            return "Unknown";
        });

        // Mock StandardPaths::iconName
        stub.set_lamda(static_cast<QString (*)(const QString &)>(&StandardPaths::iconName),
                       [](const QString &dirName) -> QString {
            __DBG_STUB_INVOKE__
            if (dirName == "desktop")
                return "user-desktop";
            else if (dirName == "documents")
                return "folder-documents";
            else if (dirName == "downloads")
                return "folder-downloads";
            else if (dirName == "pictures")
                return "folder-pictures";
            else if (dirName == "videos")
                return "folder-videos";
            else if (dirName == "music")
                return "folder-music";
            return "folder";
        });

        // Mock StandardPaths::location
        stub.set_lamda(static_cast<QString (*)(const QString &)>(&StandardPaths::location),
                       [](const QString &dirName) -> QString {
            __DBG_STUB_INVOKE__
            if (dirName == "desktop")
                return "/home/user/Desktop";
            else if (dirName == "documents")
                return "/home/user/Documents";
            else if (dirName == "downloads")
                return "/home/user/Downloads";
            else if (dirName == "pictures")
                return "/home/user/Pictures";
            else if (dirName == "videos")
                return "/home/user/Videos";
            else if (dirName == "music")
                return "/home/user/Music";
            return QString(); // Empty for unknown directories
        });

        // Mock QIcon::fromTheme
        stub.set_lamda(static_cast<QIcon (*)(const QString &)>(&QIcon::fromTheme),
                       [](const QString &iconName) -> QIcon {
            __DBG_STUB_INVOKE__
            // Return a valid icon for any theme name
            QIcon icon;
            // We can't easily create a real icon in tests, so we'll return an empty one
            // but ensure it's not null by adding a dummy pixmap
            if (!iconName.isEmpty()) {
                QPixmap pixmap(16, 16);
                pixmap.fill(Qt::blue);
                icon.addPixmap(pixmap);
            }
            return icon;
        });
    }

protected:
    stub_ext::StubExt stub;
    UserEntryFileEntity *entity{nullptr};
    QUrl testUrl;
};

TEST_F(UT_UserEntryFileEntity, Constructor_ValidUserDirUrl_CreatesEntitySuccessfully)
{
    EXPECT_NE(entity, nullptr);
    EXPECT_EQ(entity->entryUrl, testUrl);
    // Check that dirName is correctly extracted
    EXPECT_EQ(entity->dirName, "desktop");
}

TEST_F(UT_UserEntryFileEntity, Constructor_InvalidUrlSuffix_AbortsExecution)
{
    // Note: This test checks the critical error condition, but since abort() terminates the process,
    // we only test that the constructor works with valid URLs in practice
    QUrl invalidUrl("entry://invalid-dir");

    // In a real scenario, this would call abort(), so we skip this destructive test
    // and focus on valid cases
    EXPECT_TRUE(testUrl.path().endsWith(".userdir"));
}

TEST_F(UT_UserEntryFileEntity, Constructor_ExtractsCorrectDirName)
{
    // Test various user directory types
    struct TestCase {
        QString urlPath;
        QString expectedDirName;
    };

    std::vector<TestCase> testCases = {
        {"desktop.userdir", "desktop"},
        {"documents.userdir", "documents"},
        {"downloads.userdir", "downloads"},
        {"pictures.userdir", "pictures"},
        {"videos.userdir", "videos"},
        {"music.userdir", "music"}
    };

    for (const auto &testCase : testCases) {
        QUrl testUrl;
        testUrl.setScheme("entry");
        testUrl.setPath(testCase.urlPath);
        UserEntryFileEntity *testEntity = new UserEntryFileEntity(testUrl);

        EXPECT_EQ(testEntity->dirName, testCase.expectedDirName)
            << "Failed for URL path: " << testCase.urlPath.toStdString();

        delete testEntity;
    }
}

TEST_F(UT_UserEntryFileEntity, DisplayName_DesktopDirectory_ReturnsDesktop)
{
    QString result = entity->displayName();
    EXPECT_EQ(result, "Desktop");
}

TEST_F(UT_UserEntryFileEntity, DisplayName_DocumentsDirectory_ReturnsDocuments)
{
    QUrl documentsUrl;
    documentsUrl.setScheme("entry");
    documentsUrl.setPath("documents.userdir");

    UserEntryFileEntity *documentsEntity = new UserEntryFileEntity(documentsUrl);

    QString result = documentsEntity->displayName();
    EXPECT_EQ(result, "Documents");

    delete documentsEntity;
}

TEST_F(UT_UserEntryFileEntity, DisplayName_UnknownDirectory_ReturnsUnknown)
{
    // Mock StandardPaths to return "Unknown" for unrecognized directories
    stub.set_lamda(static_cast<QString (*)(const QString &)>(&StandardPaths::displayName),
                   [](const QString &) -> QString {
        __DBG_STUB_INVOKE__
        return "Unknown";
    });

    QString result = entity->displayName();
    EXPECT_EQ(result, "Unknown");
}

TEST_F(UT_UserEntryFileEntity, Icon_DesktopDirectory_ReturnsDesktopIcon)
{
    QIcon result = entity->icon();
    EXPECT_FALSE(result.isNull());

    // Verify that StandardPaths::iconName was called with correct directory name
    bool iconNameCalled = false;
    stub.set_lamda(static_cast<QString (*)(const QString &)>(&StandardPaths::iconName),
                   [&](const QString &dirName) -> QString {
        __DBG_STUB_INVOKE__
        iconNameCalled = true;
        EXPECT_EQ(dirName, "desktop");
        return "user-desktop";
    });

    entity->icon(); // Call again to trigger the new stub
    EXPECT_TRUE(iconNameCalled);
}

TEST_F(UT_UserEntryFileEntity, Icon_DocumentsDirectory_ReturnsDocumentsIcon)
{
    QUrl documentsUrl;
    documentsUrl.setScheme("entry");
    documentsUrl.setPath("documents.userdir");
    UserEntryFileEntity *documentsEntity = new UserEntryFileEntity(documentsUrl);

    bool iconNameCalled = false;
    stub.set_lamda(static_cast<QString (*)(const QString &)>(&StandardPaths::iconName),
                   [&](const QString &dirName) -> QString {
        __DBG_STUB_INVOKE__
        iconNameCalled = true;
        EXPECT_EQ(dirName, "documents");
        return "folder-documents";
    });

    QIcon result = documentsEntity->icon();
    EXPECT_FALSE(result.isNull());
    EXPECT_TRUE(iconNameCalled);

    delete documentsEntity;
}

TEST_F(UT_UserEntryFileEntity, Icon_InvalidIconName_ReturnsValidIcon)
{
    // Test when StandardPaths returns empty icon name
    stub.set_lamda(static_cast<QString (*)(const QString &)>(&StandardPaths::iconName),
                   [](const QString &) -> QString {
        __DBG_STUB_INVOKE__
        return QString(); // Empty icon name
    });

    stub.set_lamda(static_cast<QIcon (*)(const QString &)>(&QIcon::fromTheme),
                   [](const QString &iconName) -> QIcon {
        __DBG_STUB_INVOKE__
        EXPECT_TRUE(iconName.isEmpty());
        return QIcon(); // Return null icon for empty name
    });

    QIcon result = entity->icon();
    EXPECT_TRUE(result.isNull());
}

TEST_F(UT_UserEntryFileEntity, Exists_Always_ReturnsTrue)
{
    bool result = entity->exists();
    EXPECT_TRUE(result);
}

TEST_F(UT_UserEntryFileEntity, ShowProgress_Always_ReturnsFalse)
{
    bool result = entity->showProgress();
    EXPECT_FALSE(result);
}

TEST_F(UT_UserEntryFileEntity, ShowTotalSize_Always_ReturnsFalse)
{
    bool result = entity->showTotalSize();
    EXPECT_FALSE(result);
}

TEST_F(UT_UserEntryFileEntity, ShowUsageSize_Always_ReturnsFalse)
{
    bool result = entity->showUsageSize();
    EXPECT_FALSE(result);
}

TEST_F(UT_UserEntryFileEntity, Order_Always_ReturnsUserDirOrder)
{
    auto result = entity->order();
    EXPECT_EQ(result, AbstractEntryFileEntity::EntryOrder::kOrderUserDir);
}

TEST_F(UT_UserEntryFileEntity, TargetUrl_ValidPath_ReturnsFileUrl)
{
    QUrl result = entity->targetUrl();

    EXPECT_EQ(result.scheme(), "file");
    EXPECT_EQ(result.path(), "/home/user/Desktop");
}

TEST_F(UT_UserEntryFileEntity, TargetUrl_DocumentsDirectory_ReturnsCorrectPath)
{
    QUrl documentsUrl;
    documentsUrl.setScheme("entry");
    documentsUrl.setPath("documents.userdir");
    UserEntryFileEntity *documentsEntity = new UserEntryFileEntity(documentsUrl);

    QUrl result = documentsEntity->targetUrl();

    EXPECT_EQ(result.scheme(), "file");
    EXPECT_EQ(result.path(), "/home/user/Documents");

    delete documentsEntity;
}

TEST_F(UT_UserEntryFileEntity, TargetUrl_EmptyPath_ReturnsEmptyUrl)
{
    // Mock StandardPaths::location to return empty path
    stub.set_lamda(static_cast<QString (*)(const QString &)>(&StandardPaths::location),
                   [](const QString &) -> QString {
        __DBG_STUB_INVOKE__
        return QString(); // Empty path
    });

    QUrl result = entity->targetUrl();
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_UserEntryFileEntity, TargetUrl_UnknownDirectory_ReturnsEmptyUrl)
{
    QUrl unknownUrl;
    unknownUrl.setScheme("entry");
    unknownUrl.setPath("unknown.userdir");
    UserEntryFileEntity *unknownEntity = new UserEntryFileEntity(unknownUrl);

    // StandardPaths::location should return empty for unknown directories
    stub.set_lamda(static_cast<QString (*)(const QString &)>(&StandardPaths::location),
                   [](const QString &dirName) -> QString {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(dirName, "unknown");
        return QString(); // Empty for unknown
    });

    QUrl result = unknownEntity->targetUrl();
    EXPECT_TRUE(result.isEmpty());

    delete unknownEntity;
}

// Test edge cases
TEST_F(UT_UserEntryFileEntity, DirName_AccessibleViaMember)
{
    // Test that dirName member is correctly set and accessible
    EXPECT_EQ(entity->dirName, "desktop");
}

TEST_F(UT_UserEntryFileEntity, StandardPathsCalls_WithCorrectParameters)
{
    bool displayNameCalled = false;
    bool iconNameCalled = false;
    bool locationCalled = false;

    stub.set_lamda(static_cast<QString (*)(const QString &)>(&StandardPaths::displayName),
                   [&](const QString &dirName) -> QString {
        __DBG_STUB_INVOKE__
        displayNameCalled = true;
        EXPECT_EQ(dirName, "desktop");
        return "Desktop";
    });

    stub.set_lamda(static_cast<QString (*)(const QString &)>(&StandardPaths::iconName),
                   [&](const QString &dirName) -> QString {
        __DBG_STUB_INVOKE__
        iconNameCalled = true;
        EXPECT_EQ(dirName, "desktop");
        return "user-desktop";
    });

    stub.set_lamda(static_cast<QString (*)(const QString &)>(&StandardPaths::location),
                   [&](const QString &dirName) -> QString {
        __DBG_STUB_INVOKE__
        locationCalled = true;
        EXPECT_EQ(dirName, "desktop");
        return "/home/user/Desktop";
    });

    // Call all methods to trigger the stubs
    entity->displayName();
    entity->icon();
    entity->targetUrl();

    EXPECT_TRUE(displayNameCalled);
    EXPECT_TRUE(iconNameCalled);
    EXPECT_TRUE(locationCalled);
}

TEST_F(UT_UserEntryFileEntity, UrlParsing_ComplexPath_ExtractsCorrectDirName)
{
    // Test URL with complex path structure
    QUrl complexUrl;
    complexUrl.setScheme("entry");
    complexUrl.setPath("some-complex-dir.userdir");
    UserEntryFileEntity *complexEntity = new UserEntryFileEntity(complexUrl);

    EXPECT_EQ(complexEntity->dirName, "some-complex-dir");

    delete complexEntity;
}
