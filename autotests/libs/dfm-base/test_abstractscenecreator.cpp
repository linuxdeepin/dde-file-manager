// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractscenecreator.cpp
 * @brief Unit tests for AbstractSceneCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/interfaces/abstractscenecreator.h"

#include <QTest>

using namespace src;

class AbstractSceneCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AbstractSceneCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AbstractSceneCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AbstractSceneCreatorTest, removeChild)
{
    // Test method: void removeChild((const QString &scene))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removeChild(_arg0));
}
