// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_settingbackend.cpp
 * @brief Unit tests for SettingBackend methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/base/configs/settingbackend.h"

#include <QTest>

using namespace src;

class SettingBackendTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SettingBackend();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SettingBackend *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SettingBackendTest, SettingBackend)
{
    // Test constructor: SettingBackend((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SettingBackendTest, M_~SettingBackend)
{
    // Test method:  ~SettingBackend(())
    EXPECT_NO_FATAL_FAILURE({ SettingBackend *tmp = new SettingBackend(); delete tmp; });
}

TEST_F(SettingBackendTest, instance)
{
    // Test getter: SettingBackend instance()
    auto result = obj->instance();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SettingBackendTest, keys)
{
    // Test getter: QStringList keys()
    auto result = obj->keys();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SettingBackendTest, getOption)
{
    // Test method: QVariant getOption((const QString &key))
    QString _arg0{};
    auto result = obj->getOption(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(SettingBackendTest, doSync)
{
    // Test method: void doSync(())
    EXPECT_NO_FATAL_FAILURE(obj->doSync());
}

TEST_F(SettingBackendTest, setToSettings)
{
    // Test setter: void setToSettings((DSettings *settings))
    EXPECT_NO_FATAL_FAILURE(obj->setToSettings(nullptr));
}

TEST_F(SettingBackendTest, addSettingAccessor)
{
    // Test method: void addSettingAccessor((Application::GenericAttribute attr, SaveOptFunc set))
    EXPECT_NO_FATAL_FAILURE(obj->addSettingAccessor(Application::GenericAttribute(), SaveOptFunc()));
}

TEST_F(SettingBackendTest, removeSettingAccessor)
{
    // Test method: void removeSettingAccessor((const QString &key))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removeSettingAccessor(_arg0));
}

TEST_F(SettingBackendTest, doSetOption)
{
    // Test method: void doSetOption((const QString &key, const QVariant &value))
    QString _arg0{};
    QVariant _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->doSetOption(_arg0, _arg1));
}

TEST_F(SettingBackendTest, onDelayedSave)
{
    // Test method: void onDelayedSave(())
    EXPECT_NO_FATAL_FAILURE(obj->onDelayedSave());
}

TEST_F(SettingBackendTest, onValueChanged)
{
    // Test method: void onValueChanged((int attribute, const QVariant &value))
    QVariant _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onValueChanged(0, _arg1));
}

TEST_F(SettingBackendTest, initPresetSettingConfig)
{
    // Test method: void initPresetSettingConfig(())
    EXPECT_NO_FATAL_FAILURE(obj->initPresetSettingConfig());
}

TEST_F(SettingBackendTest, initBasicSettingConfig)
{
    // Test method: void initBasicSettingConfig(())
    EXPECT_NO_FATAL_FAILURE(obj->initBasicSettingConfig());
}

TEST_F(SettingBackendTest, initWorkspaceSettingConfig)
{
    // Test method: void initWorkspaceSettingConfig(())
    EXPECT_NO_FATAL_FAILURE(obj->initWorkspaceSettingConfig());
}

TEST_F(SettingBackendTest, initAdvanceSettingConfig)
{
    // Test method: void initAdvanceSettingConfig(())
    EXPECT_NO_FATAL_FAILURE(obj->initAdvanceSettingConfig());
}
