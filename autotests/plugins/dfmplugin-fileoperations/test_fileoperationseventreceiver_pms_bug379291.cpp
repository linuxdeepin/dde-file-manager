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

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/interfaces/abstractjobhandler.h>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_fileoperations;

class TestFileOpsEventReceiverBug379291 : public testing::Test
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
        file.write("test");
        file.close();
        return QUrl::fromLocalFile(filePath);
    }

    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
    FileOperationsEventReceiver *receiver { nullptr };
};

// PMS:379291 重命名为 "." 时应被拒绝并弹出保留名提示
TEST_F(TestFileOpsEventReceiverBug379291, RenameToDot_Rejected)
{
    bool dialogShown = false;
    stub.set_lamda(static_cast<int (DialogManager::*)(const QString &, const QString &, QString)>(
                           &DialogManager::showMessageDialog),
                   [&dialogShown](DialogManager *, const QString &title, const QString &, const QString &) -> int {
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

// PMS:379291 重命名为 ".." 时应被拒绝并弹出保留名提示
TEST_F(TestFileOpsEventReceiverBug379291, RenameToDotDot_Rejected)
{
    bool dialogShown = false;
    stub.set_lamda(static_cast<int (DialogManager::*)(const QString &, const QString &, QString)>(
                           &DialogManager::showMessageDialog),
                   [&dialogShown](DialogManager *, const QString &title, const QString &, const QString &) -> int {
                       __DBG_STUB_INVOKE__
                       if (title.contains("reserved"))
                           dialogShown = true;
                       return 0;
                   });

    QUrl oldUrl = createTestFile("original2.txt");
    QUrl newUrl = QUrl::fromLocalFile(tempDir->path() + "/..");

    bool result = receiver->handleOperationRenameFile(0, oldUrl, newUrl,
                                                      AbstractJobHandler::JobFlag::kNoHint);

    EXPECT_FALSE(result);
    EXPECT_TRUE(dialogShown);
}
