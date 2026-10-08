// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// PMS 129249: 复制 0KB 文件后视图不刷新（文件未及时显示），需手动刷新才可见。
// 回归点：doCopyFileTraditional 完成 0 字节文件拷贝并 refresh 后，
//        必须发布 FileUtils::notifyFileChangeManual(kFileAdded, 目标 url)，
//        使视图无需手动刷新即可看到新文件。

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QUrl>

#include "stubext.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-io/dfileinfo.h>

#include "fileoperations/fileoperationutils/docopyfileworker.h"
#include "fileoperations/fileoperationutils/workerdata.h"

DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_USE_NAMESPACE

class DoCopyFileWorkerPms129249 : public testing::Test
{
public:
    void SetUp() override
    {
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);

        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());

        workData.reset(new WorkerData);
        worker = new DoCopyFileWorker(workData);
        ASSERT_TRUE(worker);
    }

    void TearDown() override
    {
        stub.clear();
        if (worker) {
            worker->stop();
            delete worker;
            worker = nullptr;
        }
        workData.reset();
        tempDir.reset();
    }

protected:
    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
    QSharedPointer<WorkerData> workData;
    DoCopyFileWorker *worker { nullptr };
};

// PMS:129249 复制 0 字节文件：拷贝完成后应发布 kFileAdded 手动通知，目标文件即时可见
TEST_F(DoCopyFileWorkerPms129249, BUG129249_DoCopyFileTraditional_ZeroSizeFile_NotifyFileAdded)
{
    const QString srcPath = tempDir->filePath("zero-src.txt");
    QFile srcFile(srcPath);
    ASSERT_TRUE(srcFile.open(QIODevice::WriteOnly));
    srcFile.close();

    const QString dstPath = tempDir->filePath("zero-dst.txt");
    const QUrl dstUrl = QUrl::fromLocalFile(dstPath);

    auto fromInfo = DFileInfoPointer(new DFileInfo(QUrl::fromLocalFile(srcPath)));
    fromInfo->initQuerier();
    ASSERT_EQ(fromInfo->attribute(DFileInfo::AttributeID::kStandardSize).toLongLong(), 0);
    auto toInfo = DFileInfoPointer(new DFileInfo(dstUrl));

    stub.set_lamda(&DoCopyFileWorker::readAheadSourceFile,
                   [](DoCopyFileWorker *, const DFileInfoPointer &) {
                   });

    int notifyCount = 0;
    Global::FileNotifyType notifyType = static_cast<Global::FileNotifyType>(-1);
    QUrl notifyUrl;
    stub.set_lamda(static_cast<void (*)(Global::FileNotifyType, const QUrl &)>(&FileUtils::notifyFileChangeManual),
                   [&](Global::FileNotifyType type, const QUrl &url) {
                       __DBG_STUB_INVOKE__
                       ++notifyCount;
                       notifyType = type;
                       notifyUrl = url;
                   });

    bool skip = false;
    const auto result = worker->doCopyFileTraditional(fromInfo, toInfo, &skip);

    EXPECT_EQ(result, DoCopyFileWorker::NextDo::kDoCopyNext);
    EXPECT_TRUE(QFile::exists(dstPath));
    EXPECT_EQ(QFile(dstPath).size(), 0);
    EXPECT_EQ(notifyCount, 1);
    EXPECT_EQ(notifyType, Global::FileNotifyType::kFileAdded);
    EXPECT_EQ(notifyUrl, dstUrl);
}
