#include "interfaces/dthumbnailprovider.h"

#include <gtest/gtest.h>
#include <QFile>
#include <QMimeType>
#include <QCryptographicHash>
#include <QDir>
#include <QMimeDatabase>

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

        void createThumbnail(QString fileName) {
            QFileInfo info(THUMBNAIL_RESOURCE+fileName);
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
    public:
        DThumbnailProvider *thumbnailProvide;

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

TEST_F(DThumbnailProviderTest, createThumbnailWithDjuv)
{
    createThumbnail("test.djvu");
}

TEST_F(DThumbnailProviderTest, thumbnailFilePath)
{
    QFileInfo info(THUMBNAIL_RESOURCE"logo.png");
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


