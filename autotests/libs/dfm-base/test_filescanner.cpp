// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QEventLoop>
#include <QTimer>

#include <dfm-base/utils/filescanner.h>

using namespace dfmbase;

class TestFileScanner : public testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(tempDir.isValid());
        rootPath = tempDir.path();

        subdir1 = rootPath + "/subdir1";
        subdir2 = rootPath + "/subdir2";
        ASSERT_TRUE(QDir().mkpath(subdir1));
        ASSERT_TRUE(QDir().mkpath(subdir2));

        fileA = subdir1 + "/file_a.txt";
        fileB = subdir2 + "/file_b.txt";
        fileC = rootPath + "/file_c.txt";
        ASSERT_TRUE(writeFile(fileA, "aaaa"));
        ASSERT_TRUE(writeFile(fileB, "bbbb"));
        ASSERT_TRUE(writeFile(fileC, "cccc"));
    }

    void TearDown() override
    {
    }

    static bool writeFile(const QString &path, const QByteArray &content)
    {
        QFile f(path);
        if (!f.open(QIODevice::WriteOnly))
            return false;
        f.write(content);
        f.close();
        return true;
    }

    QTemporaryDir tempDir;
    QString rootPath;
    QString subdir1;
    QString subdir2;
    QString fileA;
    QString fileB;
    QString fileC;
};

TEST_F(TestFileScanner, ScanSync_Baseline_NoExclude)
{
    auto result = FileScanner::scanSync({QUrl::fromLocalFile(rootPath)});
    EXPECT_TRUE(result.isValid());
    EXPECT_EQ(result.fileCount, 3);
    EXPECT_EQ(result.directoryCount, 2);
    EXPECT_EQ(result.totalSize, 12);
}

TEST_F(TestFileScanner, ScanSync_Baseline_CountOnly)
{
    auto result = FileScanner::scanSync(
            {QUrl::fromLocalFile(rootPath)},
            FileScanner::ScanOption::CountOnly);
    EXPECT_TRUE(result.isValid());
    EXPECT_EQ(result.fileCount, 3);
    EXPECT_EQ(result.directoryCount, 2);
    EXPECT_EQ(result.totalSize, 0);
}

TEST_F(TestFileScanner, ManualExclude_SinglePath)
{
    auto scanner = new FileScanner();
    scanner->setExcludePaths({subdir1});

    QSignalSpy finishedSpy(scanner, &FileScanner::finished);
    QEventLoop loop;
    QObject::connect(scanner, &FileScanner::finished, &loop, &QEventLoop::quit);

    scanner->start({QUrl::fromLocalFile(rootPath)});
    loop.exec();

    ASSERT_EQ(finishedSpy.count(), 1);
    auto result = finishedSpy.takeFirst().at(0).value<FileScanner::ScanResult>();
    EXPECT_EQ(result.fileCount, 2);
    EXPECT_EQ(result.directoryCount, 1);
    EXPECT_EQ(result.totalSize, 8);

    delete scanner;
}

TEST_F(TestFileScanner, ManualExclude_MultiplePaths)
{
    auto scanner = new FileScanner();
    scanner->setExcludePaths({subdir1, subdir2});

    QSignalSpy finishedSpy(scanner, &FileScanner::finished);
    QEventLoop loop;
    QObject::connect(scanner, &FileScanner::finished, &loop, &QEventLoop::quit);

    scanner->start({QUrl::fromLocalFile(rootPath)});
    loop.exec();

    ASSERT_EQ(finishedSpy.count(), 1);
    auto result = finishedSpy.takeFirst().at(0).value<FileScanner::ScanResult>();
    EXPECT_EQ(result.fileCount, 1);
    EXPECT_EQ(result.directoryCount, 0);
    EXPECT_EQ(result.totalSize, 4);

    delete scanner;
}

TEST_F(TestFileScanner, ManualExclude_TrailingSlash)
{
    auto scanner = new FileScanner();
    scanner->setExcludePaths({subdir1 + "/"});

    QSignalSpy finishedSpy(scanner, &FileScanner::finished);
    QEventLoop loop;
    QObject::connect(scanner, &FileScanner::finished, &loop, &QEventLoop::quit);

    scanner->start({QUrl::fromLocalFile(rootPath)});
    loop.exec();

    ASSERT_EQ(finishedSpy.count(), 1);
    auto result = finishedSpy.takeFirst().at(0).value<FileScanner::ScanResult>();
    EXPECT_EQ(result.fileCount, 2);
    EXPECT_EQ(result.directoryCount, 1);

    delete scanner;
}

TEST_F(TestFileScanner, ManualExclude_NoMatch)
{
    auto scanner = new FileScanner();
    scanner->setExcludePaths({rootPath + "/nonexistent"});

    QSignalSpy finishedSpy(scanner, &FileScanner::finished);
    QEventLoop loop;
    QObject::connect(scanner, &FileScanner::finished, &loop, &QEventLoop::quit);

    scanner->start({QUrl::fromLocalFile(rootPath)});
    loop.exec();

    ASSERT_EQ(finishedSpy.count(), 1);
    auto result = finishedSpy.takeFirst().at(0).value<FileScanner::ScanResult>();
    EXPECT_EQ(result.fileCount, 3);
    EXPECT_EQ(result.directoryCount, 2);

    delete scanner;
}

TEST_F(TestFileScanner, ExcludePaths_GetterSetter)
{
    FileScanner scanner;
    QStringList paths = {"/tmp/foo", "/var/bar"};
    scanner.setExcludePaths(paths);
    EXPECT_EQ(scanner.excludePaths(), paths);

    scanner.setExcludePaths({});
    EXPECT_TRUE(scanner.excludePaths().isEmpty());
}

TEST_F(TestFileScanner, ScanSyncWithCallback_BasicScan)
{
    auto result = FileScanner::scanSyncWithCallback(
            {QUrl::fromLocalFile(rootPath)},
            FileScanner::ScanOption::NoOption,
            [](const FileScanner::ScanResult &) -> bool {
                return true;
            });

    EXPECT_TRUE(result.isValid());
    EXPECT_EQ(result.fileCount, 3);
    EXPECT_EQ(result.directoryCount, 2);
    EXPECT_EQ(result.totalSize, 12);
}
