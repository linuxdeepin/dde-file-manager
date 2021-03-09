/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QStandardPaths>
#include "stubext.h"
#include "views/dfileview.h"
#include "interfaces/dfilesystemmodel.h"
#include "app/filesignalmanager.h"
#include "app/define.h"
#include "utils/singleton.h"

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

TEST_F(FileViewHelperTest,select)
{
    auto configPath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
    auto desktopPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    DUrl path("file://" + configPath);
    m_view->setRootUrl(path);
    QEventLoop loop;
    QObject::connect(m_view->model(),&DFileSystemModel::sigJobFinished,&loop,[&loop](){
        loop.exit();
    });
    m_fileViewHelper->refreshFileView(m_view->windowId());
    loop.exec();
    ASSERT_NE(0,m_fileViewHelper->rowCount());

    DUrl desktop("file://" + desktopPath);
    m_fileViewHelper->select({desktop});
    ASSERT_EQ(1,m_fileViewHelper->selectedUrls().size());
    EXPECT_EQ(desktop,m_fileViewHelper->selectedUrls().first());

    m_fileViewHelper->select({});
    m_fileViewHelper->selectAll(-1);
    EXPECT_EQ(0,m_fileViewHelper->selectedUrls().size());

    m_fileViewHelper->selectAll(m_fileViewHelper->windowId());
    EXPECT_EQ(m_view->model()->rowCount(),m_fileViewHelper->selectedUrls().size());
}

TEST_F(FileViewHelperTest,set_foucs)
{
    EXPECT_EQ(false,m_view->hasFocus());
    m_fileViewHelper->setFoucsOnFileView(-1);
    EXPECT_EQ(false,m_view->hasFocus());
    m_fileViewHelper->setFoucsOnFileView(m_view->windowId());
    EXPECT_EQ(false,m_view->hasFocus());
}

TEST_F(FileViewHelperTest,preHandleCd_error_winid)
{
    DUrl to("file:///usr/bin");
    DFMUrlBaseEvent event(nullptr, to);
    auto org = m_fileViewHelper->currentUrl();
    event.setWindowId(-1);
    QVariant var;
    var.setValue(to);
    m_fileViewHelper->preHandleCd(event);
    EXPECT_EQ(org, m_fileViewHelper->currentUrl());
}

TEST_F(FileViewHelperTest,preHandleCd)
{
    DUrl to("file:///usr/bin");
    DFMUrlBaseEvent event(nullptr, to);
    event.setWindowId(m_fileViewHelper->windowId());
    QVariant var;
    var.setValue(to);

    bool net = false;
    m_fileViewHelper->connect(fileSignalManager,&FileSignalManager::requestFetchNetworks,m_fileViewHelper,[&net](){
        net = true;
    });
    qApp->processEvents();
    m_fileViewHelper->preHandleCd(event);
    qApp->processEvents();
    EXPECT_FALSE(net);
}

TEST_F(FileViewHelperTest,preHandleCd_smb)
{
    DUrl to("smb:///127.0.0.1");
    DFMUrlBaseEvent event(nullptr, to);
    event.setWindowId(m_fileViewHelper->windowId());
    QVariant var;
    var.setValue(to);

    bool net = false;
    stub_ext::StubExt stu;
    stu.set_lamda(&FileSignalManager::requestFetchNetworks,[&net](){
        net = true;
    });
    qApp->processEvents();
    m_fileViewHelper->preHandleCd(event);
    qApp->processEvents();
    EXPECT_TRUE(net);
}

TEST_F(FileViewHelperTest,preHandleCd_net)
{
    DUrl to("network:///127.0.0.1");
    DFMUrlBaseEvent event(nullptr, to);
    event.setWindowId(m_fileViewHelper->windowId());
    QVariant var;
    var.setValue(to);

    bool net = false;
    stub_ext::StubExt stu;
    stu.set_lamda(&FileSignalManager::requestFetchNetworks,[&net](){
        net = true;
    });
    qApp->processEvents();
    m_fileViewHelper->preHandleCd(event);
    qApp->processEvents();
    EXPECT_TRUE(net);
}

TEST_F(FileViewHelperTest,cd_error_winid)
{
    DUrl to("file:///usr/bin");
    DFMUrlBaseEvent event(nullptr, to);
    event.setWindowId(-1);
    QVariant var;
    var.setValue(to);
    EXPECT_NO_FATAL_FAILURE(m_fileViewHelper->cd(event));
    EXPECT_NO_FATAL_FAILURE(m_fileViewHelper->cdUp(event));
}

TEST_F(FileViewHelperTest,cd_winid)
{
    DUrl to("file:///usr/bin");
    DFMUrlBaseEvent event(nullptr, to);
    event.setWindowId(m_fileViewHelper->windowId());
    QVariant var;
    var.setValue(to);
    EXPECT_NO_FATAL_FAILURE(m_fileViewHelper->cd(event));
    EXPECT_NO_FATAL_FAILURE(m_fileViewHelper->cdUp(event));
}
