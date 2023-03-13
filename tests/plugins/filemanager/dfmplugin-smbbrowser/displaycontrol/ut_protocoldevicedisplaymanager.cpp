// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/filemanager/dfmplugin-smbbrowser/displaycontrol/protocoldevicedisplaymanager.h"
#include "plugins/filemanager/dfmplugin-smbbrowser/displaycontrol/protocoldevicedisplaymanager_p.h"

#include <gtest/gtest.h>

using namespace dfmplugin_smbbrowser;

class UT_ProtocolDeviceDisplayManager : public testing::Test
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

TEST_F(UT_ProtocolDeviceDisplayManager, Instance)
{
}

TEST_F(UT_ProtocolDeviceDisplayManager, DisplayMode) { }
TEST_F(UT_ProtocolDeviceDisplayManager, IsShowOfflineItem) { }
TEST_F(UT_ProtocolDeviceDisplayManager, HookItemInsert) { }
TEST_F(UT_ProtocolDeviceDisplayManager, HookItemsFilter) { }
TEST_F(UT_ProtocolDeviceDisplayManager, OnDevMounted) { }
TEST_F(UT_ProtocolDeviceDisplayManager, OnDevUnmounted) { }
TEST_F(UT_ProtocolDeviceDisplayManager, OnDConfigChanged) { }
TEST_F(UT_ProtocolDeviceDisplayManager, OnJsonConfigChanged) { }
TEST_F(UT_ProtocolDeviceDisplayManager, OnMenuSceneAdded) { }

class UT_ProtocolDeviceDisplayManagerPrivate : public testing::Test
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

TEST_F(UT_ProtocolDeviceDisplayManagerPrivate, Init) { }
TEST_F(UT_ProtocolDeviceDisplayManagerPrivate, OnDisplayModeChanged) { }
TEST_F(UT_ProtocolDeviceDisplayManagerPrivate, OnShowOfflineChanged) { }
TEST_F(UT_ProtocolDeviceDisplayManagerPrivate, IsSupportVEntry) { }
TEST_F(UT_ProtocolDeviceDisplayManagerPrivate, RemoveAllSmb) { }
