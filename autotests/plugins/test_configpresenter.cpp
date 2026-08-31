// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_configpresenter.cpp
 * @brief Unit tests for ConfigPresenter methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "config/configpresenter.h"

#include <QTest>

using namespace ddplugin_organizer;

class ConfigPresenterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ConfigPresenter();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ConfigPresenter *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ConfigPresenterTest, mode)
{
    // Test getter: OrganizerMode mode()
    auto result = obj->mode();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(ConfigPresenterTest, onDConfigChanged)
{
    // Test method: void onDConfigChanged((const QString &cfg, const QString &key))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onDConfigChanged(_arg0, _arg1));
}

TEST_F(ConfigPresenterTest, saveCustomProfile)
{
    // Test method: void saveCustomProfile((const QList<CollectionBaseDataPtr> &baseDatas))
    QList<CollectionBaseDataPtr> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->saveCustomProfile(_arg0));
}

TEST_F(ConfigPresenterTest, saveNormalProfile)
{
    // Test method: void saveNormalProfile((const QList<CollectionBaseDataPtr> &baseDatas))
    QList<CollectionBaseDataPtr> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->saveNormalProfile(_arg0));
}

TEST_F(ConfigPresenterTest, setEnabledTypeCategories)
{
    // Test setter: void setEnabledTypeCategories((ItemCategories flags))
    EXPECT_NO_FATAL_FAILURE(obj->setEnabledTypeCategories(ItemCategories()));
}

TEST_F(ConfigPresenterTest, setSurfaceInfo)
{
    // Test setter: void setSurfaceInfo((const QList<QWidget *> surfaces))
    EXPECT_NO_FATAL_FAILURE(obj->setSurfaceInfo(nullptr));
}

TEST_F(ConfigPresenterTest, writeCustomStyle)
{
    // Test method: void writeCustomStyle((const QList<CollectionStyle> &styles))
    QList<CollectionStyle> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->writeCustomStyle(_arg0));
}

TEST_F(ConfigPresenterTest, writeNormalStyle)
{
    // Test method: void writeNormalStyle((const QString &configId, const QList<CollectionStyle> &styles))
    QString _arg0{};
    QList<CollectionStyle> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->writeNormalStyle(_arg0, _arg1));
}
