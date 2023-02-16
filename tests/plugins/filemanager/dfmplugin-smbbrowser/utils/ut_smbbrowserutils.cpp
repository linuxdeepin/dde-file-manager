// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/filemanager/dfmplugin-smbbrowser/utils/smbbrowserutils.h"
#include "dfm-base/base/device/devicemanager.h"
#include "dfm-base/utils/dialogmanager.h"

#include <dfm-framework/dpf.h>

#include <QUrl>

#include <gtest/gtest.h>

using namespace dfmplugin_smbbrowser;

class UT_SmbBrowser : public testing::Test
{
protected:
    virtual void SetUp() override { }
    virtual void TearDown() override { stub.clear(); }

private:
    stub_ext::StubExt stub;
};

TEST_F(UT_SmbBrowser, NetworkScheme)
{
    EXPECT_TRUE(smb_browser_utils::networkScheme() == "network");
}

TEST_F(UT_SmbBrowser, NetNeighborRootUrl)
{
    QUrl u;
    u.setScheme("network");
    u.setPath("/");
    EXPECT_TRUE(smb_browser_utils::netNeighborRootUrl() == u);
    EXPECT_TRUE(smb_browser_utils::netNeighborRootUrl().path() == "/");
    EXPECT_TRUE(smb_browser_utils::netNeighborRootUrl().scheme() == "network");
}

TEST_F(UT_SmbBrowser, Icon)
{
    //    EXPECT_TRUE(ins->icon().themeName() == "network-server-symbolic");
    EXPECT_NO_FATAL_FAILURE(smb_browser_utils::icon().themeName());
}
