// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/base/configs/dconfig/private/dconfigmanager_p.h>

#include <DConfig>

#include <QPointer>

#include <gtest/gtest.h>

class UT_DConfigManager : public testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

private:
    stub_ext::StubExt stub;
};

DFMBASE_USE_NAMESPACE
DCORE_USE_NAMESPACE

#define DCONFIG_SUPPORTED (DTK_VERSION >= DTK_VERSION_CHECK(5, 5, 30, 0))

TEST_F(UT_DConfigManager, AddConfig)
{
#if DCONFIG_SUPPORTED
    DConfigManager::instance()->d->configs.insert("fake", nullptr);
    QString err;
    EXPECT_FALSE(DConfigManager::instance()->addConfig("fake", &err));
    EXPECT_TRUE(err == "config is already added");

    typedef DConfig *(*Create)(const QString &, const QString &, const QString &, QObject *);
    auto create = static_cast<Create>(DConfig::create);
    DConfig *retVal { nullptr };
    stub.set_lamda(create, [&] { __DBG_STUB_INVOKE__ return retVal; });

    EXPECT_FALSE(DConfigManager::instance()->addConfig("invalid config", &err));
    EXPECT_TRUE(err == "cannot create config");

    retVal = new DConfig("invalid name");
    QPointer<DConfig> guard(retVal);
    EXPECT_FALSE(DConfigManager::instance()->addConfig("invalid config", &err));
    EXPECT_TRUE(err == "config is not valid");
    EXPECT_TRUE(guard.isNull());

    stub.set_lamda(&DConfig::isValid, [] { __DBG_STUB_INVOKE__ return true; });
    retVal = new DConfig("maybe is valid");
    guard = retVal;
    EXPECT_TRUE(DConfigManager::instance()->addConfig("valid config", &err));
    EXPECT_TRUE(!guard.isNull());
    EXPECT_TRUE(DConfigManager::instance()->d->configs.contains("valid config"));
    EXPECT_NO_FATAL_FAILURE(DConfigManager::instance()->d->configs.clear());
    delete retVal;
#endif
}

TEST_F(UT_DConfigManager, RemoveConfig)
{
    EXPECT_TRUE(DConfigManager::instance()->removeConfig("not exist!"));

    auto conf = new DConfig("test");
    QPointer<DConfig> guard(conf);
    DConfigManager::instance()->d->configs.insert("test", conf);
    EXPECT_TRUE(DConfigManager::instance()->removeConfig("test"));
    EXPECT_TRUE(guard.isNull());
}

TEST_F(UT_DConfigManager, Keys)
{
    EXPECT_TRUE(DConfigManager::instance()->keys("not exists").isEmpty());

    auto conf = new DConfig("test");
    DConfigManager::instance()->d->configs.insert("test", conf);

    stub.set_lamda(&DConfig::keyList, [] { __DBG_STUB_INVOKE__ return QStringList("hello world"); });
    EXPECT_TRUE(DConfigManager::instance()->keys("test").count() == 1
                && DConfigManager::instance()->keys("test").first() == "hello world");

    delete conf;
    DConfigManager::instance()->d->configs.clear();
}

TEST_F(UT_DConfigManager, Contains)
{
    EXPECT_FALSE(DConfigManager::instance()->contains("hello", "world"));
    EXPECT_FALSE(DConfigManager::instance()->contains("hello", ""));

    stub.set_lamda(&DConfigManager::keys, [] { __DBG_STUB_INVOKE__ return QStringList("world"); });
    EXPECT_TRUE(DConfigManager::instance()->contains("hello", "world"));
}

TEST_F(UT_DConfigManager, Value)
{
    stub.set_lamda(&DConfig::value, [] { __DBG_STUB_INVOKE__ return QVariant("Hello DFM"); });
    EXPECT_FALSE(DConfigManager::instance()->value("invalid", "invalid").isValid());
    EXPECT_TRUE(DConfigManager::instance()->value("invalid", "invalid", "valid").isValid());
    EXPECT_TRUE(DConfigManager::instance()->value("invalid", "invalid", "valid").toString() == "valid");

    auto conf = new DConfig("test");
    DConfigManager::instance()->d->configs.insert("test", conf);
    EXPECT_TRUE(DConfigManager::instance()->value("test", "test", "valid").isValid());
    EXPECT_TRUE(DConfigManager::instance()->value("test", "test", "valid").toString() == "Hello DFM");

    delete conf;
    DConfigManager::instance()->d->configs.clear();
}

TEST_F(UT_DConfigManager, SetValue)
{
    EXPECT_NO_FATAL_FAILURE(DConfigManager::instance()->setValue("hello", "world", "hello dtk"));

    auto conf = new DConfig("test");
    DConfigManager::instance()->d->configs.insert("test", conf);
    stub.set_lamda(&DConfig::setValue, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(DConfigManager::instance()->setValue("hello", "world", "hello dtk"));
    delete conf;
    DConfigManager::instance()->d->configs.clear();
}

TEST_F(UT_DConfigManager, ValidateConfigs)
{
    QList<bool> validList { 1, 0, 1, 0 };
    stub.set_lamda(&DConfig::isValid, [&] { __DBG_STUB_INVOKE__ return validList.takeFirst(); });

    DConfigManager::instance()->d->configs.insert("hello", new DConfig("hello"));
    DConfigManager::instance()->d->configs.insert("world", new DConfig("world"));
    DConfigManager::instance()->d->configs.insert("dtk", new DConfig("dtk"));
    DConfigManager::instance()->d->configs.insert("dfm", new DConfig("dfm"));

    QStringList invalidList;
    EXPECT_FALSE(DConfigManager::instance()->validateConfigs(invalidList));
    EXPECT_TRUE(invalidList.count() == 2
                && invalidList.contains("world")
                && invalidList.contains("dtk"));

    for (auto conf : DConfigManager::instance()->d->configs.values())
        delete conf;
    DConfigManager::instance()->d->configs.clear();
}
