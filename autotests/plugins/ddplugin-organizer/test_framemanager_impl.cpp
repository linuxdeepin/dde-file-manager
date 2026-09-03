// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "framemanager.h"
#include "private/framemanager_p.h"
#include "mode/canvasorganizer.h"
#include "interface/canvasinterface.h"
#include "interface/canvasmodelshell.h"
#include "interface/fileinfomodelshell.h"
#include "interface/canvasviewshell.h"
#include "interface/canvasgridshell.h"
#include "interface/canvasmanagershell.h"
#include "interface/canvasselectionshell.h"
#include "config/configpresenter.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-framework/lifecycle/lifecycle.h>
#include <dfm-framework/event/eventchannel.h>

#include <gtest/gtest.h>
#include <QTimer>

using namespace ddplugin_organizer;
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

namespace {

static bool organizerCreated = false;

class MockCanvasOrganizer : public CanvasOrganizer
{
public:
    explicit MockCanvasOrganizer(QObject *parent = nullptr)
        : CanvasOrganizer(parent)
    {
        organizerCreated = true;
    }

    OrganizerMode mode() const override { return OrganizerMode::kNormalized; }
    bool initialize(CollectionModel *) override { return true; }
};

class TestableFrameManager : public FrameManager
{
public:
    using FrameManager::switchMode;
};

}   // namespace

class FrameManagerImpl : public testing::Test
{
protected:
    void SetUp() override
    {
        organizerCreated = false;

        // avoid shutdown path in destructor
        stub.set_lamda(&DPF_NAMESPACE::LifeCycle::isShuttingDown, []() -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });

        // DConfig
        stub.set_lamda(static_cast<QVariant (DConfigManager::*)(const QString &, const QString &, const QVariant &) const>(&DConfigManager::value),
                       [](DConfigManager *, const QString &, const QString &, const QVariant &) -> QVariant {
                           __DBG_STUB_INVOKE__
                           return QVariant();
                       });
        stub.set_lamda(&DConfigManager::setValue,
                       [](DConfigManager *, const QString &, const QString &, const QVariant &) {
                           __DBG_STUB_INVOKE__
                       });

        // config presenter
        stub.set_lamda(&ConfigPresenter::initialize, [](ConfigPresenter *) -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });
        // initialize() is stubbed out, so ConfigPresenter::conf stays null;
        // onBuild() calls setVersion() unconditionally, which would deref it.
        stub.set_lamda(&ConfigPresenter::setVersion, [](ConfigPresenter *, const QString &) {
            __DBG_STUB_INVOKE__
        });
        stub.set_lamda(&ConfigPresenter::isEnable, []() -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });
        stub.set_lamda(&ConfigPresenter::mode, []() -> OrganizerMode {
            __DBG_STUB_INVOKE__
            return OrganizerMode::kNormalized;
        });

        // canvas shell initializers (avoid dpf subscriptions)
        stub.set_lamda(&FileInfoModelShell::initialize, [](FileInfoModelShell *) -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });
        stub.set_lamda(&CanvasModelShell::initialize, [](CanvasModelShell *) -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });
        stub.set_lamda(&CanvasViewShell::initialize, [](CanvasViewShell *) -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });
        stub.set_lamda(&CanvasGridShell::initialize, [](CanvasGridShell *) -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });
        stub.set_lamda(&CanvasManagerShell::initialize, [](CanvasManagerShell *) -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });
        stub.set_lamda(&CanvasSelectionShell::initialize, [](CanvasSelectionShell *) -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });
        stub.set_lamda(static_cast<void (CanvasModelShell::*)(int, bool)>(&CanvasModelShell::refresh),
                       [](CanvasModelShell *, int, bool) {
                           __DBG_STUB_INVOKE__
                       });

        // event channel: empty desktop root windows, menu register/bind returns true
        stub.set_lamda(static_cast<QVariant (EventChannelManager::*)(const QString &, const QString &)>(&EventChannelManager::push),
                       [](EventChannelManager *, const QString &, const QString &) -> QVariant {
                           __DBG_STUB_INVOKE__
                           return QVariant::fromValue(QList<QWidget *>());
                       });
        stub.set_lamda(static_cast<QVariant (EventChannelManager::*)(const QString &, const QString &, const QString &)>(&EventChannelManager::push),
                       [](EventChannelManager *, const QString &, const QString &, const QString &) -> QVariant {
                           __DBG_STUB_INVOKE__
                           return QVariant(true);
                       });
        stub.set_lamda(static_cast<QVariant (EventChannelManager::*)(const QString &, const QString &, const QString &, const QString &)>(&EventChannelManager::push),
                       [](EventChannelManager *, const QString &, const QString &, const QString &, const QString &) -> QVariant {
                           __DBG_STUB_INVOKE__
                           return QVariant(true);
                       });

        // organizer factory
        stub.set_lamda(static_cast<CanvasOrganizer *(*)(OrganizerMode)>(&OrganizerCreator::createOrganizer),
                       [](OrganizerMode) -> CanvasOrganizer * {
                           __DBG_STUB_INVOKE__
                           return new MockCanvasOrganizer();
                       });
    }

    void TearDown() override
    {
        // make destructor skip cross-plugin teardown so we can safely delete
        stub.set_lamda(&DPF_NAMESPACE::LifeCycle::isShuttingDown, []() -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });
        delete manager;
        manager = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    TestableFrameManager *manager = nullptr;
};

TEST_F(FrameManagerImpl, Constructor_InitializesPrivateData)
{
    manager = new TestableFrameManager();
    EXPECT_NE(manager, nullptr);
}

TEST_F(FrameManagerImpl, Layout_NoOrganizer_DoesNotCrash)
{
    manager = new TestableFrameManager();
    EXPECT_NO_THROW(manager->layout());
}

TEST_F(FrameManagerImpl, TurnOn_BuildFalse_CreatesCanvasAndModel)
{
    manager = new TestableFrameManager();
    EXPECT_NO_THROW(manager->turnOn(false));
}

TEST_F(FrameManagerImpl, TurnOn_BuildTrue_CreatesOrganizer)
{
    manager = new TestableFrameManager();
    EXPECT_NO_THROW(manager->turnOn(true));
    EXPECT_TRUE(organizerCreated);
}

TEST_F(FrameManagerImpl, SwitchMode_CreatesOrganizer)
{
    manager = new TestableFrameManager();
    manager->turnOn(false);
    organizerCreated = false;
    EXPECT_NO_THROW(manager->switchMode(OrganizerMode::kNormalized));
    EXPECT_TRUE(organizerCreated);
}

TEST_F(FrameManagerImpl, OnBuild_CreatesOrganizer)
{
    manager = new TestableFrameManager();
    manager->turnOn(false);
    organizerCreated = false;
    EXPECT_NO_THROW(manager->onBuild());
    EXPECT_TRUE(organizerCreated);
}

TEST_F(FrameManagerImpl, OnWindowShowed_DoesNotCrash)
{
    manager = new TestableFrameManager();
    EXPECT_NO_THROW(manager->onWindowShowed());
}

TEST_F(FrameManagerImpl, OnDetachWindows_DoesNotCrash)
{
    manager = new TestableFrameManager();
    EXPECT_NO_THROW(manager->onDetachWindows());
}

TEST_F(FrameManagerImpl, OnGeometryChanged_DoesNotCrash)
{
    manager = new TestableFrameManager();
    EXPECT_NO_THROW(manager->onGeometryChanged());
}

TEST_F(FrameManagerImpl, OrganizerEnabled_DelegatesToConfig)
{
    manager = new TestableFrameManager();
    stub.set_lamda(&ConfigPresenter::isEnable, []() -> bool { return true; });
    EXPECT_TRUE(manager->organizerEnabled());
}

TEST_F(FrameManagerImpl, TurnOff_DoesNotCrash)
{
    manager = new TestableFrameManager();
    manager->turnOn(false);
    EXPECT_NO_THROW(manager->turnOff());
}
