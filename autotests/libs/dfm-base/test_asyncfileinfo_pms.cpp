// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_asyncfileinfo_pms.cpp
 * @brief PMS sev-2 regression tests for AsyncFileInfo
 *        (src/dfm-base/file/local/asyncfileinfo.cpp).
 *
 * Bug -> case mapping (fix commit verified via `git show`):
 *   - PMS:225369 / PMS:225387 (5d01fee1) AsyncFileInfoPrivate::attribute
 *     must forward the dfm-io value even when the per-attribute query
 *     flag (getOk) is false, and must always propagate the ok flag to
 *     the caller. Before the fix the value was dropped on !getOk, so
 *     rename/delete menu states ("canFetch" style predicates reading
 *     access::can-rename) greyed out unexpectedly.
 *
 * Both bugs share the same fix commit, so they get separate TESTs that
 * pin down complementary halves of the contract.
 */

#include <gtest/gtest.h>

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include "dfm-base/file/local/private/asyncfileinfo_p.h"
#include <dfm-io/dfileinfo.h>
#include <dfm-base/dfm_global_defines.h>

#include "stubext.h"

#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QIcon>
#include <mutex>

using namespace dfmbase;

class UT_AsyncFileInfoPms : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        std::call_once(flag, [] {
            UrlRoute::regScheme(Global::Scheme::kFile, QDir::homePath(), QIcon(), false, "file");
            InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        });
    }

    void SetUp() override
    {
        ASSERT_TRUE(tmpDir.isValid());
        dirPath = tmpDir.path();
        filePath = dirPath + "/async_pms.txt";
        QFile f(filePath);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("pms async");
        f.close();
        fileUrl = QUrl::fromLocalFile(filePath);
        dirUrl = QUrl::fromLocalFile(dirPath);
    }

    void TearDown() override
    {
        stub.clear();
    }

    QTemporaryDir tmpDir;
    QString dirPath;
    QString filePath;
    QUrl fileUrl;
    QUrl dirUrl;
    stub_ext::StubExt stub;
    static std::once_flag flag;
};

std::once_flag UT_AsyncFileInfoPms::flag;

// PMS:225369 查询完成后 attribute 必须把 dfm-io 的值与 ok 标志完整透传给调用方
TEST_F(UT_AsyncFileInfoPms, BUG225369_AttributeForwardedAfterQuery)
{
    // Arrange — force the "query finished" state and a successful dfm-io
    // attribute lookup (the async querier is not reliably schedulable in
    // the offscreen UT environment, so the dfm-io layer is stubbed).
    AsyncFileInfo info(fileUrl);
    ASSERT_TRUE(info.d != nullptr);
    ASSERT_TRUE(info.d->dfmFileInfo != nullptr);

    stub.set_lamda(&DFMIO::DFileInfo::queryAttributeFinished,
                   [](DFMIO::DFileInfo *) -> bool { return true; });
    stub.set_lamda(static_cast<QVariant (DFMIO::DFileInfo::*)(DFMIO::DFileInfo::AttributeID, bool *) const>(
                       &DFMIO::DFileInfo::attribute),
                   [](DFMIO::DFileInfo *, DFMIO::DFileInfo::AttributeID, bool *ok) -> QVariant {
                       if (ok)
                           *ok = true;
                       return QStringLiteral("async_pms.txt");
                   });

    // Act — read the attribute through the private accessor.
    bool ok = false;
    const QVariant name = info.d->attribute(DFMIO::DFileInfo::AttributeID::kStandardName, &ok);

    // Assert — both the value and the ok flag come straight from dfm-io.
    EXPECT_TRUE(ok);
    EXPECT_EQ(name.toString(), QStringLiteral("async_pms.txt"));
}

// PMS:225387 getOk=false 时不得丢弃 dfm-io 返回的 value：属性查询失败时
// dfm-io 会返回默认值且 ok=false，修复前这里直接丢值导致重命名/删除置灰
TEST_F(UT_AsyncFileInfoPms, BUG225387_FailedAttributeStillForwardsValue)
{
    // Arrange — force the "query finished" state and a failed dfm-io
    // lookup that still carries a (default) value, as happens for
    // missing gio attributes.
    AsyncFileInfo info(fileUrl);
    ASSERT_TRUE(info.d != nullptr);
    ASSERT_TRUE(info.d->dfmFileInfo != nullptr);

    stub.set_lamda(&DFMIO::DFileInfo::queryAttributeFinished,
                   [](DFMIO::DFileInfo *) -> bool { return true; });
    stub.set_lamda(static_cast<QVariant (DFMIO::DFileInfo::*)(DFMIO::DFileInfo::AttributeID, bool *) const>(
                       &DFMIO::DFileInfo::attribute),
                   [](DFMIO::DFileInfo *, DFMIO::DFileInfo::AttributeID, bool *ok) -> QVariant {
                       if (ok)
                           *ok = false;
                       return QStringLiteral("dfm-io-default-value");
                   });

    // Act
    bool ok = true;
    const QVariant value = info.d->attribute(DFMIO::DFileInfo::AttributeID::kAccessCanRename, &ok);

    // Assert — the value must be forwarded (not dropped) while the ok
    // flag keeps reporting the failure.
    EXPECT_EQ(value.toString(), QStringLiteral("dfm-io-default-value"));
    EXPECT_FALSE(ok);
}

// PMS:258661 任务栏图标刷新崩溃：刷新图标入口 customData(kItemFileRefreshIcon)
// 旧实现调用 d->updateIcon() -> LocalFileIconProvider::icon(q) 经 sharedFromThis()
// 取句柄，对非 QSharedPointer 托管的栈上对象抛 bad_weak_ptr 直接崩溃；修复后
// 原地加 iconLock 写锁并仅重置 "unknown" 占位图标，不再触碰 sharedFromThis
TEST_F(UT_AsyncFileInfoPms, BUG258661_RefreshIconOnStackObjectDoesNotTouchSharedFromThis)
{
    // 栈上对象：非 QSharedPointer 托管，旧实现经 updateIcon()->sharedFromThis() 必崩
    AsyncFileInfo info(fileUrl);
    ASSERT_TRUE(info.d != nullptr);

    // 非 "unknown" 占位图标（纯 pixmap）不受刷新影响，且全程不崩溃
    const QIcon pixmapIcon(QPixmap(8, 8));
    info.d->fileIcon = pixmapIcon;
    EXPECT_NO_FATAL_FAILURE(info.customData(dfmbase::Global::kItemFileRefreshIcon));
    EXPECT_FALSE(info.d->fileIcon.isNull());
    EXPECT_TRUE(info.d->fileIcon.pixmap(8).toImage() == pixmapIcon.pixmap(8).toImage());

    // "unknown" 占位分支重置为空图标（依赖环境主题是否含 unknown 图标，
    // offscreen 无主题时 name() 为空则跳过该子断言）
    const QIcon unknownIcon = QIcon::fromTheme(QStringLiteral("unknown"));
    if (unknownIcon.name() == QStringLiteral("unknown")) {
        info.d->fileIcon = unknownIcon;
        info.customData(dfmbase::Global::kItemFileRefreshIcon);
        EXPECT_TRUE(info.d->fileIcon.isNull());
    }
}
