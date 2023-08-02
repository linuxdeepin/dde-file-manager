// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later


#include "config/configpresenter.h"
#include "config/organizerconfig.h"
#include "config/organizerconfig_p.h"

#include "stubext.h"
#include <gtest/gtest.h>

using namespace ddplugin_organizer;

namespace {
inline constexpr char kGroupGeneral[] = "";
inline constexpr char kKeyEnable[] = "Enable";
inline constexpr char kKeyMode[] = "Mode";

inline constexpr char kGroupCollectionNormalized[] = "Collection_Normalized";
inline constexpr char kKeyClassification[] = "Classification";

inline constexpr char kGroupCollectionCustomed[] = "Collection_Customed";
inline constexpr char kGroupCollectionBase[] = "CollectionBase";
inline constexpr char kKeyName[] = "Name";
inline constexpr char kKeyKey[] = "Key";
inline constexpr char kGroupItems[] = "Items";

inline constexpr char kGroupCollectionStyle[] = "CollectionStyle";
inline constexpr char kKeyScreen[] = "screen";
inline constexpr char kKeyX[] = "X";
inline constexpr char kKeyY[] = "Y";
inline constexpr char kKeyWidth[] = "Width";
inline constexpr char kKeyHeight[] = "Height";
inline constexpr char kKeySizeMode[] = "SizeMode";

inline constexpr char kGroupClassifierType[] = "Classifier_Type";
inline constexpr char kKeyEnabledItems[] = "EnabledItems";

}
class UT_OrganizerConfig : public testing::Test
{
protected:
    virtual void SetUp() override {
        organize = new OrganizerConfig;
        organize->d->settings->clear();
    }
    virtual void TearDown() override {
        delete organize->d->settings;
        organize->d->settings = nullptr;
        delete organize;
        organize = nullptr;
        stub.clear();
    }

    OrganizerConfig *organize = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_OrganizerConfig, collectionBase1)
{
    auto fun_type = static_cast<CollectionBaseDataPtr(OrganizerConfig::*)(bool,const QString&)const>(&OrganizerConfig::collectionBase);
    stub.set_lamda(fun_type,[](OrganizerConfig *self,bool,QString){
        __DBG_STUB_INVOKE__
        CollectionBaseDataPtr ptr(new CollectionBaseData);
        ptr->name = "temp_name";
        return ptr;
    });
    organize->d->settings->setValue("Collection_Customed/CollectionBase/CollectionBase/temp_key",QString("temp_value"));

    QList<CollectionBaseDataPtr> res = organize->collectionBase(true);
    for(auto it = res.begin();it!=res.end();it++)
    {
        EXPECT_EQ((*it)->name,"temp_name");
    }
}

TEST_F(UT_OrganizerConfig, CollectionBase2)
{
    organize->d->settings->setValue("Collection_Customed/CollectionBase/temp_key/Name","temp_name");
    organize->d->settings->setValue("Collection_Customed/CollectionBase/temp_key/Key","temp_key");
    organize->d->settings->setValue("Collection_Customed/CollectionBase/temp_key/Items/items1",QUrl("temp_url"));

    CollectionBaseDataPtr base = organize->collectionBase(true, "temp_key");
    EXPECT_EQ(base->key,"temp_key");
    EXPECT_EQ(base->name,"temp_name");
    QList list{QUrl("temp_url")};
    EXPECT_EQ(base->items,list);
}

TEST_F(UT_OrganizerConfig, updateCollectionBase)
{
    CollectionBaseDataPtr base(new CollectionBaseData);
    base->name = "temp_name";
    base->key = "temp_key";
    base->items = QList{QUrl("temp_url")};

    organize->updateCollectionBase(true, base);

    organize->d->settings->beginGroup(kGroupCollectionCustomed);
    organize->d->settings->beginGroup(kGroupCollectionBase);
    organize->d->settings->beginGroup("temp_key");
    EXPECT_EQ(organize->d->settings->value(kKeyName).toString(),"temp_name");
    EXPECT_EQ(organize->d->settings->value(kKeyKey).toString(),"temp_key");
    organize->d->settings->beginGroup(kGroupItems);
    EXPECT_EQ(organize->d->settings->value(QString::number(0)),"temp_url");
    organize->d->settings->endGroup();
    organize->d->settings->endGroup();
    organize->d->settings->endGroup();
}

TEST_F(UT_OrganizerConfig, writeCollectionBase)
{
    CollectionBaseDataPtr base(new CollectionBaseData);
    base->name = "temp_name";
    base->key = "temp_key";
    base->items = QList{QUrl("temp_url")};
    QList<CollectionBaseDataPtr> list_ptr = {base};

    organize->writeCollectionBase(true, list_ptr);

    organize->d->settings->beginGroup( kGroupCollectionCustomed );
    organize->d->settings->beginGroup(kGroupCollectionBase);
    organize->d->settings->beginGroup("temp_key");
    EXPECT_EQ(organize->d->settings->value(kKeyName),"temp_name");
    EXPECT_EQ(organize->d->settings->value(kKeyKey),"temp_key");
    organize->d->settings->beginGroup(kGroupItems);
    EXPECT_EQ(organize->d->settings->value(QString::number(0)),"temp_url");
    organize->d->settings->endGroup();
    organize->d->settings->endGroup();
    organize->d->settings->endGroup();
    organize->d->settings->endGroup();
}

TEST_F(UT_OrganizerConfig, collectionStyle)
{
    CollectionStyle res= organize->collectionStyle(true,"temp_key");
    EXPECT_EQ(res.rect,QRect(-1,-1,0,0));
    EXPECT_EQ(res.key,"");
    EXPECT_EQ(res.screenIndex,-1);
}

TEST_F(UT_OrganizerConfig, updateCollectionStyle)
{
    CollectionStyle style{0,"temp_key",QRect(1,1,2,2),CollectionFrameSize::kLarge};

    organize->updateCollectionStyle(true,style);

    organize->d->settings->beginGroup(kGroupCollectionCustomed );
    organize->d->settings->beginGroup(kGroupCollectionStyle);
    organize->d->settings->beginGroup("temp_key");
    EXPECT_EQ(organize->d->settings->value(kKeyScreen),0);
    EXPECT_EQ(organize->d->settings->value(kKeyKey),"temp_key");
    EXPECT_EQ(organize->d->settings->value(kKeyX),1);
    EXPECT_EQ(organize->d->settings->value(kKeyY),1);
    EXPECT_EQ(organize->d->settings->value(kKeyWidth),2);
    EXPECT_EQ(organize->d->settings->value(kKeyHeight),2);
    EXPECT_EQ(organize->d->settings->value(kKeySizeMode),CollectionFrameSize::kLarge);
    organize->d->settings->endGroup();
    organize->d->settings->endGroup();
    organize->d->settings->endGroup();
}

TEST_F(UT_OrganizerConfig, writeCollectionStyle)
{
    CollectionStyle style{0,"temp_key",QRect(1,1,2,2),CollectionFrameSize::kLarge};

    QList<CollectionStyle> list{style};
    organize->writeCollectionStyle(true,list);
    organize->d->settings->beginGroup(kGroupCollectionCustomed );
    organize->d->settings->beginGroup(kGroupCollectionStyle);
    organize->d->settings->beginGroup("temp_key");
    EXPECT_EQ(organize->d->settings->value(kKeyScreen),0);
    EXPECT_EQ(organize->d->settings->value(kKeyKey),"temp_key");
    EXPECT_EQ(organize->d->settings->value(kKeyX),1);
    EXPECT_EQ(organize->d->settings->value(kKeyY),1);
    EXPECT_EQ(organize->d->settings->value(kKeyWidth),2);
    EXPECT_EQ(organize->d->settings->value(kKeyHeight),2);
    EXPECT_EQ(organize->d->settings->value(kKeySizeMode),CollectionFrameSize::kLarge);
    organize->d->settings->endGroup();
    organize->d->settings->endGroup();
    organize->d->settings->endGroup();
}

TEST_F(UT_OrganizerConfig, path)
{
    EXPECT_NO_FATAL_FAILURE(organize->path());
}
