// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/testing/events/testingeventrecevier.h"

#include <dfm-framework/event/event.h>

#include <QAccessible>
#include <QWidget>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;
DPF_USE_NAMESPACE

class UT_TestingEventRecevier : public testing::Test
{
protected:
    void SetUp() override
    {
        receiver = TestingEventRecevier::instance();
    }

    void TearDown() override
    {
        stub.clear();
    }

    TestingEventRecevier *receiver { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_TestingEventRecevier, instance_ReturnsSingleton)
{
    TestingEventRecevier *instance1 = TestingEventRecevier::instance();
    TestingEventRecevier *instance2 = TestingEventRecevier::instance();

    EXPECT_EQ(instance1, instance2);
    EXPECT_NE(instance1, nullptr);
}

TEST_F(UT_TestingEventRecevier, initializeConnections_InstallsAccessible)
{
    bool installFactoryCalled = false;

    stub.set_lamda(&QAccessible::installFactory,
                   [&installFactoryCalled](QAccessible::InterfaceFactory) {
                       __DBG_STUB_INVOKE__
                       installFactoryCalled = true;
                   });

    stub.set_lamda(&QAccessible::setActive,
                   [](bool) {
                       __DBG_STUB_INVOKE__
                   });

    receiver->initializeConnections();

    EXPECT_TRUE(installFactoryCalled);
}

