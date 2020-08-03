#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "view/watermaskframe.h"

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

