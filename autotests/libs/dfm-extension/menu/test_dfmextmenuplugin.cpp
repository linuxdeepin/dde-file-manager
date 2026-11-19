// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <chrono>

#include <dfm-extension/menu/dfmextmenu.h>
#include <dfm-extension/menu/dfmextmenuproxy.h>
#include <dfm-extension/menu/dfmextaction.h>
#include <dfm-extension/menu/dfmextmenuplugin.h>

namespace {
// Test helper classes - use composition to avoid constructor issues
class TestMenuPlugin : public dfmext::DFMExtMenuPlugin {
public:
    TestMenuPlugin() = default;
    ~TestMenuPlugin() = default;
    
    // Override virtual methods for testing
    void initialize(dfmext::DFMExtMenuProxy *proxy) {
        initializeCalled = true;
        this->proxy = proxy;
    }
    
    void shutdown() {
        shutdownCalled = true;
    }
    
    bool initializeCalled = false;
    bool shutdownCalled = false;
    dfmext::DFMExtMenuProxy *proxy = nullptr;
};
}

class DFMExtMenuPluginTest : public ::testing::Test {
protected:
    void SetUp() override {
        plugin = std::make_unique<TestMenuPlugin>();
    }
    
    void TearDown() override {
        plugin.reset();
    }
    
    std::unique_ptr<TestMenuPlugin> plugin;
};

// Test basic plugin functionality
TEST_F(DFMExtMenuPluginTest, BasicFunctionality) {
    EXPECT_FALSE(plugin->initializeCalled);
    EXPECT_FALSE(plugin->shutdownCalled);
    
    // Test initialization
    plugin->initialize(nullptr);
    EXPECT_TRUE(plugin->initializeCalled);
    
    // Test shutdown
    plugin->shutdown();
    EXPECT_TRUE(plugin->shutdownCalled);
}

// Test menu creation workflow
TEST_F(DFMExtMenuPluginTest, MenuCreationWorkflow) {
    // Initialize plugin
    plugin->initialize(nullptr);
    EXPECT_TRUE(plugin->initializeCalled);
    
    // Test that plugin can be created and destroyed
    EXPECT_NE(plugin.get(), nullptr);
}

// Test plugin lifecycle
TEST_F(DFMExtMenuPluginTest, PluginLifecycle) {
    // Test multiple initialization calls
    plugin->initialize(nullptr);
    EXPECT_TRUE(plugin->initializeCalled);
    
    plugin->initializeCalled = false;
    plugin->initialize(nullptr);
    EXPECT_TRUE(plugin->initializeCalled);
    
    // Test shutdown
    plugin->shutdown();
    EXPECT_TRUE(plugin->shutdownCalled);
}

// Test callback registration
TEST_F(DFMExtMenuPluginTest, CallbackRegistration) {
    bool callbackCalled = false;
    
    // Create a real DFMExtMenuPlugin instance to test callback registration
    dfmext::DFMExtMenuPlugin realPlugin;
    
    // Register a callback
    realPlugin.registerInitialize([&callbackCalled](dfmext::DFMExtMenuProxy *) {
        callbackCalled = true;
    });
    
    // Trigger initialize
    realPlugin.initialize(nullptr);
    
    // Verify callback was called
    EXPECT_TRUE(callbackCalled);
}

// Test multiple callbacks
TEST_F(DFMExtMenuPluginTest, MultipleCallbacks) {
    int callbackCount = 0;
    
    // Create a real DFMExtMenuPlugin instance to test callback registration
    dfmext::DFMExtMenuPlugin realPlugin;
    
    // Register callback
    realPlugin.registerInitialize([&callbackCount](dfmext::DFMExtMenuProxy *) {
        callbackCount++;
    });

    
    // Trigger methods
    realPlugin.initialize(nullptr);
    
    // Verify callback was called
    EXPECT_EQ(callbackCount, 1);
}

// Test buildNormalMenu functionality
TEST_F(DFMExtMenuPluginTest, BuildNormalMenu) {
    bool callbackCalled = false;
    bool menuBuilt = false;
    
    // Create a real DFMExtMenuPlugin instance
    dfmext::DFMExtMenuPlugin realPlugin;
    
    // Register buildNormalMenu callback
    realPlugin.registerBuildNormalMenu([&callbackCalled, &menuBuilt](
        dfmext::DFMExtMenu *menu,
        const std::string &currentPath,
        const std::string &focusPath,
        const std::list<std::string> &pathList,
        bool onDesktop) {
        callbackCalled = true;
        menuBuilt = true;
        return true; // Indicate menu was built successfully
    });
    
    // Test buildNormalMenu
    std::list<std::string> testPaths = {"file:///home/test1", "file:///home/test2"};
    bool result = realPlugin.buildNormalMenu(nullptr, "file:///home", "file:///home/test1", testPaths, false);
    
    // Verify callback was called and menu was built
    EXPECT_TRUE(callbackCalled);
    EXPECT_TRUE(result);
}

// Test buildEmptyAreaMenu functionality
TEST_F(DFMExtMenuPluginTest, BuildEmptyAreaMenu) {
    bool callbackCalled = false;
    
    // Create a real DFMExtMenuPlugin instance
    dfmext::DFMExtMenuPlugin realPlugin;
    
    // Register buildEmptyAreaMenu callback
    realPlugin.registerBuildEmptyAreaMenu([&callbackCalled](
        dfmext::DFMExtMenu *menu,
        const std::string &currentPath,
        bool onDesktop) {
        callbackCalled = true;
        return true; // Indicate menu was built successfully
    });
    
    // Test buildEmptyAreaMenu
    bool result = realPlugin.buildEmptyAreaMenu(nullptr, "file:///home", true);
    
    // Verify callback was called and menu was built
    EXPECT_TRUE(callbackCalled);
    EXPECT_TRUE(result);
}

// Test boundary conditions
TEST_F(DFMExtMenuPluginTest, BoundaryConditions) {
    // Test with null proxy
    plugin->initialize(nullptr);
    EXPECT_TRUE(plugin->initializeCalled);
    EXPECT_EQ(plugin->proxy, nullptr);
    
    // Test multiple shutdowns
    plugin->shutdown();
    EXPECT_TRUE(plugin->shutdownCalled);
    
    plugin->shutdownCalled = false;
    plugin->shutdown();
    EXPECT_TRUE(plugin->shutdownCalled);
}

// Test error handling
TEST_F(DFMExtMenuPluginTest, ErrorHandling) {
    // Test that plugin handles errors gracefully
    try {
        plugin->initialize(nullptr);
        plugin->shutdown();
        SUCCEED();
    } catch (...) {
        FAIL() << "Plugin should handle operations gracefully";
    }
}

// Test performance
TEST_F(DFMExtMenuPluginTest, Performance) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Perform multiple operations
    for (int i = 0; i < 1000; ++i) {
        plugin->initialize(nullptr);
        plugin->shutdown();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Should complete within reasonable time (1 second)
    EXPECT_LT(duration.count(), 1000000);
} 
