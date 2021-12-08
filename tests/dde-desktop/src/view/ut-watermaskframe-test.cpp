#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#define protected public
#define private public

#include "presenter/deepinlicensehelper.h"
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

    tub.set_lamda(ADDR(Config, getConfig), [](){return 0;});
    tub.set_lamda(ADDR(WaterMaskFrame, isNeedState), [](){return true;});
    bool done = false;
    tub.set_lamda(ADDR(WaterMaskFrame, onChangeAuthorizationLabel), [&wid, &done](WaterMaskFrame *obj, int){
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
    tub.set_lamda(ADDR(WaterMaskFrame, isNeedState), [](){return false;});
    bool getState = false;
    tub.set_lamda(ADDR(DeepinLicenseHelper, delayGetState), [&getState](){getState = true;});

    wid = new WaterMaskFrame("/usr/share/deepin/dde-desktop-watermask.json");
    tub.reset(ADDR(WaterMaskFrame, initUI));
    wid->initUI();

    QTest::qWaitFor([&getState](){return getState;}, 1000);
    EXPECT_FALSE(getState);
    EXPECT_TRUE(wid->m_textLabel->text().isEmpty());
    delete wid;
    wid = nullptr;
}

TEST(WaterMaskFrame, test_onChangeAuthorizationLabel)
{
    WaterMaskFrame* wid = new WaterMaskFrame("/usr/share/deepin/dde-desktop-watermask.json");

    int ret = Unauthorized;
    wid->onChangeAuthorizationLabel(ret);
    EXPECT_FALSE(wid->m_textLabel->text().isEmpty());

    ret = Authorized;
    wid->onChangeAuthorizationLabel(ret);
    EXPECT_TRUE(wid->m_textLabel->text().isEmpty());

    ret = AuthorizedLapse;
    wid->onChangeAuthorizationLabel(ret);
    EXPECT_FALSE(wid->m_textLabel->text().isEmpty());

    ret = TrialAuthorized;
    wid->onChangeAuthorizationLabel(ret);
    EXPECT_FALSE(wid->m_textLabel->text().isEmpty());

    ret = TrialExpired;
    wid->onChangeAuthorizationLabel(ret);
    EXPECT_FALSE(wid->m_textLabel->text().isEmpty());

}
