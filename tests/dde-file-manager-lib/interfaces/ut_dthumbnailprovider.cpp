/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: yanghao<yanghao@uniontech.com>
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

#include <gtest/gtest.h>

#include <QFile>
#include <QMimeType>
#include <QCryptographicHash>
#include <QDir>
#include <QMimeDatabase>
#include <QSemaphore>
#include <QWaitCondition>
#include <QMutex>

#define private public
#include "fileoperations/filejob.h"
#include "interfaces/dthumbnailprovider.cpp"
#undef private
#include <dfmstandardpaths.h>
#include "dfmapplication.h"
#include <dfmsettings.h>
#include "testhelper.h"

DFM_USE_NAMESPACE

namespace  {
    static bool previewImageImage;
    static bool previewTxtImage;
    static bool previewDocumentImage;
    static bool previewVideoImage;
    class DThumbnailProviderTest: public testing::Test
    {
    public:



        static void SetUpTestCase()
        {
            previewImageImage = DFMApplication::instance()->genericAttribute(DFMApplication::GA_PreviewImage).toBool();
            previewTxtImage = DFMApplication::instance()->genericAttribute(DFMApplication::GA_PreviewTextFile).toBool();
            previewDocumentImage = DFMApplication::instance()->genericAttribute(DFMApplication::GA_PreviewDocumentFile).toBool();
            previewVideoImage = DFMApplication::instance()->genericAttribute(DFMApplication::GA_PreviewVideo).toBool();
            DFMApplication::instance()->setGenericAttribute(DFMApplication::GA_PreviewImage, true);
            DFMApplication::instance()->setGenericAttribute(DFMApplication::GA_PreviewTextFile, true);
            DFMApplication::instance()->setGenericAttribute(DFMApplication::GA_PreviewDocumentFile, true);
            DFMApplication::instance()->setGenericAttribute(DFMApplication::GA_PreviewVideo, true);
        }

        static void TearDownTestCase() {
            DFMApplication::instance()->setGenericAttribute(DFMApplication::GA_PreviewImage, previewImageImage);
            DFMApplication::instance()->setGenericAttribute(DFMApplication::GA_PreviewTextFile, previewTxtImage);
            DFMApplication::instance()->setGenericAttribute(DFMApplication::GA_PreviewDocumentFile, previewDocumentImage);
            DFMApplication::instance()->setGenericAttribute(DFMApplication::GA_PreviewVideo, previewVideoImage);
        }
        void SetUp() override
        {
            std::cout << "start DThumbnailProviderTest";
            thumbnailProvide = DThumbnailProvider::instance();
        }

        void TearDown() override
        {
            std::cout << "end DThumbnailProviderTest";
        }

        QString calculateThumbnailPath(const QFileInfo &info) {
            QString thumbnailPath;
            const QString &absoluteFilePath = info.absoluteFilePath();
            const QString fileUrl = QUrl::fromLocalFile(absoluteFilePath).toString(QUrl::FullyEncoded);
            struct stat st;
            ulong inode = 0;
            QByteArray pathArry = absoluteFilePath.toUtf8();
            std::string pathStd = pathArry.toStdString();
            if (stat(pathStd.c_str(), &st) == 0)
                inode = st.st_ino;

            const QString thumbnailName = QCryptographicHash::hash((fileUrl + QString::number(inode)).toLocal8Bit(), QCryptographicHash::Md5).toHex() + ".png";
            QString normalPath = DFMStandardPaths::location(DFMStandardPaths::ThumbnailNormalPath);
            thumbnailPath = normalPath + QDir::separator() + thumbnailName;
            return thumbnailPath;
        }

        void createThumbnail(const QString &fileName) {
            QFileInfo info(THUMBNAIL_RESOURCE+fileName);
            ASSERT_TRUE(info.exists());
            QString path = thumbnailProvide->createThumbnail(info, DThumbnailProvider::Normal);

            if(!path.isEmpty()) {
                QString saveImage = calculateThumbnailPath(info);
                QFile file(saveImage);
                EXPECT_NO_FATAL_FAILURE(file.exists());
                if (file.exists()) {
                    file.remove();
                }
            }
        }
    public:
        DThumbnailProvider *thumbnailProvide;
        QWaitCondition condition;
        QMutex mutex;
    };
}


TEST_F(DThumbnailProviderTest, defaultSizeLimit)
{
    qint64 defaultSizeLimit = 1024 * 1024 * 20;
    ASSERT_EQ(defaultSizeLimit, thumbnailProvide->defaultSizeLimit());
}

TEST_F(DThumbnailProviderTest, setDefaultSizeLimit)
{
    qint64 defaultSizeLimit = 1024 * 1024 * 40;
    thumbnailProvide->setDefaultSizeLimit(defaultSizeLimit);
    ASSERT_EQ(defaultSizeLimit, thumbnailProvide->defaultSizeLimit());
}


TEST_F(DThumbnailProviderTest, sizeLimit)
{
    qint64 textSizeLimit = 1024 * 1024;
    ASSERT_EQ(textSizeLimit, thumbnailProvide->sizeLimit(QMimeDatabase().mimeTypeForName("text/plain")));
}

TEST_F(DThumbnailProviderTest, setSizeLimit)
{
    qint64 textSizeLimit = 1024 * 1024 * 20;
    QMimeType type = QMimeDatabase().mimeTypeForName("text/plain");
    thumbnailProvide->setSizeLimit(type, textSizeLimit);
    ASSERT_EQ(textSizeLimit, thumbnailProvide->sizeLimit(type));
}


TEST_F(DThumbnailProviderTest, createThumbnailWithImage)
{
    createThumbnail("logo.png");
}

TEST_F(DThumbnailProviderTest, createThumbnailWithTxt)
{
    createThumbnail("hello.txt");
}

TEST_F(DThumbnailProviderTest, createThumbnailWithPdf)
{
    createThumbnail("test.pdf");
}

TEST_F(DThumbnailProviderTest, createThumbnailWithMp4)
{
    createThumbnail("introduction.mp4");
}

/*
TEST_F(DThumbnailProviderTest, createThumbnailWithDjuv)
{
    createThumbnail("test.djvu");
}*/

TEST_F(DThumbnailProviderTest, test_thumbnailFilePath)
{
    QFileInfo info(THUMBNAIL_RESOURCE"logo.png");
    ASSERT_TRUE(info.exists());
    thumbnailProvide->createThumbnail(info, DThumbnailProvider::Normal);
    QString thumbnailFilePath = thumbnailProvide->thumbnailFilePath(info, DThumbnailProvider::Normal);
    QString saveImage = calculateThumbnailPath(info);
    ASSERT_EQ(thumbnailFilePath, saveImage);
    QFile file(saveImage);
    ASSERT_TRUE(file.exists());
    if (file.exists()) {
        file.remove();
    }
}

TEST_F(DThumbnailProviderTest, test_thumbnailFilePathEqSizeToFilePath)
{
    QFileInfo info(THUMBNAIL_RESOURCE"logo.png");
    ASSERT_TRUE(info.exists());
    QString thumbnailPath = thumbnailProvide->createThumbnail(info, DThumbnailProvider::Normal);
    QFileInfo fileInfo(thumbnailPath);
    ASSERT_TRUE(fileInfo.exists());
    EXPECT_EQ(fileInfo.absoluteFilePath(), thumbnailProvide->thumbnailFilePath(fileInfo, DThumbnailProvider::Normal));
    ASSERT_TRUE(QFile(thumbnailPath).remove());
}

TEST_F(DThumbnailProviderTest, test_thumbnailFilePath_file_no_exists)
{
    QFileInfo info(THUMBNAIL_RESOURCE"logo.png");
    ASSERT_TRUE(info.exists());
    QString thumbnailPath = thumbnailProvide->createThumbnail(info, DThumbnailProvider::Normal);
    QFile file(thumbnailPath);
    ASSERT_TRUE(file.exists());
    ASSERT_TRUE(file.remove());
    ASSERT_TRUE(thumbnailProvide->thumbnailFilePath(info, DThumbnailProvider::Normal).isEmpty());
}

TEST_F(DThumbnailProviderTest, test_thumbnailFilePath_fileReadError)
{
    QFileInfo info(THUMBNAIL_RESOURCE"logo.png");
    ASSERT_TRUE(info.exists());
    QString thumbnailPath = thumbnailProvide->createThumbnail(info, DThumbnailProvider::Normal);
    QFile file(thumbnailPath);
    ASSERT_TRUE(file.exists());
    ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QByteArray strBytes= QString("errordata").toUtf8();
    file.write(strBytes,strBytes.length());
    file.close();
    ASSERT_TRUE(thumbnailProvide->thumbnailFilePath(info, DThumbnailProvider::Normal).isEmpty());
}

TEST_F(DThumbnailProviderTest, test_appendToProduceQueue)
{
    QFileInfo pngInfo(THUMBNAIL_RESOURCE"logo.png");
    ASSERT_TRUE(pngInfo.exists());
    QFileInfo txtInfo(THUMBNAIL_RESOURCE"hello.txt");
    ASSERT_TRUE(pngInfo.exists());
    QString savePngImage = calculateThumbnailPath(pngInfo);
    QString saveTxtImage = calculateThumbnailPath(txtInfo);
    QString thumbnailPngPath;
    QString thumbnailTxtPath;
    QSemaphore sem(0);
    thumbnailProvide->appendToProduceQueue(pngInfo, DThumbnailProvider::Normal, [&](const QString & path) {
        thumbnailPngPath = path;
        sem.release();
    });
    thumbnailProvide->appendToProduceQueue(txtInfo, DThumbnailProvider::Normal, [&](const QString & path) {
        thumbnailTxtPath = path;
        sem.release();
    });
    ASSERT_TRUE(sem.tryAcquire(2, 2000));
    ASSERT_EQ(thumbnailPngPath, savePngImage);
    QFile savePngFile(savePngImage);
    ASSERT_TRUE(savePngFile.exists());
    savePngFile.remove();

    ASSERT_EQ(thumbnailTxtPath, saveTxtImage);
    QFile saveTxtFile(saveTxtImage);
    ASSERT_TRUE(saveTxtFile.exists());
    saveTxtFile.remove();
}

TEST_F(DThumbnailProviderTest, test_removeInProduceQueue)
{
    QFileInfo pngInfo(THUMBNAIL_RESOURCE"logo.png");
    ASSERT_TRUE(pngInfo.exists());
    QFileInfo txtInfo(THUMBNAIL_RESOURCE"hello.txt");
    ASSERT_TRUE(pngInfo.exists());
    QString savePngImage = calculateThumbnailPath(pngInfo);
    QString saveTxtImage = calculateThumbnailPath(txtInfo);
    QString thumbnailPngPath;
    QString thumbnailTxtPath;
    QSemaphore sem(0);
    thumbnailProvide->appendToProduceQueue(pngInfo, DThumbnailProvider::Normal, [&](const QString & path) {
        thumbnailPngPath = path;
        sem.release();
        QMutexLocker locker(&mutex);
        condition.notify_one();
    });
    QMutexLocker locker(&mutex);
    condition.wait(&mutex, 2000); // 需要等appendToProduceQueue执行完成才能进行移除
    thumbnailProvide->removeInProduceQueue(pngInfo, DThumbnailProvider::Normal);
    const QPair<QString, DThumbnailProvider::Size> &tmpKey = qMakePair(pngInfo.absoluteFilePath(), DThumbnailProvider::Normal);
    auto discardedProduceInfos = thumbnailProvide->d_func()->discardedProduceInfos;
    ASSERT_TRUE(discardedProduceInfos.contains(tmpKey));
    discardedProduceInfos.remove(tmpKey);
    sem.tryAcquire(2, 2000);
    QFile savePngFile(savePngImage);
    if(savePngFile.exists()) {
        savePngFile.remove();
    }
}

TEST_F(DThumbnailProviderTest, test_errorString)
{
    QString simpleError = "test error";
    thumbnailProvide->d_func()->errorString = simpleError;
    EXPECT_EQ(thumbnailProvide->errorString(), simpleError);
    thumbnailProvide->d_func()->errorString = "";
}

TEST_F(DThumbnailProviderTest, test_hasThumbnail_no_file)
{
   QString tmpDirPath = TestHelper::createTmpDir();
   QFileInfo dirInfo(tmpDirPath);
   ASSERT_TRUE(dirInfo.exists());
   EXPECT_FALSE(thumbnailProvide->hasThumbnail(dirInfo));
   TestHelper::deleteTmpFile(tmpDirPath);
}

TEST_F(DThumbnailProviderTest, test_hasThumbnail_file_size_zero)
{
   QString tmpFilePath = TestHelper::createTmpFile();
   QFileInfo fileInfo(tmpFilePath);
   ASSERT_TRUE(fileInfo.exists());
   EXPECT_FALSE(thumbnailProvide->hasThumbnail(fileInfo));
   TestHelper::deleteTmpFile(tmpFilePath);
}

TEST_F(DThumbnailProviderTest, test_hasThumbnail_video_file_in_copy)
{
   QFileInfo info(THUMBNAIL_RESOURCE"introduction.mp4");
   ASSERT_TRUE(info.exists());
   DUrl url = DUrl::fromLocalFile(info.filePath());
   FileJob::CopyingFiles << url;
   EXPECT_FALSE(thumbnailProvide->hasThumbnail(info));
   FileJob::CopyingFiles.removeOne(url);
}

TEST_F(DThumbnailProviderTest, test_hasThumbnail_fileSizeGtsizeLimit)
{
   qint64 sizeLimit = 1;
   QMimeType type = QMimeDatabase().mimeTypeForName("application/pdf");
   thumbnailProvide->setSizeLimit(type, sizeLimit);
   QFileInfo info(THUMBNAIL_RESOURCE"test.pdf");
   bool exists = info.exists();
   EXPECT_TRUE(exists);
   if(exists) {
    EXPECT_FALSE(thumbnailProvide->hasThumbnail(info));
   }
   thumbnailProvide->setSizeLimit(type, 1024*1024*20);
}

TEST_F(DThumbnailProviderTest, test_hasThumbnail_no_preview)
{
    bool previewImageImage = DFMApplication::instance()->genericAttribute(DFMApplication::GA_PreviewImage).toBool();
    bool previewTxtImage = DFMApplication::instance()->genericAttribute(DFMApplication::GA_PreviewTextFile).toBool();
    bool previewDocumentImage = DFMApplication::instance()->genericAttribute(DFMApplication::GA_PreviewDocumentFile).toBool();
    bool previewVideoImage = DFMApplication::instance()->genericAttribute(DFMApplication::GA_PreviewVideo).toBool();
    DFMApplication::instance()->setGenericAttribute(DFMApplication::GA_PreviewImage, false);
    DFMApplication::instance()->setGenericAttribute(DFMApplication::GA_PreviewTextFile, false);
    DFMApplication::instance()->setGenericAttribute(DFMApplication::GA_PreviewDocumentFile, false);
    DFMApplication::instance()->setGenericAttribute(DFMApplication::GA_PreviewVideo, false);
    QFileInfo info(THUMBNAIL_RESOURCE"logo.png");
    bool exists = info.exists();
    EXPECT_TRUE(exists);
    if(exists) {
        EXPECT_FALSE(thumbnailProvide->hasThumbnail(info));
    }
    QFileInfo txtInfo(THUMBNAIL_RESOURCE"hello.txt");
    exists = txtInfo.exists();
    EXPECT_TRUE(exists);
    if(exists) {
        EXPECT_FALSE(thumbnailProvide->hasThumbnail(txtInfo));
    }
    QFileInfo documentInfo(THUMBNAIL_RESOURCE"test.pdf");
    exists = documentInfo.exists();
    EXPECT_TRUE(exists);
    if(exists) {
        EXPECT_FALSE(thumbnailProvide->hasThumbnail(documentInfo));
    }
    QFileInfo videoInfo(THUMBNAIL_RESOURCE"bumble-bee1.swf");
    exists = videoInfo.exists();
    EXPECT_TRUE(exists);
    if(exists) {
        EXPECT_FALSE(thumbnailProvide->hasThumbnail(videoInfo));
    }
    QFileInfo videoInfo1(THUMBNAIL_RESOURCE"introduction.mp4");
    exists = videoInfo1.exists();
    EXPECT_TRUE(exists);
    if(exists) {
        EXPECT_FALSE(thumbnailProvide->hasThumbnail(videoInfo1));
    }
    DFMApplication::instance()->setGenericAttribute(DFMApplication::GA_PreviewImage, previewImageImage);
    DFMApplication::instance()->setGenericAttribute(DFMApplication::GA_PreviewTextFile, previewTxtImage);
    DFMApplication::instance()->setGenericAttribute(DFMApplication::GA_PreviewDocumentFile, previewDocumentImage);
    DFMApplication::instance()->setGenericAttribute(DFMApplication::GA_PreviewVideo, previewVideoImage);
}

