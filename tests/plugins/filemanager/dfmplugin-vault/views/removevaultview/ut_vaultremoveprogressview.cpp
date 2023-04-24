// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "views/removevaultview/vaultremoveprogressview.h"

#include <gtest/gtest.h>

#include <DWaterProgress>

#include <QDir>

DPVAULT_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

TEST(UT_VaultRemoveProgressView, removeVault)
{
    bool isStart { false };
    stub_ext::StubExt stub;
    stub.set_lamda(&DWaterProgress::start, [ &isStart ] { __DBG_STUB_INVOKE__ isStart = true; });

    VaultRemoveProgressView view;
    view.removeVault("", "");
    EXPECT_FALSE(isStart);
}

TEST(UT_VaultRemoveProgressView, clear)
{
    VaultRemoveProgressView view;
    view.clear();
    EXPECT_TRUE(view.vaultRmProgressBar->value() == 0);
}

TEST(UT_VaultRemoveProgressView, statisticsFiles)
{
    stub_ext::StubExt stub;
    typedef bool(QDir::*FuncType)()const;
    stub.set_lamda(static_cast<FuncType>(&QDir::exists), [] { __DBG_STUB_INVOKE__ return true;});

    VaultRemoveProgressView view;
    EXPECT_TRUE(view.statisticsFiles("/test"));
}

TEST(UT_VaultRemoveProgressView, statisticsFiles_NoExist)
{
    VaultRemoveProgressView view;
    EXPECT_FALSE(view.statisticsFiles("/test"));
}

TEST(UT_VaultRemoveProgressView, removeFileInDir)
{
    stub_ext::StubExt stub;
    typedef  bool(QFile::*FuncType)();
    stub.set_lamda(static_cast<FuncType>(&QFile::remove), [] { __DBG_STUB_INVOKE__ return true; });
    stub.set_lamda(&QDir::rmdir, [] { __DBG_STUB_INVOKE__ return true; });

    VaultRemoveProgressView view;
    view.removeFileInDir("/test123");
    QDir dir("/test123");
    EXPECT_FALSE(dir.exists("/test123"));
}

TEST(UT_VaultRemoveProgressView, onFileRemove_True)
{
    VaultRemoveProgressView view;
    view.onFileRemove(100);
    EXPECT_TRUE(view.vaultRmProgressBar->value() == 100);
}
