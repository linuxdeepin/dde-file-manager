// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "models/filters/hiddenfilefilter.h"

#include <QApplication>
#include <QUrl>
#include <QVector>
#include <QSignalSpy>

#include <gtest/gtest.h>

using namespace ddplugin_organizer;

class UT_HiddenFileFilter : public testing::Test
{
protected:
    void SetUp() override
    {
        if (!QApplication::instance()) {
            int argc = 0;
            char **argv = nullptr;
            app = new QApplication(argc, argv);
        }
        filter = new HiddenFileFilter();
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
    HiddenFileFilter *filter;
    QApplication *app = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_HiddenFileFilter, TestConstructor)
{
    EXPECT_NE(filter, nullptr);
    EXPECT_FALSE(filter->showHiddenFiles());  // Default should be false
}

TEST_F(UT_HiddenFileFilter, TestShowHiddenFiles)
{
    // Initially should be false
    EXPECT_FALSE(filter->showHiddenFiles());
    
    // Test that we can call the method multiple times
    EXPECT_FALSE(filter->showHiddenFiles());
    EXPECT_FALSE(filter->showHiddenFiles());
}

TEST_F(UT_HiddenFileFilter, TestAcceptInsert)
{
    // Test with regular file URL
    QUrl regularFile("file:///home/user/document.txt");
    bool result = filter->acceptInsert(regularFile);
    EXPECT_TRUE(result);  // Regular files should be accepted
    
    // Test with hidden file URL
    QUrl hiddenFile("file:///home/user/.hidden_file");
    result = filter->acceptInsert(hiddenFile);
    EXPECT_FALSE(result);  // Hidden files should not be accepted when show=false
    
    // Test with system hidden file
    QUrl systemHidden("file:///home/user/.config");
    result = filter->acceptInsert(systemHidden);
    EXPECT_FALSE(result);  // System hidden files should not be accepted
    
    // Test with invalid URL
    QUrl emptyUrl;
    result = filter->acceptInsert(emptyUrl);
    EXPECT_FALSE(result);  // Invalid URLs should not be accepted
    
    // Test with URL that doesn't start with file://
    QUrl httpUrl("http://example.com/file.txt");
    result = filter->acceptInsert(httpUrl);
    EXPECT_FALSE(result);  // Non-file URLs should not be accepted
}

TEST_F(UT_HiddenFileFilter, TestAcceptReset)
{
    // Test with mixed URLs
    QList<QUrl> inputUrls = {
        QUrl("file:///home/user/document.txt"),
        QUrl("file:///home/user/.hidden_file"),
        QUrl("file:///home/user/image.jpg"),
        QUrl("file:///home/user/.config"),
        QUrl("file:///home/user/video.mp4")
    };
    
    QList<QUrl> result = filter->acceptReset(inputUrls);
    
    // Should filter out hidden files when show=false
    EXPECT_EQ(result.size(), 3);  // Only non-hidden files should remain
    EXPECT_TRUE(result.contains(QUrl("file:///home/user/document.txt")));
    EXPECT_TRUE(result.contains(QUrl("file:///home/user/image.jpg")));
    EXPECT_TRUE(result.contains(QUrl("file:///home/user/video.mp4")));
    EXPECT_FALSE(result.contains(QUrl("file:///home/user/.hidden_file")));
    EXPECT_FALSE(result.contains(QUrl("file:///home/user/.config")));
    
    // Test with empty list
    QList<QUrl> emptyList;
    result = filter->acceptReset(emptyList);
    EXPECT_TRUE(result.isEmpty());
    
    // Test with all hidden files
    QList<QUrl> hiddenOnly = {
        QUrl("file:///home/user/.hidden1"),
        QUrl("file:///home/user/.hidden2"),
        QUrl("file:///home/user/.hidden3")
    };
    result = filter->acceptReset(hiddenOnly);
    EXPECT_TRUE(result.isEmpty());  // All should be filtered out
    
    // Test with all non-hidden files
    QList<QUrl> nonHiddenOnly = {
        QUrl("file:///home/user/file1.txt"),
        QUrl("file:///home/user/file2.txt"),
        QUrl("file:///home/user/file3.txt")
    };
    result = filter->acceptReset(nonHiddenOnly);
    EXPECT_EQ(result.size(), 3);  // All should remain
}

TEST_F(UT_HiddenFileFilter, TestAcceptRename)
{
    // Test renaming non-hidden to non-hidden
    QUrl oldUrl("file:///home/user/old_name.txt");
    QUrl newUrl("file:///home/user/new_name.txt");
    bool result = filter->acceptRename(oldUrl, newUrl);
    EXPECT_TRUE(result);
    
    // Test renaming non-hidden to hidden
    QUrl newHiddenUrl("file:///home/user/.hidden_name.txt");
    result = filter->acceptRename(oldUrl, newHiddenUrl);
    EXPECT_TRUE(result);  // Should accept rename operation
    
    // Test renaming hidden to non-hidden
    QUrl oldHiddenUrl("file:///home/user/.old_hidden.txt");
    result = filter->acceptRename(oldHiddenUrl, newUrl);
    EXPECT_TRUE(result);  // Should accept rename operation
    
    // Test renaming hidden to hidden
    QUrl newHiddenUrl2("file:///home/user/.new_hidden.txt");
    result = filter->acceptRename(oldHiddenUrl, newHiddenUrl2);
    EXPECT_TRUE(result);  // Should accept rename operation
    
    // Test with invalid URLs
    QUrl emptyUrl;
    result = filter->acceptRename(emptyUrl, newUrl);
    EXPECT_FALSE(result);
    
    result = filter->acceptRename(oldUrl, emptyUrl);
    EXPECT_FALSE(result);
    
    result = filter->acceptRename(emptyUrl, emptyUrl);
    EXPECT_FALSE(result);
}

TEST_F(UT_HiddenFileFilter, TestAcceptUpdate)
{
    // Test updating non-hidden file
    QUrl regularFile("file:///home/user/document.txt");
    QVector<int> roles = {1, 2, 3};
    bool result = filter->acceptUpdate(regularFile, roles);
    EXPECT_TRUE(result);  // Should always accept updates
    
    // Test updating hidden file
    QUrl hiddenFile("file:///home/user/.hidden_file");
    result = filter->acceptUpdate(hiddenFile, roles);
    EXPECT_TRUE(result);  // Should always accept updates
    
    // Test with empty roles
    QVector<int> emptyRoles;
    result = filter->acceptUpdate(regularFile, emptyRoles);
    EXPECT_TRUE(result);
    
    // Test with invalid URL
    QUrl emptyUrl;
    result = filter->acceptUpdate(emptyUrl, roles);
    EXPECT_FALSE(result);
}

TEST_F(UT_HiddenFileFilter, TestRefreshModel)
{
    EXPECT_NO_THROW(filter->refreshModel());
    
    // Test multiple calls
    EXPECT_NO_THROW(filter->refreshModel());
    EXPECT_NO_THROW(filter->refreshModel());
    EXPECT_NO_THROW(filter->refreshModel());
}

TEST_F(UT_HiddenFileFilter, TestUpdateFlag)
{
    EXPECT_NO_THROW(filter->updateFlag());
    
    // Test multiple calls
    EXPECT_NO_THROW(filter->updateFlag());
    EXPECT_NO_THROW(filter->updateFlag());
}

TEST_F(UT_HiddenFileFilter, TestHiddenFlagChanged)
{
    EXPECT_NO_THROW(filter->hiddenFlagChanged(true));
    EXPECT_NO_THROW(filter->hiddenFlagChanged(false));
    
    // Test multiple calls with same value
    EXPECT_NO_THROW(filter->hiddenFlagChanged(true));
    EXPECT_NO_THROW(filter->hiddenFlagChanged(true));
    EXPECT_NO_THROW(filter->hiddenFlagChanged(false));
    EXPECT_NO_THROW(filter->hiddenFlagChanged(false));
}

TEST_F(UT_HiddenFileFilter, TestEdgeCases)
{
    // Test URLs with various hidden patterns
    struct TestCase {
        QString url;
        bool shouldBeHidden;
    };
    
    std::vector<TestCase> testCases = {
        {"file:///home/user/.hidden", true},
        {"file:///home/user/file.txt", false},
        {"file:///home/user/.config", true},
        {"file:///home/user/.directory", true},
        {"file:///home/user/document.pdf", false},
        {"file:///home/user/.git", true},
        {"file:///home/user/normal_folder", false},
        {"file:///home/user/.hidden_folder", true},
        {"file:///home/user/file.with.dots", false},  // File with dots in middle is not hidden
        {"file:///home/user/.file.with.dots", true}   // Files starting with dot are hidden
    };
    
    for (const auto& testCase : testCases) {
        QUrl url(testCase.url);
        bool result = filter->acceptInsert(url);
        
        if (testCase.shouldBeHidden) {
            EXPECT_FALSE(result) << "URL should be rejected: " << testCase.url.toStdString();
        } else {
            EXPECT_TRUE(result) << "URL should be accepted: " << testCase.url.toStdString();
        }
    }
}

TEST_F(UT_HiddenFileFilter, TestComplexUrlScenarios)
{
    // Test URLs with query parameters and fragments
    QUrl urlWithQuery("file:///home/user/document.txt?param=value");
    bool result = filter->acceptInsert(urlWithQuery);
    EXPECT_TRUE(result);
    
    QUrl urlWithFragment("file:///home/user/document.txt#section");
    result = filter->acceptInsert(urlWithFragment);
    EXPECT_TRUE(result);
    
    // Test hidden URLs with query parameters
    QUrl hiddenUrlWithQuery("file:///home/user/.hidden.txt?param=value");
    result = filter->acceptInsert(hiddenUrlWithQuery);
    EXPECT_FALSE(result);
    
    // Test different file extensions
    std::vector<QString> extensions = {".txt", ".pdf", ".jpg", ".mp4", ".mp3", ".doc", ".zip"};
    for (const QString& ext : extensions) {
        QUrl regularFile("file:///home/user/regular" + ext);
        QUrl hiddenFile("file:///home/user/.hidden" + ext);
        
        EXPECT_TRUE(filter->acceptInsert(regularFile)) << "Regular file with " << ext.toStdString() << " should be accepted";
        EXPECT_FALSE(filter->acceptInsert(hiddenFile)) << "Hidden file with " << ext.toStdString() << " should be rejected";
    }
}

TEST_F(UT_HiddenFileFilter, TestFilterConsistency)
{
    // Test that filter behavior is consistent
    QList<QUrl> testUrls = {
        QUrl("file:///home/user/file1.txt"),
        QUrl("file:///home/user/.hidden1"),
        QUrl("file:///home/user/file2.txt"),
        QUrl("file:///home/user/.hidden2")
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

TEST_F(UT_HiddenFileFilter, TestPerformanceWithLargeLists)
{
    // Create a large list of URLs
    QList<QUrl> largeList;
    for (int i = 0; i < 1000; ++i) {
        if (i % 3 == 0) {
            largeList.append(QUrl(QString("file:///home/user/.hidden%1.txt").arg(i)));
        } else {
            largeList.append(QUrl(QString("file:///home/user/file%1.txt").arg(i)));
        }
    }
    
    // Test that filtering doesn't crash with large lists
    EXPECT_NO_THROW({
        QList<QUrl> result = filter->acceptReset(largeList);
        EXPECT_GT(result.size(), 0);
        EXPECT_LT(result.size(), largeList.size());  // Some should be filtered out
    });
}

TEST_F(UT_HiddenFileFilter, TestMethodCallsAfterDestructionPreparation)
{
    // Test that methods can be called without crashing before destruction
    EXPECT_NO_THROW(filter->refreshModel());
    EXPECT_NO_THROW(filter->updateFlag());
    EXPECT_NO_THROW(filter->hiddenFlagChanged(true));
    
    // Test accept methods one last time
    QUrl testUrl("file:///home/user/test.txt");
    EXPECT_NO_THROW(filter->acceptInsert(testUrl));
    
    QList<QUrl> testList = {testUrl};
    EXPECT_NO_THROW(filter->acceptReset(testList));
    
    QUrl testUrl2("file:///home/user/test2.txt");
    EXPECT_NO_THROW(filter->acceptRename(testUrl, testUrl2));
    
    QVector<int> roles = {1, 2};
    EXPECT_NO_THROW(filter->acceptUpdate(testUrl, roles));
    
    // Check final state
    EXPECT_FALSE(filter->showHiddenFiles());
}
