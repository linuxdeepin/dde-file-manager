#include "models/sharefileinfo.h"
#include "dfilesystemmodel.h"

#include <gtest/gtest.h>
#include <QIcon>

namespace {
class TestShareFileInfo : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestShareFileInfo";
        info = new ShareFileInfo(DUrl());
    }

    void TearDown() override
    {
        std::cout << "end TestShareFileInfo";
    }

public:
    ShareFileInfo *info;
};
} // namespace

TEST_F(TestShareFileInfo, tstBasicProperties)
{
    EXPECT_FALSE(info->exists());
    EXPECT_FALSE(info->isDir());
    EXPECT_FALSE(info->canRename());
    EXPECT_TRUE(info->isReadable());
    EXPECT_FALSE(info->isWritable());
    EXPECT_TRUE("" == info->fileDisplayName());
    EXPECT_FALSE(info->canIteratorDir());
    EXPECT_FALSE(info->makeAbsolute());
    EXPECT_TRUE(info->mimeDataUrl().isValid());
    EXPECT_TRUE(info->isShared());
    EXPECT_FALSE(info->canTag());
    EXPECT_FALSE(info->parentUrl().isValid());
    EXPECT_FALSE(info->isVirtualEntry());
    ShareFileInfo i(DUrl(USERSHARE_ROOT));
    EXPECT_TRUE(info->isVirtualEntry());
    EXPECT_FALSE(info->canDrop());
    EXPECT_TRUE((Qt::ItemIsDragEnabled | Qt::ItemIsEditable) == info->fileItemDisableFlags());
    EXPECT_TRUE(info->additionalIcon().count() > 0);
    EXPECT_TRUE(info->canRedirectionFileUrl());
    EXPECT_TRUE(info->redirectedFileUrl().isValid());
}

TEST_F(TestShareFileInfo, tstFuncsWithRole)
{
    EXPECT_FALSE(info->columnDefaultVisibleForRole(DFileSystemModel::FileLastModifiedRole));
    EXPECT_TRUE(info->columnDefaultVisibleForRole(Qt::UserRole));

    EXPECT_TRUE(info->menuActionByColumnRole(DFileSystemModel::FileUserRole + 1) == MenuAction::DeletionDate);
    EXPECT_TRUE(info->menuActionByColumnRole(DFileSystemModel::FileUserRole + 2) == MenuAction::SourcePath);
    EXPECT_TRUE(info->menuActionByColumnRole(Qt::UserRole) == MenuAction::Unknow);
}

TEST_F(TestShareFileInfo, tstMenuActionList)
{
    EXPECT_TRUE(2 == info->menuActionList(DAbstractFileInfo::SpaceArea).count());
    EXPECT_TRUE(2 == info->menuActionList(DAbstractFileInfo::SingleFile).count());
    EXPECT_TRUE(3 == info->menuActionList(DAbstractFileInfo::MultiFiles).count());
    EXPECT_TRUE(0 == info->disableMenuActionList().count());
}
