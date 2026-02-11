// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/desktop/ddplugin-canvas/model/selectionhookinterface.h"

#include <gtest/gtest.h>

using namespace ddplugin_canvas;

class UT_SelectionHookInterface : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        // Create instance of SelectionHookInterface for testing
        hookInterface = new SelectionHookInterface();
    }

    virtual void TearDown() override
    {
        delete hookInterface;
        hookInterface = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    SelectionHookInterface *hookInterface = nullptr;
};

TEST_F(UT_SelectionHookInterface, Constructor_CreateInterface_ObjectCreated)
{
    // Test constructor behavior
    EXPECT_NE(hookInterface, nullptr);
}

TEST_F(UT_SelectionHookInterface, Destructor_DeleteInterface_ObjectDestroyed)
{
    // Test destructor behavior - should not crash
    SelectionHookInterface *tempInterface = new SelectionHookInterface();
    EXPECT_NO_THROW(delete tempInterface);
}

TEST_F(UT_SelectionHookInterface, clear_DefaultImplementation_DoesNotCrash)
{
    // Test default clear implementation - should not crash
    EXPECT_NO_THROW(hookInterface->clear());
    
    // Test multiple calls
    EXPECT_NO_THROW(hookInterface->clear());
    EXPECT_NO_THROW(hookInterface->clear());
}

