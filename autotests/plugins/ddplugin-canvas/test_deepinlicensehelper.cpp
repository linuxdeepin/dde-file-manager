// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "watermask/deepinlicensehelper.h"

#include <gtest/gtest.h>
#include <QApplication>

using namespace ddplugin_canvas;

class UT_DeepinLicenseHelper : public testing::Test
{
public:
    virtual void SetUp() override
    {
        if (!QApplication::instance()) {
            int argc = 0;
            char **argv = nullptr;
            app = new QApplication(argc, argv);
        }

        helper = DeepinLicenseHelper::instance();
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

public:
    QApplication *app = nullptr;
    DeepinLicenseHelper *helper = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_DeepinLicenseHelper, instance_GetInstance_ReturnsSingleton)
{
    EXPECT_NE(helper, nullptr);
}

TEST_F(UT_DeepinLicenseHelper, init_WithValidHelper_CallsInit)
{
    EXPECT_NO_THROW(helper->init());
}

TEST_F(UT_DeepinLicenseHelper, delayGetState_WithValidHelper_StartsTimer)
{
    EXPECT_NO_THROW(helper->delayGetState());
}

TEST_F(UT_DeepinLicenseHelper, requestLicenseState_WithValidHelper_CallsRequest)
{
    EXPECT_NO_THROW(helper->requestLicenseState());
}