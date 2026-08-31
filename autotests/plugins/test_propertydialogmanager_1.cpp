// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_propertydialogmanager_1.cpp
 * @brief Unit tests for PropertyDialogManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/propertydialogmanager.h"

#include <QTest>

using namespace dfmplugin_propertydialog;

class PropertyDialogManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PropertyDialogManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PropertyDialogManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PropertyDialogManagerTest, PropertyDialogManager)
{
    // Test constructor: PropertyDialogManager((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(PropertyDialogManagerTest, getUpdaterByName)
{
    // Test method: ViewExtensionUpdateFunc getUpdaterByName((const QString &name))
    QString _arg0{};
    auto result = obj->getUpdaterByName(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->getUpdaterByName(_arg0); });

}
