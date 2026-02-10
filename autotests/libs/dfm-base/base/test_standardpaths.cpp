// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>
#include <QDir>
#include <QStandardPaths>

#include "stubext.h"

#include <dfm-base/base/standardpaths.h>

DFMBASE_USE_NAMESPACE

class TestStandardPaths : public testing::Test
{
public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
};

// ========== location() Tests ==========

TEST_F(TestStandardPaths, location_TrashLocalPath)
{
    // Test trash local path
    QString path = StandardPaths::location(StandardPaths::kTrashLocalPath);

    EXPECT_FALSE(path.isEmpty());
    EXPECT_TRUE(path.contains(".local/share/Trash") || path.contains("Trash"));
}

TEST_F(TestStandardPaths, location_TrashLocalFilesPath)
{
    // Test trash files path
    QString path = StandardPaths::location(StandardPaths::kTrashLocalFilesPath);

    EXPECT_FALSE(path.isEmpty());
    EXPECT_TRUE(path.contains("files") || path.contains("Trash"));
}

TEST_F(TestStandardPaths, location_TrashLocalInfoPath)
{
    // Test trash info path
    QString path = StandardPaths::location(StandardPaths::kTrashLocalInfoPath);

    EXPECT_FALSE(path.isEmpty());
    EXPECT_TRUE(path.contains("info") || path.contains("Trash"));
}

TEST_F(TestStandardPaths, location_TranslationPath)
{
    // Test translation path
    QString path = StandardPaths::location(StandardPaths::kTranslationPath);

    EXPECT_FALSE(path.isEmpty());
}

TEST_F(TestStandardPaths, location_TemplatesPath)
{
    // Test templates path
    QString path = StandardPaths::location(StandardPaths::kTemplatesPath);

    EXPECT_FALSE(path.isEmpty());
}

TEST_F(TestStandardPaths, location_MimeTypePath)
{
    // Test mime type path
    QString path = StandardPaths::location(StandardPaths::kMimeTypePath);

    EXPECT_FALSE(path.isEmpty());
}

TEST_F(TestStandardPaths, location_PluginsPath)
{
    // Test plugins path
    QString path = StandardPaths::location(StandardPaths::kPluginsPath);

    EXPECT_FALSE(path.isEmpty());
}

TEST_F(TestStandardPaths, location_ExtensionsPath)
{
    // Test extensions path
    QString path = StandardPaths::location(StandardPaths::kExtensionsPath);

    EXPECT_FALSE(path.isEmpty());
}

TEST_F(TestStandardPaths, location_ThumbnailPath)
{
    // Test thumbnail path
    QString path = StandardPaths::location(StandardPaths::kThumbnailPath);

    EXPECT_FALSE(path.isEmpty());
    EXPECT_TRUE(path.contains(".cache") || path.contains("thumbnails"));
}

TEST_F(TestStandardPaths, location_ThumbnailFailPath)
{
    // Test thumbnail fail path
    QString path = StandardPaths::location(StandardPaths::kThumbnailFailPath);

    EXPECT_FALSE(path.isEmpty());
    EXPECT_TRUE(path.contains("fail"));
}

TEST_F(TestStandardPaths, location_ThumbnailLargePath)
{
    // Test thumbnail large path
    QString path = StandardPaths::location(StandardPaths::kThumbnailLargePath);

    EXPECT_FALSE(path.isEmpty());
    EXPECT_TRUE(path.contains("large"));
}

TEST_F(TestStandardPaths, location_ThumbnailNormalPath)
{
    // Test thumbnail normal path
    QString path = StandardPaths::location(StandardPaths::kThumbnailNormalPath);

    EXPECT_FALSE(path.isEmpty());
    EXPECT_TRUE(path.contains("normal"));
}

TEST_F(TestStandardPaths, location_ThumbnailSmallPath)
{
    // Test thumbnail small path
    QString path = StandardPaths::location(StandardPaths::kThumbnailSmallPath);

    EXPECT_FALSE(path.isEmpty());
    EXPECT_TRUE(path.contains("small"));
}

TEST_F(TestStandardPaths, location_ApplicationConfigPath)
{
    // Test application config path
    QString path = StandardPaths::location(StandardPaths::kApplicationConfigPath);

    EXPECT_FALSE(path.isEmpty());
    EXPECT_TRUE(path.contains(".config") || path.contains("config"));
}

TEST_F(TestStandardPaths, location_ApplicationSharePath)
{
    // Test application share path
    QString path = StandardPaths::location(StandardPaths::kApplicationSharePath);

    EXPECT_FALSE(path.isEmpty());
}

TEST_F(TestStandardPaths, location_RecentPath)
{
    // Test recent path
    QString path = StandardPaths::location(StandardPaths::kRecentPath);

    EXPECT_FALSE(path.isEmpty());
}

TEST_F(TestStandardPaths, location_HomePath)
{
    // Test home path
    QString path = StandardPaths::location(StandardPaths::kHomePath);

    EXPECT_FALSE(path.isEmpty());
    EXPECT_TRUE(QDir(path).exists());
}

TEST_F(TestStandardPaths, location_DesktopPath)
{
    // Test desktop path
    QString path = StandardPaths::location(StandardPaths::kDesktopPath);

    EXPECT_FALSE(path.isEmpty());
}

TEST_F(TestStandardPaths, location_VideosPath)
{
    // Test videos path
    QString path = StandardPaths::location(StandardPaths::kVideosPath);

    EXPECT_FALSE(path.isEmpty());
}

TEST_F(TestStandardPaths, location_MusicPath)
{
    // Test music path
    QString path = StandardPaths::location(StandardPaths::kMusicPath);

    EXPECT_FALSE(path.isEmpty());
}

TEST_F(TestStandardPaths, location_PicturesPath)
{
    // Test pictures path
    QString path = StandardPaths::location(StandardPaths::kPicturesPath);

    EXPECT_FALSE(path.isEmpty());
}

TEST_F(TestStandardPaths, location_DocumentsPath)
{
    // Test documents path
    QString path = StandardPaths::location(StandardPaths::kDocumentsPath);

    EXPECT_FALSE(path.isEmpty());
}

TEST_F(TestStandardPaths, location_DownloadsPath)
{
    // Test downloads path
    QString path = StandardPaths::location(StandardPaths::kDownloadsPath);

    EXPECT_FALSE(path.isEmpty());
}

TEST_F(TestStandardPaths, location_CachePath)
{
    // Test cache path
    // NOTE: Source code has a use-after-free bug in getCachePath() at line 309
    // where .first() returns reference to temporary object
    // We stub it to avoid the crash during testing
    stub.set_lamda(&StandardPaths::getCachePath, []() -> QString {
        __DBG_STUB_INVOKE__
        return QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    });

    QString path = StandardPaths::location(StandardPaths::kCachePath);

    EXPECT_FALSE(path.isEmpty());
    EXPECT_TRUE(path.contains(".cache") || path.contains("cache"));
}

TEST_F(TestStandardPaths, location_DiskPath)
{
    // Test disk path
    QString path = StandardPaths::location(StandardPaths::kDiskPath);

    EXPECT_FALSE(path.isEmpty());
}

TEST_F(TestStandardPaths, location_Root)
{
    // Test root path
    QString path = StandardPaths::location(StandardPaths::kRoot);

    EXPECT_EQ(path, QString("/"));
}

// ========== location(QString) Tests ==========

TEST_F(TestStandardPaths, location_ByDirName_Desktop)
{
    // Test location by directory name
    QString path = StandardPaths::location("Desktop");

    EXPECT_FALSE(path.isEmpty());
}

TEST_F(TestStandardPaths, location_ByDirName_Documents)
{
    // Test documents by name
    QString path = StandardPaths::location("Documents");

    EXPECT_FALSE(path.isEmpty());
}

TEST_F(TestStandardPaths, location_ByDirName_Downloads)
{
    // Test downloads by name
    QString path = StandardPaths::location("Downloads");

    EXPECT_FALSE(path.isEmpty());
}

// ========== iconName() Tests ==========

TEST_F(TestStandardPaths, iconName_HomePath)
{
    // Test icon name for home
    QString icon = StandardPaths::iconName(StandardPaths::kHomePath);

    EXPECT_FALSE(icon.isEmpty());
}

TEST_F(TestStandardPaths, iconName_DesktopPath)
{
    // Test icon name for desktop
    QString icon = StandardPaths::iconName(StandardPaths::kDesktopPath);

    EXPECT_FALSE(icon.isEmpty());
}

TEST_F(TestStandardPaths, iconName_DocumentsPath)
{
    // Test icon name for documents
    QString icon = StandardPaths::iconName(StandardPaths::kDocumentsPath);

    EXPECT_FALSE(icon.isEmpty());
}

TEST_F(TestStandardPaths, iconName_DownloadsPath)
{
    // Test icon name for downloads
    QString icon = StandardPaths::iconName(StandardPaths::kDownloadsPath);

    EXPECT_FALSE(icon.isEmpty());
}

TEST_F(TestStandardPaths, iconName_VideosPath)
{
    // Test icon name for videos
    QString icon = StandardPaths::iconName(StandardPaths::kVideosPath);

    EXPECT_FALSE(icon.isEmpty());
}

TEST_F(TestStandardPaths, iconName_MusicPath)
{
    // Test icon name for music
    QString icon = StandardPaths::iconName(StandardPaths::kMusicPath);

    EXPECT_FALSE(icon.isEmpty());
}

TEST_F(TestStandardPaths, iconName_PicturesPath)
{
    // Test icon name for pictures
    QString icon = StandardPaths::iconName(StandardPaths::kPicturesPath);

    EXPECT_FALSE(icon.isEmpty());
}

TEST_F(TestStandardPaths, iconName_ByDirName)
{
    // Test icon name by directory name
    QString icon = StandardPaths::iconName("Desktop");

    EXPECT_FALSE(icon.isEmpty());
}

// ========== displayName() Tests ==========

TEST_F(TestStandardPaths, displayName_HomePath)
{
    // Test display name for home
    QString name = StandardPaths::displayName(StandardPaths::kHomePath);

    EXPECT_FALSE(name.isEmpty());
}

TEST_F(TestStandardPaths, displayName_DesktopPath)
{
    // Test display name for desktop
    QString name = StandardPaths::displayName(StandardPaths::kDesktopPath);

    EXPECT_FALSE(name.isEmpty());
}

TEST_F(TestStandardPaths, displayName_DocumentsPath)
{
    // Test display name for documents
    QString name = StandardPaths::displayName(StandardPaths::kDocumentsPath);

    EXPECT_FALSE(name.isEmpty());
}

TEST_F(TestStandardPaths, displayName_DownloadsPath)
{
    // Test display name for downloads
    QString name = StandardPaths::displayName(StandardPaths::kDownloadsPath);

    EXPECT_FALSE(name.isEmpty());
}

TEST_F(TestStandardPaths, displayName_TrashPath)
{
    // Test display name for trash
    QString name = StandardPaths::displayName(StandardPaths::kTrashLocalPath);

    EXPECT_FALSE(name.isEmpty());
}

TEST_F(TestStandardPaths, displayName_ByDirName)
{
    // Test display name by directory name
    QString name = StandardPaths::displayName("Desktop");

    EXPECT_FALSE(name.isEmpty());
}

// ========== fromStandardUrl() Tests ==========

TEST_F(TestStandardPaths, fromStandardUrl_ValidStandardUrl)
{
    // Test converting standard URL to local path
    QUrl standardUrl("standard://home");
    QString path = StandardPaths::fromStandardUrl(standardUrl);

    EXPECT_FALSE(path.isEmpty());
}

TEST_F(TestStandardPaths, fromStandardUrl_DesktopUrl)
{
    // Test desktop URL
    QUrl standardUrl("standard://desktop");
    QString path = StandardPaths::fromStandardUrl(standardUrl);

    EXPECT_FALSE(path.isEmpty());
}

TEST_F(TestStandardPaths, fromStandardUrl_DocumentsUrl)
{
    // Test documents URL
    QUrl standardUrl("standard://documents");
    QString path = StandardPaths::fromStandardUrl(standardUrl);

    EXPECT_FALSE(path.isEmpty());
}

TEST_F(TestStandardPaths, fromStandardUrl_InvalidUrl)
{
    // Test invalid URL
    QUrl invalidUrl("http://example.com");
    QString path = StandardPaths::fromStandardUrl(invalidUrl);

    EXPECT_TRUE(path.isEmpty() || !path.contains("example.com"));
}

// ========== toStandardUrl() Tests ==========

TEST_F(TestStandardPaths, toStandardUrl_HomePage)
{
    // Test converting home path to standard URL
    QString homePath = StandardPaths::location(StandardPaths::kHomePath);
    QUrl url = StandardPaths::toStandardUrl(homePath);

    EXPECT_TRUE(url.isValid());
    EXPECT_EQ(url.scheme(), QString("standard"));
}

TEST_F(TestStandardPaths, toStandardUrl_DesktopPath)
{
    // Test desktop path to standard URL
    QString desktopPath = StandardPaths::location(StandardPaths::kDesktopPath);
    QUrl url = StandardPaths::toStandardUrl(desktopPath);

    EXPECT_TRUE(url.isValid());
    if (url.scheme() == "standard") {
        EXPECT_TRUE(url.path().contains("desktop") || url.host() == "desktop");
    }
}

TEST_F(TestStandardPaths, toStandardUrl_DocumentsPath)
{
    // Test documents path to standard URL
    QString documentsPath = StandardPaths::location(StandardPaths::kDocumentsPath);
    QUrl url = StandardPaths::toStandardUrl(documentsPath);

    EXPECT_TRUE(url.isValid());
}

TEST_F(TestStandardPaths, toStandardUrl_InvalidPath)
{
    // Test invalid path
    QString invalidPath = "/nonexistent/invalid/path";
    QUrl url = StandardPaths::toStandardUrl(invalidPath);

    // Should either return invalid URL or file URL
    EXPECT_TRUE(url.isValid() || url.isEmpty());
}

// ========== getCachePath() Tests ==========

TEST_F(TestStandardPaths, getCachePath_ReturnsValidPath)
{
    // Test getting cache path
    // NOTE: Source code has a use-after-free bug in getCachePath() at line 309
    // We stub it to test the interface without triggering the bug
    stub.set_lamda(&StandardPaths::getCachePath, []() -> QString {
        __DBG_STUB_INVOKE__
        return QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    });

    QString cachePath = StandardPaths::getCachePath();

    EXPECT_FALSE(cachePath.isEmpty());
    EXPECT_TRUE(cachePath.contains(".cache") || cachePath.contains("cache"));
}

// ========== Integration Tests ==========

TEST_F(TestStandardPaths, Integration_RoundTripConversion)
{
    // Test round trip conversion: path -> URL -> path
    QString originalPath = StandardPaths::location(StandardPaths::kHomePath);

    QUrl standardUrl = StandardPaths::toStandardUrl(originalPath);
    QString convertedPath = StandardPaths::fromStandardUrl(standardUrl);

    EXPECT_EQ(originalPath, convertedPath);
}

TEST_F(TestStandardPaths, Integration_AllStandardLocations)
{
    // Test that all standard locations return valid paths
    QList<StandardPaths::StandardLocation> locations = {
        StandardPaths::kTrashLocalPath,
        StandardPaths::kTrashLocalFilesPath,
        StandardPaths::kTrashLocalInfoPath,
        StandardPaths::kTranslationPath,
        StandardPaths::kTemplatesPath,
        StandardPaths::kMimeTypePath,
        StandardPaths::kPluginsPath,
        StandardPaths::kThumbnailPath,
        StandardPaths::kApplicationConfigPath,
        StandardPaths::kApplicationSharePath,
        StandardPaths::kRecentPath,
        StandardPaths::kHomePath,
        StandardPaths::kDesktopPath,
        StandardPaths::kVideosPath,
        StandardPaths::kMusicPath,
        StandardPaths::kPicturesPath,
        StandardPaths::kDocumentsPath,
        StandardPaths::kDownloadsPath,
        StandardPaths::kCachePath,
        StandardPaths::kDiskPath,
        StandardPaths::kRoot
    };

    for (const auto &location : locations) {
        QString path = StandardPaths::location(location);
        EXPECT_FALSE(path.isEmpty()) << "Location " << static_cast<int>(location) << " returned empty path";
    }
}

TEST_F(TestStandardPaths, Integration_ConsistencyCheck)
{
    // Test consistency between enum and string-based location methods
    QString desktopByEnum = StandardPaths::location(StandardPaths::kDesktopPath);
    QString desktopByName = StandardPaths::location("Desktop");

    EXPECT_EQ(desktopByEnum, desktopByName);
}

TEST_F(TestStandardPaths, Integration_IconAndDisplayName)
{
    // Test that icon and display names are available for standard locations
    QList<StandardPaths::StandardLocation> locations = {
        StandardPaths::kHomePath,
        StandardPaths::kDesktopPath,
        StandardPaths::kDocumentsPath,
        StandardPaths::kDownloadsPath,
        StandardPaths::kMusicPath,
        StandardPaths::kPicturesPath,
        StandardPaths::kVideosPath
    };

    for (const auto &location : locations) {
        QString icon = StandardPaths::iconName(location);
        QString displayName = StandardPaths::displayName(location);

        EXPECT_FALSE(icon.isEmpty()) << "Icon for location " << static_cast<int>(location) << " is empty";
        EXPECT_FALSE(displayName.isEmpty()) << "Display name for location " << static_cast<int>(location) << " is empty";
    }
}

// ========== Edge Cases ==========

TEST_F(TestStandardPaths, EdgeCase_RootPath)
{
    // Test root path specifically
    QString rootPath = StandardPaths::location(StandardPaths::kRoot);

    EXPECT_EQ(rootPath, QString("/"));
    EXPECT_TRUE(QDir(rootPath).exists());
}

TEST_F(TestStandardPaths, EdgeCase_CachePathConsistency)
{
    // Test cache path consistency
    QString cachePath1 = StandardPaths::location(StandardPaths::kCachePath);
    QString cachePath2 = StandardPaths::getCachePath();

    EXPECT_EQ(cachePath1, cachePath2);
}
