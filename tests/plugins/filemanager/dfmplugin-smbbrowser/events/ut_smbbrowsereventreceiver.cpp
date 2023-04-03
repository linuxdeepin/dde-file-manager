// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "events/smbbrowsereventreceiver.h"

#include <dfm-base/utils/systempathutil.h>

#include <QUrl>

#include <gtest/gtest.h>

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
    EXPECT_FALSE(SmbBrowserEventReceiver::instance()->cancelDelete(0, QList<QUrl> { QUrl::fromLocalFile("/") }));
    EXPECT_TRUE(SmbBrowserEventReceiver::instance()->cancelDelete(0, QList<QUrl> { QUrl("smb://1.2.3.4/hello") }));
    EXPECT_TRUE(SmbBrowserEventReceiver::instance()->cancelDelete(0, QList<QUrl> { QUrl("ftp://1.2.3.4/hello") }));
    EXPECT_TRUE(SmbBrowserEventReceiver::instance()->cancelDelete(0, QList<QUrl> { QUrl("sftp://1.2.3.4/hello") }));
}
