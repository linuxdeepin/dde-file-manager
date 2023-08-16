// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "backgroundmanager.h"
#include "desktoputils/ddpugin_eventinterface_helper.h"
#include "backgroundmanager_p.h"

#include <dfm-base/dfm_desktop_defines.h>
#include <dfm-framework/dpf.h>

#include "stubext.h"
#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
DDP_BACKGROUND_USE_NAMESPACE
DPF_USE_NAMESPACE

class UT_backGroundManager : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        bgm = new BackgroundManager;
    }
    virtual void TearDown() override
    {
        delete bgm;
        stub.clear();
    }
    BackgroundManager *bgm = nullptr;
    stub_ext::StubExt stub;
};

TEST(BackgroundManager, init)
{
    BackgroundManager *bgm = new BackgroundManager;
    stub_ext::StubExt stub;

    bool call = false;
    stub.set_lamda(&BackgroundManager::restBackgroundManager, [&call]() {
        call = true;
    });

    const QStringList allSig = {
        "signal_DesktopFrame_WindowAboutToBeBuilded",
        "signal_DesktopFrame_WindowBuilded",
        "signal_DesktopFrame_GeometryChanged",
    };

    for (auto sig : allSig) {
        if (auto ptr = dpfSignalDispatcher->dispatcherMap.value(
                    EventConverter::convert("ddplugin_core", sig))) {
            __DBG_STUB_INVOKE__
            ASSERT_TRUE(ptr->handlerList.isEmpty());
        }
    }

    bgm->init();

    EXPECT_TRUE(call);
    for (auto sig : allSig) {
        if (auto ptr = dpfSignalDispatcher->dispatcherMap.value(
                    EventConverter::convert("ddplugin_core", sig))) {
            __DBG_STUB_INVOKE__
            EXPECT_FALSE(ptr->handlerList.isEmpty());
        }
    }

    delete bgm;

    for (auto sig : allSig) {
        if (auto ptr = dpfSignalDispatcher->dispatcherMap.value(
                    EventConverter::convert("ddplugin_core", sig))) {
            __DBG_STUB_INVOKE__
            EXPECT_TRUE(ptr->handlerList.isEmpty());
        }
    }
}

TEST_F(UT_backGroundManager, onBackgroundChanged)
{
    bool call = false;

    stub.set_lamda(&BackgroundBridge::isRunning, []() {
        return true;
    });

    bgm->onBackgroundChanged();
    EXPECT_FALSE(call);
    EXPECT_TRUE(bgm->d->bridge->repeat);

    stub.set_lamda(&BackgroundBridge::isRunning, []() {
        return false;
    });
    stub.set_lamda(&BackgroundBridge::request, [&call]() {
        call = true;
    });
    bgm->onBackgroundChanged();
    EXPECT_TRUE(call);
}

TEST_F(UT_backGroundManager, onGeometryChanged)
{
    QWidget *widget1 = new QWidget;
    widget1->setGeometry(0, 0, 1920, 1920);
    widget1->setProperty(DesktopFrameProperty::kPropScreenName, "testWidget1");

    QWidget *widget2 = new QWidget;
    widget2->setGeometry(1920, 0, 1920, 1920);
    widget2->setProperty(DesktopFrameProperty::kPropScreenName, "testWidget2");
    QList<QWidget *> rets { widget1, widget2 };
    stub.set_lamda(((QVariant(EventChannelManager::*)(const QString &, const QString &))
                    & EventChannelManager::push),
                   [&rets](EventChannelManager *, const QString &t1, const QString &t2) {
                       __DBG_STUB_INVOKE__
                       if (t1 == "ddplugin_core") {
                           if (t2 == "slot_DesktopFrame_RootWindows") {
                               return QVariant::fromValue(rets);
                           }
                       }

                       return QVariant();
                   });
    bool call = false;
    stub.set_lamda(&BackgroundBridge::request, [&call]() {
        call = true;
        return;
    });

    BackgroundWidgetPointer bwp1(new BackgroundDefault("testWidget1"));
    bwp1->setGeometry(QRect(0, 0, 1080, 1080));
    bgm->d->backgroundWidgets.insert(QString("testWidget1"), bwp1);
    BackgroundWidgetPointer bwp2(new BackgroundDefault("testWidget2"));
    bwp2->setGeometry(QRect(1920, 0, 1080, 1080));
    bgm->d->backgroundWidgets.insert(QString("testWidget2"), bwp2);

    bgm->onGeometryChanged();
    EXPECT_TRUE(call);
    EXPECT_EQ(bgm->d->backgroundWidgets.value("testWidget2")->geometry(), QRect(0, 0, 1920, 1920));
    delete widget1;
    delete widget2;
}

TEST_F(UT_backGroundManager, restBackgroundManager)
{
    BackgroundWidgetPointer testPointer(new BackgroundDefault("test"));
    bgm->d->backgroundWidgets.insert(QString("test"), testPointer);
    bgm->d->backgroundPaths.insert(QString("test"), QString(":/test11"));

    EXPECT_FALSE(bgm->d->backgroundWidgets.isEmpty());
    EXPECT_FALSE(bgm->d->backgroundPaths.isEmpty());

    bool callTerminate = false;
    stub.set_lamda(&BackgroundManagerPrivate::isEnableBackground, []() {
        return false;
    });
    stub.set_lamda(&BackgroundBridge::terminate, [&callTerminate]() {
        callTerminate = true;
    });
    bgm->restBackgroundManager();

    EXPECT_TRUE(callTerminate);
    EXPECT_TRUE(bgm->d->backgroundWidgets.isEmpty());
    EXPECT_TRUE(bgm->d->backgroundPaths.isEmpty());

    bool callrequest = false;
    auto isEmptyFuncPtr = static_cast<bool (QList<QWidget *>::*)() const>(&QList<QWidget *>::isEmpty);
    stub.set_lamda(&BackgroundManagerPrivate::isEnableBackground, []() {
        return true;
    });
    stub.set_lamda(isEmptyFuncPtr, []() {
        return true;
    });
    stub.set_lamda(&BackgroundBridge::forceRequest, [&callrequest]() {
        callrequest = true;
        return;
    });
    bool callBack = false;
    bgm->restBackgroundManager();
    EXPECT_FALSE(callBack);
    EXPECT_TRUE(callrequest);

    callrequest = false;
    stub.set_lamda(isEmptyFuncPtr, []() {
        return false;
    });
    stub.set_lamda(&BackgroundManager::onBackgroundBuild, [&callBack]() {
        callBack = true;
        return;
    });
    bgm->restBackgroundManager();
    EXPECT_TRUE(callBack);
    EXPECT_FALSE(callrequest);
}

TEST_F(UT_backGroundManager, createBackgroundWidget)
{
    QWidget *widget = new QWidget;
    widget->setGeometry(0, 0, 100, 100);
    widget->setProperty(DesktopFrameProperty::kPropScreenName, "test");
    stub.set_lamda(&BackgroundManagerPrivate::relativeGeometry, [widget]() {
        return QRect(QPoint(0, 0), widget->geometry().size());
    });

    BackgroundWidgetPointer res = bgm->createBackgroundWidget(widget);

    EXPECT_EQ(res->property(DesktopFrameProperty::kPropScreenName), QVariant("test"));
    EXPECT_EQ(res->property(DesktopFrameProperty::kPropWidgetName), QVariant("background"));
    EXPECT_EQ(res->property(DesktopFrameProperty::kPropWidgetLevel), QVariant(5.0));
}

TEST_F(UT_backGroundManager, onBackgroundBuild)
{
    stub.set_lamda(&BackgroundBridge::request, []() {
        return;
    });
    QWidget *widget = new QWidget;
    widget->setProperty(DesktopFrameProperty::kPropScreenName, "test");
    QList<QWidget *> rets { widget };
    stub.set_lamda(((QVariant(EventChannelManager::*)(const QString &, const QString &))
                    & EventChannelManager::push),
                   [&rets](EventChannelManager *, const QString &t1, const QString &t2) {
                       __DBG_STUB_INVOKE__
                       if (t1 == "ddplugin_core") {
                           if (t2 == "slot_DesktopFrame_RootWindows") {
                               return QVariant::fromValue(rets);
                           }
                       }

                       return QVariant();
                   });
    bgm->onBackgroundBuild();
    EXPECT_FALSE(bgm->d->backgroundWidgets.value("test").isNull());

    QWidget *widget2 = new QWidget;
    widget2->setProperty(DesktopFrameProperty::kPropScreenName, "test2");
    rets.append(widget2);
    bgm->onBackgroundBuild();
    EXPECT_EQ(bgm->d->backgroundWidgets.size(), 2);
    EXPECT_FALSE(bgm->d->backgroundWidgets.value("test2").isNull());
}

TEST_F(UT_backGroundManager, request)
{
    stub.set_lamda(&QFuture<void>::isRunning, []() {
        return true;
    });
    bool callTerminate = false;
    stub.set_lamda(&BackgroundBridge::terminate, [&callTerminate]() {
        callTerminate = true;
        return;
    });
    bgm->d->bridge->force = true;
    bgm->d->bridge->request(true);
    EXPECT_TRUE(callTerminate);

    bgm->d->bridge->force = false;
    QWidget *widget = new QWidget;
    widget->setGeometry(0, 0, 100, 100);
    widget->setProperty(DesktopFrameProperty::kPropScreenName, "test");
    QList<QWidget *> rets { widget };
    stub.set_lamda(((QVariant(EventChannelManager::*)(const QString &, const QString &))
                    & EventChannelManager::push),
                   [&rets](EventChannelManager *, const QString &t1, const QString &t2) {
                       if (t1 == "ddplugin_core") {
                           if (t2 == "slot_DesktopFrame_RootWindows") {
                               return QVariant::fromValue(rets);
                           }
                       }

                       return QVariant();
                   });

    BackgroundBridge::Requestion req;
    bool callRun = false;
    stub.set_lamda(&BackgroundBridge::runUpdate, [&req, &callRun](BackgroundBridge *self, QList<BackgroundBridge::Requestion> reqs) {
        __DBG_STUB_INVOKE__
        req = reqs.first();
        return;
    });
    {
        bgm->d->backgroundPaths.insert("test", ":/test");

        bgm->d->bridge->request(false);
        usleep(1000);
        EXPECT_EQ(req.screen, QString("test"));
        EXPECT_EQ(req.path, QString(":/test"));

        bgm->d->bridge->request(true);
        usleep(1000);
        EXPECT_EQ(req.screen, QString("test"));
        EXPECT_EQ(req.path, QString(""));
    }
    {
        rets.clear();
        bgm->d->bridge->request(true);
        EXPECT_FALSE(callRun);
    }
    delete widget;
}

class TestNullScreen : public AbstractScreen
{
public:
    virtual QString name() const { return scname; }
    QRect geometry() const { return QRect(0, 0, 200, 200); }
    QRect availableGeometry() const { return QRect(0, 0, 200, 180); }
    QRect handleGeometry() const { return QRect(0, 0, 250, 250); }
    QString scname;
};

TEST_F(UT_backGroundManager, forceRequest)
{
    stub.set_lamda(&BackgroundBridge::terminate, []() {
        return;
    });

    TestNullScreen *tmp = new TestNullScreen;
    tmp->scname = "test";
    ScreenPointer sc1(tmp);
    QList<DFMBASE_NAMESPACE::ScreenPointer> rets { sc1 };
    stub.set_lamda(((QVariant(EventChannelManager::*)(const QString &, const QString &))
                    & EventChannelManager::push),
                   [&rets](EventChannelManager *, const QString &t1, const QString &t2) {
                       if (t1 == "ddplugin_core") {
                           if (t2 == "slot_ScreenProxy_Screens") {
                               return QVariant::fromValue(rets);
                           }
                       }
                       return QVariant();
                   });
    BackgroundBridge::Requestion req;
    bool call = false;
    stub.set_lamda(&BackgroundBridge::runUpdate, [&req, &call](BackgroundBridge *self, QList<BackgroundBridge::Requestion> reqs) {
        __DBG_STUB_INVOKE__
        call = true;
        req = reqs.first();
        return;
    });

    bgm->d->bridge->forceRequest();
    usleep(1000);
    EXPECT_EQ(req.screen, QString("test"));
    EXPECT_EQ(req.size, sc1->handleGeometry().size());

    rets.clear();
    call = false;
    bgm->d->bridge->forceRequest();
    EXPECT_FALSE(call);
}

TEST_F(UT_backGroundManager, terminate)
{
    bgm->d->bridge->terminate(false);
    EXPECT_FALSE(bgm->d->bridge->getting);
    EXPECT_FALSE(bgm->d->bridge->force);
}
