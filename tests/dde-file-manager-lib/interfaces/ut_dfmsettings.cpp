/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhengyouge<zhengyouge@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "interfaces/dfmsettings.h"

#include <gtest/gtest.h>

DFM_USE_NAMESPACE

namespace  {
class TestDFMSettings : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestDFMSettings";
        setting = new DFMSettings("deepin/dde-file-manager", DFMSettings::GenericConfig);
    }

    void TearDown() override
    {
        std::cout << "end TestDFMSettings";
        delete setting;
        setting = nullptr;
    }

public:
    DFMSettings *setting;
};
}

TEST_F(TestDFMSettings, contains)
{
    EXPECT_TRUE(setting->contains("GenericAttribute", ""));
    EXPECT_TRUE(setting->contains("ApplicationAttribute", ""));

//    EXPECT_TRUE(setting->contains("GenericAttribute", "IndexSearch"));
    EXPECT_TRUE(setting->contains("GenericAttribute", "IndexInternal"));
}

TEST_F(TestDFMSettings, groups)
{
    EXPECT_TRUE(setting->groups().count() > 0);
}

TEST_F(TestDFMSettings, keys)
{
    EXPECT_TRUE(setting->keys("GenericAttribute").count() > 0);
}

TEST_F(TestDFMSettings, keyList)
{
    EXPECT_TRUE(setting->keyList("GenericAttribute").count() > 0);
}

TEST_F(TestDFMSettings, toUrlValue)
{
    DUrl url = DFMSettings::toUrlValue("/tmp");
    EXPECT_TRUE(url.isValid());
}

TEST_F(TestDFMSettings, value)
{
    EXPECT_TRUE(setting->value("GenericAttribute", "PreviewImage").toBool());
}

TEST_F(TestDFMSettings, urlValue)
{
    DUrl url = setting->urlValue("GenericAttribute", "PreviewImage");
    EXPECT_TRUE(url.isValid());
}

TEST_F(TestDFMSettings, setValue)
{
    setting->setValue("GenericAttribute", "PreviewImage", true);
}

TEST_F(TestDFMSettings, removeGroup)
{
    setting->removeGroup("11");
}

TEST_F(TestDFMSettings, isRemovable)
{
    EXPECT_TRUE(setting->isRemovable("GenericAttribute", "PreviewImage"));
}

TEST_F(TestDFMSettings, remove)
{
    setting->remove("GenericAttribute", "PreviewImage");
}

TEST_F(TestDFMSettings, clear)
{
    setting->clear();
}

TEST_F(TestDFMSettings, reload)
{
    setting->reload();
}

TEST_F(TestDFMSettings, sync)
{
    EXPECT_TRUE(setting->sync());
}

TEST_F(TestDFMSettings, autoSync)
{
    EXPECT_FALSE(setting->autoSync());
}

TEST_F(TestDFMSettings, watchChanges)
{
    EXPECT_FALSE(setting->watchChanges());
}

TEST_F(TestDFMSettings, setAutoSync)
{
    setting->setAutoSync(true);
}

TEST_F(TestDFMSettings, setWatchChanges)
{
    setting->setWatchChanges(true);
}
