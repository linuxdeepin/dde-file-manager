// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <dfm-base/dialogs/settingsdialog/settingdialog.h>

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
