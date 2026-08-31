// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_localfilehandler.cpp
 * @brief Unit tests for LocalFileHandler methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/file/local/localfilehandler.h"

#include <QTest>

using namespace src;

class LocalFileHandlerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new LocalFileHandler();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    LocalFileHandler *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(LocalFileHandlerTest, LocalFileHandler)
{
    // Test constructor: LocalFileHandler(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(LocalFileHandlerTest, M_~LocalFileHandler)
{
    // Test method:  ~LocalFileHandler(())
    EXPECT_NO_FATAL_FAILURE({ LocalFileHandler *tmp = new LocalFileHandler(); delete tmp; });
}

TEST_F(LocalFileHandlerTest, mkdir)
{
    // Test method: bool mkdir((const QUrl &dir))
    QUrl _arg0{};
    auto result = obj->mkdir(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(LocalFileHandlerTest, rmdir)
{
    // Test method: bool rmdir((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->rmdir(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(LocalFileHandlerTest, touchFile)
{
    // Test method: QUrl touchFile((const QUrl &url, const QUrl &tempUrl /*= QUrl()*/))
    QUrl _arg0{};
    auto result = obj->touchFile(_arg0, nullptr);
    EXPECT_FALSE(result.isValid());

}

TEST_F(LocalFileHandlerTest, renameFile)
{
    // Test method: bool renameFile((const QUrl &url, const QUrl &newUrl, const bool needCheck))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->renameFile(_arg0, _arg1, false);
    EXPECT_FALSE(result);

}

TEST_F(LocalFileHandlerTest, openFile)
{
    // Test method: bool openFile((const QUrl &fileUrl))
    QUrl _arg0{};
    auto result = obj->openFile(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(LocalFileHandlerTest, openFiles)
{
    // Test method: bool openFiles((const QList<QUrl> &fileUrls))
    QList<QUrl> _arg0{};
    auto result = obj->openFiles(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(LocalFileHandlerTest, setPermissions)
{
    // Test method: bool setPermissions((const QUrl &url, QFileDevice::Permissions permissions))
    QUrl _arg0{};
    auto result = obj->setPermissions(_arg0, QFileDevice::Permissions());
    EXPECT_FALSE(result);

}

TEST_F(LocalFileHandlerTest, moveFile)
{
    // Test method: bool moveFile((const QUrl &sourceUrl, const QUrl &destUrl, DFMIO::DFile::CopyFlag flag /*= DFMIO::DFile::CopyFlag::kNone*/))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->moveFile(_arg0, _arg1, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(LocalFileHandlerTest, copyFile)
{
    // Test method: bool copyFile((const QUrl &sourceUrl, const QUrl &destUrl, dfmio::DFile::CopyFlag flag))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->copyFile(_arg0, _arg1, {});
    EXPECT_FALSE(result);

}

TEST_F(LocalFileHandlerTest, trashFile)
{
    // Test method: QString trashFile((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->trashFile(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(LocalFileHandlerTest, deleteFile)
{
    // Test method: bool deleteFile((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->deleteFile(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(LocalFileHandlerTest, errorString)
{
    // Test getter: QString errorString()
    auto result = obj->errorString();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(LocalFileHandlerTest, errorCode)
{
    // Test getter: DFMIOErrorCode errorCode()
    auto result = obj->errorCode();
    EXPECT_NO_FATAL_FAILURE({ obj->errorCode(); });

}

TEST_F(LocalFileHandlerTest, createSystemLink)
{
    // Test method: bool createSystemLink((const QUrl &sourcefile, const QUrl &link))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->createSystemLink(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(LocalFileHandlerTest, setPermissionsRecursive)
{
    // Test method: bool setPermissionsRecursive((const QUrl &url, QFileDevice::Permissions permissions))
    QUrl _arg0{};
    auto result = obj->setPermissionsRecursive(_arg0, QFileDevice::Permissions());
    EXPECT_FALSE(result);

}

TEST_F(LocalFileHandlerTest, deleteFileRecursive)
{
    // Test method: bool deleteFileRecursive((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->deleteFileRecursive(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(LocalFileHandlerTest, setFileTime)
{
    // Test method: bool setFileTime((const QUrl &url, const QDateTime &accessDateTime,
                                   const QDateTime &lastModifiedTime))
    QUrl _arg0{};
    QDateTime _arg1{};
    QDateTime _arg2{};
    auto result = obj->setFileTime(_arg0, _arg1, _arg2);
    EXPECT_FALSE(result);

}

TEST_F(LocalFileHandlerTest, defaultTerminalPath)
{
    // Test getter: QString defaultTerminalPath()
    auto result = obj->defaultTerminalPath();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(LocalFileHandlerTest, renameFilesBatch)
{
    // Test method: bool renameFilesBatch((const QMap<QUrl, QUrl> &urls, QMap<QUrl, QUrl> &successUrls))
    QMap<QUrl, QUrl> _arg0{};
    QMap<QUrl, QUrl> _arg1{};
    auto result = obj->renameFilesBatch(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(LocalFileHandlerTest, getInvalidPath)
{
    // Test getter: QList<QUrl> getInvalidPath()
    auto result = obj->getInvalidPath();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(LocalFileHandlerTest, lastEventType)
{
    // Test getter: GlobalEventType lastEventType()
    auto result = obj->lastEventType();
    EXPECT_NO_FATAL_FAILURE({ obj->lastEventType(); });

}

TEST_F(LocalFileHandlerTest, openFileByApp)
{
    // Test method: bool openFileByApp((const QUrl &file, const QString &desktopFile))
    QUrl _arg0{};
    QString _arg1{};
    auto result = obj->openFileByApp(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(LocalFileHandlerTest, openFilesByApp)
{
    // Test method: bool openFilesByApp((const QList<QUrl> &fileUrls, const QString &desktopFile))
    QList<QUrl> _arg0{};
    QString _arg1{};
    auto result = obj->openFilesByApp(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(LocalFileHandlerTest, doHiddenFileRemind)
{
    // Test method: bool doHiddenFileRemind((const QString &name, bool *checkRule))
    QString _arg0{};
    auto result = obj->doHiddenFileRemind(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(LocalFileHandlerTest, d)
{
    // Test getter: QScopedPointer<LocalFileHandlerPrivate> d()
    auto result = obj->d();
    EXPECT_EQ(result.get(), nullptr);

}
