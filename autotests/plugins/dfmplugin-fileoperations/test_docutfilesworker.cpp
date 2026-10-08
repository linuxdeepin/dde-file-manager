// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// PMS 337471: 剪切粘贴时目标同名文件未做"替换"处理，rename 失败后任务静默失败（文件未被剪切）。
// 回归点：renameFileByHandler 命中已存在目标时应走三步替换（临时改名/写入/应用），静默完成替换；
//        trySameDeviceRename 对同目录新命名文件应直接重命名成功。

#include "stubext.h"

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QUrl>
#include <QTextStream>
#include <QDir>

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-io/dfileinfo.h>

#include "fileoperations/fileoperationutils/fileoperatebaseworker.h"
#include "fileoperations/fileoperationutils/workerdata.h"

// DoCutFilesWorker 构造函数为 private（仅 friend CutFiles 可构造），测试需要开放访问控制
#define private public
#define protected public
#include "fileoperations/cutfiles/docutfilesworker.h"
#undef protected
#undef private

DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_USE_NAMESPACE

namespace {
DFileInfoPointer makeDInfo337471(const QUrl &url)
{
    return DFileInfoPointer(new DFileInfo(url));
}

QByteArray readFile337471(const QString &path)
{
    QFile f(path);
    EXPECT_TRUE(f.open(QIODevice::ReadOnly));
    return f.readAll();
}

void writeFile337471(const QString &path, const QByteArray &content)
{
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write(content);
    f.close();
}
}   // namespace

class DoCutFilesWorkerTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);

        ASSERT_TRUE(tempDir.isValid());
        worker = new DoCutFilesWorker();
        worker->localFileHandler.reset(new LocalFileHandler);
        worker->targetUrl = QUrl::fromLocalFile(tempDir.path());
    }

    void TearDown() override
    {
        delete worker;
        worker = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    QTemporaryDir tempDir;
    DoCutFilesWorker *worker = nullptr;
};

// PMS:337471 剪切 A 覆盖已存在的 B：应成功替换（B 内容变为 A 内容，A 消失），不弹窗不静默失败
TEST_F(DoCutFilesWorkerTest, BUG337471_RenameFileByHandler_ReplacesExistingTargetSilently)
{
    const QString srcPath = tempDir.filePath("cut-src.txt");
    const QString dstPath = tempDir.filePath("cut-dst.txt");
    writeFile337471(srcPath, "SOURCE-CONTENT-A");
    writeFile337471(dstPath, "TARGET-CONTENT-B");

    bool skip = false;
    bool ok = worker->renameFileByHandler(makeDInfo337471(QUrl::fromLocalFile(srcPath)),
                                          makeDInfo337471(QUrl::fromLocalFile(dstPath)),
                                          &skip);
    EXPECT_TRUE(ok);
    EXPECT_FALSE(skip);
    EXPECT_FALSE(QFile::exists(srcPath));
    EXPECT_TRUE(QFile::exists(dstPath));
    EXPECT_EQ(readFile337471(dstPath), QByteArray("SOURCE-CONTENT-A"));
}

// PMS:337471 目标不存在时的普通剪切重命名仍应正常成功
TEST_F(DoCutFilesWorkerTest, BUG337471_RenameFileByHandler_PlainRenameSucceeds)
{
    const QString srcPath = tempDir.filePath("cut-plain-src.txt");
    const QString dstPath = tempDir.filePath("cut-plain-dst.txt");
    writeFile337471(srcPath, "PLAIN-CONTENT");

    bool skip = false;
    bool ok = worker->renameFileByHandler(makeDInfo337471(QUrl::fromLocalFile(srcPath)),
                                          makeDInfo337471(QUrl::fromLocalFile(dstPath)),
                                          &skip);
    EXPECT_TRUE(ok);
    EXPECT_FALSE(skip);
    EXPECT_FALSE(QFile::exists(srcPath));
    EXPECT_TRUE(QFile::exists(dstPath));
    EXPECT_EQ(readFile337471(dstPath), QByteArray("PLAIN-CONTENT"));
}

// PMS:337471 trySameDeviceRename 同设备新命名场景：应成功重命名并返回目标信息
TEST_F(DoCutFilesWorkerTest, BUG337471_TrySameDeviceRename_NewNameInTargetDir_Succeeds)
{
    const QString srcPath = tempDir.filePath("same-dev-src.txt");
    const QString dstPath = tempDir.filePath("renamed-by-cut.txt");
    writeFile337471(srcPath, "SAME-DEVICE-CONTENT");

    bool ok = false;
    bool skip = false;
    const DFileInfoPointer targetPathInfo = makeDInfo337471(QUrl::fromLocalFile(tempDir.path()));
    const DFileInfoPointer newInfo = worker->trySameDeviceRename(
            makeDInfo337471(QUrl::fromLocalFile(srcPath)),
            targetPathInfo,
            QStringLiteral("renamed-by-cut.txt"),
            &ok, &skip);

    EXPECT_TRUE(ok);
    EXPECT_FALSE(skip);
    EXPECT_FALSE(newInfo.isNull());
    EXPECT_FALSE(QFile::exists(srcPath));
    EXPECT_TRUE(QFile::exists(dstPath));
    EXPECT_EQ(readFile337471(dstPath), QByteArray("SAME-DEVICE-CONTENT"));
}
