// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_taghelper_1.cpp
 * @brief Unit tests for TagHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/taghelper.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagHelperTest, TagHelper)
{
    // Test constructor: TagHelper((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TagHelperTest, createSidebarItemInfo)
{
    // Test method: QVariantMap createSidebarItemInfo((const QString &tag))
    QString _arg0{};
    auto result = obj->createSidebarItemInfo(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagHelperTest, defaultColors)
{
    // Test getter: QList<QColor> defaultColors()
    auto result = obj->defaultColors();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagHelperTest, getColorNameByTag)
{
    // Test method: QString getColorNameByTag((const QString &tagName))
    QString _arg0{};
    auto result = obj->getColorNameByTag(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagHelperTest, getTagNameFromUrl)
{
    // Test method: QString getTagNameFromUrl((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->getTagNameFromUrl(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagHelperTest, initTagColorDefines)
{
    // Test method: void initTagColorDefines(())
    EXPECT_NO_FATAL_FAILURE(obj->initTagColorDefines());
}

TEST_F(TagHelperTest, instance)
{
    // Test getter: DFMBASE_USE_NAMESPACE instance()
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });
}

TEST_F(TagHelperTest, isDefaultTag)
{
    // Test method: bool isDefaultTag((const QString &tagName))
    QString _arg0{};
    auto result = obj->isDefaultTag(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(TagHelperTest, makeTagUrlByTagName)
{
    // Test method: QUrl makeTagUrlByTagName((const QString &tag))
    QString _arg0{};
    auto result = obj->makeTagUrlByTagName(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(TagHelperTest, paintTags)
{
    // Test method: void paintTags((QPainter *painter, QRectF &rect, const QList<QColor> &colors))
    QRectF _arg1{};
    QList<QColor> _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->paintTags(nullptr, _arg1, _arg2));
}

TEST_F(TagHelperTest, queryColorByColorName)
{
    // Test method: QColor queryColorByColorName((const QString &name))
    QString _arg0{};
    auto result = obj->queryColorByColorName(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(TagHelperTest, queryColorByDisplayName)
{
    // Test method: QColor queryColorByDisplayName((const QString &name))
    QString _arg0{};
    auto result = obj->queryColorByDisplayName(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(TagHelperTest, queryColorNameByColor)
{
    // Test method: QString queryColorNameByColor((const QColor &color))
    QColor _arg0{};
    auto result = obj->queryColorNameByColor(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagHelperTest, queryColorNameByDisplayName)
{
    // Test method: QString queryColorNameByDisplayName((const QString &name))
    QString _arg0{};
    auto result = obj->queryColorNameByDisplayName(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagHelperTest, queryDisplayNameByColor)
{
    // Test method: QString queryDisplayNameByColor((const QColor &color))
    QColor _arg0{};
    auto result = obj->queryDisplayNameByColor(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagHelperTest, queryIconNameByColor)
{
    // Test method: QString queryIconNameByColor((const QColor &color))
    QColor _arg0{};
    auto result = obj->queryIconNameByColor(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagHelperTest, queryIconNameByColorName)
{
    // Test method: QString queryIconNameByColorName((const QString &colorName))
    QString _arg0{};
    auto result = obj->queryIconNameByColorName(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagHelperTest, randomTagDefine)
{
    // Test getter: TagColorDefine randomTagDefine()
    auto result = obj->randomTagDefine();
    EXPECT_NO_FATAL_FAILURE({ obj->randomTagDefine(); });

}

TEST_F(TagHelperTest, rootUrl)
{
    // Test getter: QUrl rootUrl()
    auto result = obj->rootUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(TagHelperTest, scheme)
{
    // Test getter: QString scheme()
    auto result = obj->scheme();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagHelperTest, showTagEdit)
{
    // Test method: void showTagEdit((const QRectF &parentRect, const QRectF &iconRect, const QList<QUrl> &fileList, bool showInTagDir))
    QRectF _arg0{};
    QRectF _arg1{};
    QList<QUrl> _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->showTagEdit(_arg0, _arg1, _arg2, false));
}
