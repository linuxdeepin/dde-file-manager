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

#include <dfm-base/base/configs/settingbackend.h>
#include <dfm-base/base/application/application.h>

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
