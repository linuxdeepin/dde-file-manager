// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/shred/shredfilemodel.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/fileinfo.h>

#include <QIcon>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;
DFMBASE_USE_NAMESPACE

class UT_ShredFileModel : public testing::Test
{
protected:
    void SetUp() override
    {
        model = new ShredFileModel();
    }

    void TearDown() override
    {
        delete model;
        model = nullptr;
        stub.clear();
    }

    ShredFileModel *model { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_ShredFileModel, Constructor_InitializesModel)
{
    EXPECT_NE(model, nullptr);
    EXPECT_EQ(model->rowCount(), 0);
}

TEST_F(UT_ShredFileModel, setFileList_UpdatesModel)
{
    QList<QUrl> urls = {
        QUrl::fromLocalFile("/tmp/file1.txt"),
        QUrl::fromLocalFile("/tmp/file2.txt")
    };

    model->setFileList(urls);

    EXPECT_EQ(model->rowCount(), 2);
}

TEST_F(UT_ShredFileModel, rowCount_ReturnsCorrectCount)
{
    EXPECT_EQ(model->rowCount(), 0);

    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/test.txt") };
    model->setFileList(urls);

    EXPECT_EQ(model->rowCount(), 1);
}

TEST_F(UT_ShredFileModel, columnCount_ReturnsOne)
{
    EXPECT_EQ(model->columnCount(), 1);
}

TEST_F(UT_ShredFileModel, parent_ReturnsInvalidIndex)
{
    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/test.txt") };
    model->setFileList(urls);

    QModelIndex childIndex = model->index(0, 0);
    QModelIndex parentIndex = model->parent(childIndex);

    EXPECT_FALSE(parentIndex.isValid());
}

TEST_F(UT_ShredFileModel, index_ValidRow_ReturnsValidIndex)
{
    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/test.txt") };
    model->setFileList(urls);

    QModelIndex idx = model->index(0, 0);

    EXPECT_TRUE(idx.isValid());
    EXPECT_EQ(idx.row(), 0);
    EXPECT_EQ(idx.column(), 0);
}

TEST_F(UT_ShredFileModel, index_InvalidRow_ReturnsInvalidIndex)
{
    QModelIndex idx = model->index(100, 0);

    EXPECT_FALSE(idx.isValid());
}

TEST_F(UT_ShredFileModel, index_NegativeRow_ReturnsInvalidIndex)
{
    QModelIndex idx = model->index(-1, 0);

    EXPECT_FALSE(idx.isValid());
}

TEST_F(UT_ShredFileModel, data_InvalidIndex_ReturnsEmpty)
{
    QModelIndex idx;
    QVariant result = model->data(idx, Qt::DisplayRole);

    EXPECT_FALSE(result.isValid());
}

TEST_F(UT_ShredFileModel, data_NullFileInfo_ReturnsEmpty)
{
    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/test.txt") };
    model->setFileList(urls);

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &, const Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    QModelIndex idx = model->index(0, 0);
    QVariant result = model->data(idx, Qt::DisplayRole);

    EXPECT_FALSE(result.isValid());
}

TEST_F(UT_ShredFileModel, data_DisplayRole_ReturnsFileName)
{
    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/test.txt") };
    model->setFileList(urls);

    auto mockFileInfo = QSharedPointer<FileInfo>(new FileInfo(urls[0]));
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [&mockFileInfo](const QUrl &, const Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return mockFileInfo;
                   });

    stub.set_lamda(VADDR(FileInfo, exists),
                   [](FileInfo *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(VADDR(FileInfo, displayOf),
                   [](FileInfo *, const DisPlayInfoType) -> QString {
                       __DBG_STUB_INVOKE__
                       return "test.txt";
                   });

    QModelIndex idx = model->index(0, 0);
    QVariant result = model->data(idx, Qt::DisplayRole);

    EXPECT_EQ(result.toString(), "test.txt");
}

TEST_F(UT_ShredFileModel, data_DecorationRole_ReturnsIcon)
{
    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/test.txt") };
    model->setFileList(urls);

    auto mockFileInfo = QSharedPointer<FileInfo>(new FileInfo(urls[0]));
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [&mockFileInfo](const QUrl &, const Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return mockFileInfo;
                   });

    stub.set_lamda(VADDR(FileInfo, exists),
                   [](FileInfo *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(VADDR(FileInfo, fileIcon),
                   [](FileInfo *) -> QIcon {
                       __DBG_STUB_INVOKE__
                       return QIcon::fromTheme("text-plain");
                   });

    QModelIndex idx = model->index(0, 0);
    QVariant result = model->data(idx, Qt::DecorationRole);

    EXPECT_TRUE(result.canConvert<QIcon>());
}

TEST_F(UT_ShredFileModel, data_UnknownRole_ReturnsEmpty)
{
    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/test.txt") };
    model->setFileList(urls);

    auto mockFileInfo = QSharedPointer<FileInfo>(new FileInfo(urls[0]));
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [&mockFileInfo](const QUrl &, const Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return mockFileInfo;
                   });

    stub.set_lamda(VADDR(FileInfo, exists),
                   [](FileInfo *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    QModelIndex idx = model->index(0, 0);
    QVariant result = model->data(idx, Qt::UserRole + 100);

    EXPECT_FALSE(result.isValid());
}

