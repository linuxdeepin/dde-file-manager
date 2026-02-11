// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-tag/widgets/tagwidget.h"
#include "plugins/common/dfmplugin-tag/widgets/tagcrumbedit.h"
#include "plugins/common/dfmplugin-tag/widgets/tagcolorlistwidget.h"
#include "plugins/common/dfmplugin-tag/utils/tagmanager.h"
#include "plugins/common/dfmplugin-tag/utils/taghelper.h"

#include <gtest/gtest.h>

#include <QUrl>
#include <QColor>
#include <DCrumbEdit>

DWIDGET_USE_NAMESPACE
using namespace dfmplugin_tag;

class TagWidgetTest : public testing::Test
{

protected:
    virtual void SetUp() override
    {
        stub.set_lamda(&TagManager::getTagsColor, []() -> QMap<QString, QColor> {
            __DBG_STUB_INVOKE__
            QMap<QString, QColor> map;
            map["red"] = QColor("red");
            return map;
        });
        stub.set_lamda(&TagManager::getTagsByUrls, []() {
            __DBG_STUB_INVOKE__
            return QStringList();
        });
        stub.set_lamda(&TagManager::getAllTags, []() {
            __DBG_STUB_INVOKE__
            return QMap<QString, QColor>();
        });
        ins = new TagWidget(QUrl("file://hello/world"));
        ins->initialize();
    }
    virtual void TearDown() override
    {
        stub.clear();
        if (ins) {
            delete ins;
            ins = nullptr;
        }
    }

private:
    stub_ext::StubExt stub;
    TagWidget *ins;
};

TEST_F(TagWidgetTest, loadTags)
{
    stub.set_lamda(&TagColorListWidget::setCheckedColorList, [](TagColorListWidget *, const QList<QColor> &colorNames) {
        __DBG_STUB_INVOKE__
        qInfo() << "455555555" << colorNames;
        EXPECT_TRUE(colorNames.contains(QColor("red")));
    });
    stub.set_lamda(&TagHelper::isDefualtTag, []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    ins->loadTags(QUrl("file://hello/world"));
}

TEST_F(TagWidgetTest, shouldShow)
{
    EXPECT_TRUE(ins->shouldShow(QUrl("file://hello/world")));
}

TEST_F(TagWidgetTest, onCrumbListChanged)
{
    stub.set_lamda(&TagManager::setTagsForFiles, []() -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(&TagCrumbEdit::property, []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&TagWidget::loadTags, [](TagWidget *, const QUrl &url) {
        __DBG_STUB_INVOKE__
        EXPECT_TRUE(url.isValid());
    });
    ins->onCrumbListChanged();
}

TEST_F(TagWidgetTest, onCheckedColorChanged)
{
    stub.set_lamda(&TagWidget::loadTags, [](TagWidget *, const QUrl &url) {
        __DBG_STUB_INVOKE__
        EXPECT_TRUE(url.isValid());
    });
    stub.set_lamda(&TagColorListWidget::checkedColorList, [] {
        __DBG_STUB_INVOKE__
        return QList<QColor>();
    });
    ins->onCheckedColorChanged(QColor("red"));
}

TEST_F(TagWidgetTest, onTagChanged)
{
    stub.set_lamda(&TagWidget::loadTags, [](TagWidget *, const QUrl &url) {
        __DBG_STUB_INVOKE__
        EXPECT_TRUE(url.isValid());
    });
    QVariantMap map;
    map[QUrl("file://hello/world").path()] = "";
    ins->onTagChanged(map);
}

TEST_F(TagWidgetTest, updateCrumbsColor)
{
    auto func = static_cast<bool (TagCrumbEdit::*)(const DCrumbTextFormat &, int)>(&TagCrumbEdit::insertCrumb);
    stub.set_lamda(func, [](TagCrumbEdit *, const DCrumbTextFormat format, int) {
        __DBG_STUB_INVOKE__
        EXPECT_TRUE(format.text() == "red");
        return true;
    });
    ins->updateCrumbsColor(QMap<QString, QColor>());
    QMap<QString, QColor> map;
    map["red"] = QColor("red");
    ins->updateCrumbsColor(map);
}
