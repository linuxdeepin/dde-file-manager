// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "./private/autoactivatewindow.h"
#include "./private/autoactivatewindow_p.h"
#include <dfm-base/utils/windowutils.h>
#include "stubext.h"
#include <gtest/gtest.h>
#include <QWindow>
#include <QWidget>
#include <xcb/xcb.h>
#include <QTimer>
using namespace  ddplugin_wallpapersetting ;
class UT_AutoActivateWindowPrivate : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        window = new AutoActivateWindow();
        aa = window->d;
        aa->watchedWidget = new QWidget;
        char hostname[256];
        gethostname(hostname, sizeof(hostname));
        aa->x11Con = xcb_connect(hostname,nullptr);
    }
    virtual void TearDown() override
    {
        delete aa->watchedWidget;
        delete window;
        stub.clear();
    }
    AutoActivateWindow *window = nullptr;
    AutoActivateWindowPrivate *aa = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_AutoActivateWindowPrivate, watchOnWayland)
{

    bool call = false;
    auto fun_type = static_cast<QWindow*(QWidget::*)()const>(&QWidget::windowHandle);
    QWindow window;
    stub.set_lamda(fun_type,[&window](){
        __DBG_STUB_INVOKE__
        return &window;
    });

    auto fun_type1 = static_cast<bool(QWidget::*)()const>(&QWidget::isActiveWindow);
    stub.set_lamda(fun_type1,[](){
        __DBG_STUB_INVOKE__
        return false;
    });

    auto fun_type2 = static_cast<void(QWidget::*)()>(&QWidget::activateWindow);
    stub.set_lamda(fun_type2,[&call](){
        __DBG_STUB_INVOKE__
        call = true;
        return ;
    });
    aa->watchOnWayland(true);
    window.activeChanged();
    EXPECT_TRUE(call);
}

TEST_F(UT_AutoActivateWindowPrivate, watchOnX11)
{
    auto fun_type = static_cast<QWindow*(QWidget::*)()const>(&QWidget::windowHandle);
    QWindow window ;
    stub.set_lamda(fun_type,[&window](){
        __DBG_STUB_INVOKE__
        return &window;
    });

    stub.set_lamda(&AutoActivateWindowPrivate::initConnect,[](){
        __DBG_STUB_INVOKE__
        return true;
    });

    auto fun_type1 = static_cast<bool(QWidget::*)()const>(&QWidget::isActiveWindow);
    stub.set_lamda(fun_type1,[](){
        __DBG_STUB_INVOKE__
        return false;
    });

    auto fun_type2 = static_cast<void (QTimer::*)()>(&QTimer::start);

    bool call = false;
    stub.set_lamda(fun_type2,[&call](){
        __DBG_STUB_INVOKE__
        call = true;
    });

    aa->watchOnX11(true);
    window.activeChanged();
    EXPECT_TRUE(call);
}

TEST_F(UT_AutoActivateWindowPrivate, checkWindowOnX11)
{
    stub.set_lamda(&xcb_query_tree_reply,[](xcb_connection_t *c,
                   xcb_query_tree_cookie_t  cookie  ,
                   xcb_generic_error_t  **e){
        __DBG_STUB_INVOKE__
        xcb_query_tree_reply_t *reply = (xcb_query_tree_reply_t*)malloc(sizeof(xcb_query_tree_reply_t));
        *e = nullptr;
        return reply;
    });
    stub.set_lamda(&xcb_get_window_attributes_reply,[](xcb_connection_t  *c,
                   xcb_get_window_attributes_cookie_t   cookie  /**< */,
                   xcb_generic_error_t     **e){
        __DBG_STUB_INVOKE__
        xcb_get_window_attributes_reply_t *reply1 =
                       (xcb_get_window_attributes_reply_t*)malloc(sizeof(xcb_get_window_attributes_reply_t ));
        return reply1;
    });
    xcb_get_window_attributes_cookie_t cookie ;
    stub.set_lamda(& xcb_get_window_attributes,[&cookie](xcb_connection_t *c,
                   xcb_window_t      window){
        __DBG_STUB_INVOKE__

        return cookie;
    });

    stub.set_lamda(&xcb_query_tree_children_length ,[](const xcb_query_tree_reply_t *R){
        __DBG_STUB_INVOKE__
        return 1;
    });
    xcb_window_t res (1);
    stub.set_lamda(&xcb_query_tree_children,[&res](const xcb_query_tree_reply_t *R){
        __DBG_STUB_INVOKE__
        return &res;
    });

    bool call = false;
    stub.set_lamda(&QWidget::activateWindow,[&call](){
        __DBG_STUB_INVOKE__
        call = true;
        return ;
    });
    aa->watchedWin = (xcb_window_t)1;
    aa->checkWindowOnX11();

    EXPECT_TRUE(call);
}

TEST_F(UT_AutoActivateWindowPrivate, initConnect)
{
    aa->x11Con = nullptr;
    bool call = true;
    stub.set_lamda(&AutoActivateWindowPrivate::checkWindowOnX11,[&call](){
        __DBG_STUB_INVOKE__
        call = true;
        return;
    });

    aa->checkTimer.setInterval(1);
    aa->initConnect();
    aa->checkTimer.start();
    usleep(2000);
    EXPECT_TRUE(call);
}

class UT_AutoActivateWindow : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        aw = new AutoActivateWindow();

    }
    virtual void TearDown() override
    {
        delete aw;
        stub.clear();
    }
    AutoActivateWindow *aw = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_AutoActivateWindow, start)
{
     aw->d->run = false;
     QWidget widget;
     aw->d->watchedWidget = &widget;

     stub.set_lamda(&DFMBASE_NAMESPACE::WindowUtils::isWayLand,[](){
         __DBG_STUB_INVOKE__
         return true;
     });
     bool call = false;
     stub.set_lamda(&AutoActivateWindowPrivate::watchOnWayland,[&call](AutoActivateWindowPrivate*,bool){
         __DBG_STUB_INVOKE__
         call = true;
         return;
     });

     EXPECT_TRUE(aw->start());
     EXPECT_TRUE(call);
     EXPECT_TRUE(aw->d->run);

     aw->stop();
     EXPECT_FALSE(aw->d->run);
}

TEST_F(UT_AutoActivateWindow, setWatched)
{
    aw->d->run = false;
    QWidget win;
    aw->setWatched(&win);
    EXPECT_EQ(&win,aw->d->watchedWidget);
}

