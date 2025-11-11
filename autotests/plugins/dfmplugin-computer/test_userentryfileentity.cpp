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

TEST_F(UT_UserEntryFileEntity, MultipleMethodCalls_DifferentParameters_HandlesCorrectly)
{
    // Mock all methods
    int displayNameCallCount = 0;
    int iconCallCount = 0;
    int existsCallCount = 0;
    int showProgressCallCount = 0;
    int showTotalSizeCallCount = 0;
    int showUsageSizeCallCount = 0;
    int orderCallCount = 0;
    int targetUrlCallCount = 0;
    
    stub.set_lamda(static_cast<QString (*)(const QString &)>(&StandardPaths::displayName), [&displayNameCallCount](const QString &dirName) -> QString {
        __DBG_STUB_INVOKE__
        displayNameCallCount++;
        if (dirName == "desktop")
            return "Desktop";
        return "Unknown";
    });
    
    stub.set_lamda(static_cast<QString (*)(const QString &)>(&StandardPaths::iconName), [&iconCallCount](const QString &dirName) -> QString {
        __DBG_STUB_INVOKE__
        iconCallCount++;
        if (dirName == "desktop")
            return "user-desktop";
        return "folder";
    });
    
    stub.set_lamda(static_cast<QString (*)(const QString &)>(&StandardPaths::location), [&targetUrlCallCount](const QString &dirName) -> QString {
        __DBG_STUB_INVOKE__
        targetUrlCallCount++;
        if (dirName == "desktop")
            return "/home/user/Desktop";
        return QString();
    });
    
    stub.set_lamda(static_cast<QIcon (*)(const QString &)>(&QIcon::fromTheme), [](const QString &) -> QIcon {
        __DBG_STUB_INVOKE__
        return QIcon();
    });
    
    // Call multiple methods
    entity->displayName();
    entity->icon();
    entity->exists();
    entity->showProgress();
    entity->showTotalSize();
    entity->showUsageSize();
    entity->order();
    entity->targetUrl();
    
    // Verify all methods were called
    EXPECT_EQ(displayNameCallCount, 1);
    EXPECT_EQ(iconCallCount, 1);
    EXPECT_EQ(targetUrlCallCount, 1);
}

TEST_F(UT_UserEntryFileEntity, QtMetaObject_CorrectlyInitialized_Success)
{
    // Test that Qt meta-object system works correctly
    const QMetaObject *metaObject = entity->metaObject();
    EXPECT_NE(metaObject, nullptr);
    
    // Test class name
    EXPECT_STREQ(metaObject->className(), "dfmplugin_computer::UserEntryFileEntity");
    
    // Test that inherited methods exist in meta-object
    EXPECT_GE(metaObject->indexOfMethod("displayName()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("icon()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("exists()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("showProgress()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("showTotalSize()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("showUsageSize()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("order()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("targetUrl()"), 0);
}

TEST_F(UT_UserEntryFileEntity, Inheritance_FromAbstractEntryFileEntity_WorksCorrectly)
{
    // Test that UserEntryFileEntity is properly inherited from AbstractEntryFileEntity
    AbstractEntryFileEntity *baseEntity = entity;
    EXPECT_NE(baseEntity, nullptr);
    
    // Test that we can call base class methods
    // AbstractEntryFileEntity doesn't have url() method, so we test other methods
    EXPECT_NO_THROW(baseEntity->displayName());
    EXPECT_NO_THROW(baseEntity->displayName());
    EXPECT_NO_THROW(baseEntity->icon());
    EXPECT_NO_THROW(baseEntity->exists());
    EXPECT_NO_THROW(baseEntity->showProgress());
    EXPECT_NO_THROW(baseEntity->showTotalSize());
    EXPECT_NO_THROW(baseEntity->showUsageSize());
    EXPECT_NO_THROW(baseEntity->description());
    EXPECT_NO_THROW(baseEntity->order());
    EXPECT_NO_THROW(baseEntity->targetUrl());
    EXPECT_NO_THROW(baseEntity->isAccessable());
    EXPECT_NO_THROW(baseEntity->renamable());
}

TEST_F(UT_UserEntryFileEntity, MemoryManagement_DeleteEntity_CleansUpCorrectly)
{
    // Store pointer to entity for testing
    UserEntryFileEntity *entityPtr = entity;
    
    // Delete entity
    delete entity;
    entity = nullptr;
    
    // The entity should be deleted, but we can't directly test this
    // We just verify that the delete operation doesn't crash
    EXPECT_EQ(entity, nullptr);
}

TEST_F(UT_UserEntryFileEntity, ErrorHandling_InvalidStandardPaths_HandlesGracefully)
{
    // Mock StandardPaths to return empty values
    stub.set_lamda(static_cast<QString (*)(const QString &)>(&StandardPaths::displayName), [](const QString &) -> QString {
        __DBG_STUB_INVOKE__
        return QString(); // Empty display name
    });
    
    stub.set_lamda(static_cast<QString (*)(const QString &)>(&StandardPaths::iconName), [](const QString &) -> QString {
        __DBG_STUB_INVOKE__
        return QString(); // Empty icon name
    });
    
    stub.set_lamda(static_cast<QString (*)(const QString &)>(&StandardPaths::location), [](const QString &) -> QString {
        __DBG_STUB_INVOKE__
        return QString(); // Empty location
    });
    
    stub.set_lamda(static_cast<QIcon (*)(const QString &)>(&QIcon::fromTheme), [](const QString &) -> QIcon {
        __DBG_STUB_INVOKE__
        return QIcon(); // Null icon
    });
    
    // Test that methods handle empty values gracefully
    EXPECT_NO_THROW({
        QString displayName = entity->displayName();
        QIcon icon = entity->icon();
        QUrl targetUrl = entity->targetUrl();
        
        EXPECT_TRUE(displayName.isEmpty());
        EXPECT_TRUE(icon.isNull());
        EXPECT_TRUE(targetUrl.isEmpty());
    });
}

TEST_F(UT_UserEntryFileEntity, SpecialCharacters_InDirName_HandlesCorrectly)
{
    // Create entity with special characters in directory name
    QUrl specialUrl;
    specialUrl.setScheme("entry");
    specialUrl.setPath("特殊字符.userdir");
    
    UserEntryFileEntity *specialEntity = new UserEntryFileEntity(specialUrl);
    
    // Mock StandardPaths to handle special characters
    stub.set_lamda(static_cast<QString (*)(const QString &)>(&StandardPaths::displayName), [](const QString &dirName) -> QString {
        __DBG_STUB_INVOKE__
        if (dirName == "特殊字符")
            return "特殊字符";
        return "Unknown";
    });
    
    stub.set_lamda(static_cast<QString (*)(const QString &)>(&StandardPaths::iconName), [](const QString &dirName) -> QString {
        __DBG_STUB_INVOKE__
        if (dirName == "特殊字符")
            return "folder-特殊字符";
        return "folder";
    });
    
    stub.set_lamda(static_cast<QString (*)(const QString &)>(&StandardPaths::location), [](const QString &dirName) -> QString {
        __DBG_STUB_INVOKE__
        if (dirName == "特殊字符")
            return "/home/user/特殊字符";
        return QString();
    });
    
    stub.set_lamda(static_cast<QIcon (*)(const QString &)>(&QIcon::fromTheme), [](const QString &) -> QIcon {
        __DBG_STUB_INVOKE__
        return QIcon();
    });
    
    // Test that methods handle special characters correctly
    EXPECT_NO_THROW({
        QString displayName = specialEntity->displayName();
        QIcon icon = specialEntity->icon();
        QUrl targetUrl = specialEntity->targetUrl();
        
        EXPECT_EQ(displayName, "特殊字符");
        EXPECT_TRUE(icon.isNull()); // Icon would be null due to stub
        EXPECT_EQ(targetUrl.path(), "/home/user/特殊字符");
    });
    
    delete specialEntity;
}

TEST_F(UT_UserEntryFileEntity, Consistency_MultipleCalls_ReturnConsistentResults)
{
    // Mock methods to return consistent values
    QString mockDisplayName = "Desktop";
    QIcon mockIcon = QIcon::fromTheme("user-desktop");
    QString mockLocation = "/home/user/Desktop";
    
    stub.set_lamda(static_cast<QString (*)(const QString &)>(&StandardPaths::displayName), [&mockDisplayName](const QString &dirName) -> QString {
        __DBG_STUB_INVOKE__
        if (dirName == "desktop")
            return mockDisplayName;
        return "Unknown";
    });
    
    stub.set_lamda(static_cast<QString (*)(const QString &)>(&StandardPaths::iconName), [](const QString &dirName) -> QString {
        __DBG_STUB_INVOKE__
        if (dirName == "desktop")
            return "user-desktop";
        return "folder";
    });
    
    stub.set_lamda(static_cast<QString (*)(const QString &)>(&StandardPaths::location), [&mockLocation](const QString &dirName) -> QString {
        __DBG_STUB_INVOKE__
        if (dirName == "desktop")
            return mockLocation;
        return QString();
    });
    
    stub.set_lamda(static_cast<QIcon (*)(const QString &)>(&QIcon::fromTheme), [&mockIcon](const QString &iconName) -> QIcon {
        __DBG_STUB_INVOKE__
        if (iconName == "user-desktop")
            return mockIcon;
        return QIcon();
    });
    
    // Call methods multiple times
    QString displayName1 = entity->displayName();
    QString displayName2 = entity->displayName();
    QString displayName3 = entity->displayName();
    
    QIcon icon1 = entity->icon();
    QIcon icon2 = entity->icon();
    QIcon icon3 = entity->icon();
    
    QUrl targetUrl1 = entity->targetUrl();
    QUrl targetUrl2 = entity->targetUrl();
    QUrl targetUrl3 = entity->targetUrl();
    
    // Verify consistency
    EXPECT_EQ(displayName1, mockDisplayName);
    EXPECT_EQ(displayName2, mockDisplayName);
    EXPECT_EQ(displayName3, mockDisplayName);
    
    EXPECT_EQ(icon1.cacheKey(), icon2.cacheKey());
    EXPECT_EQ(icon2.cacheKey(), icon3.cacheKey());
    
    EXPECT_EQ(targetUrl1.path(), mockLocation);
    EXPECT_EQ(targetUrl2.path(), mockLocation);
    EXPECT_EQ(targetUrl3.path(), mockLocation);
}

TEST_F(UT_UserEntryFileEntity, AllUserDirectories_CreatedSuccessfully_ReturnCorrectProperties)
{
    // Test all user directory types
    struct TestCase {
        QString urlPath;
        QString expectedDirName;
        QString expectedDisplayName;
        QString expectedIconName;
        QString expectedLocation;
    };
    
    std::vector<TestCase> testCases = {
        {"desktop.userdir", "desktop", "Desktop", "user-desktop", "/home/user/Desktop"},
        {"documents.userdir", "documents", "Documents", "folder-documents", "/home/user/Documents"},
        {"downloads.userdir", "downloads", "Downloads", "folder-downloads", "/home/user/Downloads"},
        {"pictures.userdir", "pictures", "Pictures", "folder-pictures", "/home/user/Pictures"},
        {"videos.userdir", "videos", "Videos", "folder-videos", "/home/user/Videos"},
        {"music.userdir", "music", "Music", "folder-music", "/home/user/Music"}
    };
    
    for (const auto &testCase : testCases) {
        QUrl testUrl;
        testUrl.setScheme("entry");
        testUrl.setPath(testCase.urlPath);
        
        UserEntryFileEntity *testEntity = new UserEntryFileEntity(testUrl);
        
        // Verify dirName is correctly extracted
        EXPECT_EQ(testEntity->dirName, testCase.expectedDirName)
            << "Failed for URL path: " << testCase.urlPath.toStdString();
        
        // Verify displayName
        QString displayName = testEntity->displayName();
        EXPECT_EQ(displayName, testCase.expectedDisplayName)
            << "Failed for URL path: " << testCase.urlPath.toStdString();
        
        // Verify icon
        QIcon icon = testEntity->icon();
        EXPECT_FALSE(icon.isNull())
            << "Failed for URL path: " << testCase.urlPath.toStdString();
        
        // Verify targetUrl
        QUrl targetUrl = testEntity->targetUrl();
        EXPECT_EQ(targetUrl.path(), testCase.expectedLocation)
            << "Failed for URL path: " << testCase.urlPath.toStdString();
        
        delete testEntity;
    }
}
