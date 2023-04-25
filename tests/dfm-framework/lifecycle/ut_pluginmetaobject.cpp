// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testplugin.h"

#include <dfm-framework/lifecycle/pluginmetaobject.h>
#include <dfm-framework/lifecycle/private/pluginmetaobject_p.h>

#include <gtest/gtest.h>

#include <QDebug>

DPF_USE_NAMESPACE

class UT_PluginMetaObject : public testing::Test
{
public:
    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
    }
};

TEST_F(UT_PluginMetaObject, test_CopyConstructor_Depend)
{
    PluginDepend depend1;
    depend1.pluginName = "test_plugin";
    depend1.pluginVersion = "1.0";
    PluginDepend depend2(depend1);
    EXPECT_EQ(depend1.pluginName, depend2.pluginName);
    EXPECT_EQ(depend1.pluginVersion, depend2.pluginVersion);
}

TEST_F(UT_PluginMetaObject, test_DefaultConstructor)
{
    PluginMetaObject meta;
    EXPECT_FALSE(meta.isVirtual());
    EXPECT_EQ(meta.iid(), "");
    EXPECT_EQ(meta.name(), "");
    EXPECT_EQ(meta.version(), "");
    EXPECT_EQ(meta.category(), "");
    EXPECT_EQ(meta.description(), "");
    EXPECT_EQ(meta.urlLink(), "");
    EXPECT_EQ(meta.depends().size(), 0);
    EXPECT_EQ(meta.pluginState(), PluginMetaObject::kInvalid);
    EXPECT_EQ(meta.plugin(), nullptr);
    EXPECT_EQ(meta.errorString(), "");
}

TEST_F(UT_PluginMetaObject, ut_getters)
{
    PluginMetaObject meta;

    // Test default constructor values
    EXPECT_FALSE(meta.isVirtual());
    EXPECT_TRUE(meta.iid().isEmpty());
    EXPECT_TRUE(meta.name().isEmpty());
    EXPECT_TRUE(meta.version().isEmpty());
    EXPECT_TRUE(meta.category().isEmpty());
    EXPECT_TRUE(meta.description().isEmpty());
    EXPECT_TRUE(meta.urlLink().isEmpty());
    EXPECT_TRUE(meta.depends().isEmpty());
    EXPECT_EQ(PluginMetaObject::kInvalid, meta.pluginState());
    EXPECT_TRUE(meta.plugin().isNull());
    EXPECT_TRUE(meta.errorString().isEmpty());

    // Test setters
    meta.d->isVirtual = true;
    EXPECT_TRUE(meta.isVirtual());

    meta.d->iid = "test.iid";
    EXPECT_EQ("test.iid", meta.iid());

    meta.d->name = "test.name";
    EXPECT_EQ("test.name", meta.name());

    meta.d->version = "test.version";
    EXPECT_EQ("test.version", meta.version());

    meta.d->category = "test.category";
    EXPECT_EQ("test.category", meta.category());

    meta.d->description = "test.description";
    EXPECT_EQ("test.description", meta.description());

    meta.d->urlLink = "test.urlLink";
    EXPECT_EQ("test.urlLink", meta.urlLink());

    QList<PluginDepend> dependList;
    PluginDepend depend1, depend2;
    depend1.pluginName = "test.depend1.name";
    depend1.pluginVersion = "test.depend1.version";
    depend2.pluginName = "test.depend2.name";
    depend2.pluginVersion = "test.depend2.version";
    dependList.append(depend1);
    dependList.append(depend2);
    meta.d->depends = dependList;
    EXPECT_EQ(dependList.size(), meta.depends().size());
    EXPECT_EQ(meta.depends().at(0).name(), "test.depend1.name");
    EXPECT_EQ(meta.depends().at(0).version(), "test.depend1.version");

    meta.d->state = PluginMetaObject::kLoading;
    EXPECT_EQ(PluginMetaObject::kLoading, meta.pluginState());

    QSharedPointer<Plugin> plugin(new TestPlugin);
    meta.d->plugin = plugin;
    EXPECT_EQ(plugin, meta.plugin());

    meta.d->error = "test.errorString";
    EXPECT_EQ("test.errorString", meta.errorString());
    EXPECT_NO_FATAL_FAILURE(qDebug() << meta);
}
