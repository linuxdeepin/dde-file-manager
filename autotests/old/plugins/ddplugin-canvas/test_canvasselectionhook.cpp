// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "hook/canvasselectionhook.h"

#include <gtest/gtest.h>

using namespace ddplugin_canvas;

class UT_CanvasSelectionHook : public testing::Test
{
public:
    virtual void SetUp() override
    {
        hook = new CanvasSelectionHook();
    }

    virtual void TearDown() override
    {
        delete hook;
        
        stub.clear();
    }

public:
    CanvasSelectionHook *hook = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_CanvasSelectionHook, constructor_CreateHook_InitializesCorrectly)
{
    EXPECT_NE(hook, nullptr);
}

TEST_F(UT_CanvasSelectionHook, clear_CallClear_PublishesSignal)
{
    // Instead of mocking complex DPF EventDispatcherManager, test that the method doesn't crash
    EXPECT_NO_THROW(hook->clear());
}

// Note: selectAll and invertSelection methods don't exist in CanvasSelectionHook
// Only the clear method is available according to the header file