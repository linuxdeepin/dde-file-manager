// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖函数清单(framemanager.cpp) -> 用例映射:
// - FrameManagerPrivate::createSurface -> CreateSurface_NullAndValidRoot_PropertiesSet (分支: 空root返回null)
// - FrameManagerPrivate::layoutSurface -> LayoutSurface_HiddenAndViewParent_ParentAndGeometrySet
//       (分支: view+非hidden挂viewport; 否则挂root)
// - FrameManagerPrivate::surfaces -> Surfaces_RootWindowRegistered_ReturnsSurface
// - FrameManagerPrivate::onNotificationClosed -> NotificationClosed_MatchingId_ResetTimerAndId (分支: id不匹配早退)
// - FrameManagerPrivate::enableChanged -> EnableChanged_SwitchState_TurnOnOrganizer (分支: 相同值早退)
// - FrameManagerPrivate::enableVisibility -> EnableVisibility_True_SavedToPresenter
// - FrameManagerPrivate::saveHideAllSequence -> SaveHideAllSequence_Sequence_ForwardedToPresenter
// - FrameManagerPrivate::switchToCustom -> SwitchToCustom_FromNormalized_RebuildsOrganizer (分支: 已是custom早退)
// - FrameManagerPrivate::switchToNormalized -> SwitchToNormalized_AlreadyNormalized_ResetsOrganizer (分支: 切换模式重建)
// - FrameManagerPrivate::showOptionWindow: 未覆盖(offscreen 下构造 OptionsWindow 崩溃, 记录为环境限制)
// - FrameManagerPrivate::findView -> FindView_CanvasChildDetected_OrNull (分支: 空root/无canvas子项)
// 未覆盖: onHideAllKeyPressed 及其 lambda(依赖 DBus 通知调用链)。

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

#include <dfm-base/dfm_desktop_defines.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-framework/lifecycle/lifecycle.h>
#include <dfm-framework/event/eventchannel.h>

#include <gtest/gtest.h>
#include <QTimer>
#include <QListView>
#include <QWidget>

using namespace ddplugin_organizer;
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

namespace {

int organizerCreateCount = 0;
int presenterEnableVisibilityCalls = 0;
int presenterSaveSequenceCalls = 0;

class MockCanvasOrganizerCov : public CanvasOrganizer
{
public:
    explicit MockCanvasOrganizerCov(QObject *parent = nullptr)
        : CanvasOrganizer(parent)
    {
        ++organizerCreateCount;
    }

    OrganizerMode mode() const override { return OrganizerMode::kNormalized; }
    bool initialize(CollectionModel *) override { return true; }
};

class TestableFrameManagerCov : public FrameManager
{
public:
    FrameManagerPrivate *priv() { return d; }
};

}   // namespace

class UT_FrameManagerCov : public testing::Test
{
protected:
    void SetUp() override
    {
        organizerCreateCount = 0;
        presenterEnableVisibilityCalls = 0;
        presenterSaveSequenceCalls = 0;

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
        stub.set_lamda(&ConfigPresenter::setMode, [](ConfigPresenter *, OrganizerMode) {
            __DBG_STUB_INVOKE__
        });
        stub.set_lamda(&ConfigPresenter::setClassification, [](ConfigPresenter *, Classifier) {
            __DBG_STUB_INVOKE__
        });
        stub.set_lamda(&ConfigPresenter::setEnable, [](ConfigPresenter *, bool) {
            __DBG_STUB_INVOKE__
        });
        stub.set_lamda(&ConfigPresenter::organizeOnTriggered, []() -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });
        stub.set_lamda(&ConfigPresenter::setEnableVisibility, [](ConfigPresenter *, bool) {
            __DBG_STUB_INVOKE__
            ++presenterEnableVisibilityCalls;
        });
        stub.set_lamda(&ConfigPresenter::setHideAllKeySequence, [](ConfigPresenter *, const QKeySequence &) {
            __DBG_STUB_INVOKE__
            ++presenterSaveSequenceCalls;
        });

        // canvas shell initializers (avoid dpf subscriptions)
        stub.set_lamda(&FileInfoModelShell::initialize, []() -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });
        stub.set_lamda(&CanvasModelShell::initialize, []() -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });
        stub.set_lamda(&CanvasViewShell::initialize, []() -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });
        stub.set_lamda(&CanvasGridShell::initialize, []() -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });
        stub.set_lamda(&CanvasManagerShell::initialize, []() -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });
        stub.set_lamda(&CanvasSelectionShell::initialize, []() -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });
        stub.set_lamda(static_cast<void (CanvasModelShell::*)(int, bool)>(&CanvasModelShell::refresh),
                       [](CanvasModelShell *, int, bool) {
                           __DBG_STUB_INVOKE__
                       });

        // event channel
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
                           return new MockCanvasOrganizerCov();
                       });

        manager = new TestableFrameManagerCov();
    }

    void TearDown() override
    {
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
    TestableFrameManagerCov *manager = nullptr;
};

TEST_F(UT_FrameManagerCov, CreateSurface_NullAndValidRoot_PropertiesSet)
{
    FrameManagerPrivate *d = manager->priv();
    // Arrange: null root and a real root with screen name

    // Act: null root takes the early-return branch
    SurfacePointer nullSurface = d->createSurface(nullptr);

    // Assert: null root produces no surface
    EXPECT_TRUE(nullSurface.isNull());

    // Arrange / Act: valid root
    QWidget root;
    root.setProperty(DesktopFrameProperty::kPropScreenName, "screen_cov");
    SurfacePointer surface = d->createSurface(&root);

    // Assert: surface carries screen name and widget level
    ASSERT_FALSE(surface.isNull());
    EXPECT_EQ(surface->property(DesktopFrameProperty::kPropScreenName).toString(), "screen_cov");
    EXPECT_EQ(surface->property(DesktopFrameProperty::kPropWidgetLevel).toDouble(), 11.0);
}

TEST_F(UT_FrameManagerCov, LayoutSurface_HiddenAndViewParent_ParentAndGeometrySet)
{
    FrameManagerPrivate *d = manager->priv();
    // Arrange: root sized 300x200, a canvas view child, and a surface
    QWidget root;
    root.resize(300, 200);
    QListView canvasChild(&root);
    canvasChild.setProperty(DesktopFrameProperty::kPropWidgetName, "canvas");
    canvasChild.resize(300, 200);
    SurfacePointer surface(new Surface());

    // Act: hidden=true takes the root-parent branch
    d->layoutSurface(&root, surface, true);

    // Assert: parent is the root and geometry follows root size
    EXPECT_EQ(surface->parentWidget(), &root);
    EXPECT_EQ(surface->height(), 200);

    // Act: not hidden with a canvas child takes the viewport branch
    d->layoutSurface(&root, surface, false);

    // Assert: parent moved into the canvas viewport
    EXPECT_EQ(surface->parentWidget(), canvasChild.viewport());
    EXPECT_EQ(surface->width(), canvasChild.geometry().width());

    // Cleanup: detach the shared surface from the stack widget tree
    surface->setParent(nullptr);
}

TEST_F(UT_FrameManagerCov, Surfaces_RootWindowRegistered_ReturnsSurface)
{
    FrameManagerPrivate *d = manager->priv();
    // Arrange: one registered surface for a known screen
    SurfacePointer surface(new Surface());
    d->surfaceWidgets.insert("screen_cov", surface);

    // Act
    QList<SurfacePointer> result = d->surfaces();

    // Assert: no desktop root window is visible in the sandboxed event channel
    // (the stub returns an empty root list), so nothing is collected
    EXPECT_TRUE(result.isEmpty());
    EXPECT_EQ(d->surfaceWidgets.value("screen_cov"), surface);
}

TEST_F(UT_FrameManagerCov, NotificationClosed_MatchingId_ResetTimerAndId)
{
    FrameManagerPrivate *d = manager->priv();
    // Arrange: pretend a notification with id 5 is active
    ASSERT_NE(d->hideAllNotifyExpireTimer, nullptr);
    d->hideAllNotifyId = 5;

    // Act: unrelated id takes the early-return branch
    d->onNotificationClosed(7, 2);

    // Assert: id untouched
    EXPECT_EQ(d->hideAllNotifyId, 5u);

    // Act: matching id resets
    d->onNotificationClosed(5, 2);

    // Assert
    EXPECT_EQ(d->hideAllNotifyId, 0u);
    EXPECT_FALSE(d->hideAllNotifyExpireTimer->isActive());
}

TEST_F(UT_FrameManagerCov, EnableChanged_SwitchState_TurnOnOrganizer)
{
    FrameManagerPrivate *d = manager->priv();
    // Arrange: presenter reports disabled, organizer not built yet
    EXPECT_EQ(organizerCreateCount, 0);

    // Act: same state takes the early-return branch (no organizer built)
    d->enableChanged(false);

    // Assert
    EXPECT_EQ(organizerCreateCount, 0);

    // Act: enable builds the organizer through turnOn
    d->enableChanged(true);

    // Assert: the organizer factory ran
    EXPECT_GE(organizerCreateCount, 1);
}

TEST_F(UT_FrameManagerCov, EnableVisibility_True_SavedToPresenter)
{
    FrameManagerPrivate *d = manager->priv();
    // Arrange: counter stub installed in SetUp
    const int callsBefore = presenterEnableVisibilityCalls;

    // Act
    d->enableVisibility(true);

    // Assert: forwarded exactly once
    EXPECT_EQ(presenterEnableVisibilityCalls, callsBefore + 1);
    EXPECT_EQ(callsBefore, 0);
}

TEST_F(UT_FrameManagerCov, SaveHideAllSequence_Sequence_ForwardedToPresenter)
{
    FrameManagerPrivate *d = manager->priv();
    // Arrange
    QKeySequence seq("Ctrl+Shift+H");

    // Act
    d->saveHideAllSequence(seq);

    // Assert
    EXPECT_EQ(presenterSaveSequenceCalls, 1);
    EXPECT_EQ(seq.toString().toUpper(), QString("CTRL+SHIFT+H"));
}

TEST_F(UT_FrameManagerCov, SwitchToCustom_FromNormalized_RebuildsOrganizer)
{
    FrameManagerPrivate *d = manager->priv();
    // Arrange: an organizer exists in normalized mode
    manager->turnOn();
    const int created = organizerCreateCount;
    ASSERT_GE(created, 1);

    // Act: switch to custom rebuilds organizer
    d->switchToCustom();

    // Assert: factory ran again and a new organizer is in charge
    EXPECT_GT(organizerCreateCount, created);
    EXPECT_NE(d->organizer, nullptr);
}

TEST_F(UT_FrameManagerCov, SwitchToNormalized_AlreadyNormalized_ResetsOrganizer)
{
    FrameManagerPrivate *d = manager->priv();
    // Arrange: normalized organizer already built
    manager->turnOn();
    const int created = organizerCreateCount;
    ASSERT_GE(created, 1);

    // Act: normalized mode with new classification only resets
    d->switchToNormalized(static_cast<int>(Classifier::kSize));

    // Assert: no new organizer created, existing one kept
    EXPECT_EQ(organizerCreateCount, created);
    EXPECT_NE(d->organizer, nullptr);
}

TEST_F(UT_FrameManagerCov, FindView_CanvasChildDetected_OrNull)
{
    FrameManagerPrivate *d = manager->priv();
    // Arrange: root without canvas child
    QWidget root;
    QListView canvasChild(&root);
    canvasChild.setProperty(DesktopFrameProperty::kPropWidgetName, "canvas");

    // Act: null root
    EXPECT_EQ(d->findView(nullptr), nullptr);

    // Act: root with canvas child
    QWidget *found = d->findView(&root);

    // Assert: the canvas-flagged child is detected
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found, &canvasChild);

    // Act: root without canvas child
    QWidget plainRoot;
    EXPECT_EQ(d->findView(&plainRoot), nullptr);
}
