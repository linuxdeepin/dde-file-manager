/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#include "stubext.h"
#include "dfm-base/dialogs/settingsdialog/settingdialog.h"

#include <QSet>
#include <QFile>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE

class UT_SettingsDialog : public testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

private:
    stub_ext::StubExt stub;
};

TEST_F(UT_SettingsDialog, SetItemVisiable)
{
    auto &&set = "a.b.c";
    EXPECT_NO_FATAL_FAILURE(SettingDialog::setItemVisiable(set, false));
    EXPECT_TRUE(SettingDialog::kHiddenSettingItems.contains(set));
    EXPECT_NO_FATAL_FAILURE(SettingDialog::setItemVisiable(set, true));
    EXPECT_FALSE(SettingDialog::kHiddenSettingItems.contains(set));
}
// TEST_F(UT_SettingsDialog, CreateAutoMountCheckBox){}
// TEST_F(UT_SettingsDialog, CreateAutoMountOpenCheckBox){}
// TEST_F(UT_SettingsDialog, CreateSplitter){}
// TEST_F(UT_SettingsDialog, MountCheckBoxStateChangedHandle){}
// TEST_F(UT_SettingsDialog, AutoMountCheckBoxChangedHandle){}
TEST_F(UT_SettingsDialog, SettingFilter)
{
    QByteArray data = "";
    SettingDialog dlg;
    EXPECT_NO_FATAL_FAILURE(dlg.settingFilter(data));

    data = R"({ "key": "value" })";
    EXPECT_NO_FATAL_FAILURE(dlg.settingFilter(data));

    data = R"({
                "group": [
                    {
                        "key": "base",
                        "name": "test",
                        "groups": [
                            {
                                "key": "open_action",
                                "name": "Open",
                                "options": [
                                    {
                                        "key": "allways_open_on_new_window",
                                        "type": "checkbox",
                                        "text": qsTranslate("GenerateSettingTranslate", "Always open folder in new window"),
                                        "default": false
                                    },
                                    {
                                        "key": "open_file_action",
                                        "name": qsTranslate("GenerateSettingTranslate", "Open file:"),
                                        "type": "combobox",
                                        "items": [
                                            qsTranslate("GenerateSettingTranslate", "Click"),
                                            qsTranslate("GenerateSettingTranslate", "Double click")
                                        ],
                                        "default": 1
                                    }
                                ]
                            }
                        ]
                    }
                ]
            })";

    EXPECT_NO_FATAL_FAILURE(SettingDialog::setItemVisiable("base.open_action.open_file_action", false));
    EXPECT_NO_FATAL_FAILURE(dlg.settingFilter(data));
}

TEST_F(UT_SettingsDialog, LoadSettings)
{
    QByteArray data = R"({
                "group": [
                    {
                        "key": "base",
                        "name": "test",
                        "groups": [
                            {
                                "key": "open_action",
                                "name": "Open",
                                "options": [
                                    {
                                        "key": "allways_open_on_new_window",
                                        "type": "checkbox",
                                        "text": qsTranslate("GenerateSettingTranslate", "Always open folder in new window"),
                                        "default": false
                                    },
                                    {
                                        "key": "open_file_action",
                                        "name": qsTranslate("GenerateSettingTranslate", "Open file:"),
                                        "type": "combobox",
                                        "items": [
                                            qsTranslate("GenerateSettingTranslate", "Click"),
                                            qsTranslate("GenerateSettingTranslate", "Double click")
                                        ],
                                        "default": 1
                                    }
                                ]
                            }
                        ]
                    }
                ]
            })";

    QFile tmp("/tmp/test_tmplate.json");
    EXPECT_TRUE(tmp.open(QIODevice::WriteOnly | QIODevice::Truncate));
    tmp.write(data);
    EXPECT_NO_FATAL_FAILURE(tmp.close());

    SettingDialog dlg;
    EXPECT_NO_FATAL_FAILURE(dlg.loadSettings("/tmp/test_tmplate.json"));
}

TEST_F(UT_SettingsDialog, NeedHide)
{
    SettingDialog::kHiddenSettingItems.clear();
    auto &&set = "a.b.c";
    EXPECT_NO_FATAL_FAILURE(SettingDialog::setItemVisiable(set, false));
    EXPECT_TRUE(SettingDialog::kHiddenSettingItems.contains(set));
    EXPECT_TRUE(SettingDialog::needHide(set));
}
