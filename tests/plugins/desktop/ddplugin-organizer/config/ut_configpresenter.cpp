// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "config/configpresenter.h"
#include "config/organizerconfig.h"


#include "stubext.h"
#include <gtest/gtest.h>

using namespace ddplugin_organizer;

class UT_ConfigPresenter : public testing::Test
{
    virtual void SetUp() override {
    }
    virtual void TearDown() override {
        stub.clear();
    }
    stub_ext::StubExt stub;
};

TEST_F(UT_ConfigPresenter, profile)
{
    ConfigPresenter::instance()->conf = new OrganizerConfig;
    auto fun_type = static_cast<QList<CollectionBaseDataPtr>(OrganizerConfig::*)(bool)const>(&OrganizerConfig::collectionBase);
    stub.set_lamda(fun_type,[](OrganizerConfig *self,bool){
        __DBG_STUB_INVOKE__;
        CollectionBaseDataPtr ptr(new CollectionBaseData);
        ptr->key = "temp_key";
        ptr->name = "temp_name";
        return QList{ptr};
    });
    bool CollectionStyle_call = false;
    auto fun_type1 = static_cast<CollectionStyle(OrganizerConfig::*)(bool,const QString&)const>(&OrganizerConfig::collectionStyle);
    stub.set_lamda(fun_type1,[&CollectionStyle_call](OrganizerConfig *self, bool,const QString&){
        __DBG_STUB_INVOKE__
        CollectionStyle_call = true;
        return CollectionStyle();
    });

    bool writeColletion_call = false;
    stub.set_lamda(&OrganizerConfig::writeCollectionBase,
        [&writeColletion_call](OrganizerConfig *self, bool,const QList<CollectionBaseDataPtr>&){
        __DBG_STUB_INVOKE__
        writeColletion_call = true;
    });

    bool updataCollectionStyle = false;
    stub.set_lamda(&OrganizerConfig::updateCollectionStyle,
         [&updataCollectionStyle](OrganizerConfig *self,bool,const CollectionStyle&){
        __DBG_STUB_INVOKE__
        updataCollectionStyle = true;
    });

    bool writeColletionStyle_call = false;
    stub.set_lamda(&OrganizerConfig::writeCollectionStyle,
        [&writeColletionStyle_call](OrganizerConfig *self,bool,const QList<CollectionStyle>&){
        __DBG_STUB_INVOKE__
        writeColletionStyle_call = true;
    });

    CollectionBaseDataPtr ptr(new CollectionBaseData);
    QList<CollectionBaseDataPtr> list{ptr};
    CollectionStyle style = CollectionStyle();
    style.key = "temp_key";
    QList<CollectionStyle> style_list{style};
    QList<CollectionBaseDataPtr> res = ConfigPresenter::instance()->customProfile();
    ItemCategories flags = 0;

    ConfigPresenter::instance()->saveCustomProfile(list);
    ConfigPresenter::instance()->normalProfile();
    ConfigPresenter::instance()->saveNormalProfile(list);
    ConfigPresenter::instance()->customStyle("temp_key");
    ConfigPresenter::instance()->updateCustomStyle(style);
    ConfigPresenter::instance()->writeCustomStyle(style_list);
    ConfigPresenter::instance()->enabledTypeCategories();
    ConfigPresenter::instance()->setEnabledTypeCategories(flags);
    ConfigPresenter::instance()->normalStyle("temp_key");
    ConfigPresenter::instance()->updateNormalStyle(style);
    ConfigPresenter::instance()->writeNormalStyle(style_list);

    EXPECT_EQ(res.at(0)->key,"temp_key");
    EXPECT_EQ(res.at(0)->name,"temp_name");
    EXPECT_TRUE(writeColletion_call);
    EXPECT_TRUE(CollectionStyle_call);
    EXPECT_TRUE(updataCollectionStyle);
    EXPECT_TRUE(writeColletionStyle_call);
}

