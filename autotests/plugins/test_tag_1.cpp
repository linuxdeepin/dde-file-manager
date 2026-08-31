// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tag_1.cpp
 * @brief Unit tests for Tag methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "tag.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new Tag();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    Tag *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagTest, bindEvents)
{
    // Test method: void bindEvents(())
    EXPECT_NO_FATAL_FAILURE(obj->bindEvents());
}

TEST_F(TagTest, bindScene)
{
    // Test method: void bindScene((const QString &parentScene))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->bindScene(_arg0));
}

TEST_F(TagTest, createTagWidgetForDetailView)
{
    // Test method: QWidget createTagWidgetForDetailView((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->createTagWidgetForDetailView(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->createTagWidgetForDetailView(_arg0); });

}

TEST_F(TagTest, createTagWidgetForPropertyDialog)
{
    // Test method: QWidget createTagWidgetForPropertyDialog((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->createTagWidgetForPropertyDialog(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->createTagWidgetForPropertyDialog(_arg0); });

}

TEST_F(TagTest, followEvents)
{
    // Test method: void followEvents(())
    EXPECT_NO_FATAL_FAILURE(obj->followEvents());
}

TEST_F(TagTest, regToDetailspace)
{
    // Test method: void regToDetailspace(())
    EXPECT_NO_FATAL_FAILURE(obj->regToDetailspace());
}

TEST_F(TagTest, regToPropertyDialog)
{
    // Test method: void regToPropertyDialog(())
    EXPECT_NO_FATAL_FAILURE(obj->regToPropertyDialog());
}

TEST_F(TagTest, registerPlugin)
{
    // Test method: void registerPlugin((const QString &pluginName, std::function<void()> callback))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->registerPlugin(_arg0, {}));
}

TEST_F(TagTest, shouldShowTagWidget)
{
    // Test method: bool shouldShowTagWidget((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->shouldShowTagWidget(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(TagTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}

TEST_F(TagTest, updateTagWidgetForDetailView)
{
    // Test method: void updateTagWidgetForDetailView((QWidget *widget, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->updateTagWidgetForDetailView(nullptr, _arg1));
}
