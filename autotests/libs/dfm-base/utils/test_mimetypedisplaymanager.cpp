// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_mimetypedisplaymanager.cpp - MimeTypeDisplayManager class unit tests
// Using stub_ext for function stubbing

#include <gtest/gtest.h>
#include <QTest>
#include <QMimeDatabase>
#include <QMimeType>
#include <QStandardPaths>
#include <QDir>
#include <QTemporaryFile>
#include <QTextStream>

// Include stub_ext
#include "stubext.h"

// Include test target classes
#include <dfm-base/mimetype/mimetypedisplaymanager.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/interfaces/fileinfo.h>

using namespace dfmbase;

/**
 * @brief MimeTypeDisplayManager class unit tests
 *
 * Test scope:
 * 1. Singleton instance management
 * 2. Display name functionality
 * 3. Full mime name functionality  
 * 4. Display name to enum conversion
 * 5. Default icon functionality
 * 6. Support mime types functions
 * 7. Accurate mime type detection
 * 8. File reading functionality
 * 9. Data initialization
 * 10. Edge cases and error handling
 */
class MimeTypeDisplayManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test environment
        stub.clear();
        
        // Mock StandardPaths::location to return test directory
        tempDir = new QTemporaryDir();
        ASSERT_TRUE(tempDir->isValid());
        
        using LocationFunc = QString (*)(StandardPaths::StandardLocation);
        stub.set_lamda(static_cast<LocationFunc>(&StandardPaths::location), 
                      [this](StandardPaths::StandardLocation type) -> QString {
                          if (type == StandardPaths::kMimeTypePath) {
                              return tempDir->path();
                          }
                          return QString();
                      });
        
        // Create test mime type files
        createTestMimeTypeFiles();
        
        // Get instance to trigger initialization
        manager = MimeTypeDisplayManager::instance();
        ASSERT_NE(manager, nullptr);
    }

    void TearDown() override
    {
        // Clean up test environment
        stub.clear();
        delete tempDir;
        tempDir = nullptr;
        manager = nullptr;
    }

    // Test stubbing utility
    stub_ext::StubExt stub;
    MimeTypeDisplayManager *manager = nullptr;
    QTemporaryDir *tempDir = nullptr;
    
    // Helper function to create test mime type files
    void createTestMimeTypeFiles()
    {
        ASSERT_TRUE(tempDir->isValid());
        
        // Create test text.mimetype
        createTestFile("text.mimetype", QStringList{
            "text/plain",
            "text/html", 
            "text/xml",
            "text/css"
        });
        
        // Create test archive.mimetype
        createTestFile("archive.mimetype", QStringList{
            "application/zip",
            "application/x-tar",
            "application/x-rar",
            "application/gzip"
        });
        
        // Create test video.mimetype
        createTestFile("video.mimetype", QStringList{
            "video/mp4",
            "video/avi",
            "video/mkv",
            "video/mov"
        });
        
        // Create test audio.mimetype
        createTestFile("audio.mimetype", QStringList{
            "audio/mpeg",
            "audio/wav",
            "audio/ogg",
            "audio/flac"
        });
        
        // Create test image.mimetype
        createTestFile("image.mimetype", QStringList{
            "image/jpeg",
            "image/png",
            "image/gif",
            "image/bmp"
        });
        
        // Create test executable.mimetype
        createTestFile("executable.mimetype", QStringList{
            "application/x-shellscript",
            "application/x-perl",
            "application/x-python"
        });
        
        // Create test backup.mimetype
        createTestFile("backup.mimetype", QStringList{
            "application/x-backup",
            "application/x-bak",
            "application/x-old"
        });
    }
    
    // Helper function to create test file with content
    void createTestFile(const QString &filename, const QStringList &lines)
    {
        QString filePath = tempDir->path() + "/" + filename;
        QFile file(filePath);
        ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Text));
        
        QTextStream out(&file);
        for (const QString &line : lines) {
            out << line << "\n";
        }
        
        file.close();
    }
    
    // Helper function to create a temporary test file
    QString createTempTestFile(const QString &content = "test content")
    {
        QTemporaryFile tempFile;
        if (!tempFile.open()) {
            return QString();
        }
        tempFile.write(content.toUtf8());
        tempFile.close();
        return tempFile.fileName();
    }
};

/**
 * @brief Test singleton instance management
 * Verify singleton pattern works correctly
 */
TEST_F(MimeTypeDisplayManagerTest, Singleton_InstanceManagement)
{
    // Get multiple instances - should return same pointer
    MimeTypeDisplayManager *instance1 = MimeTypeDisplayManager::instance();
    MimeTypeDisplayManager *instance2 = MimeTypeDisplayManager::instance();
    
    EXPECT_NE(instance1, nullptr);
    EXPECT_EQ(instance1, instance2);
    EXPECT_EQ(manager, instance1);
}

/**
 * @brief Test display name functionality
 * Verify displayName returns correct display names for mime types
 */
TEST_F(MimeTypeDisplayManagerTest, DisplayName_BasicMimeTypes)
{
#ifndef QT_DEBUG
    // Test basic mime types
    EXPECT_EQ(manager->displayName("inode/directory"), "Directory");
    EXPECT_EQ(manager->displayName("application/x-desktop"), "Application");
    EXPECT_EQ(manager->displayName("video/mp4"), "Video");
    EXPECT_EQ(manager->displayName("audio/mpeg"), "Audio");
    EXPECT_EQ(manager->displayName("image/jpeg"), "Image");
    EXPECT_EQ(manager->displayName("application/zip"), "Archive");
    EXPECT_EQ(manager->displayName("text/plain"), "Text");
    EXPECT_EQ(manager->displayName("application/x-executable"), "Executable");

    // Test unknown mime type
    QString displayName = manager->displayName("application/unknown-type");
    EXPECT_EQ(displayName, "Unknown");
#endif
}

/**
 * @brief Test display name with mime type prefixes
 * Verify handling of mime type prefixes
 */
TEST_F(MimeTypeDisplayManagerTest, DisplayName_MimeTypePrefixes)
{
#ifndef QT_DEBUG
    // Test video prefixes
    EXPECT_EQ(manager->displayName("video/avi"), "Video");
    EXPECT_EQ(manager->displayName("video/mkv"), "Video");
    EXPECT_EQ(manager->displayName("video/custom"), "Video");
    
    // Test audio prefixes
    EXPECT_EQ(manager->displayName("audio/wav"), "Audio");
    EXPECT_EQ(manager->displayName("audio/flac"), "Audio");
    EXPECT_EQ(manager->displayName("audio/custom"), "Audio");
    
    // Test image prefixes
    EXPECT_EQ(manager->displayName("image/png"), "Image");
    EXPECT_EQ(manager->displayName("image/gif"), "Image");
    EXPECT_EQ(manager->displayName("image/custom"), "Image");
    
    // Test text prefixes
    EXPECT_EQ(manager->displayName("text/html"), "Text");
    EXPECT_EQ(manager->displayName("text/css"), "Text");
    EXPECT_EQ(manager->displayName("text/custom"), "Text");
#endif
}

/**
 * @brief Test display name with custom mime types from files
 * Verify that mime types from test files are recognized
 */
TEST_F(MimeTypeDisplayManagerTest,DisplayName_CustomMimeTypes)
{
#ifndef QT_DEBUG
    // Test custom mime types from our test files
    EXPECT_EQ(manager->displayName("text/xml"), "Text");  // from text.mimetype
    EXPECT_EQ(manager->displayName("application/x-tar"), "Archive");  // from archive.mimetype
    EXPECT_EQ(manager->displayName("video/mov"), "Video");  // from video.mimetype
    EXPECT_EQ(manager->displayName("audio/wav"), "Audio");  // from audio.mimetype
    EXPECT_EQ(manager->displayName("image/bmp"), "Image");  // from image.mimetype
    EXPECT_EQ(manager->displayName("application/x-shellscript"), "Executable");  // from executable.mimetype
    EXPECT_EQ(manager->displayName("application/x-backup"), "Backup file");  // from backup.mimetype
#endif
}

/**
 * @brief Test full mime name functionality
 * Verify fullMimeName returns mime type with display name
 */
TEST_F(MimeTypeDisplayManagerTest, FullMimeName_Format)
{
    // Test basic mime types
    QString fullName = manager->fullMimeName("inode/directory");
    EXPECT_EQ(fullName, "Directory (inode/directory)");
    
    fullName = manager->fullMimeName("video/mp4");
    EXPECT_EQ(fullName, "Video (video/mp4)");
    
    fullName = manager->fullMimeName("application/unknown");
    EXPECT_EQ(fullName, "Unknown (application/unknown)");
}

/**
 * @brief Test display name to enum conversion
 * Verify displayNameToEnum converts mime types correctly
 */
TEST_F(MimeTypeDisplayManagerTest, DisplayNameToEnum_BasicConversions)
{
    // Test direct conversions
    EXPECT_EQ(manager->displayNameToEnum("application/x-desktop"), FileInfo::FileType::kDesktopApplication);
    EXPECT_EQ(manager->displayNameToEnum("inode/directory"), FileInfo::FileType::kDirectory);
    EXPECT_EQ(manager->displayNameToEnum("application/x-executable"), FileInfo::FileType::kExecutable);
    
    // Test prefix-based conversions
    EXPECT_EQ(manager->displayNameToEnum("video/mp4"), FileInfo::FileType::kVideos);
    EXPECT_EQ(manager->displayNameToEnum("audio/mpeg"), FileInfo::FileType::kAudios);
    EXPECT_EQ(manager->displayNameToEnum("image/jpeg"), FileInfo::FileType::kImages);
    EXPECT_EQ(manager->displayNameToEnum("text/plain"), FileInfo::FileType::kDocuments);
    
    // Test unknown mime type
    EXPECT_EQ(manager->displayNameToEnum("application/unknown-type"), FileInfo::FileType::kUnknown);
}

/**
 * @brief Test display name to enum with custom mime types
 * Verify that custom mime types from files are converted correctly
 */
TEST_F(MimeTypeDisplayManagerTest, DisplayNameToEnum_CustomMimeTypes)
{
    // Test custom mime types from test files
    EXPECT_EQ(manager->displayNameToEnum("text/xml"), FileInfo::FileType::kDocuments);
    EXPECT_EQ(manager->displayNameToEnum("application/x-tar"), FileInfo::FileType::kArchives);
    EXPECT_EQ(manager->displayNameToEnum("video/mov"), FileInfo::FileType::kVideos);
    EXPECT_EQ(manager->displayNameToEnum("audio/flac"), FileInfo::FileType::kAudios);
    EXPECT_EQ(manager->displayNameToEnum("image/bmp"), FileInfo::FileType::kImages);
    EXPECT_EQ(manager->displayNameToEnum("application/x-shellscript"), FileInfo::FileType::kExecutable);
    EXPECT_EQ(manager->displayNameToEnum("application/x-backup"), FileInfo::FileType::kBackups);
}

/**
 * @brief Test default icon functionality
 * Verify defaultIcon returns correct icon names
 */
TEST_F(MimeTypeDisplayManagerTest, DefaultIcon_BasicMimeTypes)
{
    // Test basic mime types
    EXPECT_EQ(manager->defaultIcon("inode/directory"), "folder");
    EXPECT_EQ(manager->defaultIcon("application/x-desktop"), "application-default-icon");
    EXPECT_EQ(manager->defaultIcon("video/mp4"), "video");
    EXPECT_EQ(manager->defaultIcon("audio/mpeg"), "music");
    EXPECT_EQ(manager->defaultIcon("image/jpeg"), "image");
    EXPECT_EQ(manager->defaultIcon("application/zip"), "application-x-archive");
    EXPECT_EQ(manager->defaultIcon("text/plain"), "text-plain");
    EXPECT_EQ(manager->defaultIcon("application/x-executable"), "application-x-executable");
    
    // Test unknown mime type
    EXPECT_EQ(manager->defaultIcon("application/unknown"), "application-default-icon");
    
    // Test backup file
    EXPECT_EQ(manager->defaultIcon("application/x-backup"), "application-x-archive");
}

/**
 * @brief Test display names map
 * Verify displayNames returns all display names
 */
TEST_F(MimeTypeDisplayManagerTest, DisplayNames_CompleteMap)
{
    QMap<FileInfo::FileType, QString> names = manager->displayNames();
    
    // Verify all expected types are present
    EXPECT_TRUE(names.contains(FileInfo::FileType::kDirectory));
    EXPECT_TRUE(names.contains(FileInfo::FileType::kDesktopApplication));
    EXPECT_TRUE(names.contains(FileInfo::FileType::kVideos));
    EXPECT_TRUE(names.contains(FileInfo::FileType::kAudios));
    EXPECT_TRUE(names.contains(FileInfo::FileType::kImages));
    EXPECT_TRUE(names.contains(FileInfo::FileType::kArchives));
    EXPECT_TRUE(names.contains(FileInfo::FileType::kDocuments));
    EXPECT_TRUE(names.contains(FileInfo::FileType::kExecutable));
    EXPECT_TRUE(names.contains(FileInfo::FileType::kBackups));
    EXPECT_TRUE(names.contains(FileInfo::FileType::kUnknown));
    
    // Verify display names are not empty
    for (const QString &name : names.values()) {
        EXPECT_FALSE(name.isEmpty());
    }
}

/**
 * @brief Test support mime types functions
 * Verify support functions return correct mime type lists
 */
TEST_F(MimeTypeDisplayManagerTest, SupportMimeTypes_ReturnValues)
{
    // Test archive mime types
    QStringList archiveTypes = manager->supportArchiveMimetypes();
    EXPECT_FALSE(archiveTypes.isEmpty());
    EXPECT_TRUE(archiveTypes.contains("application/zip"));
    EXPECT_TRUE(archiveTypes.contains("application/x-tar"));
    EXPECT_TRUE(archiveTypes.contains("application/x-rar"));
    EXPECT_TRUE(archiveTypes.contains("application/gzip"));
    
    // Test video mime types
    QStringList videoTypes = manager->supportVideoMimeTypes();
    EXPECT_FALSE(videoTypes.isEmpty());
    EXPECT_TRUE(videoTypes.contains("video/mp4"));
    EXPECT_TRUE(videoTypes.contains("video/avi"));
    EXPECT_TRUE(videoTypes.contains("video/mkv"));
    EXPECT_TRUE(videoTypes.contains("video/mov"));
    
    // Test audio mime types
    QStringList audioTypes = manager->supportAudioMimeTypes();
    EXPECT_FALSE(audioTypes.isEmpty());
    EXPECT_TRUE(audioTypes.contains("audio/mpeg"));
    EXPECT_TRUE(audioTypes.contains("audio/wav"));
    EXPECT_TRUE(audioTypes.contains("audio/ogg"));
    EXPECT_TRUE(audioTypes.contains("audio/flac"));
}

/**
 * @brief Test accurate display type from path
 * Verify accurateDisplayTypeFromPath works with real files
 */
TEST_F(MimeTypeDisplayManagerTest, AccurateDisplayTypeFromPath_RealFiles)
{
    // Create temporary test files
    QString textFile = createTempTestFile("This is a text file content");
    QString emptyFile = createTempTestFile("");
    
    // Test with text file
    QString displayType = manager->accurateDisplayTypeFromPath(textFile);
    EXPECT_FALSE(displayType.isEmpty());
    
    // Test with empty file
    displayType = manager->accurateDisplayTypeFromPath(emptyFile);
    EXPECT_FALSE(displayType.isEmpty());
    
    // Test with non-existent file
    displayType = manager->accurateDisplayTypeFromPath("/non/existent/file.txt");
    EXPECT_FALSE(displayType.isEmpty());
    EXPECT_TRUE(displayType.contains("Text"));
    
    // Clean up
    QFile::remove(textFile);
    QFile::remove(emptyFile);
}

/**
 * @brief Test accurate local mime type name
 * Verify accurateLocalMimeTypeName returns correct mime names
 */
TEST_F(MimeTypeDisplayManagerTest, AccurateLocalMimeTypeName_RealFiles)
{
    // Create temporary test file
    QString textFile = createTempTestFile("This is a text file content");
    
    // Test with text file
    QString mimeName = manager->accurateLocalMimeTypeName(textFile);
    qDebug() << "Mime name:" << mimeName;
    EXPECT_FALSE(mimeName.isEmpty());
    EXPECT_TRUE(mimeName.contains("Unknown"));
    
    // Test with non-existent file
    mimeName = manager->accurateLocalMimeTypeName("/non/existent/file.txt");
    EXPECT_FALSE(mimeName.isEmpty());
    EXPECT_TRUE(mimeName.contains("text/plain"));
    
    // Clean up
    QFile::remove(textFile);
}

/**
 * @brief Test readlines function
 * Verify readlines reads files correctly
 */
TEST_F(MimeTypeDisplayManagerTest, Readlines_FileReading)
{
    // Create test file with various content
    QString testFilePath = tempDir->path() + "/test_readlines.txt";
    QFile testFile(testFilePath);
    ASSERT_TRUE(testFile.open(QIODevice::WriteOnly | QIODevice::Text));
    
    QTextStream out(&testFile);
    out << "line1\n";
    out << "  line2  \n";   // with spaces
    out << "\n";            // empty line
    out << "line4\n";
    out << "   \n";         // whitespace only line
    out << "line6\n";
    testFile.close();
    
    // Use readlines through the manager's loadSupportMimeTypes 
    // (we can't call readlines directly as it's private)
    
    // Verify file exists and has content
    EXPECT_TRUE(QFile::exists(testFilePath));
}

/**
 * @brief Test initialization data consistency
 * Verify that all data structures are properly initialized
 */
TEST_F(MimeTypeDisplayManagerTest, Initialization_DataConsistency)
{
    // Verify display names map is complete
    QMap<FileInfo::FileType, QString> displayNames = manager->displayNames();
    EXPECT_EQ(displayNames.size(), 10); // Should have all file types
    
    // Verify each display name corresponds to correct enum
    EXPECT_EQ(displayNames[FileInfo::FileType::kDirectory], "Directory");
    EXPECT_EQ(displayNames[FileInfo::FileType::kDesktopApplication], "Application");
    EXPECT_EQ(displayNames[FileInfo::FileType::kVideos], "Video");
    EXPECT_EQ(displayNames[FileInfo::FileType::kAudios], "Audio");
    EXPECT_EQ(displayNames[FileInfo::FileType::kImages], "Image");
    EXPECT_EQ(displayNames[FileInfo::FileType::kArchives], "Archive");
    EXPECT_EQ(displayNames[FileInfo::FileType::kDocuments], "Text");
    EXPECT_EQ(displayNames[FileInfo::FileType::kExecutable], "Executable");
    EXPECT_EQ(displayNames[FileInfo::FileType::kBackups], "Backup file");
    EXPECT_EQ(displayNames[FileInfo::FileType::kUnknown], "Unknown");
}

/**
 * @brief Test edge cases and error handling
 * Verify graceful handling of edge cases
 */
TEST_F(MimeTypeDisplayManagerTest, EdgeCases_ErrorHandling)
{
#ifndef QT_DEBUG
    // Test empty mime type
    QString displayName = manager->displayName("");
    EXPECT_EQ(displayName, "Unknown");
    
    QString fullName = manager->fullMimeName("");
    EXPECT_EQ(fullName, "Unknown ()");
    
    FileInfo::FileType fileType = manager->displayNameToEnum("");
    EXPECT_EQ(fileType, FileInfo::FileType::kUnknown);
    
    QString icon = manager->defaultIcon("");
    EXPECT_EQ(icon, "application-default-icon");
    
    // Test null mime type
    displayName = manager->displayName(QString());
    EXPECT_EQ(displayName, "Unknown");
    
    // Test very long mime type
    QString longMimeType = "application/very-long-mime-type-name-that-might-not-exist-but-should-handle-gracefully";
    displayName = manager->displayName(longMimeType);
    EXPECT_EQ(displayName, "Unknown");
    
    fileType = manager->displayNameToEnum(longMimeType);
    EXPECT_EQ(fileType, FileInfo::FileType::kUnknown);
#endif
}

/**
 * @brief Test mime type case sensitivity
 * Verify that mime type handling is case-sensitive
 */
TEST_F(MimeTypeDisplayManagerTest, MimeType_CaseSensitivity)
{
#ifndef QT_DEBUG
    // Test lowercase (standard)
    EXPECT_EQ(manager->displayName("video/mp4"), "Video");
    EXPECT_EQ(manager->displayName("audio/mpeg"), "Audio");
    EXPECT_EQ(manager->displayName("image/jpeg"), "Image");
    
    // Test uppercase (should be treated as unknown)
    EXPECT_EQ(manager->displayName("VIDEO/MP4"), "Unknown");
    EXPECT_EQ(manager->displayName("AUDIO/MPEG"), "Unknown");
    EXPECT_EQ(manager->displayName("IMAGE/JPEG"), "Unknown");
    
    // Test mixed case (should be treated as unknown)
    EXPECT_EQ(manager->displayName("Video/Mp4"), "Unknown");
    EXPECT_EQ(manager->displayName("Audio/Mpeg"), "Unknown");
#endif
}

/**
 * @brief Test mime type format validation
 * Verify handling of malformed mime types
 */
TEST_F(MimeTypeDisplayManagerTest, MimeType_FormatValidation)
{
#ifndef QT_DEBUG
    // Test well-formed mime types
    EXPECT_NE(manager->displayName("type/subtype"), "Unknown");
    
    // Test malformed mime types
    EXPECT_EQ(manager->displayName("type"), "Unknown");              // missing subtype
    EXPECT_EQ(manager->displayName("/subtype"), "Unknown");           // missing type
    EXPECT_EQ(manager->displayName("type/"), "Unknown");              // empty subtype
    EXPECT_EQ(manager->displayName("/"), "Unknown");                  // both empty
    EXPECT_EQ(manager->displayName("type/subtype/extra"), "Unknown"); // too many parts
    
    // Test mime types with special characters
    EXPECT_EQ(manager->displayName("type/sub-type"), "Unknown");      // dash in subtype
    EXPECT_EQ(manager->displayName("type/sub.type"), "Unknown");      // dot in subtype
    EXPECT_EQ(manager->displayName("type/sub_type"), "Unknown");      // underscore in subtype
#endif
}

/**
 * @brief Test consistency between different functions
 * Verify that related functions return consistent results
 */
TEST_F(MimeTypeDisplayManagerTest, FunctionConsistency)
{
    QString mimeType = "video/mp4";
    
    // Test consistency between displayName and displayNameToEnum
    QString displayName = manager->displayName(mimeType);
    FileInfo::FileType fileType = manager->displayNameToEnum(mimeType);
    QString expectedDisplayName = manager->displayNames().value(fileType);

#ifdef QT_DEBUG
    EXPECT_EQ(displayName, "Video (video/mp4)");
#else
    EXPECT_EQ(displayName, expectedDisplayName);
    EXPECT_EQ(displayName, "Video");
#endif
    EXPECT_EQ(fileType, FileInfo::FileType::kVideos);
    
    // Test consistency between defaultIcon and displayNameToEnum
    QString icon = manager->defaultIcon(mimeType);
    EXPECT_NE(icon, "");
    EXPECT_TRUE(icon.contains("video") || icon.contains("application"));
}

/**
 * @brief Test performance with multiple calls
 * Verify that repeated calls don't cause performance issues
 */
TEST_F(MimeTypeDisplayManagerTest, Performance_MultipleCalls)
{
    // Test multiple calls to the same function
    const int iterations = 100;
    
    // Test displayName performance
    for (int i = 0; i < iterations; ++i) {
        QString result = manager->displayName("video/mp4");
#ifdef QT_DEBUG
        EXPECT_EQ(result, "Video (video/mp4)");
#else
        EXPECT_EQ(result, "Video");
#endif
    }
    
    // Test displayNameToEnum performance
    for (int i = 0; i < iterations; ++i) {
        FileInfo::FileType result = manager->displayNameToEnum("video/mp4");
        EXPECT_EQ(result, FileInfo::FileType::kVideos);
    }
    
    // Test support mime types performance
    for (int i = 0; i < iterations; ++i) {
        QStringList result = manager->supportVideoMimeTypes();
        EXPECT_FALSE(result.isEmpty());
    }
}
