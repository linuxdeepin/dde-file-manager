#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#define protected public
#define private public

#include "view/watermaskframe.h"
#include "stubext.h"
#include "config/config.h"
#include "dbus/licenceInterface.h"
#include <QLayout>
#include <QTest>

using namespace testing;

TEST(WaterMaskFrame,check_config_file_not_existed)
{
    WaterMaskFrame wid("");
    EXPECT_EQ(false,wid.checkConfigFile(QString("/tmp/%0/xx.xx").arg(QUuid::createUuid().toString())));
}

TEST(WaterMaskFrame,check_config_file_empty)
{
    WaterMaskFrame wid("");
    EXPECT_EQ(false,wid.checkConfigFile(""));
}

TEST(WaterMaskFrame,check_config_file_existed)
{
    WaterMaskFrame wid("/usr/share/deepin/dde-desktop-watermask.json");
    const QString dirPath("/tmp/ut-dde-desktop/watermask");
    QDir dir(dirPath);
    dir.mkpath(dirPath);
    const QString filePath(dir.filePath("test.xxx"));
    QSaveFile file(filePath);
    file.open(QSaveFile::WriteOnly);
    file.commit();

    EXPECT_EQ(true,wid.checkConfigFile(filePath));
}

TEST(WaterMaskFrame, test_updateAuthorizationState)
{
    WaterMaskFrame wid("/usr/share/deepin/dde-desktop-watermask.json");
    wid.updateAuthorizationState();
}

TEST(WaterMaskFrame, test_parseJson)
{
    bool ret;
    WaterMaskFrame wid("/usr/share/deepin/dde-desktop-watermask.json");
    ret = wid.parseJson("1");
    EXPECT_FALSE(ret);
    wid.m_configs.insert("1", QJsonValue("test"));
    ret = wid.parseJson("1");
    EXPECT_TRUE(ret);
}

TEST(WaterMaskFrame, test_initui)
{
    WaterMaskFrame* wid1 = new WaterMaskFrame("/home/xxxxxxx");
    ASSERT_EQ(wid1->m_mainLayout, nullptr);
    delete wid1;
    WaterMaskFrame* wid2 = new WaterMaskFrame("/usr/share/deepin/dde-desktop-watermask.json");
    ASSERT_NE(wid2->m_mainLayout, nullptr);
    delete wid2;
}

TEST(WaterMaskFrame, test_initui_extend)
{
    stub_ext::StubExt tub;
    tub.set_lamda(ADDR(WaterMaskFrame, initUI), [](){});

    WaterMaskFrame* wid = new WaterMaskFrame("/usr/share/deepin/dde-desktop-watermask.json");
    wid->m_licenseProp->setTimeout(4000);

    tub.set_lamda(ADDR(Config, getConfig), [](){return 0;});
    tub.set_lamda(ADDR(WaterMaskFrame, isNeedState), [](){return true;});
    bool done = false;
    tub.set_lamda(ADDR(WaterMaskFrame, onActiveStateFinished), [&wid, &done](WaterMaskFrame *obj){
        if (obj == wid)
            done = true;
    });
    tub.reset(ADDR(WaterMaskFrame, initUI));
    wid->initUI();

    QTest::qWaitFor([&done](){return done;}, 3000);
    EXPECT_TRUE(done);
    EXPECT_TRUE(wid->m_textLabel->text().isEmpty());
    delete wid;
    wid = nullptr;

    tub.set_lamda(ADDR(WaterMaskFrame, initUI), [](){});
    wid = new WaterMaskFrame("/usr/share/deepin/dde-desktop-watermask.json");
    wid->m_licenseProp->setTimeout(500);
    tub.set_lamda(ADDR(WaterMaskFrame, isNeedState), [](){return false;});
    done = false;
    tub.set_lamda(ADDR(WaterMaskFrame, onActiveStateFinished), [wid, &done](WaterMaskFrame *obj){
        if (obj == wid)
            done = true;
    });
    tub.reset(ADDR(WaterMaskFrame, initUI));
    wid->initUI();
    QTest::qWaitFor([&done](){return done;}, 1000);
    EXPECT_FALSE(done);
    EXPECT_TRUE(wid->m_textLabel->text().isEmpty());
    delete wid;
    wid = nullptr;
}

TEST(WaterMaskFrame, test_onActiveStateFinished)
{
    WaterMaskFrame* wid = new WaterMaskFrame("/usr/share/deepin/dde-desktop-watermask.json");
    {
        stub_ext::StubExt stub;
        stub.set_lamda(VADDR(WaterMaskFrame,sender),[](){return nullptr;});
        EXPECT_NO_FATAL_FAILURE(wid->onActiveStateFinished());
        EXPECT_TRUE(wid->m_textLabel->text().isEmpty());
    }

    {
        stub_ext::StubExt stub;
        QDBusPendingCall call = QDBusPendingCall::fromError(QDBusError(QDBusError::NoReply,""));
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call);
        stub.set_lamda(VADDR(WaterMaskFrame, sender),[watcher](){return watcher;});
        EXPECT_NO_FATAL_FAILURE(wid->onActiveStateFinished());
        EXPECT_TRUE(wid->m_textLabel->text().isEmpty());
        delete watcher;
    }

    {
        stub_ext::StubExt stub;
        QDBusPendingCall call = QDBusPendingCall::fromCompletedCall(QDBusMessage());
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call);
        stub.set_lamda(ADDR(QDBusPendingCall, error),[](){return QDBusError(QDBusError::NoError,"");});
        stub.set_lamda(VADDR(WaterMaskFrame, sender),[watcher](){return watcher;});

        int ret = Unauthorized;
        stub.set_lamda(ADDR(QDBusPendingReplyData, argumentAt),[&ret](){
            return QVariant::fromValue(QDBusVariant(QVariant::fromValue(ret)));
        });

        EXPECT_NO_FATAL_FAILURE(wid->onActiveStateFinished());
        EXPECT_FALSE(wid->m_textLabel->text().isEmpty());

        ret = Authorized;
        EXPECT_NO_FATAL_FAILURE(wid->onActiveStateFinished());
        EXPECT_TRUE(wid->m_textLabel->text().isEmpty());

        ret = AuthorizedLapse;
        EXPECT_NO_FATAL_FAILURE(wid->onActiveStateFinished());
        EXPECT_FALSE(wid->m_textLabel->text().isEmpty());

        ret = TrialAuthorized;
        EXPECT_NO_FATAL_FAILURE(wid->onActiveStateFinished());
        EXPECT_FALSE(wid->m_textLabel->text().isEmpty());

        ret = TrialExpired;
        EXPECT_NO_FATAL_FAILURE(wid->onActiveStateFinished());
        EXPECT_FALSE(wid->m_textLabel->text().isEmpty());

        delete watcher;
    }
}
