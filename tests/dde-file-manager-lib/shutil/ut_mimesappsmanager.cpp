/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     max-lv<lvwujun@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

