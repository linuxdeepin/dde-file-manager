// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include "displaycontrol/datahelper/virtualentrydata.h"

#include <QString>
#include <QUrl>
#include <QVariantMap>
#include <QDateTime>

DPSMBBROWSER_USE_NAMESPACE

class UT_VirtualEntryData : public testing::Test
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

TEST_F(UT_VirtualEntryData, AllInOne)
{
    VirtualEntryData d1, d2;
    EXPECT_NO_FATAL_FAILURE(d1 = d2);

    EXPECT_NO_FATAL_FAILURE(d1.setDisplayName("hello"));
    EXPECT_NO_FATAL_FAILURE(d1.setKey("key"));
    EXPECT_NO_FATAL_FAILURE(d1.setHost("1.2.3.4"));
    EXPECT_NO_FATAL_FAILURE(d1.setProtocol("smb"));
    EXPECT_NO_FATAL_FAILURE(d1.setPort(139));
    EXPECT_NO_FATAL_FAILURE(d1.setTargetPath("path"));
    EXPECT_EQ(d1.getDisplayName(), "hello");
    EXPECT_EQ(d1.getHost(), "1.2.3.4");
    EXPECT_EQ(d1.getProtocol(), "smb");
    EXPECT_EQ(d1.getPort(), 139);
    EXPECT_EQ(d1.getKey(), "key");
    EXPECT_EQ(d1.getTargetPath(), "path");

    VirtualEntryData d3("smb://1.2.3.4");
    EXPECT_EQ(d3.getDisplayName(), "1.2.3.4");
    EXPECT_EQ(d3.getHost(), "1.2.3.4");
    EXPECT_EQ(d3.getProtocol(), "smb");
    EXPECT_EQ(d3.getPort(), -1);

    VirtualEntryData d4(d3);
    EXPECT_EQ(d4.getDisplayName(), "1.2.3.4");
    EXPECT_EQ(d4.getHost(), "1.2.3.4");
    EXPECT_EQ(d4.getProtocol(), "smb");
    EXPECT_EQ(d4.getPort(), -1);
}
