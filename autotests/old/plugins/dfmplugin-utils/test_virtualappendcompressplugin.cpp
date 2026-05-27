// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/appendcompress/virtualappendcompressplugin.h"
#include "plugins/common/dfmplugin-utils/appendcompress/appendcompresseventreceiver.h"

#include <dfm-framework/dpf.h>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;

class UT_VirtualAppendCompressPlugin : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.set_lamda(ADDR(DPF_NAMESPACE::Event, eventType),
                       [](DPF_NAMESPACE::Event *, const QString &, const QString &) -> int {
            __DBG_STUB_INVOKE__
            return DPF_NAMESPACE::EventTypeScope::kInValid;
        });

        plugin = new VirtualAppendCompressPlugin();
    }

    virtual void TearDown() override
    {
        delete plugin;
        plugin = nullptr;
        stub.clear();
    }

    stub_ext::StubExt stub;
    VirtualAppendCompressPlugin *plugin { nullptr };
};

/**
 * @brief Test constructor creates valid object
 * Verifies that the plugin is properly constructed with eventReceiver initialized
 */
TEST_F(UT_VirtualAppendCompressPlugin, Constructor_CreatesValidObject)
{
    __DBG_STUB_INVOKE__

    EXPECT_NE(plugin, nullptr);

    // According to dfm_ut.md, we can access private members directly
    // Verify that eventReceiver is properly initialized
    EXPECT_NE(plugin->eventReceiver.data(), nullptr);
}

/**
 * @brief Test initialize method calls eventReceiver's initEventConnect
 * Verifies that the plugin properly initializes event connections
 */
TEST_F(UT_VirtualAppendCompressPlugin, initialize_CallsInitEventConnect)
{
    __DBG_STUB_INVOKE__

    bool initEventConnectCalled = false;

    stub.set_lamda(ADDR(AppendCompressEventReceiver, initEventConnect),
                   [&initEventConnectCalled](AppendCompressEventReceiver *) {
        __DBG_STUB_INVOKE__
        initEventConnectCalled = true;
    });

    plugin->initialize();

    EXPECT_TRUE(initEventConnectCalled);
}

/**
 * @brief Test initialize can be called multiple times
 * Verifies that calling initialize multiple times doesn't cause issues
 */
TEST_F(UT_VirtualAppendCompressPlugin, initialize_MultipleCallsSafe)
{
    __DBG_STUB_INVOKE__

    int initEventConnectCallCount = 0;

    stub.set_lamda(ADDR(AppendCompressEventReceiver, initEventConnect),
                   [&initEventConnectCallCount](AppendCompressEventReceiver *) {
        __DBG_STUB_INVOKE__
        initEventConnectCallCount++;
    });

    plugin->initialize();
    plugin->initialize();
    plugin->initialize();

    // initEventConnect should be called 3 times (once per initialize call)
    EXPECT_EQ(initEventConnectCallCount, 3);
}

/**
 * @brief Test initialize with valid eventReceiver
 * Verifies that initialize works correctly when eventReceiver is valid
 */
TEST_F(UT_VirtualAppendCompressPlugin, initialize_ValidEventReceiver_Success)
{
    __DBG_STUB_INVOKE__

    // Ensure eventReceiver is not null before initialization
    ASSERT_NE(plugin->eventReceiver.data(), nullptr);

    bool initEventConnectCalled = false;
    AppendCompressEventReceiver *capturedReceiver = nullptr;

    stub.set_lamda(ADDR(AppendCompressEventReceiver, initEventConnect),
                   [&initEventConnectCalled, &capturedReceiver](AppendCompressEventReceiver *receiver) {
        __DBG_STUB_INVOKE__
        initEventConnectCalled = true;
        capturedReceiver = receiver;
    });

    plugin->initialize();

    EXPECT_TRUE(initEventConnectCalled);
    EXPECT_EQ(capturedReceiver, plugin->eventReceiver.data());
}

/**
 * @brief Test start method returns true
 * Verifies that the plugin starts successfully
 */
TEST_F(UT_VirtualAppendCompressPlugin, start_ReturnsTrue)
{
    __DBG_STUB_INVOKE__

    bool result = plugin->start();

    EXPECT_TRUE(result);
}

/**
 * @brief Test start can be called multiple times
 * Verifies that calling start multiple times always returns true
 */
TEST_F(UT_VirtualAppendCompressPlugin, start_MultipleCallsReturnsTrue)
{
    __DBG_STUB_INVOKE__

    EXPECT_TRUE(plugin->start());
    EXPECT_TRUE(plugin->start());
    EXPECT_TRUE(plugin->start());
}

/**
 * @brief Test start without initialize
 * Verifies that start can be called before initialize
 */
TEST_F(UT_VirtualAppendCompressPlugin, start_WithoutInitialize_ReturnsTrue)
{
    __DBG_STUB_INVOKE__

    // Don't call initialize before start
    bool result = plugin->start();

    EXPECT_TRUE(result);
}

/**
 * @brief Test initialize then start
 * Verifies the typical usage pattern: initialize followed by start
 */
TEST_F(UT_VirtualAppendCompressPlugin, InitializeThenStart_Success)
{
    __DBG_STUB_INVOKE__

    bool initEventConnectCalled = false;

    stub.set_lamda(ADDR(AppendCompressEventReceiver, initEventConnect),
                   [&initEventConnectCalled](AppendCompressEventReceiver *) {
        __DBG_STUB_INVOKE__
        initEventConnectCalled = true;
    });

    // Typical usage pattern
    plugin->initialize();
    bool startResult = plugin->start();

    EXPECT_TRUE(initEventConnectCalled);
    EXPECT_TRUE(startResult);
}

/**
 * @brief Test eventReceiver ownership
 * Verifies that eventReceiver is properly managed by QScopedPointer
 */
TEST_F(UT_VirtualAppendCompressPlugin, EventReceiver_ProperlyManaged)
{
    __DBG_STUB_INVOKE__

    // Access private member according to dfm_ut.md
    AppendCompressEventReceiver *receiver = plugin->eventReceiver.data();

    EXPECT_NE(receiver, nullptr);

    // Verify that the eventReceiver is the same instance after operations
    plugin->initialize();
    EXPECT_EQ(plugin->eventReceiver.data(), receiver);

    plugin->start();
    EXPECT_EQ(plugin->eventReceiver.data(), receiver);
}

/**
 * @brief Test plugin lifecycle
 * Verifies the complete plugin lifecycle: construct -> initialize -> start
 */
TEST_F(UT_VirtualAppendCompressPlugin, PluginLifecycle_CompleteFlow)
{
    __DBG_STUB_INVOKE__

    int initCallCount = 0;
    int startCallCount = 0;

    stub.set_lamda(ADDR(AppendCompressEventReceiver, initEventConnect),
                   [&initCallCount](AppendCompressEventReceiver *) {
        __DBG_STUB_INVOKE__
        initCallCount++;
    });

    // Simulate complete plugin lifecycle
    ASSERT_NE(plugin, nullptr); // Construction

    plugin->initialize(); // Initialization
    EXPECT_EQ(initCallCount, 1);

    bool startResult = plugin->start(); // Start
    EXPECT_TRUE(startResult);

    // Verify state after full lifecycle
    EXPECT_NE(plugin->eventReceiver.data(), nullptr);
}

/**
 * @brief Test start before initialize (reversed order)
 * Verifies that the plugin handles reversed initialization order
 */
TEST_F(UT_VirtualAppendCompressPlugin, StartBeforeInitialize_BothSucceed)
{
    __DBG_STUB_INVOKE__

    bool initEventConnectCalled = false;

    stub.set_lamda(ADDR(AppendCompressEventReceiver, initEventConnect),
                   [&initEventConnectCalled](AppendCompressEventReceiver *) {
        __DBG_STUB_INVOKE__
        initEventConnectCalled = true;
    });

    // Call start before initialize (unusual but should work)
    bool startResult = plugin->start();
    EXPECT_TRUE(startResult);
    EXPECT_FALSE(initEventConnectCalled);

    // Then call initialize
    plugin->initialize();
    EXPECT_TRUE(initEventConnectCalled);
}

/**
 * @brief Test eventReceiver member is not null by default
 * Verifies that eventReceiver is initialized during construction
 */
TEST_F(UT_VirtualAppendCompressPlugin, EventReceiver_NotNullByDefault)
{
    __DBG_STUB_INVOKE__

    // Create a new plugin instance
    VirtualAppendCompressPlugin *newPlugin = new VirtualAppendCompressPlugin();

    EXPECT_NE(newPlugin->eventReceiver.data(), nullptr);

    delete newPlugin;
}
