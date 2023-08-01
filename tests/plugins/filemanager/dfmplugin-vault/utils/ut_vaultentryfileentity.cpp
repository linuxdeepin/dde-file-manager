// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "utils/vaultentryfileentity.h"
#include "utils/vaulthelper.h"

#include <gtest/gtest.h>

#include <dfm-base/utils/filestatisticsjob.h>

DPVAULT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

TEST(UT_VaultEntryFileEntity, displayName)
{
    VaultEntryFileEntity entity(QUrl("dfmvault:///"));
    QString name = entity.displayName();
    QIcon icon = entity.icon();
    entity.refresh();

    EXPECT_FALSE(name.isEmpty() || icon.isNull());
}

TEST(UT_VaultEntryFileEntity, exists)
{
    VaultEntryFileEntity entity(QUrl("dfmvault:///"));
    EXPECT_TRUE(entity.exists());
}

TEST(UT_VaultEntryFileEntity, showProgress)
{
    VaultEntryFileEntity entity(QUrl("dfmvault:///"));
    EXPECT_FALSE(entity.showProgress());
}

TEST(UT_VaultEntryFileEntity, showTotalSize_one)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::state, []{
        return VaultState::kUnlocked;
    });
    typedef void(FileStatisticsJob::*FuncType)(const QList<QUrl> &);
    stub.set_lamda(static_cast<FuncType>(&FileStatisticsJob::start), []{});

    VaultEntryFileEntity entity(QUrl("dfmvault:///"));
    bool isOk = entity.showTotalSize();

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultEntryFileEntity, showTotalSize_two)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::state, []{
        return VaultState::kUnknow;
    });

    VaultEntryFileEntity entity(QUrl("dfmvault:///"));
    bool isOk = entity.showTotalSize();

    EXPECT_FALSE(isOk);
}

TEST(UT_VaultEntryFileEntity, showUsageSize)
{
    VaultEntryFileEntity entity(QUrl("dfmvault:///"));
    EXPECT_FALSE(entity.showUsageSize());
}

TEST(UT_VaultEntryFileEntity, order)
{
    VaultEntryFileEntity entity(QUrl("dfmvault:///"));
    EntryFileInfo::EntryOrder order = entity.order();

    EXPECT_TRUE(order == EntryFileInfo::EntryOrder(static_cast<int>(EntryFileInfo::EntryOrder::kOrderCustom) + 1));
}

TEST(UT_VaultEntryFileEntity, sizeTotal_one)
{
    VaultEntryFileEntity entity(QUrl("dfmvault:///"));
    entity.vaultTotal = 1;
    quint64 size = entity.sizeTotal();

    EXPECT_TRUE(size == 1);
}

TEST(UT_VaultEntryFileEntity, sizeTotal_two)
{
    VaultEntryFileEntity entity(QUrl("dfmvault:///"));
    entity.vaultTotal = 0;
    entity.totalchange = 1;
    quint64 size = entity.sizeTotal();

    EXPECT_TRUE(size == 1);
}

TEST(UT_VaultEntryFileEntity, targetUrl)
{
    VaultEntryFileEntity entiy(QUrl("dfmvault:///"));
    QUrl url = entiy.targetUrl();

    EXPECT_TRUE(url == QUrl("dfmvault:///"));
}

TEST(UT_VaultEntryFileEntity, slotFileDirSizeChange)
{
    VaultEntryFileEntity entity(QUrl("dfmvault:///"));
    entity.showSizeState = true;
    entity.vaultTotal = 1;
    entity.slotFileDirSizeChange(2, 1, 1);

    EXPECT_TRUE(entity.vaultTotal == 2);
}

TEST(UT_VaultEntryFileEntity, slotFinishedThread)
{
    VaultEntryFileEntity entity(QUrl("dfmvault:///"));
    entity.slotFinishedThread();

    EXPECT_TRUE(entity.showSizeState ==  false);
}










