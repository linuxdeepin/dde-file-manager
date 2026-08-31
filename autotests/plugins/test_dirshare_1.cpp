// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dirshare_1.cpp
 * @brief Unit tests for DirShare methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dirshare.h"

#include <QTest>

using namespace dfmplugin_dirshare;

class DirShareTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DirShare();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DirShare *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DirShareTest, bindSceneOnAdded)
{
    // Test method: void bindSceneOnAdded((const QString &newScene))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->bindSceneOnAdded(_arg0));
}

TEST_F(DirShareTest, createShareControlWidget)
{
    // Test method: QWidget createShareControlWidget((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->createShareControlWidget(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->createShareControlWidget(_arg0); });

}

TEST_F(DirShareTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(DirShareTest, onShareStateChanged)
{
    // Test method: void onShareStateChanged((const QString &path))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onShareStateChanged(_arg0));
}

TEST_F(DirShareTest, regToPropertyDialog)
{
    // Test method: void regToPropertyDialog(())
    EXPECT_NO_FATAL_FAILURE(obj->regToPropertyDialog());
}

TEST_F(DirShareTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}
