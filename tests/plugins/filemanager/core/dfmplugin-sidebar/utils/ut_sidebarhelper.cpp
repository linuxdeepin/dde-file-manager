// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/filemanager/core/dfmplugin-sidebar/utils/sidebarhelper.h"
#include "plugins/filemanager/core/dfmplugin-sidebar/treeviews/sidebaritem.h"

#include <dfm-base/base/configs/settingbackend.h>
#include <dfm-base/base/configs/configsynchronizer.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

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

TEST_F(UT_SideBarHelper, AllSideBar) { }
TEST_F(UT_SideBarHelper, FindSideBarByWindowId) { }
TEST_F(UT_SideBarHelper, AddSideBar) { }
TEST_F(UT_SideBarHelper, RemoveSideBar) { }
TEST_F(UT_SideBarHelper, WindowId) { }
TEST_F(UT_SideBarHelper, CreateItemByInfo) { }
TEST_F(UT_SideBarHelper, CreateSeparatorItem) { }
TEST_F(UT_SideBarHelper, MakeItemIdentifier) { }
TEST_F(UT_SideBarHelper, DefaultCdAction) { }
TEST_F(UT_SideBarHelper, DefaultContextMenu) { }
TEST_F(UT_SideBarHelper, RegisterSortFunc) { }
TEST_F(UT_SideBarHelper, SortFunc) { }
TEST_F(UT_SideBarHelper, UpdateSidebarSelection) { }

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
