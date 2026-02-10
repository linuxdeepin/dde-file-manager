// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/appendcompress/appendcompresshelper.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/protocolutils.h>
#include <dfm-framework/dpf.h>

#include <QProcess>
#include <QUrl>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

class UT_AppendCompressHelper : public testing::Test
{
protected:
    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

// ========== isCompressedFile 测试 ==========

TEST_F(UT_AppendCompressHelper, isCompressedFile_ZipFile_ReturnsTrue)
{
    QUrl url = QUrl::fromLocalFile("/tmp/archive.zip");

    auto mockFileInfo = QSharedPointer<FileInfo>(new FileInfo(url));
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [&mockFileInfo](const QUrl &, const Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return mockFileInfo;
                   });

    stub.set_lamda(VADDR(FileInfo, isAttributes),
                   [](FileInfo *, const OptInfoType type) -> bool {
                       __DBG_STUB_INVOKE__
                       return type == OptInfoType::kIsFile;
                   });

    stub.set_lamda(VADDR(FileInfo, nameOf),
                   [](FileInfo *, const NameInfoType type) -> QString {
                       __DBG_STUB_INVOKE__
                       if (type == NameInfoType::kMimeTypeName)
                           return "application/zip";
                       return "archive.zip";
                   });

    EXPECT_TRUE(AppendCompressHelper::isCompressedFile(url));
}

TEST_F(UT_AppendCompressHelper, isCompressedFile_7zFile_ReturnsTrue)
{
    QUrl url = QUrl::fromLocalFile("/tmp/archive.7z");

    auto mockFileInfo = QSharedPointer<FileInfo>(new FileInfo(url));
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [&mockFileInfo](const QUrl &, const Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return mockFileInfo;
                   });

    stub.set_lamda(VADDR(FileInfo, isAttributes),
                   [](FileInfo *, const OptInfoType type) -> bool {
                       __DBG_STUB_INVOKE__
                       return type == OptInfoType::kIsFile;
                   });

    stub.set_lamda(VADDR(FileInfo, nameOf),
                   [](FileInfo *, const NameInfoType type) -> QString {
                       __DBG_STUB_INVOKE__
                       if (type == NameInfoType::kMimeTypeName)
                           return "application/x-7z-compressed";
                       return "archive.7z";
                   });

    EXPECT_TRUE(AppendCompressHelper::isCompressedFile(url));
}

TEST_F(UT_AppendCompressHelper, isCompressedFile_Tar7zFile_ReturnsFalse)
{
    QUrl url = QUrl::fromLocalFile("/tmp/archive.tar.7z");

    auto mockFileInfo = QSharedPointer<FileInfo>(new FileInfo(url));
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [&mockFileInfo](const QUrl &, const Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return mockFileInfo;
                   });

    stub.set_lamda(VADDR(FileInfo, isAttributes),
                   [](FileInfo *, const OptInfoType type) -> bool {
                       __DBG_STUB_INVOKE__
                       return type == OptInfoType::kIsFile;
                   });

    stub.set_lamda(VADDR(FileInfo, nameOf),
                   [](FileInfo *, const NameInfoType type) -> QString {
                       __DBG_STUB_INVOKE__
                       if (type == NameInfoType::kMimeTypeName)
                           return "application/x-7z-compressed";
                       return "archive.tar.7z";
                   });

    EXPECT_FALSE(AppendCompressHelper::isCompressedFile(url));
}

TEST_F(UT_AppendCompressHelper, isCompressedFile_NonCompressedFile_ReturnsFalse)
{
    QUrl url = QUrl::fromLocalFile("/tmp/document.txt");

    auto mockFileInfo = QSharedPointer<FileInfo>(new FileInfo(url));
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [&mockFileInfo](const QUrl &, const Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return mockFileInfo;
                   });

    stub.set_lamda(VADDR(FileInfo, isAttributes),
                   [](FileInfo *, const OptInfoType type) -> bool {
                       __DBG_STUB_INVOKE__
                       return type == OptInfoType::kIsFile;
                   });

    stub.set_lamda(VADDR(FileInfo, nameOf),
                   [](FileInfo *, const NameInfoType type) -> QString {
                       __DBG_STUB_INVOKE__
                       if (type == NameInfoType::kMimeTypeName)
                           return "text/plain";
                       return "document.txt";
                   });

    EXPECT_FALSE(AppendCompressHelper::isCompressedFile(url));
}

TEST_F(UT_AppendCompressHelper, isCompressedFile_NullFileInfo_ReturnsFalse)
{
    QUrl url = QUrl::fromLocalFile("/tmp/nonexistent.zip");

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &, const Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    EXPECT_FALSE(AppendCompressHelper::isCompressedFile(url));
}

// ========== canAppendCompress 测试 ==========

TEST_F(UT_AppendCompressHelper, canAppendCompress_ValidParams_ReturnsTrue)
{
    QList<QUrl> fromUrls = { QUrl::fromLocalFile("/tmp/file1.txt") };
    QUrl toUrl = QUrl::fromLocalFile("/tmp/archive.zip");

    stub.set_lamda(ADDR(UniversalUtils, urlsTransformToLocal),
                   [](const QList<QUrl> &in, QList<QUrl> *out) -> bool {
                       __DBG_STUB_INVOKE__
                       *out = in;
                       return true;
                   });

    stub.set_lamda(ADDR(ProtocolUtils, isFTPFile),
                   [](const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    typedef bool (EventSequenceManager::*RunFunc)(const QString &, const QString &, const QList<QUrl> &, const QUrl &);
    stub.set_lamda(static_cast<RunFunc>(&EventSequenceManager::run),
                   [] {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    auto mockFileInfo = QSharedPointer<FileInfo>(new FileInfo(toUrl));
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [&mockFileInfo](const QUrl &, const Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return mockFileInfo;
                   });

    stub.set_lamda(VADDR(FileInfo, isAttributes),
                   [](FileInfo *, const OptInfoType type) -> bool {
                       __DBG_STUB_INVOKE__
                       return type == OptInfoType::kIsWritable;
                   });

    stub.set_lamda(ADDR(AppendCompressHelper, isCompressedFile),
                   [](const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    EXPECT_TRUE(AppendCompressHelper::canAppendCompress(fromUrls, toUrl));
}

TEST_F(UT_AppendCompressHelper, canAppendCompress_InvalidParams_ReturnsFalse)
{
    QList<QUrl> emptyUrls;
    QUrl invalidUrl;
    QUrl validUrl = QUrl::fromLocalFile("/tmp/archive.zip");

    EXPECT_FALSE(AppendCompressHelper::canAppendCompress(emptyUrls, validUrl));
    EXPECT_FALSE(AppendCompressHelper::canAppendCompress({ validUrl }, invalidUrl));
}

TEST_F(UT_AppendCompressHelper, canAppendCompress_FTPFile_ReturnsFalse)
{
    QList<QUrl> fromUrls = { QUrl("ftp://example.com/file.txt") };
    QUrl toUrl = QUrl::fromLocalFile("/tmp/archive.zip");

    stub.set_lamda(ADDR(UniversalUtils, urlsTransformToLocal),
                   [](const QList<QUrl> &in, QList<QUrl> *out) -> bool {
                       __DBG_STUB_INVOKE__
                       *out = in;
                       return true;
                   });

    stub.set_lamda(ADDR(ProtocolUtils, isFTPFile),
                   [](const QUrl &url) -> bool {
                       __DBG_STUB_INVOKE__
                       return url.scheme() == "ftp";
                   });

    EXPECT_FALSE(AppendCompressHelper::canAppendCompress(fromUrls, toUrl));
}

TEST_F(UT_AppendCompressHelper, canAppendCompress_HookProhibits_ReturnsFalse)
{
    QList<QUrl> fromUrls = { QUrl::fromLocalFile("/tmp/file1.txt") };
    QUrl toUrl = QUrl::fromLocalFile("/tmp/archive.zip");

    stub.set_lamda(ADDR(UniversalUtils, urlsTransformToLocal),
                   [](const QList<QUrl> &in, QList<QUrl> *out) -> bool {
                       __DBG_STUB_INVOKE__
                       *out = in;
                       return true;
                   });

    stub.set_lamda(ADDR(ProtocolUtils, isFTPFile),
                   [](const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    typedef bool (EventSequenceManager::*RunFunc)(const QString &, const QString &, const QList<QUrl> &, const QUrl &);
    stub.set_lamda(static_cast<RunFunc>(&EventSequenceManager::run),
                   [] {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    EXPECT_FALSE(AppendCompressHelper::canAppendCompress(fromUrls, toUrl));
}

TEST_F(UT_AppendCompressHelper, canAppendCompress_NotWritableOrNotCompressed_ReturnsFalse)
{
    QList<QUrl> fromUrls = { QUrl::fromLocalFile("/tmp/file1.txt") };
    QUrl toUrl = QUrl::fromLocalFile("/tmp/archive.zip");

    stub.set_lamda(ADDR(UniversalUtils, urlsTransformToLocal),
                   [](const QList<QUrl> &in, QList<QUrl> *out) -> bool {
                       __DBG_STUB_INVOKE__
                       *out = in;
                       return true;
                   });

    stub.set_lamda(ADDR(ProtocolUtils, isFTPFile),
                   [](const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    typedef bool (EventSequenceManager::*RunFunc)(const QString &, const QString &, const QList<QUrl> &, const QUrl &);
    stub.set_lamda(static_cast<RunFunc>(&EventSequenceManager::run),
                   [] {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    auto mockFileInfo = QSharedPointer<FileInfo>(new FileInfo(toUrl));
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [&mockFileInfo](const QUrl &, const Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return mockFileInfo;
                   });

    stub.set_lamda(VADDR(FileInfo, isAttributes),
                   [](FileInfo *, const OptInfoType) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    stub.set_lamda(ADDR(AppendCompressHelper, isCompressedFile),
                   [](const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    EXPECT_FALSE(AppendCompressHelper::canAppendCompress(fromUrls, toUrl));
}

// ========== appendCompress 测试 ==========

TEST_F(UT_AppendCompressHelper, appendCompress_ValidParams_LaunchesCompressor)
{
    QString toFilePath = "/tmp/archive.zip";
    QStringList fromFilePaths = { "/tmp/file1.txt", "/tmp/file2.txt" };

    bool startDetachedCalled = false;

    using StartDetachedFunc = bool (*)(const QString &, const QStringList &, const QString &, qint64 *);
    stub.set_lamda(static_cast<StartDetachedFunc>(QProcess::startDetached),
                   [&] {
                       __DBG_STUB_INVOKE__
                       startDetachedCalled = true;
                       return true;
                   });

    bool result = AppendCompressHelper::appendCompress(toFilePath, fromFilePaths);

    EXPECT_TRUE(result);
    EXPECT_TRUE(startDetachedCalled);
}

// ========== dragDropCompress 测试 ==========

TEST_F(UT_AppendCompressHelper, dragDropCompress_ValidParams_ReturnsTrue)
{
    QList<QUrl> fromUrls = { QUrl::fromLocalFile("/tmp/file1.txt") };
    QUrl toUrl = QUrl::fromLocalFile("/tmp/archive.zip");

    stub.set_lamda(ADDR(UniversalUtils, urlsTransformToLocal),
                   [](const QList<QUrl> &in, QList<QUrl> *out) -> bool {
                       __DBG_STUB_INVOKE__
                       *out = in;
                       return true;
                   });

    stub.set_lamda(ADDR(AppendCompressHelper, canAppendCompress),
                   [](const QList<QUrl> &, const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    auto mockFileInfo = QSharedPointer<FileInfo>(new FileInfo(fromUrls[0]));
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [&mockFileInfo](const QUrl &, const Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return mockFileInfo;
                   });

    stub.set_lamda(VADDR(FileInfo, canAttributes),
                   [](FileInfo *, const CanableInfoType) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    stub.set_lamda(ADDR(AppendCompressHelper, appendCompress),
                   [](const QString &, const QStringList &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    EXPECT_TRUE(AppendCompressHelper::dragDropCompress(toUrl, fromUrls));
}

TEST_F(UT_AppendCompressHelper, dragDropCompress_RedirectedUrls_UsesRedirectedPath)
{
    QList<QUrl> fromUrls = { QUrl::fromLocalFile("/tmp/file1.txt") };
    QUrl toUrl = QUrl::fromLocalFile("/tmp/archive.zip");
    QUrl redirectedUrl = QUrl::fromLocalFile("/tmp/redirected.txt");

    stub.set_lamda(ADDR(UniversalUtils, urlsTransformToLocal),
                   [](const QList<QUrl> &in, QList<QUrl> *out) -> bool {
                       __DBG_STUB_INVOKE__
                       *out = in;
                       return true;
                   });

    stub.set_lamda(ADDR(AppendCompressHelper, canAppendCompress),
                   [](const QList<QUrl> &, const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    auto mockFileInfo = QSharedPointer<FileInfo>(new FileInfo(fromUrls[0]));
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [&mockFileInfo](const QUrl &, const Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return mockFileInfo;
                   });

    stub.set_lamda(VADDR(FileInfo, canAttributes),
                   [](FileInfo *, const CanableInfoType type) -> bool {
                       __DBG_STUB_INVOKE__
                       return type == CanableInfoType::kCanRedirectionFileUrl;
                   });

    stub.set_lamda(VADDR(FileInfo, urlOf),
                   [&redirectedUrl](FileInfo *, const UrlInfoType) -> QUrl {
                       __DBG_STUB_INVOKE__
                       return redirectedUrl;
                   });

    QString capturedPath;
    stub.set_lamda(ADDR(AppendCompressHelper, appendCompress),
                   [&capturedPath, &redirectedUrl](const QString &, const QStringList &fromPaths) -> bool {
                       __DBG_STUB_INVOKE__
                       if (!fromPaths.isEmpty())
                           capturedPath = fromPaths[0];
                       return true;
                   });

    EXPECT_TRUE(AppendCompressHelper::dragDropCompress(toUrl, fromUrls));
    EXPECT_EQ(capturedPath, redirectedUrl.path());
}

TEST_F(UT_AppendCompressHelper, dragDropCompress_InvalidParams_ReturnsFalse)
{
    QUrl toUrl = QUrl::fromLocalFile("/tmp/archive.zip");

    stub.set_lamda(ADDR(UniversalUtils, urlsTransformToLocal),
                   [](const QList<QUrl> &, QList<QUrl> *out) -> bool {
                       __DBG_STUB_INVOKE__
                       out->clear();
                       return true;
                   });

    EXPECT_FALSE(AppendCompressHelper::dragDropCompress(toUrl, { QUrl::fromLocalFile("/tmp/file.txt") }));

    stub.set_lamda(ADDR(UniversalUtils, urlsTransformToLocal),
                   [](const QList<QUrl> &in, QList<QUrl> *out) -> bool {
                       __DBG_STUB_INVOKE__
                       *out = in;
                       return true;
                   });

    stub.set_lamda(ADDR(AppendCompressHelper, canAppendCompress),
                   [](const QList<QUrl> &, const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    EXPECT_FALSE(AppendCompressHelper::dragDropCompress(toUrl, { QUrl::fromLocalFile("/tmp/file.txt") }));
}

// ========== setMouseStyle 测试 ==========

TEST_F(UT_AppendCompressHelper, setMouseStyle_CanAppend_SetsCopyAction)
{
    QList<QUrl> fromUrls = { QUrl::fromLocalFile("/tmp/file1.txt") };
    QUrl toUrl = QUrl::fromLocalFile("/tmp/archive.zip");
    Qt::DropAction dropAction = Qt::IgnoreAction;

    stub.set_lamda(ADDR(AppendCompressHelper, isCompressedFile),
                   [](const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(ADDR(AppendCompressHelper, canAppendCompress),
                   [](const QList<QUrl> &, const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    EXPECT_TRUE(AppendCompressHelper::setMouseStyle(toUrl, fromUrls, &dropAction));
    EXPECT_EQ(dropAction, Qt::CopyAction);
}

TEST_F(UT_AppendCompressHelper, setMouseStyle_CannotAppend_SetsIgnoreAction)
{
    QList<QUrl> fromUrls = { QUrl::fromLocalFile("/tmp/file1.txt") };
    QUrl toUrl = QUrl::fromLocalFile("/tmp/archive.zip");
    Qt::DropAction dropAction = Qt::CopyAction;

    stub.set_lamda(ADDR(AppendCompressHelper, isCompressedFile),
                   [](const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(ADDR(AppendCompressHelper, canAppendCompress),
                   [](const QList<QUrl> &, const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    EXPECT_TRUE(AppendCompressHelper::setMouseStyle(toUrl, fromUrls, &dropAction));
    EXPECT_EQ(dropAction, Qt::IgnoreAction);
}

TEST_F(UT_AppendCompressHelper, setMouseStyle_NotCompressedOrEmpty_ReturnsFalse)
{
    QUrl toUrl = QUrl::fromLocalFile("/tmp/document.txt");
    Qt::DropAction dropAction = Qt::CopyAction;

    stub.set_lamda(ADDR(AppendCompressHelper, isCompressedFile),
                   [](const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    EXPECT_FALSE(AppendCompressHelper::setMouseStyle(toUrl, { QUrl::fromLocalFile("/tmp/file.txt") }, &dropAction));

    EXPECT_FALSE(AppendCompressHelper::setMouseStyle(toUrl, QList<QUrl>(), &dropAction));
}
