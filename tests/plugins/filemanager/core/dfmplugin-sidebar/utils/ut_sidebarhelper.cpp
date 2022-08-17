/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#include "stubext.h"
#include "plugins/filemanager/core/dfmplugin-sidebar/utils/sidebarhelper.h"

#include "dfm-base/base/configs/settingbackend.h"
#include "dfm-base/base/configs/configsynchronizer.h"
#include "dfm-base/base/configs/dconfig/dconfigmanager.h"

#include <gtest/gtest.h>

class UT_SideBarHelper : public testing::Test
{
protected:
    virtual void SetUp() override
    {
    }
    virtual void TearDown() override
    {
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
};

DFMBASE_USE_NAMESPACE
DPSIDEBAR_USE_NAMESPACE

TEST_F(UT_SideBarHelper, AllSideBar) {}
TEST_F(UT_SideBarHelper, FindSideBarByWindowId) {}
TEST_F(UT_SideBarHelper, AddSideBar) {}
TEST_F(UT_SideBarHelper, RemoveSideBar) {}
TEST_F(UT_SideBarHelper, WindowId) {}
TEST_F(UT_SideBarHelper, CreateDefaultItem) {}
TEST_F(UT_SideBarHelper, CreateItemByInfo) {}
TEST_F(UT_SideBarHelper, CreateSeparatorItem) {}
TEST_F(UT_SideBarHelper, MakeItemIdentifier) {}
TEST_F(UT_SideBarHelper, DefaultCdAction) {}
TEST_F(UT_SideBarHelper, DefaultContextMenu) {}
TEST_F(UT_SideBarHelper, RegisterSortFunc) {}
TEST_F(UT_SideBarHelper, SortFunc) {}
TEST_F(UT_SideBarHelper, UpdateSidebarSelection) {}

TEST_F(UT_SideBarHelper, BindSettings)
{
    using namespace dfmbase;
    typedef void (SettingBackend::*Add)(const QString &, SettingBackend::GetOptFunc, SettingBackend::SaveOptFunc);
    auto add = static_cast<Add>(&SettingBackend::addSettingAccessor);
    stub.set_lamda(add, [] { __DBG_STUB_INVOKE__ });

    EXPECT_NO_FATAL_FAILURE(SideBarHelper::bindSettings());
}

TEST_F(UT_SideBarHelper, HiddenRules)
{
    using namespace dfmbase;
    stub.set_lamda(&DConfigManager::value, [] { __DBG_STUB_INVOKE__ return QVariantMap(); });

    EXPECT_NO_FATAL_FAILURE(SideBarHelper::hiddenRules());
}

TEST_F(UT_SideBarHelper, BindRecentConf)
{
    stub.set_lamda(&ConfigSynchronizer::watchChange, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_NO_FATAL_FAILURE(SideBarHelper::bindRecentConf());
}

TEST_F(UT_SideBarHelper, SaveRecentToDConf)
{
    stub.set_lamda(&DConfigManager::setValue, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(SideBarHelper::saveRecentToConf(true));
    EXPECT_NO_FATAL_FAILURE(SideBarHelper::saveRecentToConf(false));
}

TEST_F(UT_SideBarHelper, SyncRecentToAppSet)
{
    stub.set_lamda(&Application::setGenericAttribute, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(SideBarHelper::syncRecentToAppSet("hello", "world", "true"));
    EXPECT_NO_FATAL_FAILURE(SideBarHelper::syncRecentToAppSet("hello", "world", "false"));
}

TEST_F(UT_SideBarHelper, IsRecentConfEqual)
{
    QVariantMap dcon {
        { "recent", true },
        { "computer", false }
    };
    EXPECT_TRUE(SideBarHelper::isRecentConfEqual(dcon, true));
    dcon["recent"] = false;
    EXPECT_FALSE(SideBarHelper::isRecentConfEqual(dcon, true));
}
