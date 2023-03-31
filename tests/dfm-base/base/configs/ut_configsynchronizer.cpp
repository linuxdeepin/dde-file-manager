// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <dfm-base/base/configs/configsyncdefs.h>
#include <dfm-base/base/configs/configsynchronizer.h>
#include <dfm-base/base/configs/private/configsynchronizer_p.h>
#include <dfm-base/base/configs/settingbackend.h>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE

class UT_ConfigSynchronizer : public testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

private:
    stub_ext::StubExt stub;
};

TEST_F(UT_ConfigSynchronizer, WatchChange)
{
    SyncPair pair;
    EXPECT_FALSE(ConfigSynchronizer::instance()->watchChange(pair));

    pair.cfg = { "hello", "world" };
    auto key = pair.serialize();
    ConfigSynchronizer::instance()->d->syncPairs.insert(key, pair);
    EXPECT_FALSE(ConfigSynchronizer::instance()->watchChange(pair));

    ConfigSynchronizer::instance()->d->syncPairs.clear();
    pair.set = { SettingType::kAppAttr, Application::ApplicationAttribute::kAllwayOpenOnNewWindow };
    auto add1 = static_cast<void (SettingBackend::*)(Application::ApplicationAttribute, SettingBackend::SaveOptFunc)>(&SettingBackend::addSettingAccessor);
    stub.set_lamda(add1, [] { __DBG_STUB_INVOKE__ });
    EXPECT_TRUE(ConfigSynchronizer::instance()->watchChange(pair));
    EXPECT_FALSE(ConfigSynchronizer::instance()->watchChange(pair));

    pair.set = { SettingType::kGenAttr, Application::GenericAttribute::kAlwaysShowOfflineRemoteConnections };
    auto add2 = static_cast<void (SettingBackend::*)(Application::GenericAttribute, SettingBackend::SaveOptFunc)>(&SettingBackend::addSettingAccessor);
    stub.set_lamda(add2, [] { __DBG_STUB_INVOKE__ });
    EXPECT_TRUE(ConfigSynchronizer::instance()->watchChange(pair));
    EXPECT_FALSE(ConfigSynchronizer::instance()->watchChange(pair));
}

class UT_ConfigSynchronizerPrivate : public testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

    static void SetUpTestCase()
    {
        d = ConfigSynchronizer::instance()->d.data();
    }
    static void TearDownTestCase() {}

private:
    stub_ext::StubExt stub;
    static ConfigSynchronizerPrivate *d;
};

ConfigSynchronizerPrivate *UT_ConfigSynchronizerPrivate::d { nullptr };

TEST_F(UT_ConfigSynchronizerPrivate, InitConn)
{
    EXPECT_NO_FATAL_FAILURE(d->initConn());
}

TEST_F(UT_ConfigSynchronizerPrivate, OnDConfChanged)
{
    stub.set_lamda(&ConfigSynchronizerPrivate::syncToAppSet, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(d->onDConfChanged("hello", "world"));
    EXPECT_NO_FATAL_FAILURE(d->onDConfChanged("hello", "uniontech"));
    EXPECT_NO_FATAL_FAILURE(d->onDConfChanged("hello", "deepin"));
}

TEST_F(UT_ConfigSynchronizerPrivate, SyncToAppSet)
{
    EXPECT_NO_FATAL_FAILURE(d->syncToAppSet("", "", QVariant()));

    SyncPair pair1 { { SettingType::kAppAttr, Application::ApplicationAttribute::kAllwayOpenOnNewWindow },
                     { "AppAttr", "OpenInNiewWin" } };
    SyncPair pair2 { { SettingType::kGenAttr, Application::GenericAttribute::kAlwaysShowOfflineRemoteConnections },
                     { "GenAttr", "ShowOfflineSmb" } };
    SyncPair pair3 { { SettingType::kNone, 999 }, { "invalid", "invalid" } };
    d->syncPairs.insert(pair1.serialize(), pair1);
    d->syncPairs.insert(pair2.serialize(), pair2);
    d->syncPairs.insert(pair3.serialize(), pair3);

    EXPECT_NO_FATAL_FAILURE(d->syncToAppSet("invalid", "invalid", QVariant()));
    stub.set_lamda(&Application::appAttribute, [] { __DBG_STUB_INVOKE__ return false; });
    stub.set_lamda(&Application::genericAttribute, [] { __DBG_STUB_INVOKE__ return false; });
    stub.set_lamda(&Application::setAppAttribute, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&Application::setGenericAttribute, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(d->syncToAppSet("AppAttr", "OpenInNiewWin", true));
    EXPECT_NO_FATAL_FAILURE(d->syncToAppSet("GenAttr", "ShowOfflineSmb", true));

    bool isEqual = false;
    SyncPair pair4 {
        { SettingType::kAppAttr, Application::ApplicationAttribute::kIconSizeLevel },
        { "AppAttr", "IconSizeLevel" },
        [](const QVariant &) {},
        [](const QString &, const QString &, const QVariant &) {},
        [&](const QVariant &, const QVariant &) { return isEqual; }
    };
    d->syncPairs.insert(pair4.serialize(), pair4);
    pair4.cfg = { "GenAttr", "AutoMount" };
    pair4.set = { SettingType::kGenAttr, Application::GenericAttribute::kAutoMount };
    d->syncPairs.insert(pair4.serialize(), pair4);

    isEqual = false;
    EXPECT_NO_FATAL_FAILURE(d->syncToAppSet("AppAttr", "IconSizeLevel", true));
    EXPECT_NO_FATAL_FAILURE(d->syncToAppSet("GenAttr", "AutoMount", true));

    isEqual = true;
    EXPECT_NO_FATAL_FAILURE(d->syncToAppSet("AppAttr", "IconSizeLevel", true));
    EXPECT_NO_FATAL_FAILURE(d->syncToAppSet("GenAttr", "AutoMount", true));
}
