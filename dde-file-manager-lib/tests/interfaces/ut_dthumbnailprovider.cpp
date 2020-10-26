#include "interfaces/dthumbnailprovider.h"

#include <gtest/gtest.h>
#include <QFile>
#include <QMimeType>
#include <QCryptographicHash>
#include <QDir>

#include <dfmstandardpaths.h>
#include "dfmapplication.h"
#include <dfmsettings.h>

DFM_USE_NAMESPACE

namespace  {
    class DThumbnailProviderTest: public testing::Test
    {
    public:
        void SetUp() override
        {
            std::cout << "start DThumbnailProviderTest";
            thumbnailProvide = DThumbnailProvider::instance();
        }

        void TearDown() override
        {
            std::cout << "end DThumbnailProviderTest";
        }

    public:
        DThumbnailProvider *thumbnailProvide;

    };
}

TEST_F(DThumbnailProviderTest, createThumbnailWithImage)
{
    QFileInfo info(":/test/logo.png");
    ASSERT_TRUE(info.exists());
    thumbnailProvide->createThumbnail(info, DThumbnailProvider::Size::Normal);
    const QString &absoluteFilePath = info.absoluteFilePath();
    const QString fileUrl = QUrl::fromLocalFile(absoluteFilePath).toString(QUrl::FullyEncoded);
    const QString thumbnailName = QCryptographicHash::hash(fileUrl.toLocal8Bit(), QCryptographicHash::Md5).toHex() + ".png";
    QString normalPath = DFMStandardPaths::location(DFMStandardPaths::ThumbnailNormalPath);
    QString saveImage = normalPath + QDir::separator() + thumbnailName;
    QFile file(saveImage);
    ASSERT_TRUE(file.exists());
    if (file.exists()) {
        file.remove();
    }
}

TEST_F(DThumbnailProviderTest, createThumbnailWithTxt)
{
    QFile helloFile(":/test/hello.txt");
    ASSERT_TRUE(helloFile.exists());
    QFileInfo info(helloFile);
     ASSERT_TRUE(info.exists());
    thumbnailProvide->createThumbnail(info, DThumbnailProvider::Size::Normal);
    const QString &absoluteFilePath = info.absoluteFilePath();
    const QString fileUrl = QUrl::fromLocalFile(absoluteFilePath).toString(QUrl::FullyEncoded);
    const QString thumbnailName = QCryptographicHash::hash(fileUrl.toLocal8Bit(), QCryptographicHash::Md5).toHex() + ".png";
    QString normalPath = DFMStandardPaths::location(DFMStandardPaths::ThumbnailNormalPath);
    QString saveImage = normalPath + QDir::separator() + thumbnailName;
    QFile file(saveImage);
    ASSERT_TRUE(file.exists());
    if (file.exists()) {
        file.remove();
    }
}

TEST_F(DThumbnailProviderTest, thumbnailFilePath)
{
    QFileInfo info(":/test/logo.png");
    ASSERT_TRUE(info.exists());
    thumbnailProvide->createThumbnail(info, DThumbnailProvider::Size::Normal);
    QString thumbnailFilePath = thumbnailProvide->thumbnailFilePath(info, DThumbnailProvider::Size::Normal);
    const QString &absoluteFilePath = info.absoluteFilePath();
    const QString fileUrl = QUrl::fromLocalFile(absoluteFilePath).toString(QUrl::FullyEncoded);
    const QString thumbnailName = QCryptographicHash::hash(fileUrl.toLocal8Bit(), QCryptographicHash::Md5).toHex() + ".png";
    QString normalPath = DFMStandardPaths::location(DFMStandardPaths::ThumbnailNormalPath);
    QString saveImage = normalPath + QDir::separator() + thumbnailName;
    ASSERT_EQ(thumbnailFilePath, saveImage);
    QFile file(saveImage);
    ASSERT_TRUE(file.exists());
    if (file.exists()) {
        file.remove();
    }
}
