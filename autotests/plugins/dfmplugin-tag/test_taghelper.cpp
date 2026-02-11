// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-tag/utils/taghelper.h"
#include "plugins/common/dfmplugin-tag/utils/tagmanager.h"
#include "plugins/common/dfmplugin-tag/widgets/tageditor.h"

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <gtest/gtest.h>

#include <QUrl>
#include <QColor>
#include <QPainter>
#include <QPixmap>

using namespace dfmplugin_tag;
DFMBASE_USE_NAMESPACE

class UT_TagHelper : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        helper = TagHelper::instance();
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    TagHelper *helper = nullptr;
};

TEST_F(UT_TagHelper, instance)
{
    // Test singleton instance
    TagHelper *instance1 = TagHelper::instance();
    TagHelper *instance2 = TagHelper::instance();

    EXPECT_NE(instance1, nullptr);
    EXPECT_EQ(instance1, instance2);
}

TEST_F(UT_TagHelper, scheme)
{
    // Test scheme method
    EXPECT_EQ(TagHelper::scheme(), "tag");
}

TEST_F(UT_TagHelper, rootUrl)
{
    // Test rootUrl static method
    QUrl root = TagHelper::rootUrl();

    EXPECT_EQ(root.scheme(), "tag");
    EXPECT_EQ(root.path(), "/");
}

TEST_F(UT_TagHelper, commonUrls_Empty)
{
    // Test with empty URL list
    QList<QUrl> urls;
    QList<QUrl> result = TagHelper::commonUrls(urls);

    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_TagHelper, commonUrls_NoTransform)
{
    // Test when URLs don't need transformation
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/home/user/test.txt");

    // Stub bindUrlTransform to return the same URL
    stub.set_lamda(&FileUtils::bindUrlTransform, [](const QUrl &url) -> QUrl {
        __DBG_STUB_INVOKE__
        return url;
    });

    stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &url1, const QUrl &url2) -> bool {
        __DBG_STUB_INVOKE__
        return url1 == url2;
    });

    QList<QUrl> result = TagHelper::commonUrls(urls);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result.first(), urls.first());
}

TEST_F(UT_TagHelper, commonUrls_WithTransform)
{
    // Test when URLs need transformation
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/media/mount/test.txt");

    QUrl transformedUrl = QUrl::fromLocalFile("/home/user/test.txt");

    // Stub bindUrlTransform to return a different URL
    stub.set_lamda(&FileUtils::bindUrlTransform, [transformedUrl](const QUrl &url) -> QUrl {
        __DBG_STUB_INVOKE__
        return transformedUrl;
    });

    stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &url1, const QUrl &url2) -> bool {
        __DBG_STUB_INVOKE__
        return false;   // URLs are different
    });

    QList<QUrl> result = TagHelper::commonUrls(urls);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result.first(), transformedUrl);
}

TEST_F(UT_TagHelper, defualtColors)
{
    // Test default colors list
    QList<QColor> colors = helper->defualtColors();

    // Should have 8 default colors based on initTagColorDefines
    EXPECT_EQ(colors.size(), 8);

    // Verify at least one color is valid
    EXPECT_TRUE(colors.first().isValid());
}

TEST_F(UT_TagHelper, qureyColorByColorName_Valid)
{
    // Test querying color by valid color name
    QColor color = helper->qureyColorByColorName("Orange");

    EXPECT_TRUE(color.isValid());
    EXPECT_EQ(color.name().toLower(), "#ffa503");
}

TEST_F(UT_TagHelper, qureyColorByColorName_Invalid)
{
    // Test querying color by invalid color name
    QColor color = helper->qureyColorByColorName("InvalidColor");

    EXPECT_FALSE(color.isValid());
}

TEST_F(UT_TagHelper, qureyColorByDisplayName_Valid)
{
    // Test querying color by valid display name
    QColor color = helper->qureyColorByDisplayName(QObject::tr("Orange"));

    EXPECT_TRUE(color.isValid());
}

TEST_F(UT_TagHelper, qureyColorByDisplayName_Invalid)
{
    // Test querying color by invalid display name
    QColor color = helper->qureyColorByDisplayName("InvalidDisplayName");

    EXPECT_FALSE(color.isValid());
}

TEST_F(UT_TagHelper, qureyColorNameByColor_Valid)
{
    // Test querying color name by valid color
    QColor color("#ffa503");
    QString colorName = helper->qureyColorNameByColor(color);

    EXPECT_EQ(colorName, "Orange");
}

TEST_F(UT_TagHelper, qureyColorNameByColor_Invalid)
{
    // Test querying color name by invalid color
    QColor color("#123456");   // Non-existent color
    QString colorName = helper->qureyColorNameByColor(color);

    EXPECT_TRUE(colorName.isEmpty());
}

TEST_F(UT_TagHelper, qureyIconNameByColorName_Valid)
{
    // Test querying icon name by valid color name
    QString iconName = helper->qureyIconNameByColorName("Orange");

    EXPECT_EQ(iconName, "dfm_tag_orange");
}

TEST_F(UT_TagHelper, qureyIconNameByColorName_Invalid)
{
    // Test querying icon name by invalid color name
    QString iconName = helper->qureyIconNameByColorName("InvalidColor");

    EXPECT_TRUE(iconName.isEmpty());
}

TEST_F(UT_TagHelper, qureyIconNameByColor_Valid)
{
    // Test querying icon name by valid color
    QColor color("#ffa503");
    QString iconName = helper->qureyIconNameByColor(color);

    EXPECT_EQ(iconName, "dfm_tag_orange");
}

TEST_F(UT_TagHelper, qureyIconNameByColor_Invalid)
{
    // Test querying icon name by invalid color
    QColor color("#123456");
    QString iconName = helper->qureyIconNameByColor(color);

    EXPECT_TRUE(iconName.isEmpty());
}

TEST_F(UT_TagHelper, qureyDisplayNameByColor_Valid)
{
    // Test querying display name by valid color
    QColor color("#ffa503");
    QString displayName = helper->qureyDisplayNameByColor(color);

    EXPECT_EQ(displayName, QObject::tr("Orange"));
}

TEST_F(UT_TagHelper, qureyDisplayNameByColor_Invalid)
{
    // Test querying display name by invalid color
    QColor color("#123456");
    QString displayName = helper->qureyDisplayNameByColor(color);

    EXPECT_TRUE(displayName.isEmpty());
}

TEST_F(UT_TagHelper, qureyColorNameByDisplayName_Valid)
{
    // Test querying color name by valid display name
    QString colorName = helper->qureyColorNameByDisplayName(QObject::tr("Orange"));

    EXPECT_EQ(colorName, "Orange");
}

TEST_F(UT_TagHelper, qureyColorNameByDisplayName_Invalid)
{
    // Test querying color name by invalid display name
    QString colorName = helper->qureyColorNameByDisplayName("InvalidDisplayName");

    EXPECT_TRUE(colorName.isEmpty());
}

TEST_F(UT_TagHelper, getTagNameFromUrl_Valid)
{
    // Test extracting tag name from valid tag URL
    QUrl tagUrl;
    tagUrl.setScheme("tag");
    tagUrl.setPath("/MyTag");

    QString tagName = helper->getTagNameFromUrl(tagUrl);

    EXPECT_EQ(tagName, "MyTag");
}

TEST_F(UT_TagHelper, getTagNameFromUrl_Invalid)
{
    // Test extracting tag name from non-tag URL
    QUrl fileUrl = QUrl::fromLocalFile("/home/user/test.txt");

    QString tagName = helper->getTagNameFromUrl(fileUrl);

    EXPECT_TRUE(tagName.isEmpty());
}

TEST_F(UT_TagHelper, makeTagUrlByTagName)
{
    // Test creating tag URL from tag name
    QString tagName = "MyTag";
    QUrl tagUrl = helper->makeTagUrlByTagName(tagName);

    EXPECT_EQ(tagUrl.scheme(), "tag");
    EXPECT_EQ(tagUrl.path(), "/MyTag");
}

TEST_F(UT_TagHelper, getColorNameByTag_DefaultTag)
{
    // Test getting color name for default tag
    QString colorName = helper->getColorNameByTag(QObject::tr("Orange"));

    EXPECT_EQ(colorName, "Orange");
}

TEST_F(UT_TagHelper, getColorNameByTag_CustomTag)
{
    // Test getting color name for custom tag (should return random color)
    QString colorName = helper->getColorNameByTag("CustomTag");

    // Should return one of the default color names
    EXPECT_FALSE(colorName.isEmpty());
}

TEST_F(UT_TagHelper, isDefualtTag_DefaultTag)
{
    // Test checking if tag is default tag
    bool isDefault = helper->isDefualtTag(QObject::tr("Orange"));

    EXPECT_TRUE(isDefault);
}

TEST_F(UT_TagHelper, isDefualtTag_CustomTag)
{
    // Test checking if custom tag is default tag
    bool isDefault = helper->isDefualtTag("CustomTag");

    EXPECT_FALSE(isDefault);
}

TEST_F(UT_TagHelper, paintTags)
{
    // Test painting tags
    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);
    QRectF rect(0, 0, 100, 100);
    QList<QColor> colors;
    colors << QColor("#ffa503") << QColor("#ff1c49");

    EXPECT_NO_THROW(helper->paintTags(&painter, rect, colors));

    // Rect should be modified (right side moved left)
    EXPECT_LT(rect.right(), 100);
}

TEST_F(UT_TagHelper, createSidebarItemInfo)
{
    // Test creating sidebar item info
    QString tagName = "TestTag";

    // Stub TagManager::instance to avoid initialization issues
    stub.set_lamda(&TagManager::instance, []() -> TagManager * {
        __DBG_STUB_INVOKE__
        static TagManager manager;
        return &manager;
    });

    stub.set_lamda(&TagManager::getTagIconName, [](TagManager *, const QString &) -> QString {
        __DBG_STUB_INVOKE__
        return "dfm_tag_orange";
    });

    QVariantMap infoMap = helper->createSidebarItemInfo(tagName);

    EXPECT_FALSE(infoMap.isEmpty());
    EXPECT_TRUE(infoMap.contains("Property_Key_Url"));
    EXPECT_TRUE(infoMap.contains("Property_Key_DisplayName"));
    EXPECT_EQ(infoMap["Property_Key_DisplayName"].toString(), tagName);
}

TEST_F(UT_TagHelper, showTagEdit)
{
    typedef void (*fptr)(TagEditor *, int x, int y);
    fptr p = (fptr)(&TagEditor::show);

    bool flag = false;
    stub.set_lamda(&TagManager::getTagsByUrls, []() { __DBG_STUB_INVOKE__ return QStringList(); });
    stub.set_lamda(p, [&flag]() {
        __DBG_STUB_INVOKE__
        flag = true;
    });

    helper->showTagEdit(QRectF(), QRectF(), QList<QUrl>() << QUrl("tag:/test"), true);
    EXPECT_TRUE(flag);
}

TEST_F(UT_TagHelper, crumbEditInputFilter_NullEdit)
{
    // Test crumb edit input filter with null pointer
    EXPECT_NO_THROW(helper->crumbEditInputFilter(nullptr));
}
