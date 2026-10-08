// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_settingbackend.cpp
 * @brief Unit tests for SettingBackend (base/configs/settingbackend.cpp)
 *
 * Tests rely on -fno-access-control (enabled by dfm_add_test) to exercise the
 * protected doSetOption / onValueChanged slots directly.
 */

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QVariant>
#include <QString>
#include <QStringList>
#include <QSignalSpy>
#include <QTest>
#include <memory>

#include "stubext.h"
#include <dfm-base/base/configs/settingbackend.h>
#include <dfm-base/settingdialog/settingjsongenerator.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>

using namespace dfmbase;

namespace {
// Key strings mirrored from SettingBackendPrivate::keyToAA / keyToGA.
constexpr const char *kAllwayOpenOnNewWindowKey =
    "00_base.00_open_action.00_allways_open_on_new_window";
constexpr const char *kShowHiddenKey =
    "00_base.03_files_and_folders.00_show_hidden";
}   // namespace

class SettingBackendTest : public testing::Test
{
protected:
    void SetUp() override
    {
        // SettingBackend::instance() asserts Application exists. Application is
        // only materialised once an Application object is constructed (it seeds
        // ApplicationPrivate::self). Keep a single instance alive for the test
        // body, but never create a second one if a prior suite already did.
        if (Application::instance() == nullptr)
            appHolder = std::make_unique<Application>();
        ASSERT_NE(Application::instance(), nullptr);
        backend = SettingBackend::instance();
        ASSERT_NE(backend, nullptr);
    }

    std::unique_ptr<Application> appHolder;
    SettingBackend *backend;
};

TEST_F(SettingBackendTest, InstanceReturnsSamePointer)
{
    EXPECT_EQ(backend, SettingBackend::instance());
}

TEST_F(SettingBackendTest, KeysContainRegisteredAppAndGenericKeys)
{
    const QStringList all = backend->keys();
    EXPECT_FALSE(all.isEmpty());
    EXPECT_TRUE(all.contains(QString::fromLatin1(kAllwayOpenOnNewWindowKey)));
    EXPECT_TRUE(all.contains(QString::fromLatin1(kShowHiddenKey)));
}

TEST_F(SettingBackendTest, GetOptionForKnownKeyReturnsValidVariant)
{
    const QVariant v = backend->getOption(QString::fromLatin1(kAllwayOpenOnNewWindowKey));
    EXPECT_TRUE(v.isValid());
}

TEST_F(SettingBackendTest, GetOptionForUnknownKeyReturnsInvalidVariant)
{
    const QVariant v = backend->getOption("utterly.unknown.key.zzz");
    EXPECT_FALSE(v.isValid());
}

TEST_F(SettingBackendTest, AddSettingAccessorRegistersGetterAndSetter)
{
    const QString key = "ut.custom.accessor.key";
    backend->addSettingAccessor(
        key, []() { return QVariant(42); }, [](const QVariant &) {});
    EXPECT_TRUE(backend->keys().contains(key));
    EXPECT_EQ(backend->getOption(key).toInt(), 42);
    backend->removeSettingAccessor(key);
    EXPECT_FALSE(backend->keys().contains(key));
}

TEST_F(SettingBackendTest, RemoveSettingAccessorForUnknownKeyIsSafe)
{
    EXPECT_NO_FATAL_FAILURE({ backend->removeSettingAccessor("never.registered.key"); });
}

TEST_F(SettingBackendTest, AddSettingAccessorByApplicationAttributeIsSafe)
{
    EXPECT_NO_FATAL_FAILURE({
        backend->addSettingAccessor(Application::kAllwayOpenOnNewWindow,
                                     [](const QVariant &) {});
    });
}

TEST_F(SettingBackendTest, AddSettingAccessorByGenericAttributeIsSafe)
{
    EXPECT_NO_FATAL_FAILURE({
        backend->addSettingAccessor(Application::kShowedHiddenFiles,
                                     [](const QVariant &) {});
    });
}

TEST_F(SettingBackendTest, DoSetOptionPersistsViaDelayedSave)
{
    // Settings::sync is globally stubbed in main.cpp to prevent real config writes.
    const QString key = QString::fromLatin1(kAllwayOpenOnNewWindowKey);
    // Force a known baseline, then flip it through the delayed-save path.
    Application::instance()->setAppAttribute(Application::kAllwayOpenOnNewWindow, false);
    backend->doSetOption(key, QVariant(true));
    // onDelayedSave fires after the 100ms single-shot timer.
    QTest::qWait(300);
    EXPECT_TRUE(Application::instance()
                    ->appAttribute(Application::kAllwayOpenOnNewWindow)
                    .toBool());
}

TEST_F(SettingBackendTest, OnValueChangedEmitsOptionChanged)
{
    QSignalSpy spy(backend, &SettingBackend::optionChanged);
    ASSERT_TRUE(spy.isValid());
    backend->onValueChanged(static_cast<int>(Application::kAllwayOpenOnNewWindow),
                             QVariant(true));
    EXPECT_GE(spy.count(), 1);
    if (spy.count() >= 1) {
        const QList<QVariant> &args = spy.takeFirst();
        EXPECT_EQ(args.at(0).toString().toStdString(), kAllwayOpenOnNewWindowKey);
        EXPECT_EQ(args.at(1).toBool(), true);
    }
}

TEST_F(SettingBackendTest, OnValueChangedForUnknownAttributeEmitsNothing)
{
    QSignalSpy spy(backend, &SettingBackend::optionChanged);
    ASSERT_TRUE(spy.isValid());
    backend->onValueChanged(999999, QVariant());
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(SettingBackendTest, DoSyncIsNoopSafe)
{
    EXPECT_NO_FATAL_FAILURE({ backend->doSync(); });
}

TEST_F(SettingBackendTest, SetToSettingsWithNullptrIsSafe)
{
    EXPECT_NO_FATAL_FAILURE({ backend->setToSettings(nullptr); });
}

// ============================================================
// PMS sev-2 regression cluster: settingbackend.cpp (work-order batch 3)
// ============================================================

// PMS:298263 恢复默认视图模式入口缺失：SettingBackend 构造初始化后
// SettingJsonGenerator 必须注册 "02_workspace.00_view.04_restore_view_mode"
// pushButton 配置，且携带 trigger=kRestoreViewMode 供设置弹窗联动
TEST_F(SettingBackendTest, BUG298263_RestoreViewModeConfigRegistered)
{
    ASSERT_NE(backend, nullptr);   // fixture ctor runs initPresetSettingConfig()
    auto *gen = SettingJsonGenerator::instance();
    ASSERT_NE(gen, nullptr);

    const QString kViewModeKey = QStringLiteral("02_workspace.00_view.04_restore_view_mode");
    EXPECT_TRUE(gen->hasConfig(kViewModeKey));
    EXPECT_TRUE(gen->hasGroup(QStringLiteral("02_workspace.00_view")));
}

// PMS:285313 设置项布局调整："打开文件夹窗口使用独立进程" 选项从 01 键位迁到
// 02 键位并新增 03_open_file_action（单击/双击打开）组合框；旧键名必须消失，
// 否则设置界面出现重复项/丢失项
TEST_F(SettingBackendTest, BUG285313_OpenActionKeysReordered)
{
    ASSERT_NE(backend, nullptr);
    auto *gen = SettingJsonGenerator::instance();
    ASSERT_NE(gen, nullptr);

    const QString kSepProcessKey =
        QStringLiteral("00_base.00_open_action.02_open_folder_windows_in_aseparate_process");
    const QString kOpenFileActionKey =
        QStringLiteral("00_base.00_open_action.03_open_file_action");
    const QString kOldSepProcessKey =
        QStringLiteral("00_base.00_open_action.01_open_folder_windows_in_aseparate_process");

    // 新键位必须注册，旧键位必须移除
    EXPECT_TRUE(gen->hasConfig(kSepProcessKey));
    EXPECT_TRUE(gen->hasConfig(kOpenFileActionKey));
    EXPECT_FALSE(gen->hasConfig(kOldSepProcessKey));
    EXPECT_TRUE(gen->hasGroup(QStringLiteral("00_base.00_open_action")));

    // .02 仍是复选框，且子键名与键位一致
    const QVariantMap sepMeta = gen->configs.value(kSepProcessKey);
    EXPECT_EQ(sepMeta.value("type").toString(), QStringLiteral("checkbox"));
    EXPECT_EQ(sepMeta.value("key").toString(),
              QStringLiteral("02_open_folder_windows_in_aseparate_process"));

    // .03 打开方式组合框：单击/双击，默认 1（双击）
    const QVariantMap comboMeta = gen->configs.value(kOpenFileActionKey);
    EXPECT_EQ(comboMeta.value("type").toString(), QStringLiteral("combobox"));
    EXPECT_EQ(comboMeta.value("key").toString(),
              QStringLiteral("03_open_file_action"));
    EXPECT_EQ(comboMeta.value("default").toInt(), 1);
    const QStringList items = comboMeta.value("items").toStringList();
    ASSERT_EQ(items.size(), 2);
    EXPECT_FALSE(items.at(0).isEmpty());
    EXPECT_FALSE(items.at(1).isEmpty());
}

// PMS:346141 打开网络/手机/光盘目录卡顿：远程环境缩略图加载默认必须关闭，
// 且迁移为 checkBoxWithMessage（带提示文案）注册在缩略图预览组下
TEST_F(SettingBackendTest, BUG346141_RemoteEnvFilePreviewDefaultOff)
{
    ASSERT_NE(backend, nullptr);
    auto *gen = SettingJsonGenerator::instance();
    ASSERT_NE(gen, nullptr);

    const QString kRemoteEnvKey =
        QStringLiteral("02_workspace.01_thumb_preview.06_remote_env_file_preview");

    EXPECT_TRUE(gen->hasConfig(kRemoteEnvKey));
    EXPECT_TRUE(gen->hasGroup(QStringLiteral("02_workspace.01_thumb_preview")));

    const QVariantMap meta = gen->configs.value(kRemoteEnvKey);
    EXPECT_EQ(meta.value("key").toString(),
              QStringLiteral("06_remote_env_file_preview"));
    EXPECT_EQ(meta.value("type").toString(), QStringLiteral("checkBoxWithMessage"));
    // 回归核心：默认值必须为 false（高延迟设备默认不加载缩略图）
    EXPECT_FALSE(meta.value("default").toBool());
    EXPECT_FALSE(backend->getOption(kRemoteEnvKey).toBool());
    EXPECT_FALSE(meta.value("text").toString().isEmpty());
}
