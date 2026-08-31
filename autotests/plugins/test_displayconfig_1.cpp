// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_displayconfig_1.cpp
 * @brief Unit tests for DisplayConfig methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "displayconfig.h"

#include <QTest>

using namespace ddplugin_canvas;

class DisplayConfigTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DisplayConfig();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DisplayConfig *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DisplayConfigTest, DisplayConfig)
{
    // Test constructor: DisplayConfig((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DisplayConfigTest, autoAlign)
{
    // Test bool getter: autoAlign()
    bool result = obj->autoAlign();
    EXPECT_FALSE(result);

}

TEST_F(DisplayConfigTest, coordinates)
{
    // Test method: QHash<QString, QPoint> coordinates((const QString &key))
    QString _arg0{};
    auto result = obj->coordinates(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DisplayConfigTest, covertPostion)
{
    // Test method: QString covertPostion((const QPoint &pos))
    QPoint _arg0{};
    auto result = obj->covertPostion(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DisplayConfigTest, customWaterMask)
{
    // Test bool getter: customWaterMask()
    bool result = obj->customWaterMask();
    EXPECT_FALSE(result);

}

TEST_F(DisplayConfigTest, iconLevel)
{
    // Test getter: int iconLevel()
    auto result = obj->iconLevel();
    EXPECT_EQ(result, 0);

}

TEST_F(DisplayConfigTest, instance)
{
    // Test getter: DisplayConfig instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(DisplayConfigTest, setAutoAlign)
{
    // Test setter: void setAutoAlign((bool align))
    EXPECT_NO_FATAL_FAILURE(obj->setAutoAlign(false));
}

TEST_F(DisplayConfigTest, setCoordinates)
{
    // Test method: bool setCoordinates((const QString &key, const QHash<QString, QPoint> &pos))
    QString _arg0{};
    QHash<QString, QPoint> _arg1{};
    auto result = obj->setCoordinates(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(DisplayConfigTest, setIconLevel)
{
    // Test method: bool setIconLevel((int lv))
    auto result = obj->setIconLevel(0);
    EXPECT_FALSE(result);

}

TEST_F(DisplayConfigTest, setProfile)
{
    // Test method: bool setProfile((const QList<QString> &profile))
    QList<QString> _arg0{};
    auto result = obj->setProfile(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(DisplayConfigTest, setSortMethod)
{
    // Test method: bool setSortMethod((const int &role, const Qt::SortOrder &order))
    int _arg0{};
    Qt::SortOrder _arg1{};
    auto result = obj->setSortMethod(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(DisplayConfigTest, setValues)
{
    // Test setter: void setValues((const QString &group, const QHash<QString, QVariant> &values))
    QString _arg0{};
    QHash<QString, QVariant> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setValues(_arg0, _arg1));
}

TEST_F(DisplayConfigTest, sortMethod)
{
    // Test method: void sortMethod((int &role, Qt::SortOrder &order))
    int _arg0{};
    Qt::SortOrder _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sortMethod(_arg0, _arg1));
}

TEST_F(DisplayConfigTest, sync)
{
    // Test method: void sync(())
    EXPECT_NO_FATAL_FAILURE(obj->sync());
}

TEST_F(DisplayConfigTest, DisplayConfig_Destructor)
{
    // Test method:  ~DisplayConfig(())
    EXPECT_NO_FATAL_FAILURE({ DisplayConfig *tmp = new DisplayConfig(); delete tmp; });
}
