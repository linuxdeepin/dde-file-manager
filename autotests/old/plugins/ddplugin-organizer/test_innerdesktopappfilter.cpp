// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "models/filters/innerdesktopappfilter.h"

#include <QApplication>
#include <QUrl>
#include <QMap>
#include <QList>

#include <gtest/gtest.h>

using namespace ddplugin_organizer;

class UT_InnerDesktopAppFilter : public testing::Test
{
protected:
    void SetUp() override
    {
        if (!QApplication::instance()) {
            int argc = 0;
            char **argv = nullptr;
            app = new QApplication(argc, argv);
        }
        filter = new InnerDesktopAppFilter();
    }

    void TearDown() override
    {
        delete filter;
        if (app) {
            delete app;
            app = nullptr;
        }
        stub.clear();
    }

public:
    InnerDesktopAppFilter *filter;
    QApplication *app = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_InnerDesktopAppFilter, TestConstructor)
{
    EXPECT_NE(filter, nullptr);
}

TEST_F(UT_InnerDesktopAppFilter, TestRefreshModel)
{
    EXPECT_NO_THROW(filter->refreshModel());
    
    // Test multiple calls
    EXPECT_NO_THROW(filter->refreshModel());
    EXPECT_NO_THROW(filter->refreshModel());
}

TEST_F(UT_InnerDesktopAppFilter, TestAcceptInsert)
{
    // Test with desktop application URLs
    QUrl desktopApp("file:///usr/share/applications/firefox.desktop");
    bool result = filter->acceptInsert(desktopApp);
    EXPECT_TRUE(result);  // Should accept by default
    
    // Test with regular file URLs
    QUrl regularFile("file:///home/user/document.txt");
    result = filter->acceptInsert(regularFile);
    EXPECT_TRUE(result);  // Should accept non-desktop files
    
    // Test with user desktop applications
    QUrl userDesktopApp("file:///home/user/.local/share/applications/custom.desktop");
    result = filter->acceptInsert(userDesktopApp);
    EXPECT_TRUE(result);
    
    // Test with invalid URL
    QUrl emptyUrl;
    result = filter->acceptInsert(emptyUrl);
    EXPECT_FALSE(result);  // Should reject invalid URLs
    
    // Test with non-file URLs
    QUrl httpUrl("http://example.com/app.desktop");
    result = filter->acceptInsert(httpUrl);
    EXPECT_FALSE(result);  // Should reject non-file URLs
}

TEST_F(UT_InnerDesktopAppFilter, TestAcceptReset)
{
    // Test with mixed URLs
    QList<QUrl> inputUrls = {
        QUrl("file:///usr/share/applications/firefox.desktop"),
        QUrl("file:///home/user/document.txt"),
        QUrl("file:///home/user/.local/share/applications/custom.desktop"),
        QUrl("file:///home/user/image.jpg"),
        QUrl("file:///usr/share/applications/libreoffice-writer.desktop")
    };
    
    QList<QUrl> result = filter->acceptReset(inputUrls);
    
    // Should accept all valid URLs by default
    EXPECT_EQ(result.size(), 5);
    for (const QUrl& url : inputUrls) {
        EXPECT_TRUE(result.contains(url));
    }
    
    // Test with empty list
    QList<QUrl> emptyList;
    result = filter->acceptReset(emptyList);
    EXPECT_TRUE(result.isEmpty());
    
    // Test with only desktop files
    QList<QUrl> desktopOnly = {
        QUrl("file:///usr/share/applications/firefox.desktop"),
        QUrl("file:///usr/share/applications/chromium.desktop"),
        QUrl("file:///home/user/.local/share/applications/custom.desktop")
    };
    result = filter->acceptReset(desktopOnly);
    EXPECT_EQ(result.size(), 3);
    
    // Test with only regular files
    QList<QUrl> regularOnly = {
        QUrl("file:///home/user/file1.txt"),
        QUrl("file:///home/user/file2.txt"),
        QUrl("file:///home/user/file3.txt")
    };
    result = filter->acceptReset(regularOnly);
    EXPECT_EQ(result.size(), 3);
}

TEST_F(UT_InnerDesktopAppFilter, TestAcceptRename)
{
    // Test renaming desktop to desktop
    QUrl oldDesktop("file:///usr/share/applications/old.desktop");
    QUrl newDesktop("file:///usr/share/applications/new.desktop");
    bool result = filter->acceptRename(oldDesktop, newDesktop);
    EXPECT_TRUE(result);
    
    // Test renaming regular to regular
    QUrl oldRegular("file:///home/user/old.txt");
    QUrl newRegular("file:///home/user/new.txt");
    result = filter->acceptRename(oldRegular, newRegular);
    EXPECT_TRUE(result);
    
    // Test renaming desktop to regular
    result = filter->acceptRename(oldDesktop, newRegular);
    EXPECT_TRUE(result);
    
    // Test renaming regular to desktop
    result = filter->acceptRename(oldRegular, newDesktop);
    EXPECT_TRUE(result);
    
    // Test with invalid URLs
    QUrl emptyUrl;
    result = filter->acceptRename(emptyUrl, newDesktop);
    EXPECT_FALSE(result);
    
    result = filter->acceptRename(oldDesktop, emptyUrl);
    EXPECT_FALSE(result);
    
    result = filter->acceptRename(emptyUrl, emptyUrl);
    EXPECT_FALSE(result);
}

TEST_F(UT_InnerDesktopAppFilter, TestDesktopFilePatterns)
{
    // Test various desktop file locations and patterns
    struct TestCase {
        QString url;
        bool shouldBeAccepted;
        QString description;
    };
    
    std::vector<TestCase> testCases = {
        {"file:///usr/share/applications/firefox.desktop", true, "System desktop application"},
        {"file:///usr/local/share/applications/custom.desktop", true, "Local desktop application"},
        {"file:///home/user/.local/share/applications/user.desktop", true, "User desktop application"},
        {"file:///home/user/Desktop/app.desktop", true, "Desktop application file"},
        {"file:///home/user/application.desktop", true, "Home desktop application"},
        {"file:///home/user/document.txt", true, "Regular text file"},
        {"file:///home/user/image.jpg", true, "Regular image file"},
        {"file:///home/user/video.mp4", true, "Regular video file"},
        {"file:///tmp/app.desktop", true, "Temporary desktop file"},
        {"file:///opt/app.desktop", true, "Optional desktop file"}
    };
    
    for (const auto& testCase : testCases) {
        QUrl url(testCase.url);
        bool result = filter->acceptInsert(url);
        
        if (testCase.shouldBeAccepted) {
            EXPECT_TRUE(result) << testCase.description.toStdString() << ": " << testCase.url.toStdString();
        } else {
            EXPECT_FALSE(result) << testCase.description.toStdString() << ": " << testCase.url.toStdString();
        }
    }
}

TEST_F(UT_InnerDesktopAppFilter, TestEdgeCases)
{
    // Test with URLs containing special characters
    QUrl specialChars("file:///home/user/app with spaces.desktop");
    bool result = filter->acceptInsert(specialChars);
    EXPECT_TRUE(result);
    
    QUrl unicodeChars("file:///home/user/应用程序.desktop");
    result = filter->acceptInsert(unicodeChars);
    EXPECT_TRUE(result);
    
    // Test with non-existent desktop files
    QUrl nonExistent("file:///usr/share/applications/nonexistent.desktop");
    result = filter->acceptInsert(nonExistent);
    EXPECT_TRUE(result);  // Should accept even if file doesn't exist
    
    // Test with non-desktop file that has .desktop extension
    QUrl fakeDesktop("file:///home/user/not_really_desktop.desktop.txt");
    result = filter->acceptInsert(fakeDesktop);
    EXPECT_TRUE(result);
    
    // Test with different case extensions
    QUrl upperCase("file:///home/user/APP.DESKTOP");
    result = filter->acceptInsert(upperCase);
    EXPECT_TRUE(result);
}

TEST_F(UT_InnerDesktopAppFilter, TestFilterConsistency)
{
    // Test that filter behavior is consistent
    QList<QUrl> testUrls = {
        QUrl("file:///usr/share/applications/firefox.desktop"),
        QUrl("file:///home/user/document.txt"),
        QUrl("file:///home/user/.local/share/applications/custom.desktop")
    };
    
    // Multiple calls should return the same result
    QList<QUrl> result1 = filter->acceptReset(testUrls);
    QList<QUrl> result2 = filter->acceptReset(testUrls);
    QList<QUrl> result3 = filter->acceptReset(testUrls);
    
    EXPECT_EQ(result1.size(), result2.size());
    EXPECT_EQ(result2.size(), result3.size());
    
    for (const QUrl& url : result1) {
        EXPECT_TRUE(result2.contains(url));
        EXPECT_TRUE(result3.contains(url));
    }
    
    // Test individual acceptInsert consistency
    for (const QUrl& url : testUrls) {
        bool result1 = filter->acceptInsert(url);
        bool result2 = filter->acceptInsert(url);
        bool result3 = filter->acceptInsert(url);
        
        EXPECT_EQ(result1, result2);
        EXPECT_EQ(result2, result3);
    }
}

TEST_F(UT_InnerDesktopAppFilter, TestComplexUrlScenarios)
{
    // Test URLs with query parameters and fragments
    QUrl urlWithQuery("file:///usr/share/applications/firefox.desktop?param=value");
    bool result = filter->acceptInsert(urlWithQuery);
    EXPECT_TRUE(result);
    
    QUrl urlWithFragment("file:///usr/share/applications/firefox.desktop#section");
    result = filter->acceptInsert(urlWithFragment);
    EXPECT_TRUE(result);
    
    // Test with various file extensions
    std::vector<QString> extensions = {".desktop", ".txt", ".pdf", ".jpg", ".mp4", ".mp3", ".doc"};
    for (const QString& ext : extensions) {
        QUrl testUrl("file:///home/user/test" + ext);
        result = filter->acceptInsert(testUrl);
        EXPECT_TRUE(result) << "File with extension " << ext.toStdString() << " should be accepted";
    }
    
    // Test with nested paths
    QUrl nestedPath("file:///home/user/deep/nested/path/app.desktop");
    result = filter->acceptInsert(nestedPath);
    EXPECT_TRUE(result);
    
    // Test with relative paths (should still work)
    QUrl relativePath("app.desktop");
    result = filter->acceptInsert(relativePath);
    EXPECT_FALSE(result);  // Relative paths are typically invalid for file URLs
}

TEST_F(UT_InnerDesktopAppFilter, TestLargeListPerformance)
{
    // Create a large list of URLs
    QList<QUrl> largeList;
    for (int i = 0; i < 1000; ++i) {
        if (i % 2 == 0) {
            largeList.append(QUrl(QString("file:///usr/share/applications/app%1.desktop").arg(i)));
        } else {
            largeList.append(QUrl(QString("file:///home/user/file%1.txt").arg(i)));
        }
    }
    
    // Test that filtering doesn't crash with large lists
    EXPECT_NO_THROW({
        QList<QUrl> result = filter->acceptReset(largeList);
        EXPECT_GT(result.size(), 0);
        // Should accept most if not all URLs
    });
}

TEST_F(UT_InnerDesktopAppFilter, TestSpecialFileTypes)
{
    // Test with various file types that might appear on desktop
    std::vector<QString> fileTypes = {
        "application.desktop",
        "link.desktop",
        "directory.desktop",
        "script.desktop",
        "shortcut.desktop"
    };
    
    for (const QString& fileType : fileTypes) {
        QUrl testUrl("file:///home/user/" + fileType);
        bool result = filter->acceptInsert(testUrl);
        EXPECT_TRUE(result) << "File type " << fileType.toStdString() << " should be accepted";
    }
    
    // Test with some edge case names
    std::vector<QString> edgeCases = {
        ".desktop",           // Hidden desktop file
        "desktop.",           // File starting with desktop
        "file.desk",          // Partial extension
        "file.desktop.txt",   // Double extension
        "desktopfile",        // No extension
        "DESKTOP.desktop",    // Uppercase
        "Desktop.desktop"     // Mixed case
    };
    
    for (const QString& edgeCase : edgeCases) {
        QUrl testUrl("file:///home/user/" + edgeCase);
        bool result = filter->acceptInsert(testUrl);
        EXPECT_TRUE(result) << "Edge case " << edgeCase.toStdString() << " should be accepted";
    }
}

TEST_F(UT_InnerDesktopAppFilter, TestMethodCallsAfterDestructionPreparation)
{
    // Test that methods can be called without crashing before destruction
    EXPECT_NO_THROW(filter->refreshModel());
    
    // Test accept methods one last time
    QUrl testUrl("file:///home/user/test.txt");
    EXPECT_NO_THROW(filter->acceptInsert(testUrl));
    
    QList<QUrl> testList = {testUrl};
    EXPECT_NO_THROW(filter->acceptReset(testList));
    
    QUrl testUrl2("file:///home/user/test2.txt");
    EXPECT_NO_THROW(filter->acceptRename(testUrl, testUrl2));
    
    // Multiple refresh calls should not crash
    EXPECT_NO_THROW(filter->refreshModel());
    EXPECT_NO_THROW(filter->refreshModel());
    EXPECT_NO_THROW(filter->refreshModel());
}
