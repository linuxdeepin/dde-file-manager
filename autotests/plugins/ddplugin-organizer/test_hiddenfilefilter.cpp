// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
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
    // Without registered URL schemes, createFileInfo fails and returns nullptr.
    // HiddenFileFilter accepts all URLs when FileInfo cannot be created.
    QUrl regularFile("file:///home/user/document.txt");
    bool result = filter->acceptInsert(regularFile);
    EXPECT_TRUE(result);

    QUrl hiddenFile("file:///home/user/.hidden_file");
    result = filter->acceptInsert(hiddenFile);
    EXPECT_TRUE(result);  // Can't determine hidden status

    QUrl systemHidden("file:///home/user/.config");
    result = filter->acceptInsert(systemHidden);
    EXPECT_TRUE(result);  // Can't determine hidden status

    QUrl emptyUrl;
    result = filter->acceptInsert(emptyUrl);
    EXPECT_TRUE(result);  // Empty URL also passes through

    QUrl httpUrl("http://example.com/file.txt");
    result = filter->acceptInsert(httpUrl);
    EXPECT_TRUE(result);  // Non-file URL: createFileInfo fails, accepts
}

TEST_F(UT_HiddenFileFilter, TestAcceptReset)
{
    // Without createFileInfo, all URLs pass through the filter
    QList<QUrl> inputUrls = {
        QUrl("file:///home/user/document.txt"),
        QUrl("file:///home/user/.hidden_file"),
        QUrl("file:///home/user/image.jpg"),
        QUrl("file:///home/user/.config"),
        QUrl("file:///home/user/video.mp4")
    };

    QList<QUrl> result = filter->acceptReset(inputUrls);
    // All URLs pass through when createFileInfo fails
    EXPECT_EQ(result.size(), 5);

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
    // All pass through when createFileInfo fails
    EXPECT_EQ(result.size(), 3);

    // Test with all non-hidden files
    QList<QUrl> nonHiddenOnly = {
        QUrl("file:///home/user/file1.txt"),
        QUrl("file:///home/user/file2.txt"),
        QUrl("file:///home/user/file3.txt")
    };
    result = filter->acceptReset(nonHiddenOnly);
    EXPECT_EQ(result.size(), 3);
}

TEST_F(UT_HiddenFileFilter, TestAcceptRename)
{
    QUrl oldUrl("file:///home/user/old_name.txt");
    QUrl newUrl("file:///home/user/new_name.txt");
    bool result = filter->acceptRename(oldUrl, newUrl);
    EXPECT_TRUE(result);  // createFileInfo fails, accepts

    QUrl newHiddenUrl("file:///home/user/.hidden_name.txt");
    result = filter->acceptRename(oldUrl, newHiddenUrl);
    EXPECT_TRUE(result);

    QUrl oldHiddenUrl("file:///home/user/.old_hidden.txt");
    result = filter->acceptRename(oldHiddenUrl, newUrl);
    EXPECT_TRUE(result);

    QUrl newHiddenUrl2("file:///home/user/.new_hidden.txt");
    result = filter->acceptRename(oldHiddenUrl, newHiddenUrl2);
    EXPECT_TRUE(result);

    QUrl emptyUrl;
    result = filter->acceptRename(emptyUrl, newUrl);
    EXPECT_TRUE(result);  // Empty URL also passes through

    result = filter->acceptRename(oldUrl, emptyUrl);
    EXPECT_TRUE(result);

    result = filter->acceptRename(emptyUrl, emptyUrl);
    EXPECT_TRUE(result);
}

TEST_F(UT_HiddenFileFilter, TestAcceptUpdate)
{
    QUrl regularFile("file:///home/user/document.txt");
    QVector<int> roles = {1, 2, 3};
    bool result = filter->acceptUpdate(regularFile, roles);
    EXPECT_TRUE(result);

    QUrl hiddenFile("file:///home/user/.hidden_file");
    result = filter->acceptUpdate(hiddenFile, roles);
    EXPECT_TRUE(result);

    QVector<int> emptyRoles;
    result = filter->acceptUpdate(regularFile, emptyRoles);
    EXPECT_TRUE(result);

    QUrl emptyUrl;
    result = filter->acceptUpdate(emptyUrl, roles);
    EXPECT_TRUE(result);
}

TEST_F(UT_HiddenFileFilter, TestRefreshModel)
{
    EXPECT_NO_THROW(filter->refreshModel());
    EXPECT_NO_THROW(filter->refreshModel());
    EXPECT_NO_THROW(filter->refreshModel());
}

TEST_F(UT_HiddenFileFilter, TestUpdateFlag)
{
    EXPECT_NO_THROW(filter->updateFlag());
    EXPECT_NO_THROW(filter->updateFlag());
    EXPECT_NO_THROW(filter->updateFlag());
}

TEST_F(UT_HiddenFileFilter, TestHiddenFlagChanged)
{
    EXPECT_NO_THROW(filter->hiddenFlagChanged(true));
    EXPECT_NO_THROW(filter->hiddenFlagChanged(false));
    EXPECT_NO_THROW(filter->hiddenFlagChanged(true));
    EXPECT_NO_THROW(filter->hiddenFlagChanged(true));
    EXPECT_NO_THROW(filter->hiddenFlagChanged(false));
    EXPECT_NO_THROW(filter->hiddenFlagChanged(false));
}

TEST_F(UT_HiddenFileFilter, TestEdgeCases)
{
    // Without createFileInfo, all URLs are accepted regardless of hidden pattern
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
        {"file:///home/user/file.with.dots", false},
        {"file:///home/user/.file.with.dots", true}
    };

    for (const auto& testCase : testCases) {
        QUrl url(testCase.url);
        bool result = filter->acceptInsert(url);
        // Without FileInfo, all URLs are accepted
        EXPECT_TRUE(result) << "URL should be accepted (no FileInfo): " << testCase.url.toStdString();
    }
}

TEST_F(UT_HiddenFileFilter, TestComplexUrlScenarios)
{
    // Without createFileInfo, all URLs are accepted
    QUrl urlWithQuery("file:///home/user/document.txt?param=value");
    bool result = filter->acceptInsert(urlWithQuery);
    EXPECT_TRUE(result);

    QUrl urlWithFragment("file:///home/user/document.txt#section");
    result = filter->acceptInsert(urlWithFragment);
    EXPECT_TRUE(result);

    QUrl hiddenUrlWithQuery("file:///home/user/.hidden.txt?param=value");
    result = filter->acceptInsert(hiddenUrlWithQuery);
    EXPECT_TRUE(result);  // Can't determine hidden status

    std::vector<QString> extensions = {".txt", ".pdf", ".jpg", ".mp4", ".mp3", ".doc", ".zip"};
    for (const QString& ext : extensions) {
        QUrl regularFile("file:///home/user/regular" + ext);
        QUrl hiddenFile("file:///home/user/.hidden" + ext);

        EXPECT_TRUE(filter->acceptInsert(regularFile)) << "Regular file with " << ext.toStdString() << " should be accepted";
        EXPECT_TRUE(filter->acceptInsert(hiddenFile)) << "Hidden file with " << ext.toStdString() << " passes through (no FileInfo)";
    }
}

TEST_F(UT_HiddenFileFilter, TestFilterConsistency)
{
    QList<QUrl> testUrls = {
        QUrl("file:///home/user/file1.txt"),
        QUrl("file:///home/user/.hidden1"),
        QUrl("file:///home/user/file2.txt"),
        QUrl("file:///home/user/.hidden2")
    };

    QList<QUrl> result1 = filter->acceptReset(testUrls);
    QList<QUrl> result2 = filter->acceptReset(testUrls);
    QList<QUrl> result3 = filter->acceptReset(testUrls);

    EXPECT_EQ(result1.size(), result2.size());
    EXPECT_EQ(result2.size(), result3.size());

    for (const QUrl& url : result1) {
        EXPECT_TRUE(result2.contains(url));
        EXPECT_TRUE(result3.contains(url));
    }

    for (const QUrl& url : testUrls) {
        bool r1 = filter->acceptInsert(url);
        bool r2 = filter->acceptInsert(url);
        bool r3 = filter->acceptInsert(url);

        EXPECT_EQ(r1, r2);
        EXPECT_EQ(r2, r3);
    }
}

TEST_F(UT_HiddenFileFilter, TestPerformanceWithLargeLists)
{
    QList<QUrl> largeList;
    for (int i = 0; i < 1000; ++i) {
        if (i % 3 == 0) {
            largeList.append(QUrl(QString("file:///home/user/.hidden%1.txt").arg(i)));
        } else {
            largeList.append(QUrl(QString("file:///home/user/file%1.txt").arg(i)));
        }
    }

    // Without createFileInfo, all URLs pass through
    EXPECT_NO_THROW({
        QList<QUrl> result = filter->acceptReset(largeList);
        EXPECT_EQ(result.size(), largeList.size());  // All pass through
    });
}

TEST_F(UT_HiddenFileFilter, TestMethodCallsAfterDestructionPreparation)
{
    EXPECT_NO_THROW(filter->refreshModel());
    EXPECT_NO_THROW(filter->updateFlag());
    EXPECT_NO_THROW(filter->hiddenFlagChanged(true));

    QUrl testUrl("file:///home/user/test.txt");
    EXPECT_NO_THROW(filter->acceptInsert(testUrl));

    QList<QUrl> testList = {testUrl};
    EXPECT_NO_THROW(filter->acceptReset(testList));

    QUrl testUrl2("file:///home/user/test2.txt");
    EXPECT_NO_THROW(filter->acceptRename(testUrl, testUrl2));

    QVector<int> roles = {1, 2};
    EXPECT_NO_THROW(filter->acceptUpdate(testUrl, roles));

    EXPECT_FALSE(filter->showHiddenFiles());
}
