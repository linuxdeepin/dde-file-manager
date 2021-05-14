#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QTimer>
#include <dfmevent.h>
#include "stubext.h"
#define private public
#include "interfaces/dfilesystemmodel.h"
#include "interfaces/dfilesystemmodel_p.h"
#undef private
#include "views/dfileview.h"
#include "views/fileviewhelper.h"
#include "testhelper.h"
#include "dfileservices.h"

namespace  {
class TestDFileSystemModel : public testing::Test
{

    void SetUp() override
    {
        std::cout << "start TestDFileSystemModel" << std::endl;
        m_view = new DFileView;
        m_helper = new FileViewHelper(m_view);
        m_model = new DFileSystemModel(m_helper);
        QString tmpDirPath = TestHelper::createTmpDir();
        tmpDirUrl = DUrl::fromLocalFile(tmpDirPath);
        QString tempFilePath = TestHelper::createTmpFileName("1.txt", tmpDirPath);
        tmpFileUrl = DUrl::fromLocalFile(tempFilePath);
        QString tempFilePath2 = TestHelper::createTmpFileName("2.txt", tmpDirPath);
        tmpFileUrl2 = DUrl::fromLocalFile(tempFilePath2);
    }

    void TearDown() override
    {
        std::cout << "end TestDFileSystemModel" << std::endl;
        delete  m_model;
        delete  m_helper;
        delete  m_view;
        TestHelper::deleteTmpFile(tmpDirUrl.path());
        TestHelper::runInLoop([]() {}, 10);
    }

public:
    DFileSystemModel *m_model;
    DFileView *m_view;
    FileViewHelper *m_helper;
    DUrl tmpFileUrl;
    DUrl tmpFileUrl2;
    DUrl tmpDirUrl;
};


TEST_F(TestDFileSystemModel, test_parent)
{
    ASSERT_NE(m_model->parent(), nullptr);
}

TEST_F(TestDFileSystemModel, test_setRootUrl)
{
    QModelIndex rootIndex = m_model->setRootUrl(tmpDirUrl);
    TestHelper::runInLoop([&] {
        ASSERT_TRUE(m_model->canFetchMore(rootIndex));
        m_model->fetchMore(rootIndex);
    });
    QString tempFilePath = TestHelper::createTmpFileName("3.txt", tmpDirUrl.path());
    DUrl newTmpFileUrl = DUrl::fromLocalFile(tempFilePath);
    TestHelper::runInLoop([&] {
    }, 200);
    EXPECT_NO_FATAL_FAILURE(m_model->rowCount(rootIndex));
    TestHelper::rename(tempFilePath, (tempFilePath + ".back"));
    TestHelper::deleteTmpFile(tempFilePath + ".back");
    TestHelper::runInLoop([&] {
    }, 200);

    EXPECT_NO_FATAL_FAILURE(m_model->rowCount(rootIndex));
}

TEST_F(TestDFileSystemModel, test_sortedUrls)
{
    QModelIndex rootIndex = m_model->setRootUrl(tmpDirUrl);
    TestHelper::runInLoop([&] {
        ASSERT_TRUE(m_model->canFetchMore(rootIndex));
        m_model->fetchMore(rootIndex);
    }, 200);
    DUrlList list = m_model->sortedUrls();
    ASSERT_EQ(list.count(), 2);
}
#ifndef __arm__
TEST_F(TestDFileSystemModel, test_data)
{
    QModelIndex rootIndex = m_model->setRootUrl(tmpDirUrl);
    TestHelper::runInLoop([&] {
        ASSERT_TRUE(m_model->canFetchMore(rootIndex));
        m_model->fetchMore(rootIndex);
    });
    DAbstractFileInfoPointer tmpFileInfo = DFileService::instance()->createFileInfo(nullptr, tmpFileUrl);
    QModelIndex index = m_model->index(0, 0);
    QString name = m_model->data(index,   Qt::DisplayRole).toString();
    ASSERT_EQ(tmpFileUrl.fileName(), name);
    ASSERT_EQ(m_model->data(index, DFileSystemModel::FilePathRole).toString(), tmpFileInfo->filePath());
    ASSERT_EQ(m_model->data(index, DFileSystemModel::FileDisplayNameRole).toString(), tmpFileInfo->fileDisplayName());
    ASSERT_EQ(m_model->data(index, DFileSystemModel::FileNameRole).toString(), tmpFileInfo->fileName());
    ASSERT_EQ(m_model->data(index, DFileSystemModel::FileNameOfRenameRole).toString(), tmpFileInfo->fileNameOfRename());
    ASSERT_EQ(m_model->data(index, DFileSystemModel::FileBaseNameRole).toString(), tmpFileInfo->baseName());
    ASSERT_EQ(m_model->data(index, DFileSystemModel::FileBaseNameOfRenameRole).toString(), tmpFileInfo->baseNameOfRename());
    ASSERT_EQ(m_model->data(index, DFileSystemModel::FileSuffixRole).toString(), tmpFileInfo->suffix());
    ASSERT_EQ(m_model->data(index, DFileSystemModel::FileSuffixOfRenameRole).toString(), tmpFileInfo->suffixOfRename());
    ASSERT_EQ((m_model->data(index, DFileSystemModel::FileIconRole).value<QIcon>()).name(), tmpFileInfo->fileIcon().name());
    ASSERT_EQ(m_model->data(index, Qt::TextAlignmentRole).toInt(), Qt::AlignVCenter);
    ASSERT_EQ(m_model->data(index, DFileSystemModel::FileLastModifiedRole).toString(), tmpFileInfo->lastModifiedDisplayName());
    ASSERT_EQ(m_model->data(index, DFileSystemModel::FileLastModifiedDateTimeRole).toDateTime(), tmpFileInfo->lastModified());
    ASSERT_EQ(m_model->data(index, DFileSystemModel::FileSizeRole).toString(), tmpFileInfo->sizeDisplayName());
    ASSERT_EQ(m_model->data(index, DFileSystemModel::FileSizeInKiloByteRole).value<qint64>(), tmpFileInfo->size());
    ASSERT_EQ(m_model->data(index, DFileSystemModel::FileMimeTypeRole).toString(), tmpFileInfo->mimeTypeDisplayName());
    ASSERT_EQ(m_model->data(index, DFileSystemModel::FileCreatedRole).toString(), tmpFileInfo->createdDisplayName());
    ASSERT_EQ(m_model->data(index, DFileSystemModel::FilePinyinName).toString(), tmpFileInfo->fileDisplayPinyinName());
    //ToolTipRole
    ASSERT_EQ(m_model->data(index, DFileSystemModel::ExtraProperties).toHash(), tmpFileInfo->extraProperties());

    ASSERT_TRUE(m_model->data(index, DFileSystemModel::UnknowRole).isNull());
    //FileIconModelToolTipRole
}
#endif
TEST_F(TestDFileSystemModel, test_fileInfo)
{
    QModelIndex rootIndex = m_model->setRootUrl(tmpDirUrl);
    TestHelper::runInLoop([&] {
        ASSERT_TRUE(m_model->canFetchMore(rootIndex));
        m_model->fetchMore(rootIndex);
    }, 500);
    DAbstractFileInfoPointer tmpFileInfo = DFileService::instance()->createFileInfo(nullptr, tmpFileUrl);
    DAbstractFileInfoPointer fileInfo = m_model->fileInfo(tmpFileUrl);
    ASSERT_EQ(tmpFileInfo->filePath(), fileInfo->filePath());
    DAbstractFileInfoPointer fileInfo1 = m_model->fileInfo(m_model->index(tmpFileUrl));
    ASSERT_EQ(tmpFileInfo->filePath(), fileInfo1->filePath());
}

TEST_F(TestDFileSystemModel, test_parentFileInfo)
{
    QModelIndex rootIndex = m_model->setRootUrl(tmpDirUrl);
    TestHelper::runInLoop([&] {
        ASSERT_TRUE(m_model->canFetchMore(rootIndex));
        m_model->fetchMore(rootIndex);
    }, 500);
    DAbstractFileInfoPointer tmpFileInfo = DFileService::instance()->createFileInfo(nullptr, tmpFileUrl);
    DAbstractFileInfoPointer fileInfo = m_model->parentFileInfo(tmpFileUrl);
    ASSERT_EQ(m_model->rootUrl(), fileInfo->fileUrl());
}

TEST_F(TestDFileSystemModel, test_parent_index)
{
    QModelIndex rootIndex = m_model->setRootUrl(tmpDirUrl);
    TestHelper::runInLoop([&] {
        ASSERT_TRUE(m_model->canFetchMore(rootIndex));
        m_model->fetchMore(rootIndex);
    }, 500);
    QModelIndex index = m_model->parent(m_model->index(tmpFileUrl));
    ASSERT_EQ(rootIndex, index);
}

TEST_F(TestDFileSystemModel, test_roleName)
{
    ASSERT_EQ(m_model->roleName(DFileSystemModel::FileDisplayNameRole), m_model->tr("Name"));
    ASSERT_EQ(m_model->roleName(DFileSystemModel::FileNameRole), m_model->tr("Name"));
    ASSERT_EQ(m_model->roleName(DFileSystemModel::FileLastModifiedRole), m_model->tr("Time modified"));
    ASSERT_EQ(m_model->roleName(DFileSystemModel::FileSizeRole), m_model->tr("Size"));
    ASSERT_EQ(m_model->roleName(DFileSystemModel::FileMimeTypeRole), m_model->tr("Type"));
    ASSERT_EQ(m_model->roleName(DFileSystemModel::FileCreatedRole), m_model->tr("Time created"));
    ASSERT_EQ(m_model->roleName(DFileSystemModel::FileLastReadRole), m_model->tr("Last access"));
    ASSERT_EQ(m_model->roleName(DFileSystemModel::FilePathRole), m_model->tr("Path"));
    ASSERT_TRUE(m_model->roleName(DFileSystemModel::Unknow).isEmpty());
}

TEST_F(TestDFileSystemModel, test_columnToRole)
{
    ASSERT_EQ(m_model->columnToRole(0), DFileSystemModel::UnknowRole);
    m_model->setRootUrl(tmpDirUrl);
    ASSERT_EQ(m_model->columnToRole(0), DFileSystemModel::FileDisplayNameRole);
}

TEST_F(TestDFileSystemModel, test_roleToColumn)
{
    ASSERT_EQ(m_model->roleToColumn(DFileSystemModel::FileLastModifiedRole), -1);
    m_model->setRootUrl(tmpDirUrl);
    ASSERT_EQ(m_model->roleToColumn(DFileSystemModel::UnknowRole), -1);
    ASSERT_EQ(m_model->roleToColumn(DFileSystemModel::FileLastModifiedRole), 1);
}

TEST_F(TestDFileSystemModel, test_flags)
{
    ASSERT_EQ(m_model->flags(QModelIndex()), Qt::NoItemFlags);
    QModelIndex rootIndex = m_model->setRootUrl(tmpDirUrl);
    Qt::ItemFlags flags = m_model->flags(rootIndex);
    ASSERT_TRUE(flags & Qt::ItemIsDropEnabled);
    ASSERT_TRUE(flags & Qt::ItemIsEditable);
    ASSERT_TRUE(flags & Qt::ItemIsDragEnabled);
}

TEST_F(TestDFileSystemModel, test_supportedDragActions)
{
    ASSERT_EQ(m_model->supportedDragActions(), Qt::CopyAction | Qt::MoveAction | Qt::LinkAction);
    m_model->setRootUrl(tmpDirUrl);
    Qt::DropActions dropActions = m_model->supportedDragActions();
    DAbstractFileInfoPointer tmpFileInfo = DFileService::instance()->createFileInfo(nullptr, tmpFileUrl);
    ASSERT_EQ(dropActions, tmpFileInfo->supportedDragActions());
}

TEST_F(TestDFileSystemModel, test_supportedDropActions)
{
    ASSERT_EQ(m_model->supportedDropActions(), Qt::CopyAction | Qt::MoveAction | Qt::LinkAction);
    m_model->setRootUrl(tmpDirUrl);
    Qt::DropActions dropActions = m_model->supportedDropActions();
    DAbstractFileInfoPointer tmpFileInfo = DFileService::instance()->createFileInfo(nullptr, tmpFileUrl);
    ASSERT_EQ(dropActions, tmpFileInfo->supportedDropActions());
}

TEST_F(TestDFileSystemModel, test_mimeTypes)
{
    QStringList list = m_model->mimeTypes();
    ASSERT_EQ(list.first(), QLatin1String("text/uri-list"));
}

#ifndef __arm__
TEST_F(TestDFileSystemModel, test_getUrlByIndex)
{
    DUrl url = m_model->getUrlByIndex(QModelIndex());
    ASSERT_TRUE(url.isEmpty());
    QModelIndex rootIndex = m_model->setRootUrl(tmpDirUrl);
    TestHelper::runInLoop([&] {
        ASSERT_TRUE(m_model->canFetchMore(rootIndex));
        m_model->fetchMore(rootIndex);
    }, 1000);
    QModelIndex index = m_model->index(0, 0);
    url = m_model->getUrlByIndex(index);
    DAbstractFileInfoPointer tmpFileInfo = DFileService::instance()->createFileInfo(nullptr, tmpFileUrl);
    ASSERT_EQ(url, tmpFileUrl);
}
#endif

TEST_F(TestDFileSystemModel, test_setEnabledSort)
{
    m_model->setEnabledSort(true);
    ASSERT_TRUE(m_model->enabledSort());
    m_model->setEnabledSort(false);
    ASSERT_FALSE(m_model->enabledSort());
}

TEST_F(TestDFileSystemModel, test_setReadOnly)
{
    m_model->setReadOnly(true);
    ASSERT_TRUE(m_model->isReadOnly());
    m_model->setReadOnly(false);
    ASSERT_FALSE(m_model->isReadOnly());
}

TEST_F(TestDFileSystemModel, test_setState)
{
    DFileSystemModel::State busyState = DFileSystemModel::State::Busy;
    m_model->setState(busyState);
    ASSERT_EQ(m_model->state(), busyState);
}


TEST_F(TestDFileSystemModel, test_setFilters)
{
    QDir::Filter filters = QDir::Filter::Files;
    m_model->setFilters(filters);
    ASSERT_EQ(m_model->filters(), filters);
    m_model->setFilters(filters);
    ASSERT_EQ(m_model->filters(), filters);
}


TEST_F(TestDFileSystemModel, test_setColumnActiveRole_no_rootNode)
{

    DFileSystemModel::Roles role = DFileSystemModel::Roles::ExtraProperties;
    m_model->setColumnActiveRole(1, role);
    ASSERT_EQ(m_model->columnActiveRole(1), DFileSystemModel::Roles::UnknowRole);
}

TEST_F(TestDFileSystemModel, test_setSortColumn)
{
    DFileSystemModel::Roles unknowRole = DFileSystemModel::Roles::UnknowRole;
    Qt::SortOrder order = Qt::SortOrder::DescendingOrder;
    m_model->setSortColumn(1, order);
    ASSERT_EQ(m_model->sortRole(), unknowRole);
    ASSERT_EQ(m_model->sortOrder(), order);
    QModelIndex rootIndex = m_model->setRootUrl(tmpDirUrl);
    TestHelper::runInLoop([&] {
        ASSERT_TRUE(m_model->canFetchMore(rootIndex));
        m_model->fetchMore(rootIndex);
    }, 1000);
    m_model->setSortColumn(0, order);
    ASSERT_EQ(m_model->sortRole(), DFileSystemModel::FileDisplayNameRole);
    ASSERT_EQ(m_model->sortOrder(), order);
}


TEST_F(TestDFileSystemModel, test_setSortRole)
{

    DFileSystemModel::Roles role = DFileSystemModel::Roles::ExtraProperties;
    Qt::SortOrder order = Qt::SortOrder::DescendingOrder;
    m_model->setSortRole(role, order);
    ASSERT_EQ(m_model->sortRole(), role);
    ASSERT_EQ(m_model->sortOrder(), order);
}

TEST_F(TestDFileSystemModel, test_setNameFilters)
{
    QString fileName = "testFileName";
    QStringList nameFilters = QStringList() << fileName;
    m_model->setNameFilters(nameFilters);
    ASSERT_EQ(m_model->nameFilters(), nameFilters);
    m_model->setNameFilters(nameFilters);
    QStringList filters = m_model->nameFilters();
    ASSERT_EQ(filters.first(), fileName);
}

TEST_F(TestDFileSystemModel, test_setSortOrder)
{
    Qt::SortOrder order = Qt::DescendingOrder;
    m_model->setSortOrder(order);
    ASSERT_EQ(m_model->sortOrder(), order);
}

TEST_F(TestDFileSystemModel, test_setAdvanceSearchFilter)
{
    QMap<int, QVariant> formData;
    QString searchRange = qApp->translate("DFMAdvanceSearchBar", "Current directory");
    QString fileType = qApp->translate("MimeTypeDisplayManager", "Video");
    QString dateRange = qApp->translate("DFMAdvanceSearchBar", "Today");
    QString size = qApp->translate("DFMAdvanceSearchBar", "0 ~ 100 KB");

    formData[SEARCH_RANGE] = searchRange;
    formData[FILE_TYPE] = fileType;
    formData[SIZE_RANGE] = size;
    formData[DATE_RANGE] = dateRange;
    formData[ACCESS_DATE_RANGE] = dateRange;
    formData[CREATE_DATE_RANGE] = dateRange;
    QModelIndex rootIndex = m_model->setRootUrl(tmpDirUrl);
    TestHelper::runInLoop([&] {
        ASSERT_TRUE(m_model->canFetchMore(rootIndex));
        m_model->fetchMore(rootIndex);
        m_model->setAdvanceSearchFilter(formData, true, true);
    }, 500);
    ASSERT_TRUE(m_model->advanceSearchFilter());

}

TEST(FileSystemNodeTest, setNodeVisible)
{
    QReadWriteLock lk;
    DAbstractFileInfoPointer info;
    FileSystemNode node(nullptr, info, nullptr, &lk);
    FileSystemNodePointer data;
    node.visibleChildren.append(data);
    node.setNodeVisible(data, false);
    EXPECT_TRUE(node.visibleChildren.isEmpty());

    node.setNodeVisible(data, true);
    EXPECT_FALSE(node.visibleChildren.isEmpty());
}

TEST(FileSystemNodeTest, noLockAppendChildren)
{
    QReadWriteLock lk;
    DAbstractFileInfoPointer info;
    FileSystemNode node(nullptr, info, nullptr, &lk);
    FileSystemNodePointer data;
    DUrl url("/usr/bin");
    node.noLockAppendChildren(url, data);
    EXPECT_FALSE(node.visibleChildren.isEmpty());
}

TEST(FileSystemNodeTest, appendChildren)
{
    QReadWriteLock lk;
    DAbstractFileInfoPointer info;
    FileSystemNode node(nullptr, info, nullptr, &lk);
    FileSystemNodePointer data;
    DUrl url("/usr/bin");
    node.appendChildren(url, data);
    EXPECT_FALSE(node.visibleChildren.isEmpty());
}

TEST(FileSystemNodeTest, takeNodeByUrl)
{
    QReadWriteLock lk;
    DAbstractFileInfoPointer info;
    FileSystemNode node(nullptr, info, nullptr, &lk);
    FileSystemNodePointer data;
    DUrl url("/usr/bin");
    node.children.insert(url, data);

    auto ret = node.takeNodeByUrl(url);
    EXPECT_EQ(ret, data);
}

}
