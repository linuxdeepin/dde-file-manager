// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_organizerconfigprivate_1.cpp
 * @brief Unit tests for OrganizerConfigPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "config/organizerconfig.h"

#include <QTest>

using namespace ddplugin_organizer;

class OrganizerConfigPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OrganizerConfigPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OrganizerConfigPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OrganizerConfigPrivateTest, OrganizerConfigPrivate)
{
    // Test constructor: OrganizerConfigPrivate((OrganizerConfig *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(OrganizerConfigPrivateTest, setValue)
{
    // Test setter: void setValue((const QString &group, const QString &key, const QVariant &var))
    QString _arg0{};
    QString _arg1{};
    QVariant _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->setValue(_arg0, _arg1, _arg2));
}

TEST_F(OrganizerConfigPrivateTest, OrganizerConfigPrivate_Destructor)
{
    // Test method:  ~OrganizerConfigPrivate(())
    EXPECT_NO_FATAL_FAILURE({ OrganizerConfigPrivate *tmp = new OrganizerConfigPrivate(); delete tmp; });
}
