// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QUrl>

#include "stubext.h"

#include "fileoperationsevent/fileoperationseventreceiver.h"
#include "fileoperations/filecopymovejob.h"
#include "fileoperations/operationsstackproxy.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/utils/hidefilehelper.h>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_fileoperations;

class RenameReservedNameTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();

        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);

        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());

        receiver = FileOperationsEventReceiver::instance();
    }

    void TearDown() override
    {
        stub.clear();
        tempDir.reset();
    }

    QUrl createTestFile(const QString &fileName)
    {
        QString filePath = tempDir->path() + "/" + fileName;
        QFile file(filePath);
        file.open(QIODevice::WriteOnly);
        file.write("test content");
        file.close();
        return QUrl::fromLocalFile(filePath);
    }

    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
    FileOperationsEventReceiver *receiver { nullptr };
};

TEST_F(RenameReservedNameTest, RenameToDot_ReturnsFalse_AndShowsReservedDialog)
{
    bool dialogShown = false;
    stub.set_lamda(static_cast<int (DialogManager::*)(const QString &, const QString &, QString)>(
                       &DialogManager::showMessageDialog),
                   [&dialogShown](DialogManager *, const QString &title, const QString &, QString) {
                       __DBG_STUB_INVOKE__
                       if (title.contains("reserved"))
                           dialogShown = true;
                       return 0;
                   });

    QUrl oldUrl = createTestFile("original.txt");
    QUrl newUrl = QUrl::fromLocalFile(tempDir->path() + "/.");

    bool result = receiver->handleOperationRenameFile(0, oldUrl, newUrl,
                                                     AbstractJobHandler::JobFlag::kNoHint);

    EXPECT_FALSE(result);
    EXPECT_TRUE(dialogShown);
}

TEST_F(RenameReservedNameTest, RenameToDotDot_ReturnsFalse_AndShowsReservedDialog)
{
    bool dialogShown = false;
    stub.set_lamda(static_cast<int (DialogManager::*)(const QString &, const QString &, QString)>(
                       &DialogManager::showMessageDialog),
                   [&dialogShown](DialogManager *, const QString &title, const QString &, QString) {
                       __DBG_STUB_INVOKE__
                       if (title.contains("reserved"))
                           dialogShown = true;
                       return 0;
                   });

    QUrl oldUrl = createTestFile("original.txt");
    QUrl newUrl = QUrl::fromLocalFile(tempDir->path() + "/..");

    bool result = receiver->handleOperationRenameFile(0, oldUrl, newUrl,
                                                     AbstractJobHandler::JobFlag::kNoHint);

    EXPECT_FALSE(result);
    EXPECT_TRUE(dialogShown);
}

TEST_F(RenameReservedNameTest, RenameToNormalName_NotBlockedByReservedCheck)
{
    stub.set_lamda(static_cast<int (DialogManager::*)(const QString &, const QString &, QString)>(
                       &DialogManager::showMessageDialog),
                   [](DialogManager *, const QString &, const QString &, QString) {
                       __DBG_STUB_INVOKE__
                       return 0;
                   });

    stub.set_lamda(&LocalFileHandler::renameFile,
                   [](LocalFileHandler *, const QUrl &, const QUrl &, const bool) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    QUrl oldUrl = createTestFile("original.txt");
    QUrl newUrl = QUrl::fromLocalFile(tempDir->path() + "/renamed.txt");

    bool result = receiver->handleOperationRenameFile(0, oldUrl, newUrl,
                                                     AbstractJobHandler::JobFlag::kNoHint);

    EXPECT_TRUE(result);
}
