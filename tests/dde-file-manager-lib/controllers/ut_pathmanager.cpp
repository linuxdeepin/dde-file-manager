/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
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

#include "controllers/pathmanager.cpp"
#include "dfmstandardpaths.h"
#include "dabstractfileinfo.h"
#include "stub.h"
#include "addr_pri.h"

namespace  {
class TestPathManager : public testing::Test
{
public:
    virtual void SetUp() override
    {
        m_pManager = new PathManager();
    }

    virtual void TearDown() override
    {
        if (m_pManager) {
            delete m_pManager;
            m_pManager = nullptr;
        }
    }

public:
    PathManager *m_pManager = nullptr;
};
}

typedef QMap<QString, QString> StringStringMap;
ACCESS_PRIVATE_FIELD(PathManager, StringStringMap, m_systemPathDisplayNamesMap);
ACCESS_PRIVATE_FIELD(PathManager, StringStringMap, m_systemPathsMap);
ACCESS_PRIVATE_FIELD(PathManager, StringStringMap, m_systemPathIconNamesMap);
ACCESS_PRIVATE_FIELD(PathManager, QSet<QString>, m_systemPathsSet);

TEST_F(TestPathManager, test_init)
{
    ASSERT_NE(m_pManager, nullptr);

    m_pManager->initPaths();
    auto &systemPathDisplayNamesMap = access_private_field::PathManagerm_systemPathDisplayNamesMap(*m_pManager);
    auto &systemPathsMap = access_private_field::PathManagerm_systemPathsMap(*m_pManager);
    auto &systemPathIconNamesMap = access_private_field::PathManagerm_systemPathIconNamesMap(*m_pManager);
    auto &systemPathsSet = access_private_field::PathManagerm_systemPathsSet(*m_pManager);
    EXPECT_NE(systemPathDisplayNamesMap.count(), 0);
    EXPECT_NE(systemPathIconNamesMap.count(), 0);
    EXPECT_NE(systemPathsMap.count(), 0);
    EXPECT_NE(systemPathsSet.count(), 0);

    EXPECT_EQ(m_pManager->getSystemPathDisplayName("Home"), QObject::tr("Home"));
    EXPECT_EQ(m_pManager->getSystemPathDisplayName("IfNotExist"), QString());

    EXPECT_EQ(m_pManager->getSystemPathDisplayNameByPath(DFMStandardPaths::location(DFMStandardPaths::HomePath)), QObject::tr("Home"));
    EXPECT_EQ(m_pManager->getSystemPathDisplayNameByPath("/IfNotExist"), QString());
}

TEST_F(TestPathManager, test_loadSystemPaths)
{
    ASSERT_NE(m_pManager, nullptr);

    m_pManager->loadSystemPaths();

    auto &systemPathsMap = access_private_field::PathManagerm_systemPathsMap(*m_pManager);
    EXPECT_NE(systemPathsMap.count(), 0);
}

TEST_F(TestPathManager, test_getSystemPath)
{
    ASSERT_NE(m_pManager, nullptr);

    EXPECT_EQ(m_pManager->getSystemPath("Desktop"), DFMStandardPaths::location(DFMStandardPaths::DesktopPath));
    EXPECT_EQ(m_pManager->getSystemPath("Videos"), DFMStandardPaths::location(DFMStandardPaths::VideosPath));
}

TEST_F(TestPathManager, test_getSystemPathsMap)
{
    ASSERT_NE(m_pManager, nullptr);

    m_pManager->initPaths();
    EXPECT_NE(m_pManager->systemPathsMap().count(), 0);
}

TEST_F(TestPathManager, test_getSystemPathDisplayNamesMap)
{
    ASSERT_NE(m_pManager, nullptr);

    m_pManager->initPaths();
    EXPECT_NE(m_pManager->systemPathDisplayNamesMap().count(), 0);
}

TEST_F(TestPathManager, test_getSystemPathIconName)
{
    ASSERT_NE(m_pManager, nullptr);

    m_pManager->initPaths();
    EXPECT_EQ(m_pManager->getSystemPathIconName("Home"), "user-home");
    EXPECT_EQ(m_pManager->getSystemPathIconName("IfNotExist"), QString());
}

TEST_F(TestPathManager, test_getSystemPathIconNameByPath)
{
    ASSERT_NE(m_pManager, nullptr);

    m_pManager->initPaths();
    EXPECT_EQ(m_pManager->getSystemPathIconNameByPath(DFMStandardPaths::location(DFMStandardPaths::HomePath)), "user-home");
    EXPECT_EQ(m_pManager->getSystemPathIconNameByPath("/IfNotExist"), QString());
}

TEST_F(TestPathManager, test_getMountRangePathGroup)
{
    ASSERT_NE(m_pManager, nullptr);

    EXPECT_EQ(m_pManager->getMountRangePathGroup().count(), 4);
}

TEST_F(TestPathManager, test_getSystemDiskAndDataDiskPathGroup)
{
    ASSERT_NE(m_pManager, nullptr);

    EXPECT_EQ(m_pManager->getSystemDiskAndDataDiskPathGroup().count(), 4);
}

TEST_F(TestPathManager, test_isSystemPath)
{
    ASSERT_NE(m_pManager, nullptr);

    m_pManager->initPaths();
    EXPECT_TRUE(m_pManager->isSystemPath(DFMStandardPaths::location(DFMStandardPaths::HomePath)));
    EXPECT_FALSE(m_pManager->isSystemPath("/IfNotExist"));
}

TEST_F(TestPathManager, test_cleanPath)
{
    ASSERT_NE(m_pManager, nullptr);

    QString testStr("/data/test_pathManager/");
    cleanPath(testStr);

    EXPECT_EQ(testStr, "/test_pathManager");
}

TEST_F(TestPathManager, test_isOptical)
{
    ASSERT_NE(m_pManager, nullptr);

    DAbstractFileInfoPointer info(new DAbstractFileInfo(DUrl("/")));
    EXPECT_FALSE(m_pManager->isOptical(info));
}

TEST_F(TestPathManager, test_isVisiblePartitionPath)
{
    ASSERT_NE(m_pManager, nullptr);

    DAbstractFileInfoPointer info(new DAbstractFileInfo(DUrl("/")));
    EXPECT_TRUE(m_pManager->isVisiblePartitionPath(info));
}
