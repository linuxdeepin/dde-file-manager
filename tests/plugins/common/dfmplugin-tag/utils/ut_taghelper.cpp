// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "utils/taghelper.h"
#include "utils/tagmanager.h"
#include "widgets/tageditor.h"
#include "utils/filetagcache.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/utils/clipboard.h>

#include <gtest/gtest.h>

#include <DCrumbEdit>
#include <QPaintEvent>
#include <QPainter>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_tag;

class TagHelperTest : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        ins = TagHelper::instance();
    }
    virtual void TearDown() override
    {
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
    TagHelper *ins;
};

TEST_F(TagHelperTest, rootUrl)
{
    QUrl rootUrl;
    rootUrl.setScheme("tag");
    rootUrl.setPath("/");
    EXPECT_TRUE(ins->rootUrl() == rootUrl);
}

TEST_F(TagHelperTest, defualtColors)
{
    EXPECT_TRUE(!ins->defualtColors().isEmpty());
}

TEST_F(TagHelperTest, qureyColorByColorName)
{
    EXPECT_TRUE(ins->qureyColorByColorName(QString("Orange")) == QColor("#ffa503"));
    EXPECT_TRUE(ins->qureyColorByColorName(QString("Orange666")) == QColor());
}

TEST_F(TagHelperTest, qureyColorByDisplayName)
{
    EXPECT_TRUE(ins->qureyColorByDisplayName(QString("Orange")) == QColor("#ffa503"));
    EXPECT_TRUE(ins->qureyColorByDisplayName(QString("Orange666")) == QColor());
}

TEST_F(TagHelperTest, qureyColorNameByColor)
{
    EXPECT_TRUE(ins->qureyColorNameByColor(QColor("#ffa503")) == QString("Orange"));
    EXPECT_TRUE(ins->qureyColorNameByColor(QColor()) == QString());
}

TEST_F(TagHelperTest, qureyIconNameByColorName)
{
    EXPECT_TRUE(ins->qureyIconNameByColorName(QString("Orange")) == QString("dfm_tag_orange"));
    EXPECT_TRUE(ins->qureyIconNameByColorName(QString()) == QString());
}

TEST_F(TagHelperTest, qureyIconNameByColor)
{
    EXPECT_TRUE(ins->qureyIconNameByColor(QColor("#ffa503")) == QString("dfm_tag_orange"));
    EXPECT_TRUE(ins->qureyIconNameByColor(QColor()) == QString());
}

TEST_F(TagHelperTest, qureyDisplayNameByColor)
{
    EXPECT_TRUE(ins->qureyDisplayNameByColor(QColor("#ffa503")) == QString("Orange"));
    EXPECT_TRUE(ins->qureyDisplayNameByColor(QColor()) == QString());
}

TEST_F(TagHelperTest, qureyColorNameByDisplayName)
{
    EXPECT_TRUE(ins->qureyColorNameByDisplayName(QString("Orange")) == QString("Orange"));
    EXPECT_TRUE(ins->qureyColorNameByDisplayName(QString()) == QString());
}

TEST_F(TagHelperTest, getTagNameFromUrl)
{
    EXPECT_TRUE(ins->getTagNameFromUrl(QUrl("tag://test/test")) == QString("test"));
    EXPECT_TRUE(ins->getTagNameFromUrl(QUrl("file://test/test")) == QString());
}

TEST_F(TagHelperTest, makeTagUrlByTagName)
{
    EXPECT_TRUE(ins->makeTagUrlByTagName(QString("test")) == QUrl("tag:/test"));
}

TEST_F(TagHelperTest, getColorNameByTag)
{
    EXPECT_TRUE(ins->getColorNameByTag(QString("Orange")) == QString("Orange"));
    EXPECT_TRUE(ins->getColorNameByTag(QString()) != QString());
}

TEST_F(TagHelperTest, isDefualtTag)
{
    EXPECT_TRUE(ins->isDefualtTag(QString("Orange")));
    EXPECT_TRUE(!ins->isDefualtTag(QString()));
}

TEST_F(TagHelperTest, paintTags)
{
    QPainter painter;
    QRectF rect;
    QList<QColor> colors;
    bool flag = false;
    stub.set_lamda(&QPainter::drawPath, [&flag]() {
        __DBG_STUB_INVOKE__
        flag = true;
    });
    ins->paintTags(&painter, rect, colors << QColor("#ffa503"));
    EXPECT_TRUE(flag);
}

TEST_F(TagHelperTest, createSidebarItemInfo)
{
    stub.set_lamda(&TagManager::renameHandle, []() { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&TagManager::getTagIconName, []() { __DBG_STUB_INVOKE__ return QString(); });
    EXPECT_TRUE(!ins->createSidebarItemInfo(QString("Orange")).isEmpty());
}

TEST_F(TagHelperTest, showTagEdit)
{
    //virtual func
    typedef void (*fptr)(TagEditor *, int x, int y);
    fptr p = (fptr)(&TagEditor::show);

    bool flag = false;
    stub.set_lamda(&TagManager::getTagsByUrls, []() { __DBG_STUB_INVOKE__ return QStringList(); });
    stub.set_lamda(p, [&flag]() {
        __DBG_STUB_INVOKE__
        flag = true;
    });
    ins->showTagEdit(QRectF(), QRectF(), QList<QUrl>() << QUrl("tag:/test"), true);
    EXPECT_TRUE(flag);
}

TEST_F(TagHelperTest, crumbEditInputFilter)
{
    stub.set_lamda(&DTK_WIDGET_NAMESPACE::DCrumbEdit::toPlainText, []() {
        __DBG_STUB_INVOKE__
        return QString("This is a string with special characters: \\ / : ' * ? \" < > | % &");
    });

    bool flag = false;
    DTK_WIDGET_NAMESPACE::DCrumbEdit edit;
    stub.set_lamda(&TagManager::getTagsColor, []() {
        __DBG_STUB_INVOKE__
        QMap<QString, QColor> map;
        map["red"] = QColor("red");
        return map;
    });
    stub.set_lamda(&DTK_WIDGET_NAMESPACE::DCrumbEdit::setProperty, [&flag]() {
        __DBG_STUB_INVOKE__
        flag = true;
        return true;
    });
    ins->crumbEditInputFilter(&edit);
    EXPECT_TRUE(flag);
}
