// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QUrl>
#include <QFileDevice>
#include "stubext.h"

#include "events/propertyeventcall.h"
#include "dfmplugin_propertydialog_global.h"

DPPROPERTYDIALOG_USE_NAMESPACE

class TestPropertyEventCall : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

// Test PropertyEventCall class
TEST_F(TestPropertyEventCall, PropertyEventCallSendSetPermissionManager)
{
    quint64 winID = 12345;
    QUrl url = QUrl::fromLocalFile("/tmp/test.txt");
    QFileDevice::Permissions permissions = QFileDevice::ReadOwner | QFileDevice::WriteOwner;
    
    EXPECT_NO_THROW(PropertyEventCall::sendSetPermissionManager(winID, url, permissions));
}

TEST_F(TestPropertyEventCall, PropertyEventCallSendFileHide)
{
    quint64 winID = 12345;
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/tmp/test1.txt") << QUrl::fromLocalFile("/tmp/test2.txt");
    
    EXPECT_NO_THROW(PropertyEventCall::sendFileHide(winID, urls));
}

// Test Q_DECLARE_METATYPE for QFileDevice::Permissions
TEST_F(TestPropertyEventCall, MetaTypeDeclarationForQFileDevicePermissions)
{
    int permissionTypeId = qMetaTypeId<QFileDevice::Permissions>();
    EXPECT_GT(permissionTypeId, 0);
}