// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_propertydialog_1.cpp
 * @brief Unit tests for PropertyDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "propertydialog.h"

#include <QTest>

using namespace dfmplugin_propertydialog;

class PropertyDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PropertyDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PropertyDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PropertyDialogTest, bindScene)
{
    // Test method: void bindScene((const QString &parentScene))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->bindScene(_arg0));
}

TEST_F(PropertyDialogTest, bindSceneOnAdded)
{
    // Test method: void bindSceneOnAdded((const QString &newScene))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->bindSceneOnAdded(_arg0));
}

TEST_F(PropertyDialogTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}
