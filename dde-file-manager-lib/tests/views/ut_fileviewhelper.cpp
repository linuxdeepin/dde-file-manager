#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QStandardPaths>
#include "views/dfileview.h"
#include "interfaces/dfilesystemmodel.h"
#define private public
#include "views/fileviewhelper.h"

namespace  {
    class FileViewHelperTest : public testing::Test
    {
    public:
        FileViewHelper *m_fileViewHelper;
        DFileView *m_view;
        virtual void SetUp() override
        {
            m_view = new DFileView;
            m_fileViewHelper = m_view->fileViewHelper();
        }

        virtual void TearDown() override
        {
            delete m_view;
            m_view = nullptr;
        }
    };
}

TEST_F(FileViewHelperTest,parent)
{
    ASSERT_NE(nullptr,m_fileViewHelper);
    EXPECT_EQ(m_view,m_fileViewHelper->parent());
}

TEST_F(FileViewHelperTest,windowId)
{
    EXPECT_EQ(m_view->windowId(),m_fileViewHelper->windowId());
}

TEST_F(FileViewHelperTest,is_selected)
{
    QModelIndex index;
    EXPECT_EQ(m_view->isSelected(index),m_fileViewHelper->isSelected(index));
}

TEST_F(FileViewHelperTest,is_droptarget)
{
    QModelIndex index;
    EXPECT_EQ(m_view->isDropTarget(index),m_fileViewHelper->isDropTarget(index));
}

TEST_F(FileViewHelperTest,selected_indexs_count)
{
    EXPECT_EQ(m_view->selectedIndexCount(),m_fileViewHelper->selectedIndexsCount());
}

TEST_F(FileViewHelperTest,row_count)
{
    EXPECT_EQ(m_view->rowCount(),m_fileViewHelper->rowCount());
}

TEST_F(FileViewHelperTest,index_of_row)
{
    QModelIndex index;
    EXPECT_EQ(m_view->indexOfRow(index),m_fileViewHelper->indexOfRow(index));
}

TEST_F(FileViewHelperTest,model)
{
    EXPECT_EQ(m_view->model(),m_fileViewHelper->model());
}

TEST_F(FileViewHelperTest,currentUrl)
{
    EXPECT_EQ(m_view->rootUrl(),m_fileViewHelper->currentUrl());
}

TEST_F(FileViewHelperTest,fileinfo)
{
    QModelIndex index;
    EXPECT_EQ(m_view->model()->fileInfo(index),m_fileViewHelper->fileInfo(index));
}

TEST_F(FileViewHelperTest,itemdelegate)
{
    EXPECT_EQ(m_view->itemDelegate(),m_fileViewHelper->itemDelegate());
}

TEST_F(FileViewHelperTest,column_rolelist)
{
    EXPECT_EQ(m_view->columnRoleList(),m_fileViewHelper->columnRoleList());
}

TEST_F(FileViewHelperTest,column_width)
{
    EXPECT_EQ(m_view->columnWidth(0),m_fileViewHelper->columnWidth(0));
}

//TEST_F(FileViewHelperTest,select)
//{
//    auto configPath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
//    auto desktopPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
//    DUrl path(configPath);
//    m_view->cd(path);

//    DUrl desktop(desktopPath);
//    m_fileViewHelper->select({desktop});
//    ASSERT_EQ(1,m_fileViewHelper->selectedUrls().size());
//    EXPECT_EQ(desktop,m_fileViewHelper->selectedUrls().first());
//}
