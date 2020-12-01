#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#define protected public
#define private public
#include "view/watermaskframe.h"
#include "../third-party/cpp-stub/stub.h"
#include "config/config.h"

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
//stub
QVariant myConfig()
{
    return 0;
}
TEST(WaterMaskFrame, test_initui)
{

    WaterMaskFrame* wid = new WaterMaskFrame("/usr/share/deepin/dde-desktop-watermask.json");
    Stub tub, tub1;
    tub.set(ADDR(Config, getConfig), myConfig);
    wid->initUI();
    QJsonObject object;
    wid->m_configs = object;
    wid->initUI();
    delete wid;
}
