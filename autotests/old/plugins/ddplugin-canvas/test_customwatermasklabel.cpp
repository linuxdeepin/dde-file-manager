// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "watermask/customwatermasklabel.h"

#include <gtest/gtest.h>
#include <QApplication>
#include <QWidget>
#include <QPoint>
#include <QSize>

using namespace ddplugin_canvas;

class UT_CustomWaterMaskLabel : public testing::Test
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

        // Create test instance
        label = new CustomWaterMaskLabel(parentWidget);
    }

    virtual void TearDown() override
    {
        if (label) {
            delete label;
            label = nullptr;
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
    CustomWaterMaskLabel *label = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_CustomWaterMaskLabel, constructor_CreateLabel_InitializesCorrectly)
{
    EXPECT_NE(label, nullptr);
    EXPECT_EQ(label->parent(), parentWidget);
}

TEST_F(UT_CustomWaterMaskLabel, loadConfig_WithValidLabel_LoadsConfig)
{
    EXPECT_NO_THROW(label->loadConfig());
}

TEST_F(UT_CustomWaterMaskLabel, refresh_WithValidLabel_RefreshesLabel)
{
    EXPECT_NO_THROW(label->refresh());
}

TEST_F(UT_CustomWaterMaskLabel, setPosition_WithValidLabel_SetsPosition)
{
    EXPECT_NO_THROW(label->setPosition());
}

TEST_F(UT_CustomWaterMaskLabel, onConfigChanged_WithValidParameters_HandlesConfigChange)
{
    QString cfg = "org.deepin.dde.file-manager.desktop";
    QString key = "enableMask";

    EXPECT_NO_THROW(label->onConfigChanged(cfg, key));
}