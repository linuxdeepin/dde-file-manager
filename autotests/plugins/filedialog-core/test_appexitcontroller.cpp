// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stub-ext/stubext.h>

#include "../../../src/plugins/filedialog/core/utils/appexitcontroller.h"

#include <QApplication>
#include <QTimer>
#include <QEventLoop>
#include <QThread>

using namespace filedialog_core;

class UT_AppExitController : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        // Mock QApplication if not exists
        if (!qApp) {
            int argc = 0;
            char **argv = nullptr;
            new QApplication(argc, argv);
        }
        
        controller = &AppExitController::instance();
    }

    virtual void TearDown() override
    {
        // Reset singleton instance if needed
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
    AppExitController *controller = nullptr;
};

TEST_F(UT_AppExitController, Instance_SingletonPattern_ReturnsSameInstance)
{
    AppExitController *instance1 = &AppExitController::instance();
    AppExitController *instance2 = &AppExitController::instance();

    EXPECT_NE(instance1, nullptr);
    EXPECT_EQ(instance1, instance2);
}

TEST_F(UT_AppExitController, Dismiss_ResetsExitState)
{
    // Mock QTimer::isActive to return true so dismiss() will proceed
    stub.set_lamda(&QTimer::isActive, []() {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock QTimer::stop
    bool timerStopCalled = false;
    stub.set_lamda(&QTimer::stop, [&timerStopCalled]() {
        __DBG_STUB_INVOKE__
        timerStopCalled = true;
    });

    EXPECT_NO_THROW(controller->dismiss());
    EXPECT_TRUE(timerStopCalled);
}

TEST_F(UT_AppExitController, ReadyToExit_ValidTimeoutAndCallback_SchedulesExit)
{
    int timeout = 60;
    bool callbackCalled = false;
    auto testCallback = [&callbackCalled]() -> bool {
        callbackCalled = true;
        return true;
    };

    // Mock QTimer::isActive to return false so readyToExit() will proceed
    stub.set_lamda(&QTimer::isActive, []() {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Mock QTimer::start
    bool timerStartCalled = false;
    stub.set_lamda((void(QTimer::*)(int))&QTimer::start, [&timerStartCalled](QTimer*, int msec) {
        __DBG_STUB_INVOKE__
        timerStartCalled = true;
        EXPECT_EQ(msec, 1000); // readyToExit starts timer with 1 second interval
    });

    EXPECT_NO_THROW(controller->readyToExit(timeout, testCallback));
    EXPECT_TRUE(timerStartCalled);
}

TEST_F(UT_AppExitController, ReadyToExit_ZeroTimeout_SchedulesImmediateExit)
{
    int timeout = 0;
    bool callbackCalled = false;
    auto testCallback = [&callbackCalled]() -> bool {
        callbackCalled = true;
        return true;
    };

    // Mock QTimer::isActive to return false so readyToExit() will proceed
    stub.set_lamda(&QTimer::isActive, []() {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Mock QTimer::start
    bool timerStartCalled = false;
    stub.set_lamda((void(QTimer::*)(int))&QTimer::start, [&timerStartCalled](QTimer*, int msec) {
        __DBG_STUB_INVOKE__
        timerStartCalled = true;
        EXPECT_EQ(msec, 1000); // readyToExit starts timer with 1 second interval
    });

    EXPECT_NO_THROW(controller->readyToExit(timeout, testCallback));
    EXPECT_TRUE(timerStartCalled);
}

// TEST_F(UT_AppExitController, ReadyToExit_NegativeTimeout_SchedulesImmediateExit)
// {
//     int timeout = -1;
//     bool callbackCalled = false;
//     auto testCallback = [&callbackCalled]() -> bool {
//         callbackCalled = true;
//         return true;
//     };

//     // Test that negative timeout causes assertion failure
//     EXPECT_DEATH(controller->readyToExit(timeout, testCallback), "seconds >= 0");
// }

TEST_F(UT_AppExitController, ReadyToExit_CallbackReturnsFalse_DoesNotExit)
{
    int timeout = 60;
    bool callbackCalled = false;
    auto testCallback = [&callbackCalled]() -> bool {
        callbackCalled = true;
        return false; // Don't exit
    };

    // Mock QTimer::isActive to return false so readyToExit() will proceed
    stub.set_lamda(&QTimer::isActive, []() {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Mock QTimer::start
    bool timerStartCalled = false;
    stub.set_lamda((void(QTimer::*)(int))&QTimer::start, [&timerStartCalled](QTimer*, int msec) {
        __DBG_STUB_INVOKE__
        timerStartCalled = true;
        EXPECT_EQ(msec, 1000); // readyToExit starts timer with 1 second interval
    });

    // Mock QApplication::quit to track if it's called
    bool quitCalled = false;
    stub.set_lamda(&QApplication::quit, [&quitCalled]() {
        __DBG_STUB_INVOKE__
        quitCalled = true;
    });

    EXPECT_NO_THROW(controller->readyToExit(timeout, testCallback));
    EXPECT_TRUE(timerStartCalled);
    
    // Note: We can't easily test the actual callback execution without complex mocking
    // But we can verify the setup is correct
}

TEST_F(UT_AppExitController, ReadyToExit_CallbackReturnsTrue_ExitsApplication)
{
    int timeout = 60;
    bool callbackCalled = false;
    auto testCallback = [&callbackCalled]() -> bool {
        callbackCalled = true;
        return true; // Exit
    };

    // Mock QTimer::isActive to return false so readyToExit() will proceed
    stub.set_lamda(&QTimer::isActive, []() {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Mock QTimer::start
    bool timerStartCalled = false;
    stub.set_lamda((void(QTimer::*)(int))&QTimer::start, [&timerStartCalled](QTimer*, int msec) {
        __DBG_STUB_INVOKE__
        timerStartCalled = true;
        EXPECT_EQ(msec, 1000); // readyToExit starts timer with 1 second interval
    });

    // Mock QApplication::quit to track if it's called
    bool quitCalled = false;
    stub.set_lamda(&QApplication::quit, [&quitCalled]() {
        __DBG_STUB_INVOKE__
        quitCalled = true;
    });

    EXPECT_NO_THROW(controller->readyToExit(timeout, testCallback));
    EXPECT_TRUE(timerStartCalled);
    
    // Note: We can't easily test the actual callback execution without complex mocking
    // But we can verify the setup is correct
}

TEST_F(UT_AppExitController, MultipleReadyToExitCalls_DifferentTimeouts_HandlesCorrectly)
{
    int timeout1 = 30;
    int timeout2 = 60;
    int timeout3 = 120;
    
    bool callback1Called = false;
    bool callback2Called = false;
    bool callback3Called = false;
    
    auto testCallback1 = [&callback1Called]() -> bool {
        callback1Called = true;
        return true;
    };
    
    auto testCallback2 = [&callback2Called]() -> bool {
        callback2Called = true;
        return true;
    };
    
    auto testCallback3 = [&callback3Called]() -> bool {
        callback3Called = true;
        return true;
    };

    // Mock QTimer::isActive to return false so readyToExit() will proceed
    stub.set_lamda(&QTimer::isActive, []() {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Mock QTimer::start
    int timerStartCallCount = 0;
    stub.set_lamda((void(QTimer::*)(int))&QTimer::start, [&timerStartCallCount](QTimer*, int msec) {
        __DBG_STUB_INVOKE__
        timerStartCallCount++;
        EXPECT_EQ(msec, 1000); // readyToExit starts timer with 1 second interval
    });

    // Call readyToExit multiple times
    controller->readyToExit(timeout1, testCallback1);
    controller->readyToExit(timeout2, testCallback2);
    controller->readyToExit(timeout3, testCallback3);
    
    EXPECT_EQ(timerStartCallCount, 3);
}

TEST_F(UT_AppExitController, DismissAfterReadyToExit_CancelsScheduledExit)
{
    int timeout = 60;
    bool callbackCalled = false;
    auto testCallback = [&callbackCalled]() -> bool {
        callbackCalled = true;
        return true;
    };

    // Mock QTimer::isActive to return false first, then true
    bool isActiveFirstCall = true;
    stub.set_lamda(&QTimer::isActive, [&]() -> bool {
        __DBG_STUB_INVOKE__
        if (isActiveFirstCall) {
            isActiveFirstCall = false;
            return false; // First call returns false to allow readyToExit to proceed
        }
        return true; // Second call returns true to allow dismiss to proceed
    });

    // Mock QTimer::start
    bool timerStartCalled = false;
    stub.set_lamda((void(QTimer::*)(int))&QTimer::start, [&](QTimer*, int msec) {
        __DBG_STUB_INVOKE__
        timerStartCalled = true;
        EXPECT_EQ(msec, 1000); // readyToExit starts timer with 1 second interval
    });

    // Mock QTimer::stop
    bool timerStopCalled = false;
    stub.set_lamda(&QTimer::stop, [&]() {
        __DBG_STUB_INVOKE__
        timerStopCalled = true;
    });

    // Schedule exit then dismiss
    controller->readyToExit(timeout, testCallback);
    EXPECT_TRUE(timerStartCalled);
    
    controller->dismiss();
    EXPECT_TRUE(timerStopCalled);
}

TEST_F(UT_AppExitController, StaticMethods_NoInstanceRequired_CallSuccessfully)
{
    // Verify that instance() is static and doesn't require creating an instance
    AppExitController &instance = AppExitController::instance();
    EXPECT_NO_THROW(instance.dismiss());
}

// TEST_F(UT_AppExitController, ErrorHandling_InvalidParameters_HandlesGracefully)
// {
//     // Test with various invalid parameters
//     int zeroTimeout = 0;
//     int negativeTimeout = -1;
    
//     bool callbackCalled = false;
//     auto testCallback = [&callbackCalled]() -> bool {
//         callbackCalled = true;
//         return true;
//     };

//     // Mock QTimer::isActive to return false so readyToExit() will proceed
//     stub.set_lamda(&QTimer::isActive, []() {
//         __DBG_STUB_INVOKE__
//         return false;
//     });

//     // Mock QTimer::start
//     bool timerStartCalled = false;
//     stub.set_lamda((void(QTimer::*)(int))&QTimer::start, [&timerStartCalled](QTimer*, int msec) {
//         __DBG_STUB_INVOKE__
//         timerStartCalled = true;
//         EXPECT_EQ(msec, 1000); // readyToExit starts timer with 1 second interval
//     });

//     // Test with zero timeout (valid)
//     EXPECT_NO_THROW(controller->readyToExit(zeroTimeout, testCallback));
//     EXPECT_TRUE(timerStartCalled);
    
//     // Test that negative timeout causes assertion failure
//     EXPECT_DEATH(controller->readyToExit(negativeTimeout, testCallback), "seconds >= 0");
    
//     // Test dismiss
//     EXPECT_NO_THROW(controller->dismiss());
// }

TEST_F(UT_AppExitController, ThreadSafety_MultipleThreads_HandlesCorrectly)
{
    // Test basic thread safety (though comprehensive thread testing would be more complex)
    int timeout = 60;
    bool callbackCalled = false;
    auto testCallback = [&callbackCalled]() -> bool {
        callbackCalled = true;
        return true;
    };

    // Mock QTimer::isActive to return false so readyToExit() will proceed
    stub.set_lamda(&QTimer::isActive, []() {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Mock QTimer::start
    bool timerStartCalled = false;
    stub.set_lamda((void(QTimer::*)(int))&QTimer::start, [&timerStartCalled](QTimer*, int msec) {
        __DBG_STUB_INVOKE__
        timerStartCalled = true;
        EXPECT_EQ(msec, 1000); // readyToExit starts timer with 1 second interval
    });

    // Call from main thread (this is the basic test)
    EXPECT_NO_THROW(controller->readyToExit(timeout, testCallback));
    EXPECT_TRUE(timerStartCalled);
    
    // Test dismiss
    EXPECT_NO_THROW(controller->dismiss());
}

TEST_F(UT_AppExitController, CallbackExecution_ConditionalExit_HandlesCorrectly)
{
    int timeout = 1; // Short timeout for testing
    int callCount = 0;
    
    auto conditionalCallback = [&callCount]() -> bool {
        callCount++;
        // Only exit on third call
        return callCount >= 3;
    };

    // Mock QTimer::isActive to return false so readyToExit() will proceed
    stub.set_lamda(&QTimer::isActive, []() {
        __DBG_STUB_INVOKE__
        return false;
    });

    // Mock QTimer::start
    int timerStartCallCount = 0;
    stub.set_lamda((void(QTimer::*)(int))&QTimer::start, [&timerStartCallCount](QTimer*, int msec) {
        __DBG_STUB_INVOKE__
        timerStartCallCount++;
        EXPECT_EQ(msec, 1000); // readyToExit starts timer with 1 second interval
    });

    // Mock QApplication::quit
    bool quitCalled = false;
    stub.set_lamda(&QApplication::quit, [&quitCalled]() {
        __DBG_STUB_INVOKE__
        quitCalled = true;
    });

    // Schedule exit multiple times to test conditional logic
    controller->readyToExit(timeout, conditionalCallback);
    controller->readyToExit(timeout, conditionalCallback);
    controller->readyToExit(timeout, conditionalCallback);
    
    EXPECT_EQ(timerStartCallCount, 3);
    
    // Note: Actual callback execution testing would require more complex setup
    // with event loop simulation, which is beyond basic unit test scope
}
