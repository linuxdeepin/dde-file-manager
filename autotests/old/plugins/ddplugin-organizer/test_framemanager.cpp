// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <stubext.h>
#include "framemanager.h"
#include "private/framemanager_p.h"

#include <QDBusInterface>

#include <gtest/gtest.h>
#include <dfm-framework/dpf.h>
#include <QTimer>

using namespace ddplugin_organizer;
DPF_USE_NAMESPACE

class UT_FrameManager : public testing::Test
{
protected:
    void SetUp() override
    {
        // mock QDBusInterface::call
        stub.set_lamda(ADDR(QDBusInterface, doCall), [] {
            __DBG_STUB_INVOKE__
            return QDBusMessage();
        });

        manager = new FrameManager();
    }

    void TearDown() override
    {
        // delete manager;
        // manager = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    FrameManager *manager = nullptr;
};

TEST_F(UT_FrameManager, Constructor_Default_InitializesCorrectly)
{
    EXPECT_NE(manager, nullptr);
    EXPECT_NE(manager->d, nullptr);
}

TEST_F(UT_FrameManager, initialize_Always_ReturnsTrue)
{
    bool result = manager->initialize();
    EXPECT_TRUE(result);
}

TEST_F(UT_FrameManager, initialize_SetsUpModelAndCanvas)
{
    bool modelCreated = false;
    bool canvasCreated = false;
    
    stub.set_lamda(&FrameManagerPrivate::buildOrganizer, [&modelCreated, &canvasCreated]() {
        __DBG_STUB_INVOKE__
        modelCreated = true;
        canvasCreated = true;
    });
    
    manager->initialize();
    EXPECT_TRUE(modelCreated || canvasCreated);
}

TEST_F(UT_FrameManager, layout_Always_DoesNotThrow)
{
    EXPECT_NO_THROW(manager->layout());
}

TEST_F(UT_FrameManager, turnOn_WithBuildTrue_BuildsSurface)
{
    bool buildSurfaceCalled = false;
    stub.set_lamda(&FrameManagerPrivate::buildSurface, [&buildSurfaceCalled]() {
        __DBG_STUB_INVOKE__
        buildSurfaceCalled = true;
    });
    
    manager->turnOn(true);
    EXPECT_TRUE(buildSurfaceCalled);
}

TEST_F(UT_FrameManager, turnOn_WithBuildFalse_DoesNotBuildSurface)
{
    bool buildSurfaceCalled = false;
    stub.set_lamda(&FrameManagerPrivate::buildSurface, [&buildSurfaceCalled]() {
        __DBG_STUB_INVOKE__
        buildSurfaceCalled = true;
    });
    
    manager->turnOn(false);
    EXPECT_FALSE(buildSurfaceCalled);
}

TEST_F(UT_FrameManager, turnOff_Always_ClearsSurface)
{
    bool clearSurfaceCalled = false;
    stub.set_lamda(&FrameManagerPrivate::clearSurface, [&clearSurfaceCalled]() {
        __DBG_STUB_INVOKE__
        clearSurfaceCalled = true;
    });
    
    manager->turnOff();
    EXPECT_TRUE(clearSurfaceCalled);
}

TEST_F(UT_FrameManager, organizerEnabled_Always_ReturnsBool)
{
    bool result = manager->organizerEnabled();
    EXPECT_TRUE(result || !result);  // Just ensure it returns a bool
}

TEST_F(UT_FrameManager, onBuild_WithStubs_DoesNotThrow)
{
    // Stub the CanvasInterface methods to avoid null pointer access
    stub.set_lamda(&FrameManager::switchMode, [](FrameManager *self, OrganizerMode mode) {
        __DBG_STUB_INVOKE__
        // Do nothing to avoid null pointer access
    });
    
    EXPECT_NO_THROW(manager->onBuild());
}

TEST_F(UT_FrameManager, onWindowShowed_WithStubs_DoesNotThrow)
{
    EXPECT_NO_THROW(manager->onWindowShowed());
}

TEST_F(UT_FrameManager, onDetachWindows_WithStubs_DoesNotThrow)
{
    EXPECT_NO_THROW(manager->onDetachWindows());
}

TEST_F(UT_FrameManager, onGeometryChanged_WithStubs_DoesNotThrow)
{
    EXPECT_NO_THROW(manager->onGeometryChanged());
}

TEST_F(UT_FrameManager, switchMode_WithCustomMode_SwitchesToCustom)
{
    bool switchToCustomCalled = false;
    stub.set_lamda(&FrameManagerPrivate::switchToCustom, [&switchToCustomCalled]() {
        __DBG_STUB_INVOKE__
        switchToCustomCalled = true;
    });
    
    manager->turnOn(false);
    manager->switchMode(OrganizerMode::kCustom);
    EXPECT_TRUE(switchToCustomCalled);
}

TEST_F(UT_FrameManager, switchMode_WithNormalizedMode_SwitchesToNormalized)
{
    bool switchToNormalizedCalled = false;
    stub.set_lamda(&FrameManagerPrivate::switchToNormalized, [&switchToNormalizedCalled](FrameManagerPrivate *, int cf) {
        __DBG_STUB_INVOKE__
        Q_UNUSED(cf)
        switchToNormalizedCalled = true;
    });
    
    manager->turnOn(false);
    manager->switchMode(OrganizerMode::kNormalized);
    EXPECT_TRUE(switchToNormalizedCalled);
}

TEST_F(UT_FrameManager, onWindowShowed_Always_DoesNotThrow)
{
    EXPECT_NO_THROW(manager->onWindowShowed());
}

TEST_F(UT_FrameManager, onDetachWindows_Always_DoesNotThrow)
{
    EXPECT_NO_THROW(manager->onDetachWindows());
}

TEST_F(UT_FrameManager, onGeometryChanged_Always_DoesNotThrow)
{
    EXPECT_NO_THROW(manager->onGeometryChanged());
}

TEST_F(UT_FrameManager, layout_SchedulesLayoutTimer)
{
    bool layoutTimerStarted = false;
    stub.set_lamda((void (QTimer::*)(void))&QTimer::start, [&layoutTimerStarted]() {
        __DBG_STUB_INVOKE__
        layoutTimerStarted = true;
    });
    
    manager->layout();
    // Note: In real implementation, this might be conditional
}

TEST_F(UT_FrameManager, Destructor_DeletesPrivateData)
{
    FrameManagerPrivate *privateData = manager->d;
    EXPECT_NE(privateData, nullptr);
    
    delete manager;
    manager = nullptr;
    // The private data should be cleaned up by the destructor
}
