// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_mountcontroladaptor.cpp
 * @brief Unit tests for MountControlAdaptor methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "config/dbus/MountControlAdaptor.h"

#include <QTest>

using namespace src;

class MountControlAdaptorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MountControlAdaptor();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MountControlAdaptor *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MountControlAdaptorTest, Mount)
{
    // Test method: QVariantMap Mount((const QString &path, const QVariantMap &opts))
    QString _arg0{};
    QVariantMap _arg1{};
    auto result = obj->Mount(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(MountControlAdaptorTest, MountControlAdaptor)
{
    // Test constructor: MountControlAdaptor((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(MountControlAdaptorTest, Unmount)
{
    // Test method: QVariantMap Unmount((const QString &path, const QVariantMap &opts))
    QString _arg0{};
    QVariantMap _arg1{};
    auto result = obj->Unmount(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}
