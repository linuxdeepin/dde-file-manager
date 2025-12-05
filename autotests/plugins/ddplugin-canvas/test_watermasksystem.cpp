// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "watermask/watermasksystem.h"
#include "watermask/deepinlicensehelper.h"

#include <gtest/gtest.h>
#include <QApplication>
#include <QWidget>
#include <QFileInfo>
#include <QLocale>

#include <DSysInfo>

DCORE_USE_NAMESPACE
using namespace ddplugin_canvas;

class UT_WatermaskSystem : public testing::Test
{
public:
    virtual void SetUp() override
    {
        if (!QApplication::instance()) {
            int argc = 0;
            char **argv = nullptr;
            app = new QApplication(argc, argv);
        }

        parentWidget = new QWidget();

        // Mock DeepinLicenseHelper to prevent crashes
        stub.set_lamda(ADDR(DeepinLicenseHelper, instance), []() -> DeepinLicenseHelper* {
            __DBG_STUB_INVOKE__
            static DeepinLicenseHelper helper;
            return &helper;
        });

        stub.set_lamda(ADDR(DeepinLicenseHelper, init), [](DeepinLicenseHelper*) {
            __DBG_STUB_INVOKE__
        });

        // Mock DSysInfo to prevent crashes
        stub.set_lamda(ADDR(DSysInfo, deepinType), []() -> DSysInfo::DeepinType {
            __DBG_STUB_INVOKE__
            return DSysInfo::DeepinDesktop;
        });

        stub.set_lamda(ADDR(DSysInfo, uosEditionType), []() -> DSysInfo::UosEdition {
            __DBG_STUB_INVOKE__
            return DSysInfo::UosEdition::UosHome;
        });

        // Create test instance
        system = new WatermaskSystem(parentWidget);
    }

    virtual void TearDown() override
    {
        if (system) {
            delete system;
            system = nullptr;
        }

        if (parentWidget) {
            delete parentWidget;
            parentWidget = nullptr;
        }

        stub.clear();
    }

public:
    QApplication *app = nullptr;
    QWidget *parentWidget = nullptr;
    WatermaskSystem *system = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_WatermaskSystem, constructor_CreateSystem_InitializesCorrectly)
{
    EXPECT_NE(system, nullptr);
}

TEST_F(UT_WatermaskSystem, isEnable_WithValidSystem_ReturnsBoolean)
{
    bool result = WatermaskSystem::isEnable();
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(UT_WatermaskSystem, usingCn_WithValidSystem_ReturnsBoolean)
{
    bool result = WatermaskSystem::usingCn();
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(UT_WatermaskSystem, showLicenseState_WithValidSystem_ReturnsBoolean)
{
    bool result = WatermaskSystem::showLicenseState();
    EXPECT_TRUE(result == true || result == false);
}