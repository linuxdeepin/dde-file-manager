// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// PMS sev-2 regression tests for dfmplugin-smbbrowser (computer/sidebar url matching).

#include <gtest/gtest.h>

#include "displaycontrol/utilities/protocoldisplayutilities.h"

#include <QUrl>

DPSMBBROWSER_USE_NAMESPACE

class SmbBrowserPmsRegressionTest : public testing::Test
{
public:
    void SetUp() override { }
    void TearDown() override { }
};

// PMS:264711 sidebarUrlEquals only compared smb-scheme items so a computer view
// "vsmb" item never matched its sidebar "smb" counterpart; the fix normalizes
// vsmb->smb (host + trailing-slash tolerant path) in that single direction.
TEST_F(SmbBrowserPmsRegressionTest, BUG264711_SidebarUrlEquals_VsmbMatchesSmb)
{
    const QUrl vsmb = QUrl("vsmb://10.0.0.8/share");
    const QUrl smb = QUrl("smb://10.0.0.8/share");

    // The exact regression case: computer vsmb item vs sidebar smb item.
    EXPECT_TRUE(computer_sidebar_event_calls::sidebarUrlEquals(vsmb, smb));

    // Trailing slash differences must not break the comparison.
    EXPECT_TRUE(computer_sidebar_event_calls::sidebarUrlEquals(QUrl("vsmb://10.0.0.8/share/"),
                                                               QUrl("smb://10.0.0.8/share")));
    EXPECT_TRUE(computer_sidebar_event_calls::sidebarUrlEquals(QUrl("vsmb://10.0.0.8/share"),
                                                               QUrl("smb://10.0.0.8/share/")));

    // Different host or different path still must not match.
    EXPECT_FALSE(computer_sidebar_event_calls::sidebarUrlEquals(vsmb, QUrl("smb://10.0.0.9/share")));
    EXPECT_FALSE(computer_sidebar_event_calls::sidebarUrlEquals(vsmb, QUrl("smb://10.0.0.8/other")));

    // Only the vsmb->smb direction is normalized.
    EXPECT_FALSE(computer_sidebar_event_calls::sidebarUrlEquals(smb, vsmb));
    EXPECT_FALSE(computer_sidebar_event_calls::sidebarUrlEquals(smb, QUrl("smb://10.0.0.8/share")));
    EXPECT_FALSE(computer_sidebar_event_calls::sidebarUrlEquals(QUrl("file:///share"), smb));
}
