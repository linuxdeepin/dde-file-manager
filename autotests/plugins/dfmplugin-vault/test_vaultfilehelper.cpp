// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QUrl>

#include "stubext.h"

#include "utils/vaultfilehelper.h"
#include "utils/vaulthelper.h"
#include "utils/pathmanager.h"

#include <dfm-base/utils/windowutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/fileutils.h>

DPVAULT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

class VaultFileHelperImpl : public testing::Test
{
public:
    void SetUp() override
    {
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());

        testFile = tempDir->path() + "/test.txt";
        QFile f(testFile);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("hello");
        f.close();

        stub.set_lamda(&PathManager::makeVaultLocalPath, [this](const QString &, const QString &) -> QString {
            return tempDir->path();
        });
    }

    void TearDown() override
    {
        stub.clear();
        tempDir.reset();
    }

    QUrl vaultUrlFor(const QString &path) const
    {
        QUrl url;
        url.setScheme("dfmvault");
        url.setPath(path);
        return url;
    }

protected:
    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
    QString testFile;
};

TEST_F(VaultFileHelperImpl, Scheme)
{
    EXPECT_EQ(VaultFileHelper::scheme(), QString("dfmvault"));
}

TEST_F(VaultFileHelperImpl, CutFile_WrongTargetScheme)
{
    VaultFileHelper *h = VaultFileHelper::instance();
    QUrl target = QUrl::fromLocalFile(testFile);
    EXPECT_FALSE(h->cutFile(0, {}, target, {}));
}

TEST_F(VaultFileHelperImpl, CutFile_VaultTarget)
{
    VaultFileHelper *h = VaultFileHelper::instance();
    QUrl target = vaultUrlFor(tempDir->path());
    EXPECT_TRUE(h->cutFile(0, {}, target, {}));
}

TEST_F(VaultFileHelperImpl, CopyFile_WrongTargetScheme)
{
    VaultFileHelper *h = VaultFileHelper::instance();
    QUrl target = QUrl::fromLocalFile(testFile);
    EXPECT_FALSE(h->copyFile(0, {}, target, {}));
}

TEST_F(VaultFileHelperImpl, CopyFile_VaultTarget)
{
    VaultFileHelper *h = VaultFileHelper::instance();
    QUrl target = vaultUrlFor(tempDir->path());
    EXPECT_TRUE(h->copyFile(0, {}, target, {}));
}

TEST_F(VaultFileHelperImpl, MoveToTrash_EmptySources)
{
    VaultFileHelper *h = VaultFileHelper::instance();
    EXPECT_FALSE(h->moveToTrash(0, {}, {}));
}

TEST_F(VaultFileHelperImpl, MoveToTrash_NonVaultSource)
{
    VaultFileHelper *h = VaultFileHelper::instance();
    QList<QUrl> sources = { QUrl::fromLocalFile("/etc/passwd") };
    EXPECT_FALSE(h->moveToTrash(0, sources, {}));
}

TEST_F(VaultFileHelperImpl, MoveToTrash_VaultSource)
{
    VaultFileHelper *h = VaultFileHelper::instance();
    QList<QUrl> sources = { vaultUrlFor(testFile) };
    EXPECT_TRUE(h->moveToTrash(0, sources, {}));
}

TEST_F(VaultFileHelperImpl, DeleteFile_EmptySources)
{
    VaultFileHelper *h = VaultFileHelper::instance();
    EXPECT_FALSE(h->deleteFile(0, {}, {}));
}

TEST_F(VaultFileHelperImpl, DeleteFile_WrongScheme)
{
    VaultFileHelper *h = VaultFileHelper::instance();
    QList<QUrl> sources = { QUrl::fromLocalFile(testFile) };
    EXPECT_FALSE(h->deleteFile(0, sources, {}));
}

TEST_F(VaultFileHelperImpl, DeleteFile_VaultSource)
{
    VaultFileHelper *h = VaultFileHelper::instance();
    QList<QUrl> sources = { vaultUrlFor(testFile) };
    EXPECT_TRUE(h->deleteFile(0, sources, {}));
}

TEST_F(VaultFileHelperImpl, OpenFileInPlugin_Empty)
{
    VaultFileHelper *h = VaultFileHelper::instance();
    EXPECT_FALSE(h->openFileInPlugin(0, {}));
}

TEST_F(VaultFileHelperImpl, OpenFileInPlugin_WrongScheme)
{
    VaultFileHelper *h = VaultFileHelper::instance();
    QList<QUrl> urls = { QUrl::fromLocalFile(testFile) };
    EXPECT_FALSE(h->openFileInPlugin(0, urls));
}

TEST_F(VaultFileHelperImpl, OpenFileInPlugin_VaultUrl)
{
    VaultFileHelper *h = VaultFileHelper::instance();
    QList<QUrl> urls = { vaultUrlFor(testFile) };
    EXPECT_TRUE(h->openFileInPlugin(0, urls));
}

TEST_F(VaultFileHelperImpl, RenameFile_WrongOldScheme)
{
    VaultFileHelper *h = VaultFileHelper::instance();
    QUrl oldUrl = QUrl::fromLocalFile(testFile);
    QUrl newUrl = vaultUrlFor(tempDir->path() + "/new.txt");
    EXPECT_FALSE(h->renameFile(0, oldUrl, newUrl, {}));
}

TEST_F(VaultFileHelperImpl, RenameFile_VaultUrls)
{
    VaultFileHelper *h = VaultFileHelper::instance();
    QUrl oldUrl = vaultUrlFor(testFile);
    QUrl newUrl = vaultUrlFor(tempDir->path() + "/new.txt");
    EXPECT_TRUE(h->renameFile(0, oldUrl, newUrl, {}));
}

TEST_F(VaultFileHelperImpl, MakeDir_WrongScheme)
{
    VaultFileHelper *h = VaultFileHelper::instance();
    QUrl url = QUrl::fromLocalFile(tempDir->path() + "/dir");
    EXPECT_FALSE(h->makeDir(0, url, {}, {}, nullptr));
}

TEST_F(VaultFileHelperImpl, MakeDir_VaultUrl)
{
    VaultFileHelper *h = VaultFileHelper::instance();
    QUrl url = vaultUrlFor(tempDir->path() + "/dir");
    EXPECT_TRUE(h->makeDir(0, url, {}, QVariant(), nullptr));
}

TEST_F(VaultFileHelperImpl, TouchFile_WrongScheme)
{
    VaultFileHelper *h = VaultFileHelper::instance();
    QUrl url = QUrl::fromLocalFile(tempDir->path() + "/file");
    EXPECT_FALSE(h->touchFile(0, url, {}, DFMGLOBAL_NAMESPACE::CreateFileType::kCreateFileTypeText, "", {}, nullptr, nullptr));
}

TEST_F(VaultFileHelperImpl, TouchFile_VaultUrl)
{
    VaultFileHelper *h = VaultFileHelper::instance();
    QUrl url = vaultUrlFor(tempDir->path() + "/file");
    bool called = false;
    auto cb = [&called](const AbstractJobHandler::CallbackArgus &) { called = true; };
    EXPECT_TRUE(h->touchFile(0, url, {}, DFMGLOBAL_NAMESPACE::CreateFileType::kCreateFileTypeText, "txt", {}, cb, nullptr));
    EXPECT_TRUE(called);
}

TEST_F(VaultFileHelperImpl, TouchCustomFile_WrongScheme)
{
    VaultFileHelper *h = VaultFileHelper::instance();
    QUrl url = QUrl::fromLocalFile(tempDir->path() + "/file");
    EXPECT_FALSE(h->touchCustomFile(0, url, {}, {}, "txt", {}, nullptr, nullptr));
}

TEST_F(VaultFileHelperImpl, TouchCustomFile_VaultUrl)
{
    VaultFileHelper *h = VaultFileHelper::instance();
    QUrl url = vaultUrlFor(tempDir->path() + "/file");
    bool called = false;
    auto cb = [&called](const AbstractJobHandler::CallbackArgus &) { called = true; };
    EXPECT_TRUE(h->touchCustomFile(0, url, {}, {}, "txt", {}, cb, nullptr));
    EXPECT_TRUE(called);
}

TEST_F(VaultFileHelperImpl, WriteUrlsToClipboard_Empty)
{
    VaultFileHelper *h = VaultFileHelper::instance();
    EXPECT_FALSE(h->writeUrlsToClipboard(0, ClipBoard::kCopyAction, {}));
}

TEST_F(VaultFileHelperImpl, WriteUrlsToClipboard_WrongScheme)
{
    VaultFileHelper *h = VaultFileHelper::instance();
    QList<QUrl> urls = { QUrl::fromLocalFile(testFile) };
    EXPECT_FALSE(h->writeUrlsToClipboard(0, ClipBoard::kCopyAction, urls));
}

TEST_F(VaultFileHelperImpl, WriteUrlsToClipboard_VaultUrl)
{
    VaultFileHelper *h = VaultFileHelper::instance();
    QList<QUrl> urls = { vaultUrlFor(testFile) };
    EXPECT_TRUE(h->writeUrlsToClipboard(0, ClipBoard::kCopyAction, urls));
}

TEST_F(VaultFileHelperImpl, RenameFiles_WrongScheme)
{
    VaultFileHelper *h = VaultFileHelper::instance();
    QList<QUrl> urls = { QUrl::fromLocalFile(testFile) };
    EXPECT_FALSE(h->renameFiles(0, urls, {}, false));
}

TEST_F(VaultFileHelperImpl, RenameFiles_VaultUrl)
{
    VaultFileHelper *h = VaultFileHelper::instance();
    QList<QUrl> urls = { vaultUrlFor(testFile) };
    EXPECT_TRUE(h->renameFiles(0, urls, { "a", "b" }, false));
}

TEST_F(VaultFileHelperImpl, RenameFilesAddText_WrongScheme)
{
    VaultFileHelper *h = VaultFileHelper::instance();
    QList<QUrl> urls = { QUrl::fromLocalFile(testFile) };
    EXPECT_FALSE(h->renameFilesAddText(0, urls, { "a", AbstractJobHandler::FileNameAddFlag::kPrefix }));
}

TEST_F(VaultFileHelperImpl, RenameFilesAddText_VaultUrl)
{
    VaultFileHelper *h = VaultFileHelper::instance();
    QList<QUrl> urls = { vaultUrlFor(testFile) };
    EXPECT_TRUE(h->renameFilesAddText(0, urls, { "a", AbstractJobHandler::FileNameAddFlag::kPrefix }));
}

TEST_F(VaultFileHelperImpl, CheckDragDropAction_Move)
{
    stub.set_lamda(&WindowUtils::keyAltIsPressed, []() -> bool { return true; });
    stub.set_lamda(&WindowUtils::keyCtrlIsPressed, []() -> bool { return false; });

    VaultFileHelper *h = VaultFileHelper::instance();
    Qt::DropAction action = Qt::IgnoreAction;
    EXPECT_TRUE(h->checkDragDropAction({ vaultUrlFor(testFile) }, vaultUrlFor(tempDir->path()), &action));
    EXPECT_EQ(action, Qt::MoveAction);
}

TEST_F(VaultFileHelperImpl, CheckDragDropAction_Copy)
{
    stub.set_lamda(&WindowUtils::keyAltIsPressed, []() -> bool { return false; });
    stub.set_lamda(&WindowUtils::keyCtrlIsPressed, []() -> bool { return true; });

    VaultFileHelper *h = VaultFileHelper::instance();
    Qt::DropAction action = Qt::IgnoreAction;
    EXPECT_TRUE(h->checkDragDropAction({ vaultUrlFor(testFile) }, vaultUrlFor(tempDir->path()), &action));
    EXPECT_EQ(action, Qt::CopyAction);
}

TEST_F(VaultFileHelperImpl, CheckDragDropAction_DefaultVaultToVault)
{
    stub.set_lamda(&WindowUtils::keyAltIsPressed, []() -> bool { return false; });
    stub.set_lamda(&WindowUtils::keyCtrlIsPressed, []() -> bool { return false; });

    VaultFileHelper *h = VaultFileHelper::instance();
    Qt::DropAction action = Qt::IgnoreAction;
    EXPECT_TRUE(h->checkDragDropAction({ vaultUrlFor(testFile) }, vaultUrlFor(tempDir->path()), &action));
    EXPECT_EQ(action, Qt::MoveAction);
}

TEST_F(VaultFileHelperImpl, CheckDragDropAction_DefaultInOut)
{
    stub.set_lamda(&WindowUtils::keyAltIsPressed, []() -> bool { return false; });
    stub.set_lamda(&WindowUtils::keyCtrlIsPressed, []() -> bool { return false; });

    VaultFileHelper *h = VaultFileHelper::instance();
    Qt::DropAction action = Qt::IgnoreAction;
    EXPECT_TRUE(h->checkDragDropAction({ QUrl::fromLocalFile("/etc/passwd") }, vaultUrlFor(tempDir->path()), &action));
    EXPECT_EQ(action, Qt::CopyAction);
}

TEST_F(VaultFileHelperImpl, HandleDropFiles_Move)
{
    stub.set_lamda(&WindowUtils::keyAltIsPressed, []() -> bool { return true; });
    stub.set_lamda(&WindowUtils::keyCtrlIsPressed, []() -> bool { return false; });
    stub.set_lamda(&UniversalUtils::urlsTransformToLocal, [](const QList<QUrl> &, QList<QUrl> *out) -> bool {
        if (out)
            *out << QUrl::fromLocalFile("/tmp/transformed");
        return true;
    });

    VaultFileHelper *h = VaultFileHelper::instance();
    EXPECT_TRUE(h->handleDropFiles({ vaultUrlFor(testFile) }, vaultUrlFor(tempDir->path())));
}

TEST_F(VaultFileHelperImpl, SetPermision_WrongScheme)
{
    VaultFileHelper *h = VaultFileHelper::instance();
    QUrl url = QUrl::fromLocalFile(testFile);
    bool ok = false;
    QString error;
    EXPECT_FALSE(h->setPermision(0, url, QFileDevice::ReadOwner, &ok, &error));
}

TEST_F(VaultFileHelperImpl, SetPermision_VaultUrl)
{
    VaultFileHelper *h = VaultFileHelper::instance();
    QUrl url = vaultUrlFor(testFile);
    bool ok = false;
    QString error;
    EXPECT_TRUE(h->setPermision(0, url, QFileDevice::ReadOwner, &ok, &error));
    EXPECT_TRUE(ok);
}
