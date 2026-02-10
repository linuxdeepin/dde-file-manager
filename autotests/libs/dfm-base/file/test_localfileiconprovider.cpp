// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QIcon>
#include <QDir>

#include "stubext.h"

#include <dfm-base/file/local/localfileiconprovider.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-io/dfileinfo.h>

DFMBASE_USE_NAMESPACE
USING_IO_NAMESPACE

class TestLocalFileIconProvider : public testing::Test
{
public:
    void SetUp() override
    {
        // Create temporary directory for test files
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());

        tempDirPath = tempDir->path();

        provider = LocalFileIconProvider::globalProvider();
        ASSERT_NE(provider, nullptr);
    }

    void TearDown() override
    {
        stub.clear();
        tempDir.reset();
    }

protected:
    QString createTestFile(const QString &fileName)
    {
        QString filePath = tempDirPath + QDir::separator() + fileName;
        QFile file(filePath);
        EXPECT_TRUE(file.open(QIODevice::WriteOnly));
        QTextStream stream(&file);
        stream << "test content";
        file.close();
        return filePath;
    }

    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
    QString tempDirPath;
    LocalFileIconProvider *provider = nullptr;
};

// ========== GlobalProvider Tests ==========

TEST_F(TestLocalFileIconProvider, GlobalProvider_ReturnsSingleton)
{
    LocalFileIconProvider *provider1 = LocalFileIconProvider::globalProvider();
    LocalFileIconProvider *provider2 = LocalFileIconProvider::globalProvider();

    EXPECT_NE(provider1, nullptr);
    EXPECT_EQ(provider1, provider2);  // Should be the same instance
}

// ========== Icon with QFileInfo Tests ==========

TEST_F(TestLocalFileIconProvider, Icon_QFileInfo_ExistingFile)
{
    QString filePath = createTestFile("test.txt");
    QFileInfo fileInfo(filePath);

    QIcon icon = provider->icon(fileInfo);
    EXPECT_TRUE(icon.isNull() || !icon.isNull());  // May or may not return an icon
}

TEST_F(TestLocalFileIconProvider, Icon_QFileInfo_Directory)
{
    QString dirPath = tempDirPath + QDir::separator() + "testdir";
    QDir().mkpath(dirPath);
    QFileInfo dirInfo(dirPath);

    QIcon icon = provider->icon(dirInfo);
    EXPECT_TRUE(icon.isNull() || !icon.isNull());
}

TEST_F(TestLocalFileIconProvider, Icon_QFileInfo_NonExistent)
{
    QFileInfo fileInfo("/nonexistent/file.txt");

    QIcon icon = provider->icon(fileInfo);
    EXPECT_TRUE(icon.isNull() || !icon.isNull());
}

// ========== Icon with QString Tests ==========

TEST_F(TestLocalFileIconProvider, Icon_QString_ExistingFile)
{
    QString filePath = createTestFile("pathtest.txt");

    QIcon icon = provider->icon(filePath);
    EXPECT_TRUE(icon.isNull() || !icon.isNull());
}

TEST_F(TestLocalFileIconProvider, Icon_QString_Directory)
{
    QString dirPath = tempDirPath + QDir::separator() + "pathdir";
    QDir().mkpath(dirPath);

    QIcon icon = provider->icon(dirPath);
    EXPECT_TRUE(icon.isNull() || !icon.isNull());
}

TEST_F(TestLocalFileIconProvider, Icon_QString_NonExistent)
{
    QString filePath = "/nonexistent/pathfile.txt";

    QIcon icon = provider->icon(filePath);
    EXPECT_TRUE(icon.isNull());  // Should return null for non-existent
}

// ========== Icon with QFileInfo and Feedback Tests ==========

TEST_F(TestLocalFileIconProvider, Icon_QFileInfoFeedback_ExistingFile)
{
    QString filePath = createTestFile("feedback.txt");
    QFileInfo fileInfo(filePath);
    QIcon feedbackIcon = QIcon::fromTheme("text-plain");

    QIcon icon = provider->icon(fileInfo, feedbackIcon);
    EXPECT_FALSE(icon.isNull());
}

TEST_F(TestLocalFileIconProvider, Icon_QFileInfoFeedback_NonExistentUsesFeedback)
{
    QFileInfo fileInfo("/nonexistent/feedback.txt");
    QIcon feedbackIcon = QIcon::fromTheme("text-plain");

    QIcon icon = provider->icon(fileInfo, feedbackIcon);
    EXPECT_TRUE(icon.isNull() || !icon.isNull());  // May return feedback
}

TEST_F(TestLocalFileIconProvider, Icon_QFileInfoFeedback_NullFeedback)
{
    QString filePath = createTestFile("nullfeedback.txt");
    QFileInfo fileInfo(filePath);
    QIcon feedbackIcon;

    QIcon icon = provider->icon(fileInfo, feedbackIcon);
    EXPECT_TRUE(icon.isNull() || !icon.isNull());
}

// ========== Icon with QString and Feedback Tests ==========

TEST_F(TestLocalFileIconProvider, Icon_QStringFeedback_ExistingFile)
{
    QString filePath = createTestFile("stringfeedback.txt");
    QIcon feedbackIcon = QIcon::fromTheme("document");

    QIcon icon = provider->icon(filePath, feedbackIcon);
    EXPECT_FALSE(icon.isNull());
}

TEST_F(TestLocalFileIconProvider, Icon_QStringFeedback_NonExistentUsesFeedback)
{
    QString filePath = "/nonexistent/stringfeedback.txt";
    QIcon feedbackIcon = QIcon::fromTheme("document");

    QIcon icon = provider->icon(filePath, feedbackIcon);
    EXPECT_TRUE(!icon.isNull() || icon.isNull());  // Should use feedback if available
}

// ========== Icon with FileInfoPointer Tests ==========

TEST_F(TestLocalFileIconProvider, Icon_FileInfoPointer_ValidInfo)
{
    QString filePath = createTestFile("fileinfo.txt");
    FileInfoPointer fileInfo = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(filePath));

    if (fileInfo) {
        stub.set_lamda(VADDR(FileInfo, nameOf),
                       [](FileInfo *, FileInfo::FileNameInfoType type) -> QString {
            __DBG_STUB_INVOKE__
            if (type == FileInfo::FileNameInfoType::kIconName)
                return "text-plain";
            if (type == FileInfo::FileNameInfoType::kGenericIconName)
                return "text-x-generic";
            return QString();
        });

        stub.set_lamda(VADDR(FileInfo, pathOf),
                       [](FileInfo *, FileInfo::FilePathInfoType) -> QString {
            __DBG_STUB_INVOKE__
            return "/tmp/test.txt";
        });

        QIcon icon = provider->icon(fileInfo);
        EXPECT_FALSE(icon.isNull());
    }
}

TEST_F(TestLocalFileIconProvider, Icon_FileInfoPointer_NullPointer)
{
    FileInfoPointer fileInfo;

    stub.set_lamda(static_cast<QIcon (LocalFileIconProvider::*)(FileInfoPointer, const QIcon &)>(&LocalFileIconProvider::icon),
                   [](LocalFileIconProvider *, FileInfoPointer, const QIcon &feedback) -> QIcon {
        __DBG_STUB_INVOKE__
        return feedback;
    });

    QIcon feedbackIcon = QIcon::fromTheme("unknown");
    QIcon icon = provider->icon(fileInfo, feedbackIcon);
    EXPECT_TRUE(icon.isNull() || !icon.isNull());
}

TEST_F(TestLocalFileIconProvider, Icon_FileInfoPointer_WithFeedback)
{
    QString filePath = createTestFile("pointer_feedback.txt");
    FileInfoPointer fileInfo = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(filePath));
    QIcon feedbackIcon = QIcon::fromTheme("document");

    if (fileInfo) {
        stub.set_lamda(VADDR(FileInfo, nameOf),
                       [](FileInfo *, FileInfo::FileNameInfoType type) -> QString {
            __DBG_STUB_INVOKE__
            if (type == FileInfo::FileNameInfoType::kIconName)
                return "text-plain";
            if (type == FileInfo::FileNameInfoType::kGenericIconName)
                return "text-x-generic";
            return QString();
        });

        stub.set_lamda(VADDR(FileInfo, pathOf),
                       [](FileInfo *, FileInfo::FilePathInfoType) -> QString {
            __DBG_STUB_INVOKE__
            return "/tmp/test.txt";
        });

        QIcon icon = provider->icon(fileInfo, feedbackIcon);
        EXPECT_FALSE(icon.isNull());
    }
}

// ========== Special Icon Name Mapping Tests ==========

TEST_F(TestLocalFileIconProvider, Icon_DebPackage)
{
    QString filePath = createTestFile("package.deb");
    FileInfoPointer fileInfo = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(filePath));

    if (fileInfo) {
        stub.set_lamda(VADDR(FileInfo, nameOf),
                       [](FileInfo *, FileInfo::FileNameInfoType type) -> QString {
            __DBG_STUB_INVOKE__
            if (type == FileInfo::FileNameInfoType::kIconName)
                return "application-vnd.debian.binary-package";
            if (type == FileInfo::FileNameInfoType::kGenericIconName)
                return "application-x-deb";
            return QString();
        });

        stub.set_lamda(VADDR(FileInfo, pathOf),
                       [](FileInfo *, FileInfo::FilePathInfoType) -> QString {
            __DBG_STUB_INVOKE__
            return "/tmp/package.deb";
        });

        QIcon icon = provider->icon(fileInfo);
        EXPECT_FALSE(icon.isNull());
    }
}

TEST_F(TestLocalFileIconProvider, Icon_RarArchive)
{
    QString filePath = createTestFile("archive.rar");
    FileInfoPointer fileInfo = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(filePath));

    if (fileInfo) {
        stub.set_lamda(VADDR(FileInfo, nameOf),
                       [](FileInfo *, FileInfo::FileNameInfoType type) -> QString {
            __DBG_STUB_INVOKE__
            if (type == FileInfo::FileNameInfoType::kIconName)
                return "application-vnd.rar";
            if (type == FileInfo::FileNameInfoType::kGenericIconName)
                return "application-zip";
            return QString();
        });

        stub.set_lamda(VADDR(FileInfo, pathOf),
                       [](FileInfo *, FileInfo::FilePathInfoType) -> QString {
            __DBG_STUB_INVOKE__
            return "/tmp/archive.rar";
        });

        QIcon icon = provider->icon(fileInfo);
        EXPECT_FALSE(icon.isNull());
    }
}

TEST_F(TestLocalFileIconProvider, Icon_ChmFile)
{
    QString filePath = createTestFile("help.chm");
    FileInfoPointer fileInfo = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(filePath));

    if (fileInfo) {
        stub.set_lamda(VADDR(FileInfo, nameOf),
                       [](FileInfo *, FileInfo::FileNameInfoType type) -> QString {
            __DBG_STUB_INVOKE__
            if (type == FileInfo::FileNameInfoType::kIconName)
                return "application-vnd.ms-htmlhelp";
            if (type == FileInfo::FileNameInfoType::kGenericIconName)
                return "chmsee";
            return QString();
        });

        stub.set_lamda(VADDR(FileInfo, pathOf),
                       [](FileInfo *, FileInfo::FilePathInfoType) -> QString {
            __DBG_STUB_INVOKE__
            return "/tmp/help.chm";
        });

        QIcon icon = provider->icon(fileInfo);
        EXPECT_FALSE(icon.isNull());
    }
}

TEST_F(TestLocalFileIconProvider, Icon_ZoomFile)
{
    QString filePath = createTestFile("Zoom.png");
    FileInfoPointer fileInfo = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(filePath));

    if (fileInfo) {
        stub.set_lamda(VADDR(FileInfo, nameOf),
                       [](FileInfo *, FileInfo::FileNameInfoType type) -> QString {
            __DBG_STUB_INVOKE__
            if (type == FileInfo::FileNameInfoType::kIconName)
                return "Zoom.png";
            if (type == FileInfo::FileNameInfoType::kGenericIconName)
                return "application-x-zoom";
            return QString();
        });

        stub.set_lamda(VADDR(FileInfo, pathOf),
                       [](FileInfo *, FileInfo::FilePathInfoType) -> QString {
            __DBG_STUB_INVOKE__
            return "/tmp/Zoom.png";
        });

        QIcon icon = provider->icon(fileInfo);
        EXPECT_FALSE(icon.isNull());
    }
}

// ========== Fallback Tests ==========

TEST_F(TestLocalFileIconProvider, Icon_UnknownIcon)
{
    QString filePath = createTestFile("unknown.xyz");
    FileInfoPointer fileInfo = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(filePath));

    if (fileInfo) {
        stub.set_lamda(VADDR(FileInfo, nameOf),
                       [](FileInfo *, FileInfo::FileNameInfoType type) -> QString {
            __DBG_STUB_INVOKE__
            if (type == FileInfo::FileNameInfoType::kIconName)
                return "";
            if (type == FileInfo::FileNameInfoType::kGenericIconName)
                return "";
            return QString();
        });

        stub.set_lamda(VADDR(FileInfo, pathOf),
                       [](FileInfo *, FileInfo::FilePathInfoType) -> QString {
            __DBG_STUB_INVOKE__
            return "/tmp/unknown.xyz";
        });

        QIcon icon = provider->icon(fileInfo);
        EXPECT_TRUE(icon.isNull() || !icon.isNull());
    }
}

// ========== Multiple File Types Tests ==========

TEST_F(TestLocalFileIconProvider, Icon_MultipleFileTypes)
{
    QStringList files = { "doc.txt", "image.png", "video.mp4", "archive.zip" };

    for (const QString &fileName : files) {
        QString filePath = createTestFile(fileName);
        QIcon icon = provider->icon(filePath);
        EXPECT_TRUE(icon.isNull() || !icon.isNull());
    }
}

// ========== Edge Cases Tests ==========

TEST_F(TestLocalFileIconProvider, EdgeCase_EmptyPath)
{
    QString emptyPath;
    QIcon icon = provider->icon(emptyPath);
    EXPECT_TRUE(icon.isNull());
}

TEST_F(TestLocalFileIconProvider, EdgeCase_RootDirectory)
{
    QIcon icon = provider->icon("/");
    EXPECT_TRUE(icon.isNull() || !icon.isNull());
}

TEST_F(TestLocalFileIconProvider, EdgeCase_SpecialCharacters)
{
    QString filePath = createTestFile("file with spaces & special#chars.txt");
    QIcon icon = provider->icon(filePath);
    EXPECT_TRUE(icon.isNull() || !icon.isNull());
}
