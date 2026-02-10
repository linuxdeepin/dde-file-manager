// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/desktop/ddplugin-canvas/broker/canvasmanagerbroker.h"
#include "plugins/desktop/ddplugin-canvas/canvasmanager.h"

#include <QUrl>
#include <QVariant>

#include <gtest/gtest.h>

using namespace ddplugin_canvas;

class UT_CanvasManagerBroker : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        // Create a mock CanvasManager for the broker
        mockManager = new CanvasManager(nullptr);
        broker = new CanvasManagerBroker(mockManager, nullptr);
    }

    virtual void TearDown() override
    {
        delete broker;
        broker = nullptr;
        delete mockManager;
        mockManager = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    CanvasManagerBroker *broker = nullptr;
    CanvasManager *mockManager = nullptr;
};

TEST_F(UT_CanvasManagerBroker, iconLevel)
{
    bool instanceCalled = false;
    bool iconLevelCalled = false;
    
    // Stub CanvasManager::instance to return mock manager
    stub.set_lamda(ADDR(CanvasManager, instance), [&instanceCalled] {
        __DBG_STUB_INVOKE__
        instanceCalled = true;
        static CanvasManager mockManager(nullptr);
        return &mockManager;
    });
    
    // Stub CanvasManager::iconLevel to return test value
    stub.set_lamda(ADDR(CanvasManager, iconLevel), [&iconLevelCalled]() -> int {
        __DBG_STUB_INVOKE__
        iconLevelCalled = true;
        return 2;
    });

    int level = broker->iconLevel();
    EXPECT_TRUE(iconLevelCalled);
    EXPECT_EQ(level, 2);
}

TEST_F(UT_CanvasManagerBroker, setIconLevel)
{
    bool setIconLevelCalled = false;
    int capturedLevel = 0;
    
    // Stub CanvasManager::setIconLevel to capture parameter
    // Note: CanvasManagerBroker calls canvas->setIconLevel() directly on the passed pointer
    stub.set_lamda(ADDR(CanvasManager, setIconLevel), [&setIconLevelCalled, &capturedLevel](CanvasManager*, int level) {
        __DBG_STUB_INVOKE__
        setIconLevelCalled = true;
        capturedLevel = level;
    });

    broker->setIconLevel(3);
    EXPECT_TRUE(setIconLevelCalled);
    EXPECT_EQ(capturedLevel, 3);
}

TEST_F(UT_CanvasManagerBroker, autoArrange)
{
    bool autoArrangeCalled = false;
    
    // Stub CanvasManager::autoArrange to return test value
    // Note: CanvasManagerBroker calls canvas->autoArrange() directly on the passed pointer
    stub.set_lamda(ADDR(CanvasManager, autoArrange), [&autoArrangeCalled](CanvasManager*) -> bool {
        __DBG_STUB_INVOKE__
        autoArrangeCalled = true;
        return true;
    });

    bool result = broker->autoArrange();
    EXPECT_TRUE(autoArrangeCalled);
    EXPECT_TRUE(result);
}

TEST_F(UT_CanvasManagerBroker, setAutoArrange)
{
    bool setAutoArrangeCalled = false;
    bool capturedValue = false;
    
    // Stub CanvasManager::setAutoArrange to capture parameter
    // Note: CanvasManagerBroker calls canvas->setAutoArrange() directly on the passed pointer
    stub.set_lamda(ADDR(CanvasManager, setAutoArrange), [&setAutoArrangeCalled, &capturedValue](CanvasManager*, bool on) {
        __DBG_STUB_INVOKE__
        setAutoArrangeCalled = true;
        capturedValue = on;
    });

    broker->setAutoArrange(false);
    EXPECT_TRUE(setAutoArrangeCalled);
    EXPECT_FALSE(capturedValue);
}
