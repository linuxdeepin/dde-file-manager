// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractfilepreviewplugin.cpp
 * @brief Unit tests for AbstractFilePreviewPlugin base class (abstractfilepreviewplugin.cpp)
 *
 * AbstractFilePreviewPlugin is an abstract QObject base for file-preview
 * factories. Only the constructor has a .cpp body; create() is a pure
 * virtual factory method. A TestablePreviewPlugin stub records the key it
 * was asked to create and returns nullptr (a contract-permitted result),
 * exercising the base-class constructor without any real preview backend or
 * file I/O. Returning nullptr avoids needing a concrete AbstractBasePreview
 * (which itself declares many pure virtuals).
 */

#include <gtest/gtest.h>
#include <dfm-base/interfaces/abstractfilepreviewplugin.h>

#include <QObject>
#include <QString>

using namespace dfmbase;

namespace {
class TestablePreviewPlugin : public AbstractFilePreviewPlugin
{
public:
    explicit TestablePreviewPlugin(QObject *parent = nullptr)
        : AbstractFilePreviewPlugin(parent) {}

    AbstractBasePreview *create(const QString &key) override
    {
        m_lastKey = key;
        return nullptr;   // contract-permitted: "no preview for this key"
    }

    QString m_lastKey;
};
}   // namespace

TEST(AbstractFilePreviewPluginTest, ConstructorSetsParent)
{
    QObject parent;
    auto *plugin = new TestablePreviewPlugin(&parent);
    EXPECT_EQ(plugin->parent(), &parent);
    delete plugin;
}

TEST(AbstractFilePreviewPluginTest, ConstructorAcceptsNullParent)
{
    auto *plugin = new TestablePreviewPlugin(nullptr);
    EXPECT_EQ(plugin->parent(), nullptr);
    delete plugin;
}

TEST(AbstractFilePreviewPluginTest, CreateRecordsKeyAndReturnsNull)
{
    TestablePreviewPlugin plugin;
    AbstractBasePreview *preview = plugin.create(QStringLiteral("image"));
    EXPECT_EQ(preview, nullptr);
    EXPECT_EQ(plugin.m_lastKey, QStringLiteral("image"));
}

TEST(AbstractFilePreviewPluginTest, CreateAcceptsEmptyKey)
{
    TestablePreviewPlugin plugin;
    AbstractBasePreview *preview = plugin.create(QString());
    EXPECT_EQ(preview, nullptr);
    EXPECT_TRUE(plugin.m_lastKey.isEmpty());
}
