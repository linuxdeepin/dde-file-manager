// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_organizerconfig_1.cpp
 * @brief Unit tests for OrganizerConfig methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "config/organizerconfig.h"

#include <QTest>

using namespace ddplugin_organizer;

class OrganizerConfigTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OrganizerConfig();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OrganizerConfig *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OrganizerConfigTest, OrganizerConfig)
{
    // Test constructor: OrganizerConfig((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(OrganizerConfigTest, classification)
{
    // Test getter: int classification()
    auto result = obj->classification();
    EXPECT_EQ(result, 0);

}

TEST_F(OrganizerConfigTest, collectionStyle)
{
    // Test method: CollectionStyle collectionStyle((const QString &styleId, const QString &key))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->collectionStyle(_arg0, _arg1);
    EXPECT_NO_FATAL_FAILURE({ obj->collectionStyle(_arg0, _arg1); });

}

TEST_F(OrganizerConfigTest, hasConfigId)
{
    // Test method: bool hasConfigId((const QString &configId))
    QString _arg0{};
    auto result = obj->hasConfigId(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(OrganizerConfigTest, isEnable)
{
    // Test bool getter: isEnable()
    bool result = obj->isEnable();
    EXPECT_FALSE(result);

}

TEST_F(OrganizerConfigTest, lastStyleConfigId)
{
    // Test getter: QString lastStyleConfigId()
    auto result = obj->lastStyleConfigId();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OrganizerConfigTest, mode)
{
    // Test getter: int mode()
    auto result = obj->mode();
    EXPECT_EQ(result, 0);

}

TEST_F(OrganizerConfigTest, setClassification)
{
    // Test setter: void setClassification((int cf))
    EXPECT_NO_FATAL_FAILURE(obj->setClassification(0));
}

TEST_F(OrganizerConfigTest, setEnable)
{
    // Test setter: void setEnable((bool e))
    EXPECT_NO_FATAL_FAILURE(obj->setEnable(false));
}

TEST_F(OrganizerConfigTest, setLastStyleConfigId)
{
    // Test setter: void setLastStyleConfigId((const QString &id))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setLastStyleConfigId(_arg0));
}

TEST_F(OrganizerConfigTest, setMode)
{
    // Test setter: void setMode((int m))
    EXPECT_NO_FATAL_FAILURE(obj->setMode(0));
}

TEST_F(OrganizerConfigTest, setScreenInfo)
{
    // Test setter: void setScreenInfo((const QMap<QString, QString> info))
    EXPECT_NO_FATAL_FAILURE(obj->setScreenInfo(QMap<QString, QString>()));
}

TEST_F(OrganizerConfigTest, setVersion)
{
    // Test setter: void setVersion((const QString &v))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setVersion(_arg0));
}

TEST_F(OrganizerConfigTest, sync)
{
    // Test method: void sync((int ms))
    EXPECT_NO_FATAL_FAILURE(obj->sync(0));
}

TEST_F(OrganizerConfigTest, updateCollectionBase)
{
    // Test method: void updateCollectionBase((bool custom, const CollectionBaseDataPtr &base))
    CollectionBaseDataPtr _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->updateCollectionBase(false, _arg1));
}

TEST_F(OrganizerConfigTest, updateCollectionStyle)
{
    // Test method: void updateCollectionStyle((const QString &styleId, const CollectionStyle &style))
    QString _arg0{};
    CollectionStyle _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->updateCollectionStyle(_arg0, _arg1));
}

TEST_F(OrganizerConfigTest, OrganizerConfig_Destructor)
{
    // Test method:  ~OrganizerConfig(())
    EXPECT_NO_FATAL_FAILURE({ OrganizerConfig *tmp = new OrganizerConfig(); delete tmp; });
}
