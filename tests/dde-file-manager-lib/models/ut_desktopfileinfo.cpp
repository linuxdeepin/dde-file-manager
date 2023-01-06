// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "models/desktopfileinfo.h"

#include <gtest/gtest.h>
#include <QIcon>

namespace {
class TestDesktopFileInfo : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestDesktopFileInfo";

        info = new DesktopFileInfo(DUrl("file:///test"));
    }

    void TearDown() override
    {
        std::cout << "end TestDesktopFileInfo";
        delete info;
    }

public:
    DesktopFileInfo *info;
};
} // namespace

TEST_F(TestDesktopFileInfo, getName)
{
    EXPECT_STREQ("", info->getName().toStdString().c_str());
}

TEST_F(TestDesktopFileInfo, getExec)
{
    EXPECT_STREQ("", info->getExec().toStdString().c_str());
}

TEST_F(TestDesktopFileInfo, getIconName)
{
    EXPECT_STREQ("", info->getIconName().toStdString().c_str());
}

TEST_F(TestDesktopFileInfo, getType)
{
    EXPECT_STREQ("Application", info->getType().toStdString().c_str());
}

TEST_F(TestDesktopFileInfo, fileDisplayName)
{
    EXPECT_STREQ("test", info->fileDisplayName().toStdString().c_str());
}

TEST_F(TestDesktopFileInfo, fileNameOfRename)
{
    EXPECT_STREQ("test", info->fileNameOfRename().toStdString().c_str());
}

TEST_F(TestDesktopFileInfo, baseNameOfRename)
{
    EXPECT_STREQ("test", info->baseNameOfRename().toStdString().c_str());
}

TEST_F(TestDesktopFileInfo, suffixOfRename)
{
    EXPECT_STREQ("", info->suffixOfRename().toStdString().c_str());
}

TEST_F(TestDesktopFileInfo, iconName)
{
    info->refresh();
    EXPECT_STREQ("", info->iconName().toStdString().c_str());
}

TEST_F(TestDesktopFileInfo, genericIconName)
{
    EXPECT_STREQ("application-default-icon", info->genericIconName().toStdString().c_str());
}

TEST_F(TestDesktopFileInfo, fileInfoCanNotDrop)
{
    EXPECT_FALSE(info->canDrop());
}

TEST_F(TestDesktopFileInfo, fileInfoCanNotTag)
{
    EXPECT_FALSE(info->canTag());
}

TEST_F(TestDesktopFileInfo, fileInfoCanMoveOrCopy)
{
    EXPECT_TRUE(info->canMoveOrCopy());
}

TEST_F(TestDesktopFileInfo, trashUrlScheme)
{
    EXPECT_STREQ("file", info->trashDesktopFileUrl().scheme().toStdString().c_str());
}

TEST_F(TestDesktopFileInfo, computerUrlScheme)
{
    EXPECT_STREQ("file", info->computerDesktopFileUrl().scheme().toStdString().c_str());
}

TEST_F(TestDesktopFileInfo, homeUrlScheme)
{
    EXPECT_STREQ("file", info->homeDesktopFileUrl().scheme().toStdString().c_str());
}

TEST_F(TestDesktopFileInfo, tstConstructWithQFileInfo)
{
    DesktopFileInfo *p = new DesktopFileInfo(QFileInfo("/usr/bin/dde-file-manager"));
    EXPECT_FALSE(p->getName() == "dde-file-manager");
    delete p;
}

#ifndef __arm__
TEST_F(TestDesktopFileInfo, tstFileIcon)
{
    info->refresh(true);
    auto p = info->fileIcon();
    EXPECT_NO_FATAL_FAILURE(p.isNull());
}
TEST_F(TestDesktopFileInfo, tstMenuActionList)
{
    info->menuActionList();
    DesktopFileInfo f(DUrl("trash:///"));
    f.menuActionList();
    f.disableMenuActionList();
}
#endif

TEST_F(TestDesktopFileInfo, tstAdditionalIcon)
{
    EXPECT_TRUE(0 == info->additionalIcon().count());
}

TEST_F(TestDesktopFileInfo, tstSupportDragActions)
{
    DesktopFileInfo f(DUrl("trash:///"));
    EXPECT_FALSE(Qt::IgnoreAction == f.supportedDragActions());
    info->supportedDragActions();
}

TEST_F(TestDesktopFileInfo, tstCanDrop)
{
    DesktopFileInfo f(DUrl("computer:///"));
    EXPECT_FALSE(f.canDrop());
    info->canDrop();
}
