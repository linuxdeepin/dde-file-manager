#include "shutil/mimesappsmanager.h"
#include "shutil/fileutils.h"

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

namespace  {
    class TestMimesAppsManager : public testing::Test {
    public:
        void SetUp() override
        {
        }
        void TearDown() override
        {
        }

    public:
        QStringList getPicturelist()
        {
            return FileUtils::filesList("/home/max/Pictures/Wallpapers");
        }
    };
}

TEST_F(TestMimesAppsManager, get_app_info_from_mimetype)
{
    QString onePicture = getPicturelist().first();
    QMimeType mimetype = MimesAppsManager::getMimeType(onePicture);
    QString strMimetype = MimesAppsManager::getMimeTypeByFileName(onePicture);

    QString strDefaultApp = MimesAppsManager::getDefaultAppByFileName(onePicture);
    QString defaultAppMimetype = MimesAppsManager::getDefaultAppByMimeType(mimetype);
    QString strdefaultAppMimetype = MimesAppsManager::getDefaultAppByMimeType(strMimetype);

    QString defaultDisplayName =  MimesAppsManager::getDefaultAppDisplayNameByMimeType(mimetype);
    QString defaultDisplayNameGio =  MimesAppsManager::getDefaultAppDisplayNameByGio(strMimetype);
    QString defaultDisplayNameDesttop =  MimesAppsManager::getDefaultAppDesktopFileByMimeType(strMimetype);

    EXPECT_TRUE( defaultDisplayNameDesttop.contains(strDefaultApp));
    EXPECT_TRUE( defaultDisplayNameDesttop.contains(defaultAppMimetype));
    EXPECT_TRUE( defaultDisplayNameDesttop.contains(strdefaultAppMimetype));
    EXPECT_EQ( defaultDisplayName, defaultDisplayNameGio);

    EXPECT_TRUE( MimesAppsManager::setDefautlAppForTypeByGio(strMimetype,defaultDisplayNameDesttop));
}


TEST_F(TestMimesAppsManager, get_app_info_from_mimetype_func2)
{
    QString onePicture = getPicturelist().first();
    QMimeType mimetype = MimesAppsManager::getMimeType(onePicture);
    QString strMimetype = MimesAppsManager::getMimeTypeByFileName(onePicture);

    QStringList recommendApps = MimesAppsManager::getRecommendedApps(DUrl::fromLocalFile( onePicture ));

    QStringList recommendAppGio = MimesAppsManager::getRecommendedAppsByQio(mimetype);
    QStringList recommendAppGioStr = MimesAppsManager::getRecommendedAppsByGio(strMimetype);
    QStringList recommendAppGioUrl = MimesAppsManager::getrecommendedAppsFromMimeWhiteList(DUrl::fromLocalFile( onePicture ));

    EXPECT_FALSE( recommendApps.isEmpty() );
    EXPECT_FALSE( recommendAppGioStr.isEmpty() );
}

TEST_F(TestMimesAppsManager, get_app_info_from_mimetype_func3)
{
    MimesAppsManager::initMimeTypeApps();
    MimesAppsManager::loadDDEMimeTypes();

    EXPECT_FALSE( MimesAppsManager::getApplicationsFolders().isEmpty() );
    EXPECT_FALSE( MimesAppsManager::getMimeAppsCacheFile().isEmpty() );
    EXPECT_FALSE( MimesAppsManager::getMimeInfoCacheFilePath().isEmpty() );
    EXPECT_FALSE( MimesAppsManager::getMimeInfoCacheFileRootPath().isEmpty() );
    EXPECT_FALSE( MimesAppsManager::getDesktopFilesCacheFile().isEmpty() );
    EXPECT_FALSE( MimesAppsManager::getDesktopIconsCacheFile().isEmpty() );
    EXPECT_FALSE( MimesAppsManager::getDesktopFiles().isEmpty() );
    EXPECT_FALSE( MimesAppsManager::getDDEMimeTypeFile().isEmpty() );
    EXPECT_FALSE( MimesAppsManager::getDesktopObjs().isEmpty() );
}


