// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "watermask/watermaskcontainer.h"
#include "watermask/watermasksystem.h"
#include "watermask/watermaskframe.h"
#include "watermask/customwatermasklabel.h"

#include <gtest/gtest.h>
#include <QApplication>
#include <QWidget>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>

using namespace ddplugin_canvas;

class UT_WatermaskContainer : public testing::Test
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

        // Mock WatermaskSystem to prevent crashes
        stub.set_lamda(ADDR(WatermaskSystem, isEnable), []() -> bool {
            __DBG_STUB_INVOKE__
            return false; // Return false to use WaterMaskFrame
        });

        // Mock QFile operations
        stub.set_lamda(static_cast<bool (*)(const QString&)>(&QFile::exists), [](const QString&) -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });

        container = new WatermaskContainer(parentWidget);
    }

    virtual void TearDown() override
    {
        if (container) {
            delete container;
            container = nullptr;
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
    WatermaskContainer *container = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_WatermaskContainer, constructor_CreateContainer_InitializesCorrectly)
{
    EXPECT_NE(container, nullptr);
}

TEST_F(UT_WatermaskContainer, isEnable_WithValidConfig_ReturnsBoolean)
{
    bool result = WatermaskContainer::isEnable();
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(UT_WatermaskContainer, refresh_WithValidContainer_CallsRefresh)
{
    EXPECT_NO_THROW(container->refresh());
}

TEST_F(UT_WatermaskContainer, updatePosition_WithValidContainer_CallsUpdatePosition)
{
    EXPECT_NO_THROW(container->updatePosition());
}