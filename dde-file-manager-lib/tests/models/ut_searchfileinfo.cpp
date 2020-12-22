#include "dfilesystemmodel.h"

#include <gtest/gtest.h>
#include <QProcess>
#include <QIcon>

#include "models/searchfileinfo.h"
namespace  {
class TestSearchFileInfo : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestSearchFileInfo";
        QProcess::execute("touch /tmp/1.txt");
        DUrl fileUrl;
        fileUrl.setScheme(SEARCH_SCHEME);
        fileUrl.setQuery("url=file:///tmp&keyword=hello");
        fileUrl.setSearchedFileUrl(DUrl::fromLocalFile("/tmp/1.txt"));
        fileUrl.setFragment("file:///tmp/1.txt");
        info = new SearchFileInfo(fileUrl);
    }

    void TearDown() override
    {
        std::cout << "end TestSearchFileInfo";
        QProcess::execute("rm -f /tmp/1.txt");
        delete info;
        info = nullptr;
    }

public:
    SearchFileInfo *info;
};
}

TEST_F(TestSearchFileInfo, exists)
{
    EXPECT_TRUE(info->exists());
}

TEST_F(TestSearchFileInfo, isReadable)
{
    EXPECT_TRUE(info->isReadable());
}

TEST_F(TestSearchFileInfo, isDir)
{
    EXPECT_FALSE(info->isDir());
}

TEST_F(TestSearchFileInfo, isVirtualEntry)
{
    EXPECT_FALSE(info->isVirtualEntry());
}

TEST_F(TestSearchFileInfo, filesCount)
{
    int cnt = info->filesCount();
    EXPECT_EQ(-1, cnt);
}

TEST_F(TestSearchFileInfo, parentUrl)
{
    DUrl url = info->parentUrl();
    EXPECT_TRUE(url.isValid());
}

TEST_F(TestSearchFileInfo, userColumnRoles)
{
    QList<int> colRoles = info->userColumnRoles();
    EXPECT_TRUE(colRoles.count() > 0);
}

TEST_F(TestSearchFileInfo, userColumnDisplayName)
{
    QString displayName = info->userColumnDisplayName(DFileSystemModel::FileUserRole + 1).toString();
    EXPECT_EQ("Path", displayName);
    displayName = info->userColumnDisplayName(DFileSystemModel::FileUserRole + 2).toString();
    EXPECT_EQ("Time modified", displayName);
    displayName = info->userColumnDisplayName(DFileSystemModel::FilePathRole).toString();
    EXPECT_EQ("Path", displayName);
}

TEST_F(TestSearchFileInfo, userColumnData)
{
    QString displayName = info->userColumnData(DFileSystemModel::FileUserRole + 2).toString();
    EXPECT_EQ("", displayName);
    displayName = info->userColumnData(DFileSystemModel::FilePathRole).toString();
    EXPECT_EQ("/tmp", displayName);
    info->userColumnData(Qt::UserRole);
}

TEST_F(TestSearchFileInfo, userColumnChildRoles)
{
    QList<int> roles = info->userColumnChildRoles(0);
    EXPECT_TRUE(roles.isEmpty());
}

TEST_F(TestSearchFileInfo, columnDefaultVisibleForRole)
{
    EXPECT_TRUE(info->columnDefaultVisibleForRole(0));
}

TEST_F(TestSearchFileInfo, canRedirectionFileUrl)
{
    EXPECT_TRUE(info->canRedirectionFileUrl());
}

TEST_F(TestSearchFileInfo, redirectedFileUrl)
{
    DUrl url = info->redirectedFileUrl();
    EXPECT_EQ(DUrl::fromLocalFile("/tmp/1.txt"), url);
}

TEST_F(TestSearchFileInfo, canIteratorDir)
{
    EXPECT_FALSE(info->canIteratorDir());
}

TEST_F(TestSearchFileInfo, menuActionList)
{
    EXPECT_TRUE(info->menuActionList().count() > 0);
    EXPECT_TRUE(info->menuActionList(DAbstractFileInfo::MenuType::SpaceArea).count() > 0);
}

TEST_F(TestSearchFileInfo, disableMenuActionList)
{
    EXPECT_TRUE(info->disableMenuActionList().count() > 0);
}

TEST_F(TestSearchFileInfo, menuActionByColumnRole)
{
    EXPECT_EQ(MenuAction::AbsolutePath, info->menuActionByColumnRole(DFileSystemModel::FilePathRole));
    EXPECT_EQ(MenuAction::Name, info->menuActionByColumnRole(DFileSystemModel::FileNameRole));
}

TEST_F(TestSearchFileInfo, sortSubMenuActionUserColumnRoles)
{
    EXPECT_TRUE(info->sortSubMenuActionUserColumnRoles().count() > 0);
}

TEST_F(TestSearchFileInfo, isEmptyFloder)
{
    EXPECT_FALSE(info->isEmptyFloder());
}

TEST_F(TestSearchFileInfo, hasOrderly)
{
    EXPECT_FALSE(info->hasOrderly());
}

TEST_F(TestSearchFileInfo, getUrlByNewFileName)
{
    DUrl url = info->getUrlByNewFileName("2.txt");
    EXPECT_EQ("/tmp/2.txt", url.toLocalFile());
}

TEST_F(TestSearchFileInfo, loadingTip)
{
    EXPECT_EQ("Searching...", info->loadingTip());
}

TEST_F(TestSearchFileInfo, subtitleForEmptyFloder)
{
    EXPECT_EQ("No results", info->subtitleForEmptyFloder());
}

TEST_F(TestSearchFileInfo, fileDisplayName)
{
    EXPECT_EQ("1.txt", info->fileDisplayName());
}

TEST_F(TestSearchFileInfo, mimeDataUrl)
{
    DUrl url = info->mimeDataUrl();
    EXPECT_EQ("/tmp/1.txt", url.toLocalFile());
}

TEST_F(TestSearchFileInfo, toLocalFile)
{
    EXPECT_EQ("/tmp/1.txt", info->toLocalFile());
}

TEST_F(TestSearchFileInfo, tstSetColumnCompact)
{
    info->setColumnCompact(true);
}

TEST_F(TestSearchFileInfo, tstUserColumnWidth)
{
    QFont f;
    QFontMetrics fm(f);
    EXPECT_EQ(-1, info->userColumnWidth(DFileSystemModel::FileUserRole + 1, fm));
    EXPECT_EQ(fm.width("0000/00/00 00:00:00"), info->userColumnWidth(DFileSystemModel::FileUserRole + 2, fm));
}

TEST_F(TestSearchFileInfo, tstCompareFunByColumn)
{
    EXPECT_TRUE(info->compareFunByColumn(DFileSystemModel::FilePathRole));
    EXPECT_FALSE(info->compareFunByColumn(Qt::UserRole));
}

TEST_F(TestSearchFileInfo, tstFileIcon)
{
    EXPECT_FALSE(info->fileIcon().isNull());
}
