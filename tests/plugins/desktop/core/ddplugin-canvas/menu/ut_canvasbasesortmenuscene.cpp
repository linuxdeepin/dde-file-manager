// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.

// SPDX-License-Identifier: GPL-3.0-or-later

#include "menu/canvasbasesortmenuscene.h"
#include "menu/canvasbasesortmenuscene_p.h"

#include <QMenu>
#include <mutex>

#include "stubext.h"
#include <gtest/gtest.h>

DDP_CANVAS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

class UT_CanvasBaseSortMenuScenePrivate : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        qq = new CanvasBaseSortMenuScene();
        canvas = new CanvasBaseSortMenuScenePrivate(qq);

    }
    virtual void TearDown() override
    {
        delete canvas;
        delete qq;
        stub.clear();
    }
    CanvasBaseSortMenuScene *qq = nullptr;
    CanvasBaseSortMenuScenePrivate *canvas = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_CanvasBaseSortMenuScenePrivate, sortPrimaryMenu)
{
    QString str1 = "separator-line";
    QString str2 = "temp_string2";
    QString str3 = "temp_string3";
    stub.set_lamda(&CanvasBaseSortMenuScenePrivate::primaryMenuRule,[str1,str2,str3](){
        __DBG_STUB_INVOKE__
        QStringList ret{str1,str2,str3};
        return ret;
    });
    QMenu menu;
    QAction action1 ;
    action1.setProperty("actionID",str1);
    QAction action2 ;
    action2.setProperty("actionID",str2);
    QAction action3 ;
    action3.setProperty("actionID",str3);
    menu.insertAction(nullptr,&action1);
    menu.insertAction(nullptr,&action3);
    menu.insertAction(nullptr,&action2);

    canvas->sortPrimaryMenu(&menu);

    int index1 = menu.actions().indexOf(&action1);
    EXPECT_EQ(index1,0);
    int index2 = menu.actions().indexOf(&action2);
    EXPECT_EQ(index2,2);
    int index3 = menu.actions().indexOf(&action3);
    EXPECT_EQ(index3,3);

}

TEST_F(UT_CanvasBaseSortMenuScenePrivate, sortSecondaryMenu)
{
    stub.set_lamda(&CanvasBaseSortMenuScenePrivate::secondaryMenuRule,[](){
        __DBG_STUB_INVOKE__
        QMap<QString, QStringList> ret;
        ret.insert("new-document",
                   QStringList { "new-office-text",
                                 "new-spreadsheets",
                                 "new-presentation",
                                  "new-plain-text" });
      return ret;
    });
    QMenu menu ;
    QAction action ;
    action.setProperty("actionID","new-document");
    QAction action1 ;
    action1.setProperty("actionID","new-office-text");
    QAction action2 ;
    action2.setProperty("actionID","new-plain-text");
    QAction action3 ;
    action3.setProperty("actionID","new-spreadsheets");
    menu.insertAction(nullptr,&action1);
    menu.insertAction(nullptr,&action3);
    menu.insertAction(nullptr,&action2);
    action.setMenu(&menu);

    QMenu firstMenu ;
    firstMenu.insertAction(nullptr,&action);

    canvas->sortSecondaryMenu(&firstMenu);

    int index1 = firstMenu.actions().at(0)->menu()->actions().indexOf(&action1);
    int index2 = firstMenu.actions().at(0)->menu()->actions().indexOf(&action2);
    int index3 = firstMenu.actions().at(0)->menu()->actions().indexOf(&action3);
    EXPECT_EQ(index1,0);
    EXPECT_EQ(index2,2);
    EXPECT_EQ(index3,1);
}

TEST_F(UT_CanvasBaseSortMenuScenePrivate, sendToRule)
{
    QStringList list = canvas->sendToRule();
    int index1 = list.indexOf("send-to-removable-");
    int index2 = list.indexOf("send-file-to-burnning-");
    EXPECT_EQ(index1,0);
    EXPECT_EQ(index2,1);
}

TEST_F(UT_CanvasBaseSortMenuScenePrivate, stageToRule)
{
    QStringList list = canvas->stageToRule();
    int index = list.indexOf("_stage-file-to-burning-");
    EXPECT_EQ(index,0);
}

TEST_F(UT_CanvasBaseSortMenuScenePrivate, primaryMenuRule)
{
    QStringList list = canvas->primaryMenuRule();
    EXPECT_EQ(list.size(),46);
}

TEST_F(UT_CanvasBaseSortMenuScenePrivate, secondaryMenuRule)
{
    stub.set_lamda(&CanvasBaseSortMenuScenePrivate::stageToRule,
                   [](CanvasBaseSortMenuScenePrivate*)->QStringList {
        __DBG_STUB_INVOKE__
       return QStringList{"stageToRule"};
    });
    stub.set_lamda(&CanvasBaseSortMenuScenePrivate::sendToRule,
                   [](CanvasBaseSortMenuScenePrivate*)->QStringList {
        __DBG_STUB_INVOKE__
        return QStringList{"sendToRule"};
    });
    QMap<QString, QStringList> res = canvas->secondaryMenuRule();

    QStringList list1{ "open-with-app","open-with-custom" };

    QStringList list2{ "new-office-text",
                       "new-spreadsheets",
                       "new-presentation",
                       "new-plain-text" };


    QStringList list3{ "display-as-icon",
                       "display-as-list" };
    QStringList list4{ "sort-by-name",
                       "sort-by-path",
                       "sort-by-source-path",
                       "sort-by-lastRead",
                       "sort-by-time-modified",
                       "sort-by-time-deleted",
                       "sort-by-size",
                       "sort-by-type" };
    QStringList list5{ "tiny",
                       "small",
                       "medium",
                       "large",
                       "super-large" };
    QStringList list6{"stageToRule"};
    QStringList list7{"create-system-link",
                      "send-to-desktop",
                      "sendToRule"};
    QStringList list8{"share-to-bluetooth"};
    EXPECT_EQ(res.size(),8);
    EXPECT_EQ(res.value("open-with"), list1);
    EXPECT_EQ(res.value("new-document"),list2);
    EXPECT_EQ(res.value("display-as"),list3);
    EXPECT_EQ(res.value("sort-by"),list4);
    EXPECT_EQ(res.value("icon-size"),list5);
    EXPECT_EQ(res.value("stage-file-to-burning"),list6);
    EXPECT_EQ(res.value("send-to"),list7);
    EXPECT_EQ(res.value("share"),list8);
}



TEST(CanvasBaseSortMenuScene, tset)
{
    stub_ext::StubExt stub;

    stub.set_lamda(&CanvasBaseSortMenuScenePrivate::sortPrimaryMenu,
                   [](CanvasBaseSortMenuScenePrivate*,QMenu *){__DBG_STUB_INVOKE__});
    stub.set_lamda(&CanvasBaseSortMenuScenePrivate::sortSecondaryMenu,
                   [](CanvasBaseSortMenuScenePrivate*,QMenu *){__DBG_STUB_INVOKE__});

    bool initileCall = false;
    typedef bool(*fun_type1)(const QHash<QString,QVariant>&);
    stub.set_lamda((fun_type1)(&AbstractMenuScene::initialize),
                   [&initileCall](const QVariantHash&)->bool{
        __DBG_STUB_INVOKE__
        initileCall = true;
        return true;
    });

    bool createCall = false;
    typedef bool(*fun_type2)(QMenu *);
    stub.set_lamda((fun_type2)(&AbstractMenuScene::create),
                   [&createCall](QMenu *)->bool{
        __DBG_STUB_INVOKE__
        createCall = true;
        return true;
    });

    bool updateCall = false;
    typedef void(*fun_type3)(QMenu *);
    stub.set_lamda((fun_type3)(&AbstractMenuScene::updateState),
                   [&updateCall](QMenu *){
        __DBG_STUB_INVOKE__
        updateCall = true;
    });

    bool triggeredCall = false;
    typedef bool(*fun_type4)(QAction *);
    stub.set_lamda((fun_type4)(&AbstractMenuScene::triggered),
                   [&triggeredCall](QAction *){
        __DBG_STUB_INVOKE__
        triggeredCall = true;
        return false;
    });

    bool sceneCall = false;
    typedef AbstractMenuScene*(*fun_type5)(QAction *);
    stub.set_lamda((fun_type5)(&AbstractMenuScene::scene),
                   [&sceneCall](QAction *){
        __DBG_STUB_INVOKE__
        sceneCall = true;
        return nullptr;
    });

    CanvasBaseSortMenuScene canvas;

    QString name = canvas.name();
    EXPECT_EQ(name,"CanvasBaseSortMenu");

    QVariantHash params;
    canvas.initialize(params);
    EXPECT_TRUE(initileCall);

    canvas.create(nullptr);
    EXPECT_TRUE(createCall);

    canvas.updateState(nullptr);
    EXPECT_TRUE(createCall);

    canvas.triggered(nullptr);
    EXPECT_TRUE(triggeredCall);

    canvas.scene(nullptr);
    EXPECT_TRUE(sceneCall);
}
