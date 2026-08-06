// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_pluginmetaobject_ext.cpp
 * @brief Additional unit tests for PluginMetaObject (lifecycle/pluginmetaobject.cpp)
 *        covering the uncovered accessors (pluginState, plugin, quickMetaData,
 *        iid) and the two QDebug operator<< overloads (by-ref and by-pointer).
 */

#include <gtest/gtest.h>
#include <QDebug>
#include <QString>
#include <QSharedPointer>

#include <dfm-framework/lifecycle/pluginmetaobject.h>

using namespace dpf;

TEST(PluginMetaObjectExtTest, DefaultPluginStateIsInvalid)
{
    PluginMetaObject obj;
    EXPECT_EQ(obj.pluginState(), PluginMetaObject::State::kInvalid);
}

TEST(PluginMetaObjectExtTest, DefaultPluginIsNull)
{
    PluginMetaObject obj;
    EXPECT_TRUE(obj.plugin().isNull());
}

TEST(PluginMetaObjectExtTest, DefaultQuickMetaDataIsEmpty)
{
    PluginMetaObject obj;
    EXPECT_TRUE(obj.quickMetaData().isEmpty());
}

TEST(PluginMetaObjectExtTest, DefaultIidIsEmpty)
{
    PluginMetaObject obj;
    EXPECT_TRUE(obj.iid().isEmpty());
}

TEST(PluginMetaObjectExtTest, DefaultNameIsEmpty)
{
    PluginMetaObject obj;
    EXPECT_TRUE(obj.name().isEmpty());
}

TEST(PluginMetaObjectExtTest, DefaultVersionIsEmpty)
{
    PluginMetaObject obj;
    EXPECT_TRUE(obj.version().isEmpty());
}

TEST(PluginMetaObjectExtTest, DefaultCategoryIsEmpty)
{
    PluginMetaObject obj;
    EXPECT_TRUE(obj.category().isEmpty());
}

TEST(PluginMetaObjectExtTest, DefaultDescriptionIsEmpty)
{
    PluginMetaObject obj;
    EXPECT_TRUE(obj.description().isEmpty());
}

TEST(PluginMetaObjectExtTest, DefaultErrorStringIsEmpty)
{
    PluginMetaObject obj;
    EXPECT_TRUE(obj.errorString().isEmpty());
}

TEST(PluginMetaObjectExtTest, DefaultIsVirtualIsFalse)
{
    PluginMetaObject obj;
    EXPECT_FALSE(obj.isVirtual());
}

TEST(PluginMetaObjectExtTest, DefaultDependsIsEmpty)
{
    PluginMetaObject obj;
    EXPECT_TRUE(obj.depends().isEmpty());
}

TEST(PluginMetaObjectExtTest, DefaultCustomDataIsEmpty)
{
    PluginMetaObject obj;
    EXPECT_TRUE(obj.customData().isEmpty());
}

TEST(PluginMetaObjectExtTest, DebugOutputByRef)
{
    PluginMetaObject obj;
    QString output;
    QDebug debug(&output);
    debug << obj;
    EXPECT_FALSE(output.isEmpty());
}

TEST(PluginMetaObjectExtTest, DebugOutputByPointer)
{
    PluginMetaObject obj;
    PluginMetaObjectPointer ptr = PluginMetaObjectPointer::create();
    QString output;
    QDebug debug(&output);
    debug << ptr;
    EXPECT_FALSE(output.isEmpty());
}
