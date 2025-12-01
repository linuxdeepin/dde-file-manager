// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/reportlog/datas/appstartupreportdata.h"
#include "plugins/common/dfmplugin-utils/reportlog/datas/blockmountreportdata.h"
#include "plugins/common/dfmplugin-utils/reportlog/datas/desktopstartupreportdata.h"
#include "plugins/common/dfmplugin-utils/reportlog/datas/enterdirreportdata.h"
#include "plugins/common/dfmplugin-utils/reportlog/datas/filemenureportdata.h"
#include "plugins/common/dfmplugin-utils/reportlog/datas/searchreportdata.h"
#include "plugins/common/dfmplugin-utils/reportlog/datas/sidebarreportdata.h"
#include "plugins/common/dfmplugin-utils/reportlog/datas/smbreportdata.h"
#include "plugins/common/dfmplugin-utils/reportlog/datas/vaultreportdata.h"

#include <QDateTime>
#include <QJsonObject>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;

// ========== AppStartupReportData 测试 ==========

class UT_AppStartupReportData : public testing::Test
{
protected:
    AppStartupReportData data;
};

TEST_F(UT_AppStartupReportData, type_ReturnsCorrectType)
{
    EXPECT_EQ(data.type(), "AppStartup");
}

TEST_F(UT_AppStartupReportData, prepareData_ContainsTidAndSysTime)
{
    QVariantMap args;
    args.insert("testKey", "testValue");

    QJsonObject result = data.prepareData(args);

    EXPECT_EQ(result.value("tid").toInt(), 1000500006);
    EXPECT_TRUE(result.contains("sysTime"));
    EXPECT_EQ(result.value("testKey").toString(), "testValue");
}

// ========== BlockMountReportData 测试 ==========

class UT_BlockMountReportData : public testing::Test
{
protected:
    BlockMountReportData data;
};

TEST_F(UT_BlockMountReportData, type_ReturnsCorrectType)
{
    EXPECT_EQ(data.type(), "BlockMount");
}

TEST_F(UT_BlockMountReportData, prepareData_ContainsTidAndOpTime)
{
    QVariantMap args;
    args.insert("device", "/dev/sda1");

    QJsonObject result = data.prepareData(args);

    EXPECT_EQ(result.value("tid").toInt(), 1000500004);
    EXPECT_TRUE(result.contains("opTime"));
    EXPECT_EQ(result.value("device").toString(), "/dev/sda1");
}

// ========== DesktopStartUpReportData 测试 ==========

class UT_DesktopStartUpReportData : public testing::Test
{
protected:
    DesktopStartUpReportData data;
};

TEST_F(UT_DesktopStartUpReportData, type_ReturnsCorrectType)
{
    EXPECT_EQ(data.type(), "DesktopStartup");
}

TEST_F(UT_DesktopStartUpReportData, prepareData_ContainsTidAndSysTime)
{
    QVariantMap args;

    QJsonObject result = data.prepareData(args);

    EXPECT_EQ(result.value("tid").toInt(), 1000500008);
    EXPECT_TRUE(result.contains("sysTime"));
}

// ========== EnterDirReportData 测试 ==========

class UT_EnterDirReportData : public testing::Test
{
protected:
    EnterDirReportData data;
};

TEST_F(UT_EnterDirReportData, type_ReturnsCorrectType)
{
    EXPECT_EQ(data.type(), "EnterDirectory");
}

TEST_F(UT_EnterDirReportData, prepareData_ContainsTidAndSysTime)
{
    QVariantMap args;
    args.insert("path", "/home/user");

    QJsonObject result = data.prepareData(args);

    EXPECT_EQ(result.value("tid").toInt(), 1000500007);
    EXPECT_TRUE(result.contains("sysTime"));
    EXPECT_EQ(result.value("path").toString(), "/home/user");
}

// ========== FileMenuReportData 测试 ==========

class UT_FileMenuReportData : public testing::Test
{
protected:
    FileMenuReportData data;
};

TEST_F(UT_FileMenuReportData, type_ReturnsCorrectType)
{
    EXPECT_EQ(data.type(), "FileMenu");
}

TEST_F(UT_FileMenuReportData, prepareData_ContainsTidAndSysTime)
{
    QVariantMap args;
    args.insert("action", "copy");

    QJsonObject result = data.prepareData(args);

    EXPECT_EQ(result.value("tid").toInt(), 1000500005);
    EXPECT_TRUE(result.contains("sysTime"));
    EXPECT_EQ(result.value("action").toString(), "copy");
}

// ========== SearchReportData 测试 ==========

class UT_SearchReportData : public testing::Test
{
protected:
    SearchReportData data;
};

TEST_F(UT_SearchReportData, type_ReturnsCorrectType)
{
    EXPECT_EQ(data.type(), "Search");
}

TEST_F(UT_SearchReportData, prepareData_ContainsTidAndSysTime)
{
    QVariantMap args;
    args.insert("keyword", "test");

    QJsonObject result = data.prepareData(args);

    EXPECT_EQ(result.value("tid").toInt(), 1000500002);
    EXPECT_TRUE(result.contains("sysTime"));
    EXPECT_EQ(result.value("keyword").toString(), "test");
}

// ========== SidebarReportData 测试 ==========

class UT_SidebarReportData : public testing::Test
{
protected:
    SidebarReportData data;
};

TEST_F(UT_SidebarReportData, type_ReturnsCorrectType)
{
    EXPECT_EQ(data.type(), "Sidebar");
}

TEST_F(UT_SidebarReportData, prepareData_ContainsTidAndSysTime)
{
    QVariantMap args;
    args.insert("item", "bookmark");

    QJsonObject result = data.prepareData(args);

    EXPECT_EQ(result.value("tid").toInt(), 1000500003);
    EXPECT_TRUE(result.contains("sysTime"));
    EXPECT_EQ(result.value("item").toString(), "bookmark");
}

// ========== SmbReportData 测试 ==========

class UT_SmbReportData : public testing::Test
{
protected:
    SmbReportData data;
};

TEST_F(UT_SmbReportData, type_ReturnsCorrectType)
{
    EXPECT_EQ(data.type(), "Smb");
}

TEST_F(UT_SmbReportData, prepareData_SuccessResult_ClearsErrorFields)
{
    QVariantMap args;
    args.insert("result", true);

    QJsonObject result = data.prepareData(args);

    EXPECT_EQ(result.value("tid").toInt(), 1000500001);
    EXPECT_TRUE(result.contains("resultTime"));
    EXPECT_EQ(result.value("errorId").toInt(), 0);
    EXPECT_EQ(result.value("errorSysMsg").toString(), "");
    EXPECT_EQ(result.value("errorUiMsg").toString(), "");
}

TEST_F(UT_SmbReportData, prepareData_FailedResult_KeepsErrorFields)
{
    QVariantMap args;
    args.insert("result", false);
    args.insert("errorId", 100);
    args.insert("errorSysMsg", "connection failed");

    QJsonObject result = data.prepareData(args);

    EXPECT_EQ(result.value("tid").toInt(), 1000500001);
    EXPECT_TRUE(result.contains("resultTime"));
    EXPECT_EQ(result.value("errorId").toInt(), 100);
    EXPECT_EQ(result.value("errorSysMsg").toString(), "connection failed");
}

// ========== VaultReportData 测试 ==========

class UT_VaultReportData : public testing::Test
{
protected:
    VaultReportData data;
};

TEST_F(UT_VaultReportData, type_ReturnsCorrectType)
{
    EXPECT_EQ(data.type(), "Vault");
}

TEST_F(UT_VaultReportData, prepareData_ContainsTidAndSysTime)
{
    QVariantMap args;
    args.insert("operation", "unlock");

    QJsonObject result = data.prepareData(args);

    EXPECT_EQ(result.value("tid").toInt(), 1000500000);
    EXPECT_TRUE(result.contains("sysTime"));
    EXPECT_EQ(result.value("operation").toString(), "unlock");
}
