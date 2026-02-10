// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include "events/smbbrowsereventreceiver.h"

#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/dfm_global_defines.h>

#include <QUrl>
#include <QString>
#include <QList>
#include <QRegularExpression>
#include <QObject>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_smbbrowser;

class UT_SmbBrowserEventReceiver : public testing::Test
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

TEST_F(UT_SmbBrowserEventReceiver, Instance)
{
    EXPECT_NO_FATAL_FAILURE(SmbBrowserEventReceiver::instance());
}

TEST_F(UT_SmbBrowserEventReceiver, DetailViewIcon)
{
    QString icon;
    EXPECT_FALSE(SmbBrowserEventReceiver::instance()->detailViewIcon(QUrl::fromLocalFile("/"), &icon));
    EXPECT_FALSE(SmbBrowserEventReceiver::instance()->detailViewIcon({}, nullptr));

    QUrl u("network:///");
    QString retIcon;
    DFMBASE_USE_NAMESPACE
    stub.set_lamda(&SystemPathUtil::systemPathIconName, [&] { __DBG_STUB_INVOKE__ return retIcon; });
    EXPECT_FALSE(SmbBrowserEventReceiver::instance()->detailViewIcon(u, &icon));
    retIcon = "test";
    EXPECT_TRUE(SmbBrowserEventReceiver::instance()->detailViewIcon(u, &icon));
    EXPECT_EQ(icon, retIcon);
}

TEST_F(UT_SmbBrowserEventReceiver, CancelDelete)
{
    EXPECT_FALSE(SmbBrowserEventReceiver::instance()->cancelDelete(0, QList<QUrl> { QUrl::fromLocalFile("/") }, QUrl("usershare:///hello")));
    EXPECT_TRUE(SmbBrowserEventReceiver::instance()->cancelDelete(0, QList<QUrl> { QUrl("smb://1.2.3.4/hello") }, QUrl("usershare:///hello")));
    EXPECT_TRUE(SmbBrowserEventReceiver::instance()->cancelDelete(0, QList<QUrl> { QUrl("ftp://1.2.3.4/hello") }, QUrl("usershare:///hello")));
    EXPECT_TRUE(SmbBrowserEventReceiver::instance()->cancelDelete(0, QList<QUrl> { QUrl("sftp://1.2.3.4/hello") }, QUrl("usershare:///hello")));
}
