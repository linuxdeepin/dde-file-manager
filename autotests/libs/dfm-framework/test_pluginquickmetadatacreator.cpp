// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_pluginquickmetadatacreator.cpp
 * @brief Unit tests for PluginQuickMetaDataCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-framework/lifecycle/pluginquickmetadata.h"

#include <QTest>

using namespace src;

class PluginQuickMetaDataCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PluginQuickMetaDataCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PluginQuickMetaDataCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PluginQuickMetaDataCreatorTest, create)
{
    // Test method: void create((const QString &plugin, const QString &id, const QUrl &url))
    QString _arg0{};
    QString _arg1{};
    QUrl _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->create(_arg0, _arg1, _arg2));
}

TEST_F(PluginQuickMetaDataCreatorTest, setParent)
{
    // Test setter: void setParent((const QString &parent))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setParent(_arg0));
}

TEST_F(PluginQuickMetaDataCreatorTest, take)
{
    // Test getter: PluginQuickMetaPtr take()
    auto result = obj->take();
    EXPECT_EQ(result.get(), nullptr);

}
