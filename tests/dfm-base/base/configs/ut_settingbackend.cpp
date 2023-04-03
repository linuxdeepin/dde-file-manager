// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <dfm-base/base/configs/settingbackend.h>
#include <dfm-base/base/configs/private/settingbackend_p.h>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
class UT_SettingBackend : public testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

private:
    stub_ext::StubExt stub;
};

TEST_F(UT_SettingBackend, Keys)
{
    int aaCount = SettingBackend::instance()->d->keyToAA.keys().count();
    int gaCount = SettingBackend::instance()->d->keyToGA.keys().count();
    EXPECT_TRUE(SettingBackend::instance()->keys().count() >= aaCount + gaCount);
}

TEST_F(UT_SettingBackend, GetOption)
{
    QVariant retVal1, retVal2;
    stub.set_lamda(&SettingBackendPrivate::getAsAppAttr, [&] { __DBG_STUB_INVOKE__ return retVal1; });
    stub.set_lamda(&SettingBackendPrivate::getAsGenAttr, [&] { __DBG_STUB_INVOKE__ return retVal2; });

    retVal1 = QString("hello world");
    EXPECT_TRUE(SettingBackend::instance()->getOption("hello").isValid());
    EXPECT_TRUE(SettingBackend::instance()->getOption("hello").toString() == retVal1);

    retVal1 = QVariant();
    retVal2 = QString("Hello DDE");
    EXPECT_TRUE(SettingBackend::instance()->getOption("hello").isValid());
    EXPECT_TRUE(SettingBackend::instance()->getOption("hello").toString() == retVal2);

    retVal2 = QVariant();
    EXPECT_NO_FATAL_FAILURE(SettingBackend::instance()->getOption("whatever"));
}

TEST_F(UT_SettingBackend, DoSync)
{
    EXPECT_NO_FATAL_FAILURE(SettingBackend::instance()->doSync());
}

TEST_F(UT_SettingBackend, AddSettingAccessor)
{
    typedef void (SettingBackend::*Add1)(const QString &, SettingBackend::GetOptFunc, SettingBackend::SaveOptFunc);
    auto add1 = static_cast<Add1>(&SettingBackend::addSettingAccessor);
    stub.set_lamda(add1, [] { __DBG_STUB_INVOKE__ });

    EXPECT_NO_FATAL_FAILURE(SettingBackend::instance()->addSettingAccessor(static_cast<Application::ApplicationAttribute>(100), nullptr));
    EXPECT_NO_FATAL_FAILURE(SettingBackend::instance()->addSettingAccessor(Application::ApplicationAttribute::kAllwayOpenOnNewWindow, nullptr));
    EXPECT_NO_FATAL_FAILURE(SettingBackend::instance()->addSettingAccessor(static_cast<Application::GenericAttribute>(100), nullptr));
    EXPECT_NO_FATAL_FAILURE(SettingBackend::instance()->addSettingAccessor(Application::GenericAttribute::kAlwaysShowOfflineRemoteConnections, nullptr));

    stub.clear();
    EXPECT_NO_FATAL_FAILURE(SettingBackend::instance()->addSettingAccessor("hello world", nullptr, nullptr));
    EXPECT_NO_FATAL_FAILURE(SettingBackend::instance()->addSettingAccessor("hello dtk", [] { return QVariant(); }, [](const QVariant &) {}));
}

TEST_F(UT_SettingBackend, DoSetOption)
{
    stub.set_lamda(&SettingBackendPrivate::saveAsAppAttr, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&SettingBackendPrivate::saveAsGenAttr, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&SettingBackendPrivate::saveByFunc, [] { __DBG_STUB_INVOKE__ });

    EXPECT_NO_FATAL_FAILURE(SettingBackend::instance()->doSetOption("hello", "world"));
    EXPECT_NO_FATAL_FAILURE(SettingBackend::instance()->doSetOption("hello", "dtk"));
    EXPECT_NO_FATAL_FAILURE(SettingBackend::instance()->doSetOption("hello", "deepin"));
}

TEST_F(UT_SettingBackend, OnValueChanged)
{
    EXPECT_NO_FATAL_FAILURE(SettingBackend::instance()->onValueChanged(1000, "invalid attr"));
}

class UT_SettingBackendPrivate : public testing::Test
{
protected:
    virtual void SetUp() override { d = SettingBackend::instance()->d.data(); }
    virtual void TearDown() override { stub.clear(); }

private:
    stub_ext::StubExt stub;
    SettingBackendPrivate *d { nullptr };
};

TEST_F(UT_SettingBackendPrivate, SaveAsAppAttr)
{
    stub.set_lamda(&Application::setAppAttribute, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(d->saveAsAppAttr("invalid key", "invalid value"));
    EXPECT_NO_FATAL_FAILURE(d->saveAsAppAttr("base.open_action.allways_open_on_new_window", "valid value"));
}

TEST_F(UT_SettingBackendPrivate, GetAsAppAttr)
{
    stub.set_lamda(&Application::appAttribute, [] { __DBG_STUB_INVOKE__ return "valid return value"; });
    EXPECT_FALSE(d->getAsAppAttr("invalidKey").isValid());
    EXPECT_TRUE(d->getAsAppAttr("base.open_action.allways_open_on_new_window").isValid());
}

TEST_F(UT_SettingBackendPrivate, SaveAsGenAttr)
{
    stub.set_lamda(&Application::setGenericAttribute, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(d->saveAsGenAttr("invalid key", "invalid value"));
    EXPECT_NO_FATAL_FAILURE(d->saveAsGenAttr("base.hidden_files.show_suffix", "valid value"));
}

TEST_F(UT_SettingBackendPrivate, GetAsGenAttr)
{
    stub.set_lamda(&Application::genericAttribute, [] { __DBG_STUB_INVOKE__ return "valid return value"; });
    EXPECT_FALSE(d->getAsGenAttr("invalidKey").isValid());
    EXPECT_TRUE(d->getAsGenAttr("base.hidden_files.show_suffix").isValid());
}

TEST_F(UT_SettingBackendPrivate, SaveByFunc)
{
    EXPECT_NO_FATAL_FAILURE(d->saveByFunc("empty saver", "invalid value"));
    d->setters.insert("test", [](const QVariant &var) {});
    EXPECT_NO_FATAL_FAILURE(d->saveByFunc("test", "invalid value"));
}

TEST_F(UT_SettingBackendPrivate, GetByFunc)
{
    EXPECT_FALSE(d->getByFunc("no getter").isValid());
    d->getters.insert("test", [] { return "valid getter"; });
    EXPECT_TRUE(d->getByFunc("test").isValid());
    EXPECT_TRUE(d->getByFunc("test").toString() == "valid getter");
}
