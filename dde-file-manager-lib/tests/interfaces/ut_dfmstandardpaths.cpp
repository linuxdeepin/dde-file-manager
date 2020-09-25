#include "interfaces/dfmstandardpaths.h"

#include <gtest/gtest.h>

#include <QDir>
#include <durl.h>
#include <QDebug>

namespace
{
    class DFMStandardPathsTest: public testing::Test
    {
    public:
        void SetUp() override
        {
            std::cout << "start DFMStandardPathsTest";
        }

        void TearDown() override
        {
            std::cout << "end DFMStandardPathsTest";
        }
    };
}

TEST_F(DFMStandardPathsTest, location)
{
    QString homePath = QDir::homePath();
    QString trashPath = DFMStandardPaths::location(DFMStandardPaths::StandardLocation::TrashPath);
    ASSERT_EQ(homePath + "/.local/share/Trash", trashPath);
    QString trashFilesPath = DFMStandardPaths::location(DFMStandardPaths::StandardLocation::TrashFilesPath);
    ASSERT_EQ(homePath + "/.local/share/Trash/files", trashFilesPath);
    QString trashInfosPath = DFMStandardPaths::location(DFMStandardPaths::StandardLocation::TrashInfosPath);
    ASSERT_EQ(homePath + "/.local/share/Trash/info", trashInfosPath);
    QString thumbnailPath = DFMStandardPaths::location(DFMStandardPaths::StandardLocation::ThumbnailPath);
    ASSERT_EQ(homePath + "/.cache/thumbnails", thumbnailPath);
    QString thumbnailFailPath = DFMStandardPaths::location(DFMStandardPaths::StandardLocation::ThumbnailFailPath);
    ASSERT_EQ(homePath + "/.cache/thumbnails/fail", thumbnailFailPath);
    QString thumbnailLargePath = DFMStandardPaths::location(DFMStandardPaths::StandardLocation::ThumbnailLargePath);
    ASSERT_EQ(homePath + "/.cache/thumbnails/large", thumbnailLargePath);
    QString thumbnailNormalPath = DFMStandardPaths::location(DFMStandardPaths::StandardLocation::ThumbnailNormalPath);
    ASSERT_EQ(homePath + "/.cache/thumbnails/normal", thumbnailNormalPath);
    QString thumbnailSmallPath = DFMStandardPaths::location(DFMStandardPaths::StandardLocation::ThumbnailSmallPath);
    ASSERT_EQ(homePath + "/.cache/thumbnails/small", thumbnailSmallPath);
    QString recentPath = DFMStandardPaths::location(DFMStandardPaths::StandardLocation::RecentPath);
    ASSERT_EQ("recent:///", recentPath);
    QString diskPath = DFMStandardPaths::location(DFMStandardPaths::StandardLocation::DiskPath);
    QString cachePath = DFMStandardPaths::location(DFMStandardPaths::StandardLocation::CachePath);
    ASSERT_EQ(homePath + "/.cache/deepin/test-dde-file-manager-lib", cachePath);
    ASSERT_EQ(QDir::rootPath(), diskPath);
    QString rootPath = DFMStandardPaths::location(DFMStandardPaths::StandardLocation::Root);
    ASSERT_EQ("/", rootPath);
    QString vaultPath = DFMStandardPaths::location(DFMStandardPaths::StandardLocation::Vault);
    ASSERT_EQ(DFMVAULT_ROOT, vaultPath);
}

TEST_F(DFMStandardPathsTest, fromStandardUrl)
{

    QString homeStandardPath = DFMStandardPaths::fromStandardUrl(DUrl("standard://home"));
    QString homePath = QDir::homePath();
    ASSERT_EQ(homeStandardPath, homePath);
    QString videosStandardPath = DFMStandardPaths::fromStandardUrl(DUrl("standard://videos"));
    ASSERT_EQ(videosStandardPath, homePath + "/Videos");
}

TEST_F(DFMStandardPathsTest, toStandardUrl)
{
    QString homePath = QDir::homePath();
    DUrl homeStandardUrl = DFMStandardPaths::toStandardUrl(homePath);
    ASSERT_EQ(homeStandardUrl, DUrl("standard://home"));
    DUrl videosStandardUrl = DFMStandardPaths::toStandardUrl(homePath + "/Videos");
    ASSERT_EQ(videosStandardUrl, DUrl("standard://videos"));
}

