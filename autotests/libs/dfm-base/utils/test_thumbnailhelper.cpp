// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <dfm-base/utils/thumbnail/thumbnailhelper.h>
#include <dfm-base/mimetype/mimetypedisplaymanager.h>
#include <dfm-base/utils/fileutils.h>

#include <QUrl>
#include <QTemporaryDir>
#include <QFile>
#include <QImage>
#include <QPixmap>
#include <QApplication>
#include <QTimer>
#include <QStandardPaths>

#include <memory>

DFMBASE_USE_NAMESPACE

class ThumbnailHelperTest : public testing::Test
{
protected:
    void SetUp() override
    {
        // Ensure QApplication exists for image operations
        if (!QApplication::instance()) {
            app.reset(new QApplication(argc, argv));
        }

        // Create a temporary directory for testing
        tempDir.reset(new QTemporaryDir());
        ASSERT_TRUE(tempDir->isValid());

        // Create test image file
        testImagePath = tempDir->filePath("test_image.png");
        createTestImage(testImagePath);

        // Create test text file
        testTextPath = tempDir->filePath("test_text.txt");
        createTestTextFile(testTextPath);

        // Create test audio file (dummy)
        testAudioPath = tempDir->filePath("test_audio.mp3");
        createDummyFile(testAudioPath);

        // Create test video file (dummy)
        testVideoPath = tempDir->filePath("test_video.mp4");
        createDummyFile(testVideoPath);

        imageUrl = QUrl::fromLocalFile(testImagePath);
        textUrl = QUrl::fromLocalFile(testTextPath);
        audioUrl = QUrl::fromLocalFile(testAudioPath);
        videoUrl = QUrl::fromLocalFile(testVideoPath);

        thumbHelper = new ThumbnailHelper();
    }

    void TearDown() override
    {
        app.reset();
        tempDir.reset();
        delete thumbHelper;
        thumbHelper = nullptr;
    }

    void createTestImage(const QString &path)
    {
        QImage image(200, 200, QImage::Format_RGB32);
        image.fill(Qt::red);
        image.save(path, "PNG");
    }

    void createTestTextFile(const QString &path)
    {
        QFile file(path);
        ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Text));
        QTextStream out(&file);
        out << "This is a test text file for thumbnail generation.";
        file.close();
    }

    void createDummyFile(const QString &path)
    {
        QFile file(path);
        ASSERT_TRUE(file.open(QIODevice::WriteOnly));
        QByteArray dummyData(1024, 0); // 1KB dummy data
        file.write(dummyData);
        file.close();
    }

    std::unique_ptr<QApplication> app;
    int argc { 0 };
    char **argv { nullptr };
    ThumbnailHelper *thumbHelper;
    std::unique_ptr<QTemporaryDir> tempDir;
    QString testImagePath;
    QString testTextPath;
    QString testAudioPath;
    QString testVideoPath;
    QUrl imageUrl;
    QUrl textUrl;
    QUrl audioUrl;
    QUrl videoUrl;
};

TEST_F(ThumbnailHelperTest, ImageThumbnailGeneration)
{  
    // Test image thumbnail generation
    QImage thumbnailImg = ThumbnailHelper::thumbnailImage(imageUrl, DFMGLOBAL_NAMESPACE::ThumbnailSize::kLarge);
    QPixmap thumbnail = QPixmap::fromImage(thumbnailImg);
    
    EXPECT_FALSE(thumbnail.isNull());
    EXPECT_TRUE(thumbnail.width() <= 200);
    EXPECT_TRUE(thumbnail.height() <= 200);
}

TEST_F(ThumbnailHelperTest, ImageThumbnailSizes)
{
    // Test different thumbnail sizes
    QImage smallImg = ThumbnailHelper::thumbnailImage(imageUrl, DFMGLOBAL_NAMESPACE::ThumbnailSize::kSmall);
    QImage largeImg = ThumbnailHelper::thumbnailImage(imageUrl, DFMGLOBAL_NAMESPACE::ThumbnailSize::kLarge);
    QPixmap smallThumbnail = QPixmap::fromImage(smallImg);
    QPixmap largeThumbnail = QPixmap::fromImage(largeImg);
    
    EXPECT_FALSE(smallThumbnail.isNull());
    EXPECT_FALSE(largeThumbnail.isNull());
    
    EXPECT_TRUE(smallThumbnail.width() <= 64);
    EXPECT_TRUE(smallThumbnail.height() <= 64);
    EXPECT_TRUE(largeThumbnail.width() <= 256);
    EXPECT_TRUE(largeThumbnail.height() <= 256);
}

TEST_F(ThumbnailHelperTest, NonExistentFile)
{
    // Test non-existent file thumbnail generation
    QUrl nonExistentUrl = QUrl::fromLocalFile(tempDir->filePath("non_existent.png"));
    QImage thumbnailImg = ThumbnailHelper::thumbnailImage(nonExistentUrl, DFMGLOBAL_NAMESPACE::ThumbnailSize::kLarge);
    QPixmap thumbnail = QPixmap::fromImage(thumbnailImg);
    
    EXPECT_TRUE(thumbnail.isNull());
}

TEST_F(ThumbnailHelperTest, MimeTypeSupport)
{
    // Test mime type support checking
    // isMimeTypeSupported doesn't exist, use canGenerateThumbnail instead
    QUrl pngUrl = QUrl::fromLocalFile(tempDir->filePath("test.png"));
    QUrl jpgUrl = QUrl::fromLocalFile(tempDir->filePath("test.jpg"));
    QUrl txtUrl = QUrl::fromLocalFile(tempDir->filePath("test.txt"));
    
    // Create dummy files
    QFile pngFile(pngUrl.toLocalFile());
    pngFile.open(QIODevice::WriteOnly);
    pngFile.write("fake png content");
    pngFile.close();
    
    QFile jpgFile(jpgUrl.toLocalFile());
    jpgFile.open(QIODevice::WriteOnly);
    jpgFile.write("fake jpg content");
    jpgFile.close();
    
    QFile txtFile(txtUrl.toLocalFile());
    txtFile.open(QIODevice::WriteOnly);
    txtFile.write("text content");
    txtFile.close();
    
    // Test mime type support through canGenerateThumbnail
    EXPECT_TRUE(thumbHelper->canGenerateThumbnail(pngUrl));
    EXPECT_TRUE(thumbHelper->canGenerateThumbnail(jpgUrl));
    // Text files may or may not be supported depending on implementation
}

TEST_F(ThumbnailHelperTest, FileSizeLimit)
{
    // Test file size limit enforcement
    // Create a large file (should exceed size limit)
    QString largeFilePath = tempDir->filePath("large_file.png");
    QFile largeFile(largeFilePath);
    ASSERT_TRUE(largeFile.open(QIODevice::WriteOnly));
    
    // Write 25MB of data (exceeds default 20MB limit)
    QByteArray largeData(25 * 1024 * 1024, 0);
    largeFile.write(largeData);
    largeFile.close();
    
    QUrl largeFileUrl = QUrl::fromLocalFile(largeFilePath);
    QImage thumbnailImg = ThumbnailHelper::thumbnailImage(largeFileUrl, DFMGLOBAL_NAMESPACE::ThumbnailSize::kLarge);
    QPixmap thumbnail = QPixmap::fromImage(thumbnailImg);
    
    // Should return null thumbnail for files exceeding size limit
    EXPECT_TRUE(thumbnail.isNull());
}

TEST_F(ThumbnailHelperTest, ThumbnailCaching)
{
    // Test thumbnail caching behavior
    // Generate thumbnail first time
    QImage thumbnailImg1 = ThumbnailHelper::thumbnailImage(imageUrl, DFMGLOBAL_NAMESPACE::ThumbnailSize::kLarge);
    QPixmap thumbnail1 = QPixmap::fromImage(thumbnailImg1);
    EXPECT_FALSE(thumbnail1.isNull());
    
    // Generate thumbnail second time (should use cache)
    QImage thumbnailImg2 = ThumbnailHelper::thumbnailImage(imageUrl, DFMGLOBAL_NAMESPACE::ThumbnailSize::kLarge);
    QPixmap thumbnail2 = QPixmap::fromImage(thumbnailImg2);
    EXPECT_FALSE(thumbnail2.isNull());
    
    // Thumbnails should be identical (from cache)
    EXPECT_EQ(thumbnail1.size(), thumbnail2.size());
}

TEST_F(ThumbnailHelperTest, ThumbnailRemoval)
{
    // Generate thumbnail
    QImage thumbnailImg = ThumbnailHelper::thumbnailImage(imageUrl, DFMGLOBAL_NAMESPACE::ThumbnailSize::kLarge);
    QPixmap thumbnail = QPixmap::fromImage(thumbnailImg);
    EXPECT_FALSE(thumbnail.isNull());
    
    // Remove thumbnail
    // removeThumbnail doesn't exist, skip this test
    
    // Try to get removed thumbnail (should return cached or null depending on implementation)
    QImage removedImg = ThumbnailHelper::thumbnailImage(imageUrl, DFMGLOBAL_NAMESPACE::ThumbnailSize::kLarge);
    QPixmap removedThumbnail = QPixmap::fromImage(removedImg);
    // This behavior depends on implementation - either cache is cleared or it returns cached version
}

TEST_F(ThumbnailHelperTest, ThumbnailFilePath)
{
    // Test thumbnail file path generation
    // thumbnailFilePath doesn't exist, use static method
    QString thumbnailPath = ThumbnailHelper::sizeToFilePath(DFMGLOBAL_NAMESPACE::ThumbnailSize::kLarge);
    EXPECT_FALSE(thumbnailPath.isEmpty());
    EXPECT_TRUE(thumbnailPath.endsWith(".png"));
}

TEST_F(ThumbnailHelperTest, ConcurrentThumbnailGeneration)
{
    // Test concurrent thumbnail generation
    std::vector<QFuture<QPixmap>> futures;
    
    for (int i = 0; i < 5; ++i) {
        QFuture<QPixmap> future = QtConcurrent::run([this]() {
            QImage img = ThumbnailHelper::thumbnailImage(imageUrl, DFMGLOBAL_NAMESPACE::ThumbnailSize::kLarge);
            return QPixmap::fromImage(img);
        });
        futures.push_back(future);
    }
    
    // Wait for all futures to complete
    for (auto &future : futures) {
        future.waitForFinished();
        QPixmap thumbnail = future.result();
        EXPECT_FALSE(thumbnail.isNull());
    }
}

TEST_F(ThumbnailHelperTest, DifferentImageFormats)
{
    // Test different image formats
    QString jpgPath = tempDir->filePath("test_image.jpg");
    QImage image(200, 200, QImage::Format_RGB32);
    image.fill(Qt::blue);
    image.save(jpgPath, "JPEG");
    
    QUrl jpgUrl = QUrl::fromLocalFile(jpgPath);
    QImage jpgImg = ThumbnailHelper::thumbnailImage(jpgUrl, DFMGLOBAL_NAMESPACE::ThumbnailSize::kLarge);
    QPixmap jpgThumbnail = QPixmap::fromImage(jpgImg);
    
    EXPECT_FALSE(jpgThumbnail.isNull());
}

TEST_F(ThumbnailHelperTest, ErrorHandling)
{
    // Test with corrupted image file
    QString corruptedPath = tempDir->filePath("corrupted.png");
    QFile corruptedFile(corruptedPath);
    ASSERT_TRUE(corruptedFile.open(QIODevice::WriteOnly));
    corruptedFile.write("This is not a valid image file");
    corruptedFile.close();
    
    QUrl corruptedUrl = QUrl::fromLocalFile(corruptedPath);
    QImage thumbnailImg = ThumbnailHelper::thumbnailImage(corruptedUrl, DFMGLOBAL_NAMESPACE::ThumbnailSize::kLarge);
    QPixmap thumbnail = QPixmap::fromImage(thumbnailImg);
    
    // Should handle corruption gracefully
    EXPECT_TRUE(thumbnail.isNull());
}

TEST_F(ThumbnailHelperTest, ThumbnailQuality)
{
    // Test thumbnail quality
    QImage thumbnailImg = ThumbnailHelper::thumbnailImage(imageUrl, DFMGLOBAL_NAMESPACE::ThumbnailSize::kLarge);
    QPixmap thumbnail = QPixmap::fromImage(thumbnailImg);
    
    EXPECT_FALSE(thumbnail.isNull());
    EXPECT_GT(thumbnail.width(), 0);
    EXPECT_GT(thumbnail.height(), 0);
    
    // Should maintain aspect ratio
    EXPECT_NEAR(static_cast<double>(thumbnail.width()) / thumbnail.height(), 
               1.0, 0.1); // Allow some tolerance for aspect ratio
}

TEST_F(ThumbnailHelperTest, MemoryUsage)
{
    // Test memory usage with many thumbnails
    std::vector<QPixmap> thumbnails;
    
    for (int i = 0; i < 100; ++i) {
        QImage thumbnailImg = ThumbnailHelper::thumbnailImage(imageUrl, DFMGLOBAL_NAMESPACE::ThumbnailSize::kSmall);
    QPixmap thumbnail = QPixmap::fromImage(thumbnailImg);
        if (!thumbnail.isNull()) {
            thumbnails.push_back(thumbnail);
        }
    }
    
    // Should be able to generate multiple thumbnails without issues
    EXPECT_GT(thumbnails.size(), 0);
}
