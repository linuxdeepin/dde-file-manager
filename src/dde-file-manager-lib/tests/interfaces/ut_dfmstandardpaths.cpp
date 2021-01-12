#include <gtest/gtest.h>

#include <QDir>
#include <durl.h>
#include <QDebug>
#include <QApplication>

#define private public
#include "interfaces/dfmstandardpaths.h"
#undef private


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

TEST_F(DFMStandardPathsTest, test_location)
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
    QString translationPath = DFMStandardPaths::location(DFMStandardPaths::StandardLocation::TranslationPath);
    ASSERT_EQ(translationPath, qApp->applicationDirPath() + "/translations");
    QString templatesPath = DFMStandardPaths::location(DFMStandardPaths::StandardLocation::TemplatesPath);
    ASSERT_EQ(templatesPath, qApp->applicationDirPath() + "/templates");
    QString mimeTypePath = DFMStandardPaths::location(DFMStandardPaths::StandardLocation::MimeTypePath);
    ASSERT_EQ(mimeTypePath, qApp->applicationDirPath() + "/mimetypes");
    QString pluginsPath = DFMStandardPaths::location(DFMStandardPaths::StandardLocation::PluginsPath);
    ASSERT_EQ(pluginsPath, QString::fromLocal8Bit(PLUGINDIR).split(':').last());
    QString applicationConfigPath = DFMStandardPaths::location(DFMStandardPaths::StandardLocation::ApplicationConfigPath);
    ASSERT_EQ(applicationConfigPath, QString("%1/%2/deepin/%3").arg(QDir::homePath(), ".config", QMAKE_TARGET));
    QString applicationSharePath = DFMStandardPaths::location(DFMStandardPaths::StandardLocation::ApplicationSharePath);
    ASSERT_EQ(applicationSharePath, APPSHAREDIR);
    QString networkRootPath = DFMStandardPaths::location(DFMStandardPaths::StandardLocation::NetworkRootPath);
    ASSERT_EQ(networkRootPath, NETWORK_ROOT);
    QString userShareRootPath = DFMStandardPaths::location(DFMStandardPaths::StandardLocation::UserShareRootPath);
    ASSERT_EQ(userShareRootPath, USERSHARE_ROOT);
    QString computerRootPath = DFMStandardPaths::location(DFMStandardPaths::StandardLocation::ComputerRootPath);
    ASSERT_EQ(computerRootPath, COMPUTER_ROOT);
}

TEST_F(DFMStandardPathsTest, test_fromStandardUrl)
{
    QString homeStandardPath = DFMStandardPaths::fromStandardUrl(DUrl("standard://home"));
    QString homePath = QDir::homePath();
    ASSERT_EQ(homeStandardPath, homePath);
    QString videosStandardPath = DFMStandardPaths::fromStandardUrl(DUrl("standard://videos"));
    ASSERT_EQ(videosStandardPath, homePath + "/Videos");
}

TEST_F(DFMStandardPathsTest, test_fromStandardUrl_no_standard)
{
    QString noStandardPath = DFMStandardPaths::fromStandardUrl(DUrl("file://videos"));
    ASSERT_TRUE(noStandardPath.isEmpty());
}

TEST_F(DFMStandardPathsTest, test_toStandardUrl)
{
    QString homePath = QDir::homePath();
    DUrl homeStandardUrl = DFMStandardPaths::toStandardUrl(homePath);
    ASSERT_EQ(homeStandardUrl, DUrl("standard://home"));
    DUrl videosStandardUrl = DFMStandardPaths::toStandardUrl(homePath + "/Videos");
    ASSERT_EQ(videosStandardUrl, DUrl("standard://videos"));
}

TEST_F(DFMStandardPathsTest, test_getConfigPath)
{
    QString configPath;
    QString projectName;
#ifndef QMAKE_TARGET
#define QMAKE_TARGET "test-dde-file-manager"
    projectName = QMAKE_TARGET;
    configPath = DFMStandardPaths::getConfigPath();
#undef QMAKE_TARGET
#else
    projectName = QMAKE_TARGET;
    configPath = DFMStandardPaths::getConfigPath();
#endif
    QFile file(configPath);
    ASSERT_TRUE(file.exists());
    ASSERT_EQ(configPath,  QString("%1/%2/deepin/%3").arg(QDir::homePath(), ".config", projectName));
    file.remove();
}

TEST_F(DFMStandardPathsTest, test_DFMStandardPaths)
{
    DFMStandardPaths *p = new DFMStandardPaths();
    ASSERT_NE(p, nullptr);
    delete p;
    p= nullptr;
}
