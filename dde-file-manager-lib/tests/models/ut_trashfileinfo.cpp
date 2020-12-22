#include <gtest/gtest.h>
#include <QIcon>
#include "stub.h"

#include "dfilesystemmodel.h"
#include "controllers/trashmanager.h"

#define private public
#define protected public
#include "models/trashfileinfo.h"
#include "models/trashfileinfo_p.h"

namespace {
class TestTrashFileInfo : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestTrashFileInfo";
        info = new TrashFileInfo(DUrl("trash:///"));
    }

    void TearDown() override
    {
        std::cout << "end TestTrashFileInfo";
        delete info;
        info = nullptr;
    }

public:
    TrashFileInfo *info;
};
} // namespace

TEST_F(TestTrashFileInfo, BoolPropertyTest)
{
    EXPECT_TRUE(info->exists());
    EXPECT_FALSE(info->canRename());
    EXPECT_TRUE(info->isReadable());
    EXPECT_TRUE(info->isWritable());
    EXPECT_FALSE(info->canShare());
    EXPECT_TRUE(info->isDir());
    EXPECT_TRUE(info->canIteratorDir());
    EXPECT_TRUE(info->makeAbsolute());
    EXPECT_FALSE(info->restore());
    info->setColumnCompact(true);
    EXPECT_TRUE(Qt::CopyAction == info->supportedDragActions());
    EXPECT_TRUE((Qt::MoveAction == info->supportedDropActions() || Qt::IgnoreAction == info->supportedDropActions()));
    TrashFileInfo newTrash(DUrl("file:///test"));
    EXPECT_FALSE(newTrash.restore());
}

TEST_F(TestTrashFileInfo, UrlTest)
{
    EXPECT_STREQ("", info->originUrl().path().toStdString().c_str());
    EXPECT_STREQ("/", info->goToUrlWhenDeleted().path().toStdString().c_str());
    EXPECT_TRUE(info->mimeDataUrl().path().contains(".local/share/Trash/files"));
}

TEST_F(TestTrashFileInfo, StringPropertyTest)
{
    EXPECT_STREQ("Trash", info->fileDisplayName().toStdString().c_str());
    EXPECT_STREQ("Folder is empty", info->subtitleForEmptyFloder().toStdString().c_str());
    EXPECT_STREQ("", info->sourceFilePath().toStdString().c_str());
}

TEST_F(TestTrashFileInfo, tstPermissions)
{
    EXPECT_TRUE(info->permissions() & ~QFileDevice::WriteOwner);
}

TEST_F(TestTrashFileInfo, tstMenuActionList)
{
    EXPECT_TRUE(7 <= info->menuActionList(DAbstractFileInfo::SpaceArea).count());
    EXPECT_TRUE(6 <= info->menuActionList(DAbstractFileInfo::SingleFile).count());
    EXPECT_TRUE(6 <= info->menuActionList(DAbstractFileInfo::MultiFiles).count());

    EXPECT_TRUE(1 <= info->disableMenuActionList().count());
}

TEST_F(TestTrashFileInfo, tstFuncsWithRoles)
{
    EXPECT_TRUE(5 <= info->userColumnRoles().count());
    EXPECT_FALSE((info->userColumnData(DFileSystemModel::FileUserRole + 1).value<QPair<QString, QString>>()).first.isEmpty());
    EXPECT_TRUE((info->userColumnData(DFileSystemModel::FileUserRole + 2).value<QPair<QString, QString>>()).first.isEmpty());
    EXPECT_TRUE(info->userColumnData(DFileSystemModel::FileUserRole + 3).isValid());
    EXPECT_TRUE(info->userColumnData(DFileSystemModel::FileUserRole + 4).isValid());
    EXPECT_FALSE(info->userColumnData(DFileSystemModel::FileUserRole + 5).isValid());

    EXPECT_TRUE(qApp->translate("DFileSystemModel", "Name") == info->userColumnDisplayName(DFileSystemModel::FileUserRole + 1));
    EXPECT_TRUE(QObject::tr("Time deleted") == info->userColumnDisplayName(DFileSystemModel::FileUserRole + 2));
    EXPECT_TRUE(QObject::tr("Source Path", "TrashFileInfo") == info->userColumnDisplayName(DFileSystemModel::FileUserRole + 3));
    EXPECT_TRUE(QObject::tr("Time deleted") == info->userColumnDisplayName(DFileSystemModel::FileUserRole + 4));
    EXPECT_FALSE(!info->userColumnDisplayName(DFileSystemModel::FileUserRole + 5).toString().isEmpty());

    EXPECT_TRUE(0 == info->userColumnChildRoles(0).count());

    QFont f;
    QFontMetrics fm(f);
    EXPECT_TRUE(-1 == info->userColumnWidth(DFileSystemModel::FileUserRole + 1, fm));
    EXPECT_TRUE(fm.width("0000/00/00 00:00:00") == info->userColumnWidth(DFileSystemModel::FileUserRole + 100, fm));

    EXPECT_TRUE(info->columnDefaultVisibleForRole(0));

    EXPECT_TRUE(MenuAction::SourcePath == info->menuActionByColumnRole(DFileSystemModel::FileUserRole + 3));
    EXPECT_TRUE(MenuAction::DeletionDate == info->menuActionByColumnRole(DFileSystemModel::FileUserRole + 4));
    EXPECT_FALSE(info->menuActionByColumnRole(Qt::UserRole) > 0);

    EXPECT_TRUE(5 == info->sortSubMenuActionUserColumnRoles().count());

    EXPECT_TRUE(info->compareFunByColumn(DFileSystemModel::FileUserRole + 3));
    EXPECT_TRUE(info->compareFunByColumn(DFileSystemModel::FileUserRole + 4));
    EXPECT_FALSE(info->compareFunByColumn(DFileSystemModel::FileUserRole + 5));
}

TEST_F(TestTrashFileInfo, tstFileIcon)
{
    EXPECT_FALSE(info->fileIcon().isNull());
    EXPECT_TRUE(info->additionalIcon().isEmpty());
}

TEST_F(TestTrashFileInfo, tstUpdateInfo)
{
    bool (*exists_stub)(void *) = [](void *) { return true; };
    bool (*QFile_exist)(const QString &) = &QFile::exists;
    Stub st;
    st.set(QFile_exist, exists_stub);
    TrashFileInfo *trash = new TrashFileInfo(DUrl("trash:///tst.txt"));
    trash->d_func()->updateInfo();
    trash->d_func()->inheritParentTrashInfo();
    trash->deletionDate();
}
