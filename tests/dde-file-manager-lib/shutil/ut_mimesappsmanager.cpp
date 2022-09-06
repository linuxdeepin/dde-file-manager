// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shutil/fileutils.h"

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#define private public
#define protected public

#include "shutil/mimesappsmanager.h"
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
            return FileUtils::filesList(QString("%1/Pictures/Wallpapers").arg(QDir::homePath()));
        }

        QString getPictureHome()
        {
            return (QString("%1/Pictures/Wallpapers").arg(QDir::homePath()));
        }

        QString createOneFile(const QString& filename)
        {
            QString folderPath = getTestFolder();

            QDir dir;
            if (!dir.exists(folderPath))
            {
                dir.mkpath(folderPath);
            }

            QString filePath = QString("%1/%2").arg(folderPath).arg(filename);

            QFile file(filePath);
            QString content= "test for shutil/mimesappsmanager.h";
            //判断文件是否存在
            if(!file.exists())
            {
                file.open(QIODevice::ReadWrite | QIODevice::Text);
                QByteArray str = content.toUtf8();
                file.write(str);
            }

            file.close();
            return filePath;
        }

        // return file folder
        QString getTestFolder()
        {
            return QString("%1/test_shutil_mime").arg(QDir::currentPath());
        }

    };
}

TEST_F(TestMimesAppsManager, get_app_info_from_mimetype)
{
    ASSERT_FALSE(getPicturelist().isEmpty());
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
    ASSERT_FALSE(getPicturelist().isEmpty());
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

TEST_F(TestMimesAppsManager, get_app_manager_worker_object)
{
//    MimesAppsManager* appsManager = new MimesAppsManager();
//    sleep(1);

//    appsManager->m_mimeAppsWorker->handleDirectoryChanged(getPictureHome());

//    QString mimeType_1 = createOneFile("test_mime_1.txt");
//    appsManager->m_mimeAppsWorker->handleFileChanged(mimeType_1);

//    QByteArray databuffer = appsManager->m_mimeAppsWorker->readData(mimeType_1);

//    EXPECT_FALSE( databuffer.isEmpty() );

//    appsManager->m_mimeAppsWorker->writeData(mimeType_1,databuffer );
}

