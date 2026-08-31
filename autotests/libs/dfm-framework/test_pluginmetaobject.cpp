// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_pluginmetaobject.cpp
 * @brief Unit tests for PluginMetaObject methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-framework/lifecycle/pluginmetaobject.h"

#include <QTest>

using namespace src;

class PluginMetaObjectTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PluginMetaObject();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PluginMetaObject *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PluginMetaObjectTest, PluginMetaObject)
{
    // Test constructor: PluginMetaObject(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(PluginMetaObjectTest, version)
{
    // Test getter: DPF_BEGIN_NAMESPACE version()
    auto result = obj->version();
    EXPECT_NO_FATAL_FAILURE({ obj->version(); });

}

TEST_F(PluginMetaObjectTest, description)
{
    // Test getter: QString description()
    auto result = obj->description();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(PluginMetaObjectTest, category)
{
    // Test getter: QString category()
    auto result = obj->category();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(PluginMetaObjectTest, depends)
{
    // Test getter: QList<PluginDepend> depends()
    auto result = obj->depends();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(PluginMetaObjectTest, plugin)
{
    // Test getter: QSharedPointer<Plugin> plugin()
    auto result = obj->plugin();
    EXPECT_EQ(result.get(), nullptr);

}

TEST_F(PluginMetaObjectTest, iid)
{
    // Test getter: QString iid()
    auto result = obj->iid();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(PluginMetaObjectTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(PluginMetaObjectTest, urlLink)
{
    // Test getter: QString urlLink()
    auto result = obj->urlLink();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(PluginMetaObjectTest, customData)
{
    // Test getter: QVariantMap customData()
    auto result = obj->customData();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(PluginMetaObjectTest, pluginState)
{
    // Test getter: PluginMetaObject::State pluginState()
    auto result = obj->pluginState();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(PluginMetaObjectTest, errorString)
{
    // Test getter: QString errorString()
    auto result = obj->errorString();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(PluginMetaObjectTest, isVirtual)
{
    // Test bool getter: isVirtual()
    bool result = obj->isVirtual();
    EXPECT_FALSE(result);

}

TEST_F(PluginMetaObjectTest, fileName)
{
    // Test getter: QString fileName()
    auto result = obj->fileName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(PluginMetaObjectTest, quickMetaData)
{
    // Test getter: QList<PluginQuickMetaPtr> quickMetaData()
    auto result = obj->quickMetaData();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(PluginMetaObjectTest, State)
{
    // Test method:  State(())
    EXPECT_NO_FATAL_FAILURE(obj->State());
}

TEST_F(PluginMetaObjectTest, M_(PluginMetaObject))
{
    // Test getter: Q_DISABLE_COPY (PluginMetaObject)()
    EXPECT_NO_FATAL_FAILURE({ obj->(PluginMetaObject)(); });
}

TEST_F(PluginMetaObjectTest, d)
{
    // Test getter: QSharedPointer<PluginMetaObjectPrivate> d()
    auto result = obj->d();
    EXPECT_EQ(result.get(), nullptr);

}
