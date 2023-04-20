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
    virtual void SetUp() override { }
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
    stub.set(secret_password_clear, stubSecretClear);
    EXPECT_NO_FATAL_FAILURE(ins->clearPasswd("smb://1.2.3.4"));
    EXPECT_NO_FATAL_FAILURE(ins->clearPasswd("ftp://1.2.3.4"));
    EXPECT_NO_FATAL_FAILURE(ins->clearPasswd("helllllll"));
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
