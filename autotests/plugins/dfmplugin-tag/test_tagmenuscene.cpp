// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "menu/private/tagmenuscene_p.h"
#include "menu/tagmenuscene.h"
#include "utils/taghelper.h"
#include "utils/tagmanager.h"
#include "widgets/tagcolorlistwidget.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/base/schemefactory.h>

#include <gtest/gtest.h>

#include <QMenu>
#include <QPaintEvent>
#include <QWidgetAction>
#include <QPainter>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_tag;

class TagMenuSceneTest : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        auto creator = new TagMenuCreator();
        scene = static_cast<dfmplugin_tag::TagMenuScene *>(creator->create());
        d = scene->d.data();
    }
    virtual void TearDown() override
    {
        stub.clear();
        delete scene;
    }

protected:
    stub_ext::StubExt stub;
    TagMenuScene *scene { nullptr };
    TagMenuScenePrivate *d { nullptr };
};

TEST_F(TagMenuSceneTest, name)
{
    EXPECT_TRUE(scene->name() == "TagMenu");
}

TEST_F(TagMenuSceneTest, initialize)
{
    FileInfoPointer info(new FileInfo(QUrl("file:///test")));
    stub.set_lamda(&InfoFactory::create<FileInfo>, [info](const QUrl &url, const Global::CreateFileInfoType type, QString *errorString) {
        return info;
    });
    QList<QUrl> urls { QUrl::fromLocalFile("/hello/world"),
                       QUrl::fromLocalFile("/i/can/eat/glass/without/hurt") };
    EXPECT_TRUE(scene->initialize({ { "currentDir", true } }));
    EXPECT_TRUE(scene->initialize(
            { { "selectFiles", QVariant::fromValue<QList<QUrl>>(urls) } }));
    EXPECT_TRUE(scene->initialize({ { "onDesktop", true } }));
    EXPECT_EQ(d->onDesktop, true);
    EXPECT_TRUE(scene->initialize({ { "isEmptyArea", true } }));
    EXPECT_TRUE(scene->initialize({ { "indexFlags", true } }));
}

TEST_F(TagMenuSceneTest, create)
{
    EXPECT_FALSE(scene->create(nullptr));

    QMenu menu;
    d->focusFile = QUrl("file:///test");
    d->focusFileInfo.reset(new FileInfo(d->focusFile));
    stub.set_lamda(&TagManager::getTagsByUrls, [] {
        __DBG_STUB_INVOKE__
        return QList<QString>() << "red";
    });
    stub.set_lamda(VADDR(FileInfo, exists), [] { __DBG_STUB_INVOKE__ return true; });
    auto func = static_cast<bool (TagManager::*)(const QUrl &) const>(&TagManager::canTagFile);
    stub.set_lamda(func, []() -> bool { __DBG_STUB_INVOKE__ return true; });
    EXPECT_TRUE(scene->create(&menu));
}

TEST_F(TagMenuSceneTest, triggered)
{
    auto act = new QAction("hello");
    stub.set_lamda(&TagHelper::showTagEdit, [] { __DBG_STUB_INVOKE__ });
    act->setProperty(ActionPropertyKey::kActionID, "hello");
    scene->d->predicateAction.insert(TagActionId::kActTagAddKey, act);
    scene->d->focusFile.setUrl("/hello");

    FileInfoPointer info(new FileInfo(QUrl("file:///test")));
    stub.set_lamda(&InfoFactory::create<FileInfo>, [info](const QUrl &url, const Global::CreateFileInfoType type, QString *errorString) {
        return info;
    });

    EXPECT_TRUE(scene->initialize({ { "onDesktop", true }, { "onCollection", true } }));
    EXPECT_FALSE(scene->triggered(act));
    EXPECT_TRUE(scene->initialize({ { "onDesktop", true }, { "onCollection", false } }));
    EXPECT_FALSE(scene->triggered(act));
    EXPECT_TRUE(scene->initialize({ { "onDesktop", true } }));
    EXPECT_FALSE(scene->triggered(act));
    EXPECT_TRUE(scene->initialize({ { "onDesktop", false } }));
    EXPECT_FALSE(scene->triggered(act));
}

TEST_F(TagMenuSceneTest, scene)
{
    EXPECT_EQ(nullptr, scene->scene(nullptr));

    auto act = new QAction("hello");
    d->predicateAction.insert("hello", act);
    EXPECT_STREQ(scene->scene(act)->metaObject()->className(),
                 "dfmplugin_tag::TagMenuScene");

    d->predicateAction.clear();
    scene->scene(act);
    delete act;
    act = nullptr;
}

TEST_F(TagMenuSceneTest, onHoverChanged)
{
    scene->d->selectFiles << QUrl::fromLocalFile("/test");
    stub.set_lamda(&TagManager::getTagsByUrls,
                   []() { __DBG_STUB_INVOKE__ return QStringList(); });
    bool flag = false;
    TagColorListWidget *tagWidget = new TagColorListWidget();
    stub.set_lamda(&TagMenuScene::getMenuListWidget, [tagWidget]() {
        __DBG_STUB_INVOKE__
        return tagWidget;
    });
    stub.set_lamda(&TagColorListWidget::setToolTipText,
                   [&flag]() { __DBG_STUB_INVOKE__ flag = true; });
    stub.set_lamda(&TagManager::getTagsColor, []() {
        __DBG_STUB_INVOKE__
        QMap<QString, QColor> map;
        map["red"] = QColor("red");
        return map;
    });
    scene->onHoverChanged(QColor("red"));
    EXPECT_TRUE(flag);
    delete tagWidget;
    tagWidget = nullptr;
}

TEST_F(TagMenuSceneTest, onColorClicked)
{
    TagColorListWidget *tagWidget = new TagColorListWidget();
    stub.set_lamda(&TagMenuScene::getMenuListWidget, [tagWidget]() {
        __DBG_STUB_INVOKE__
        return tagWidget;
    });

    bool flag = false;
    stub.set_lamda(&TagManager::addTagsForFiles, [&flag]() {
        __DBG_STUB_INVOKE__ flag = true;
        return true;
    });
    stub.set_lamda(&TagManager::removeTagsOfFiles, [&flag]() {
        __DBG_STUB_INVOKE__ flag = true;
        return true;
    });
    scene->onColorClicked(QColor("red"));
    EXPECT_TRUE(flag);
    delete tagWidget;
    tagWidget = nullptr;
}

TEST_F(TagMenuSceneTest, getSurfaceRect)
{
    QWidget *w = new QWidget();
    QWidget w2;
    w2.setFixedSize(10, 10);
    w2.setProperty("WidgetName", QString("organizersurface"));
    w->setParent(&w2);
    EXPECT_TRUE(d->getSurfaceRect(w) == w2.rect());
}

TEST_F(TagMenuSceneTest, getMenuListWidget)
{
    bool isRun = false;
    d->predicateAction.insert(TagActionId::kActTagColorListKey, new QWidgetAction(scene));
    stub.set_lamda(&QWidgetAction::defaultWidget, [&isRun]() {
        __DBG_STUB_INVOKE__
        isRun = true;
        return nullptr;
    });
    scene->getMenuListWidget();
    EXPECT_TRUE(isRun);
}

TEST_F(TagMenuSceneTest, createColorListAction)
{
    QMenu m;
    bool isRun = false;
    stub.set_lamda(&TagManager::getTagsByUrls, [&isRun]() {
        isRun = true;
        return QStringList() << "red";
    });
    stub.set_lamda(&TagHelper::isDefualtTag, []() { return true; });
    stub.set_lamda(&TagHelper::qureyColorByDisplayName, []() { return QColor("red"); });
    scene->createColorListAction(&m);
    EXPECT_TRUE(isRun);
}

TEST_F(TagMenuSceneTest, updateState)
{
    QMenu m;
    bool isRun = false;
    stub.set_lamda(&QMenu::removeAction, [&isRun]() { isRun = true; });
    scene->updateState(&m);
    EXPECT_TRUE(isRun);
}
