// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "plugins/filemanager/core/dfmplugin-computer/utils/remotepasswdmanager.h"

#include <QFile>
#include <QTextStream>

#include <gtest/gtest.h>
#include <stdio.h>

DPCOMPUTER_USE_NAMESPACE

class UT_RemotePasswdManager : public testing::Test
{
protected:
protected:
    virtual void SetUp() override
    {
        stub.set_lamda(&RemotePasswdManager::configPath, [] { __DBG_STUB_INVOKE__ return "/tmp/tst_smb.json"; });
    }
    virtual void TearDown() override { stub.clear(); }

private:
    stub_ext::StubExt stub;
    RemotePasswdManager *ins { RemotePasswdManager::instance() };
};

static void stubSecretClear(const SecretSchema *, GCancellable *, GAsyncReadyCallback, gpointer, ...) {
    __DBG_STUB_INVOKE__
}

TEST_F(UT_RemotePasswdManager, ClearPasswd)
{
    stub.set_lamda(&RemotePasswdManager::parseServer, [] { __DBG_STUB_INVOKE__ return "1.2.3.4"; });
    stub.set(secret_password_clear, stubSecretClear);
    EXPECT_NO_FATAL_FAILURE(ins->clearPasswd("smb://1.2.3.4"));
    EXPECT_NO_FATAL_FAILURE(ins->clearPasswd("ftp://1.2.3.4"));
    EXPECT_NO_FATAL_FAILURE(ins->clearPasswd("helllllll"));
}

TEST_F(UT_RemotePasswdManager, LoadConfig)
{
    EXPECT_NO_FATAL_FAILURE(ins->loadConfig());

    QFile f(ins->configPath());
    f.open(QIODevice::ReadWrite | QIODevice::Truncate);
    QTextStream in(&f);
    in << "{}";
    f.close();

    EXPECT_NO_FATAL_FAILURE(ins->loadConfig());
    ::remove(ins->configPath().toStdString().c_str());
}

TEST_F(UT_RemotePasswdManager, SaveConfig)
{
    EXPECT_NO_FATAL_FAILURE(ins->saveConfig());

    QFile f(ins->configPath());
    f.open(QIODevice::ReadWrite | QIODevice::Truncate);
    QTextStream in(&f);
    in << "{}";
    f.close();

    EXPECT_NO_FATAL_FAILURE(ins->saveConfig());
    ::remove(ins->configPath().toStdString().c_str());
}

TEST_F(UT_RemotePasswdManager, GetLoginInfo)
{
    QFile f(ins->configPath());
    f.open(QIODevice::ReadWrite | QIODevice::Truncate);
    QTextStream in(&f);
    in << "{";
    in << "    \"smb://127.0.0.1/util-dfm/\": {";
    in << "        \"GAskPasswordFlags\": 31,";
    in << "        \"anonymous\": false,";
    in << "        \"domain\": \"WORKGROUP\",";
    in << "        \"id\": \"smb://127.0.0.1/util-dfm/\",";
    in << "        \"message\": \"共享 util-dfm 于 127.0.0.1 需要密码\",";
    in << "        \"passwordSave\": 2,";
    in << "        \"username\": \"xust\"";
    in << "    }";
    in << "}";
    f.close();

    EXPECT_NO_FATAL_FAILURE(ins->loadConfig());
    EXPECT_NO_FATAL_FAILURE(ins->getLoginInfo("smb://127.0.0.1"));
    EXPECT_FALSE(ins->getLoginInfo("smb://127.0.0.1").isEmpty());
    EXPECT_TRUE(ins->getLoginInfo("smb://192.168.1.1").isEmpty());
}

TEST_F(UT_RemotePasswdManager, ParseServer) {}

TEST_F(UT_RemotePasswdManager, ConfigPath)
{
    stub.clear();
    EXPECT_TRUE(ins->configPath().endsWith("samba.json"));
}

TEST_F(UT_RemotePasswdManager, SmbSchema)
{
    EXPECT_NO_FATAL_FAILURE(ins->smbSchema());
}

TEST_F(UT_RemotePasswdManager, FtpSchema)
{
    EXPECT_NO_FATAL_FAILURE(ins->ftpSchema());
}

static bool stubClearPasswdFinished()
{
    __DBG_STUB_INVOKE__
    return true;
}

TEST_F(UT_RemotePasswdManager, OnPasswdCleared)
{
    stub.set(secret_password_clear_finish, stubClearPasswdFinished);
    EXPECT_NO_FATAL_FAILURE(ins->onPasswdCleared(nullptr, nullptr, nullptr));
}
