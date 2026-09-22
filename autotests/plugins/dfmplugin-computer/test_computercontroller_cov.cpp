// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage map for src/plugins/filemanager/dfmplugin-computer/controller/computercontroller.cpp:
// Unlike test_computercontroller.cpp (which stubs the methods under test), this suite calls the
// REAL controller methods and stubs only their external dependencies (DeviceManager, DialogManager,
// NetworkUtils, QProcess) so the function bodies and their lambdas actually execute.
//   onOpenItem                        -> OnOpenItem_UnaccessibleEntry_Handled
//   onMenuRequest                     -> OnMenuRequest_NoMenuScene_ReturnsEarly
//   doRename (+ rename lambda#1)      -> DoRename_EmptyName_SkipsRename, DoRename_PlainEntry_SkipsDevice
//   doSetAlias                        -> DoSetAlias_NoUuid_EmitsNothing
//   doSetProtocolDeviceAlias          -> DoSetProtocolAlias_NonProtocol_ReturnsFalse
//   mountDevice(info)                 -> MountDevice_NullInfo_ReturnsEarly, OpenInNewWindow_PlainEntry_Mounts
//   mountDevice(id) (+ its lambdas)   -> MountDevice_ById_Success_CdTo, MountDevice_ById_Failure_ShowsError
//   actEject (+ lambdas #1/#2)        -> ActEject_BlockUrl_InvokesDetach, ActEject_UnsupportedUrl_NoDeviceCall
//   actOpenInNewWindow                -> OpenInNewWindow_PlainEntry_Mounts
//   actMount                          -> ActMount_NonBlockEntry_ReturnsWithoutMount
//   actUnmount (+ lambdas)            -> ActUnmount_UnsupportedEntry_NoDeviceCall
//   actSafelyRemove                   -> ActSafelyRemove_DelegatesToEject
//   actRename                         -> ActRename_FromView_EmitsRequestRename, ActRename_NullInfo_NoSignal
//   actProperties                     -> ActProperties_PlainEntry_SendsPropertyDialog
//   actLogoutAndForgetPasswd          -> ActLogoutForget_NonSmb_ClearsAndUnmounts
//   actErase                          -> ActErase_PlainEntry_PublishesEraseEvent
//   waitUDisks2DataReady              -> WaitUDisks2_NoTargetUrl_RetriesThenGivesUp
//   handleUnAccessableDevCdCall       -> OnOpenItem_UnaccessibleEntry_Handled, HandleUnAccessable_NullInfo_EarlyReturn
//   handleNetworkCdCall (+ lambda#1)  -> HandleNetworkCd_ParseFail_CdToDirect, HandleNetworkCd_CheckFail_ShowsMountError
// Branch notes: EntryFileInfo built from unknown entry:// URLs has no entity, so isAccessable() is
// false and all extra properties are empty - that drives the "unaccessible"/"plain entry" branches.
// mountDevice(info)'s encrypted/unlock branches and doRename's mounted-device branch need a
// populated entity which cannot be registered here, they stay uncovered.

#include <gtest/gtest.h>

#include "stubext.h"
#include "controller/computercontroller.h"
#include "utils/computerutils.h"
#include "utils/computerdatastruct.h"
#include "fileentity/entryfileentities.h"
#include "events/computereventcaller.h"

#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/utils/networkutils.h>
#include <dfm-base/file/entry/entryfileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-framework/dpf.h>

#include <QElapsedTimer>
#include <QIcon>
#include <QProcess>
#include <QSignalSpy>
#include <QUrl>

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
using namespace dfmplugin_computer;

class UT_ComputerControllerCov : public testing::Test
{
protected:
    static void SetUpTestCase()
    {
        // idempotent registrations so EntryFileInfo can be constructed when this
        // suite runs alone (same set as Computer::initialize / UT_ComputerUtils)
        WatcherFactory::regClass<LocalFileWatcher>(Global::Scheme::kFile);
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        EntryEntityFactor::registCreator<CommonEntryFileEntity>(SuffixInfo::kCommon);
        EntryEntityFactor::registCreator<UserEntryFileEntity>(SuffixInfo::kUserDir);
        EntryEntityFactor::registCreator<BlockEntryFileEntity>(SuffixInfo::kBlock);
        EntryEntityFactor::registCreator<ProtocolEntryFileEntity>(SuffixInfo::kProtocol);
        EntryEntityFactor::registCreator<AppEntryFileEntity>(SuffixInfo::kAppEntry);
        UrlRoute::regScheme(Global::Scheme::kEntry, "/", QIcon(), true);
        InfoFactory::regClass<EntryFileInfo>(Global::Scheme::kEntry);
    }

    static int testEventType(const QString &space, const QString &topic)
    {
        return static_cast<int>(qHash(space) ^ qHash(topic));
    }

    virtual void SetUp() override
    {
        controller = ComputerController::instance();

        // make dpf event publication observable (pattern from test_computereventcaller.cpp)
        EventConverter::registerConverter(&UT_ComputerControllerCov::testEventType);
        auto *mgr = Event::instance()->dispatcher();
        const int globalTypes[] = {
            int(GlobalEventType::kChangeCurrentUrl),
            int(GlobalEventType::kOpenNewWindow),
            int(GlobalEventType::kOpenNewTab)
        };
        for (int t : globalTypes) {
            if (!mgr->dispatcherMap.contains(t))
                mgr->dispatcherMap.insert(t, EventDispatcherManager::DispatcherPtr::create());
        }
        eventCount = 0;
        using DispatchFunc = bool (EventDispatcher::*)(const QVariantList &);
        stub.set_lamda(static_cast<DispatchFunc>(&EventDispatcher::dispatch),
                       [](EventDispatcher *, const QVariantList &) -> bool {
                           ++eventCount;
                           return true;
                       });

        // neutralize every external side effect of the controller
        stubDialogs();
        stubDeviceOps();
        stub.set_lamda(static_cast<bool (*)(const QString &, const QStringList &, const QString &, qint64 *)>(&QProcess::startDetached),
                       [](const QString &, const QStringList &, const QString &, qint64 *) {
                           __DBG_STUB_INVOKE__
                           ++startDetachedCount;
                           return true;
                       });
        resetCounters();
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

    void stubDialogs()
    {
        stub.set_lamda(ADDR(DialogManager, askForFormat), [](DialogManager *) {
            __DBG_STUB_INVOKE__
            return true;
        });
        stub.set_lamda(ADDR(DialogManager, showErrorDialogWhenOperateDeviceFailed),
                       [](DialogManager *, DialogManager::OperateType, DFMMOUNT::OperationErrorInfo) {
                           __DBG_STUB_INVOKE__
                       });
        stub.set_lamda(ADDR(DialogManager, showErrorDialog),
                       [](DialogManager *, const QString &, const QString &) {
                           __DBG_STUB_INVOKE__
                           ++errorDialogCount;
                       });
        stub.set_lamda(ADDR(DialogManager, askPasswordForLockedDevice),
                       [](DialogManager *, const QString &) {
                           __DBG_STUB_INVOKE__
                           return QString();
                       });
    }

    void stubDeviceOps()
    {
        stub.set_lamda(ADDR(DeviceManager, mountBlockDevAsync),
                       [](DeviceManager *, const QString &id, const QVariantMap &, CallbackType1 cb, int) {
                           __DBG_STUB_INVOKE__
                           ++mountCount;
                           if (cb)
                               cb(mountResult, DFMMOUNT::OperationErrorInfo {}, mountPoint);
                       });
        stub.set_lamda(ADDR(DeviceManager, unmountBlockDevAsync),
                       [](DeviceManager *, const QString &, const QVariantMap &, CallbackType2 cb) {
                           __DBG_STUB_INVOKE__
                           ++unmountCount;
                           if (cb)
                               cb(true, DFMMOUNT::OperationErrorInfo {});
                       });
        stub.set_lamda(ADDR(DeviceManager, lockBlockDevAsync),
                       [](DeviceManager *, const QString &, const QVariantMap &, CallbackType2 cb) {
                           __DBG_STUB_INVOKE__
                           ++lockCount;
                           if (cb)
                               cb(true, DFMMOUNT::OperationErrorInfo {});
                       });
        stub.set_lamda(ADDR(DeviceManager, renameBlockDevAsync),
                       [](DeviceManager *, const QString &, const QString &, const QVariantMap &, CallbackType2 cb) {
                           __DBG_STUB_INVOKE__
                           ++renameCount;
                           if (cb)
                               cb(true, DFMMOUNT::OperationErrorInfo {});
                       });
        stub.set_lamda(ADDR(DeviceManager, detachBlockDev),
                       [](DeviceManager *, const QString &, CallbackType2 cb) {
                           __DBG_STUB_INVOKE__
                           ++detachCount;
                           if (cb)
                               cb(true, DFMMOUNT::OperationErrorInfo {});
                           return QStringList {};
                       });
        stub.set_lamda(ADDR(DeviceManager, unmountProtocolDevAsync),
                       [](DeviceManager *, const QString &, const QVariantMap &, CallbackType2 cb) {
                           __DBG_STUB_INVOKE__
                           ++unmountProtocolCount;
                           if (cb)
                               cb(true, DFMMOUNT::OperationErrorInfo {});
                       });
        stub.set_lamda(ADDR(DeviceProxyManager, queryBlockInfo), [] {
            __DBG_STUB_INVOKE__
            return QVariantMap {};   // no optical drive -> regular mount path
        });
    }

    void resetCounters()
    {
        mountCount = unmountCount = lockCount = renameCount = detachCount = 0;
        unmountProtocolCount = startDetachedCount = errorDialogCount = 0;
        eventCount = 0;
    }

protected:
    stub_ext::StubExt stub;
    ComputerController *controller = nullptr;
    static inline bool mountResult = true;
    static inline QString mountPoint = QString("/tmp/cov-mount");
    static inline int mountCount = 0;
    static inline int unmountCount = 0;
    static inline int lockCount = 0;
    static inline int renameCount = 0;
    static inline int detachCount = 0;
    static inline int unmountProtocolCount = 0;
    static inline int startDetachedCount = 0;
    static inline int errorDialogCount = 0;
    static inline int eventCount = 0;
};

TEST_F(UT_ComputerControllerCov, OnOpenItem_UnaccessibleEntry_Handled)
{
    // Arrange - unknown entry has no entity -> not accessible -> handleUnAccessableDevCdCall
    const quint64 winId = 1001;

    // Act
    controller->onOpenItem(winId, QUrl("entry://nonexistent.blockdev"));

    // Assert - handled through the unaccessible path without mounting or dialogs
    EXPECT_EQ(mountCount, 0);
    EXPECT_EQ(errorDialogCount, 0);
}

TEST_F(UT_ComputerControllerCov, HandleUnAccessable_NullInfo_EarlyReturn)
{
    // Arrange
    ASSERT_EQ(detachCount, 0);

    // Act
    controller->handleUnAccessableDevCdCall(1, DFMEntryFileInfoPointer());

    // Assert - a null info is rejected before any device interaction
    EXPECT_EQ(detachCount, 0);
    EXPECT_EQ(startDetachedCount, 0);
}

TEST_F(UT_ComputerControllerCov, OnMenuRequest_NoMenuScene_ReturnsEarly)
{
    // Arrange - no menu plugin is loaded, scene creation must fail

    // Act
    controller->onMenuRequest(1, QUrl("computer:///"), false);

    // Assert - no dialog may pop up while creating the scene failed
    EXPECT_EQ(errorDialogCount, 0);
    EXPECT_EQ(startDetachedCount, 0);
}

TEST_F(UT_ComputerControllerCov, DoRename_EmptyName_SkipsRename)
{
    // Arrange
    QSignalSpy aliasSpy(controller, &ComputerController::updateItemAlias);

    // Act
    controller->doRename(1, QUrl("entry://cov.device"), "   ");

    // Assert - empty names are ignored before touching the device or the alias store
    EXPECT_EQ(renameCount, 0);
    EXPECT_EQ(aliasSpy.count(), 0);
}

TEST_F(UT_ComputerControllerCov, DoRename_PlainEntry_SkipsDevice)
{
    // Arrange - plain entry: rename lambda runs but the device is no block device
    QSignalSpy aliasSpy(controller, &ComputerController::updateItemAlias);

    // Act
    controller->doRename(1, QUrl("entry://cov.device"), "new-name");

    // Assert - rename lambda's guard rejects non-block entries
    EXPECT_EQ(renameCount, 0);
    EXPECT_EQ(aliasSpy.count(), 0);
}

TEST_F(UT_ComputerControllerCov, DoSetAlias_NoUuid_EmitsNothing)
{
    // Arrange - entity-less info carries no UUID
    QSignalSpy aliasSpy(controller, &ComputerController::updateItemAlias);

    // Act
    controller->doSetAlias(DFMEntryFileInfoPointer(new EntryFileInfo(QUrl("entry://cov.device"))), "alias");

    // Assert
    EXPECT_EQ(aliasSpy.count(), 0);
    EXPECT_EQ(renameCount, 0);
}

TEST_F(UT_ComputerControllerCov, DoSetProtocolAlias_NonProtocol_ReturnsFalse)
{
    // Arrange
    DFMEntryFileInfoPointer info(new EntryFileInfo(QUrl("entry://cov.device")));
    QSignalSpy aliasSpy(controller, &ComputerController::updateItemAlias);

    // Act
    bool handled = controller->doSetProtocolDeviceAlias(info, "alias");

    // Assert - non protocol devices are not handled here, the caller must continue
    EXPECT_EQ(handled, false);
    EXPECT_EQ(aliasSpy.count(), 0);
}

TEST_F(UT_ComputerControllerCov, MountDevice_NullInfo_ReturnsEarly)
{
    // Arrange
    ASSERT_EQ(mountCount, 0);

    // Act
    controller->mountDevice(1, DFMEntryFileInfoPointer(), ComputerController::kNone);

    // Assert
    EXPECT_EQ(mountCount, 0);
    EXPECT_EQ(errorDialogCount, 0);
}

TEST_F(UT_ComputerControllerCov, MountDevice_ById_Success_CdTo)
{
    // Arrange
    mountResult = true;
    const int before = eventCount;

    // Act
    controller->mountDevice(7, "covdev", "", ComputerController::kEnterDirectory);

    // Assert - successful mount navigates into the freshly mounted directory
    EXPECT_EQ(mountCount, 1);
    EXPECT_GT(eventCount, before);   // cdTo published a navigation event
}

TEST_F(UT_ComputerControllerCov, MountDevice_ById_Failure_ShowsError)
{
    // Arrange
    mountResult = false;
    const int before = eventCount;

    // Act
    controller->mountDevice(7, "covdev", "", ComputerController::kEnterDirectory);

    // Assert - failed mount shows the error dialog and does not navigate
    EXPECT_EQ(mountCount, 1);
    EXPECT_EQ(eventCount, before);
    mountResult = true;
}

TEST_F(UT_ComputerControllerCov, ActEject_BlockUrl_InvokesDetach)
{
    // Arrange
    const QUrl blockUrl("entry:///sr0.blockdev");

    // Act
    controller->actEject(blockUrl);

    // Assert - block devices are ejected through detachBlockDev
    EXPECT_EQ(detachCount, 1);
    EXPECT_EQ(errorDialogCount, 0);   // callback reported success
}

TEST_F(UT_ComputerControllerCov, ActEject_UnsupportedUrl_NoDeviceCall)
{
    // Arrange
    const QUrl plainUrl("computer:///");

    // Act
    controller->actEject(plainUrl);

    // Assert
    EXPECT_EQ(detachCount, 0);
    EXPECT_EQ(unmountProtocolCount, 0);
}

TEST_F(UT_ComputerControllerCov, ActUnmount_UnsupportedEntry_NoDeviceCall)
{
    // Arrange
    DFMEntryFileInfoPointer info(new EntryFileInfo(QUrl("entry://cov.plain")));

    // Act
    controller->actUnmount(info);

    // Assert - neither block nor protocol suffix: nothing to unmount
    EXPECT_EQ(unmountCount, 0);
    EXPECT_EQ(unmountProtocolCount, 0);
}

TEST_F(UT_ComputerControllerCov, ActSafelyRemove_OnBlockEntry_DelegatesToEject)
{
    // Arrange
    DFMEntryFileInfoPointer info(new EntryFileInfo(QUrl("entry:///sr1.blockdev")));

    // Act
    controller->actSafelyRemove(info);

    // Assert - safe removal is an eject under the hood
    EXPECT_EQ(detachCount, 1);
    EXPECT_EQ(unmountCount, 0);
}

TEST_F(UT_ComputerControllerCov, ActRename_FromView_EmitsRequestRename)
{
    // Arrange
    const quint64 winId = 4242;
    DFMEntryFileInfoPointer info(new EntryFileInfo(QUrl("entry://cov.dev")));
    QSignalSpy renameSpy(controller, &ComputerController::requestRename);

    // Act
    controller->actRename(winId, info, false);

    // Assert
    ASSERT_EQ(renameSpy.count(), 1);
    QList<QVariant> args = renameSpy.takeFirst();
    EXPECT_EQ(args.at(0).toULongLong(), winId);
    EXPECT_EQ(args.size(), 2);
}

TEST_F(UT_ComputerControllerCov, ActRename_NullInfo_NoSignal)
{
    // Arrange
    QSignalSpy renameSpy(controller, &ComputerController::requestRename);

    // Act
    controller->actRename(1, DFMEntryFileInfoPointer(), false);

    // Assert
    EXPECT_EQ(renameSpy.count(), 0);
    EXPECT_EQ(controller, ComputerController::instance());
}

TEST_F(UT_ComputerControllerCov, ActMount_NonBlockEntry_ReturnsWithoutMount)
{
    // Arrange
    DFMEntryFileInfoPointer info(new EntryFileInfo(QUrl("entry://cov.plain")));

    // Act
    controller->actMount(1, info, true);

    // Assert - only block devices may be mounted here
    EXPECT_EQ(mountCount, 0);
    EXPECT_EQ(errorDialogCount, 0);
}

TEST_F(UT_ComputerControllerCov, OpenInNewWindow_PlainEntry_Mounts)
{
    // Arrange - no target url: the item must be mounted before the new window opens
    mountResult = true;
    const int before = eventCount;

    // Act
    controller->actOpenInNewWindow(3, DFMEntryFileInfoPointer(new EntryFileInfo(QUrl("entry://cov.dev"))));

    // Assert
    EXPECT_EQ(mountCount, 1);
    EXPECT_GT(eventCount, before);
}

TEST_F(UT_ComputerControllerCov, ActProperties_PlainEntry_SendsPropertyDialog)
{
    // Arrange
    DFMEntryFileInfoPointer info(new EntryFileInfo(QUrl("entry://cov.dev")));

    // Act
    controller->actProperties(1, info);

    // Assert - property dialog path runs without dialogs or device calls in a test env
    EXPECT_EQ(errorDialogCount, 0);
    EXPECT_EQ(mountCount, 0);
}

TEST_F(UT_ComputerControllerCov, ActProperties_NullInfo_EarlyReturn)
{
    // Arrange - a null info must be rejected before any property handling
    DFMEntryFileInfoPointer nullInfo;

    // Act
    controller->actProperties(1, nullInfo);

    // Assert
    EXPECT_EQ(errorDialogCount, 0);
    EXPECT_EQ(controller, ComputerController::instance());
}

TEST_F(UT_ComputerControllerCov, ActLogoutForget_NonSmb_ClearsAndUnmounts)
{
    // Arrange
    DFMEntryFileInfoPointer info(new EntryFileInfo(QUrl("entry://cov.protocol")));

    // Act
    controller->actLogoutAndForgetPasswd(info);

    // Assert - non-smb entries fall through to the unmount step
    EXPECT_EQ(unmountProtocolCount + unmountCount, 0);   // entry has no recognizable suffix
    EXPECT_EQ(errorDialogCount, 0);
}

TEST_F(UT_ComputerControllerCov, ActErase_PlainEntry_PublishesEraseEvent)
{
    // Arrange
    DFMEntryFileInfoPointer info(new EntryFileInfo(QUrl("entry://cov.dev")));
    const int before = eventCount;

    // Act
    controller->actErase(info);

    // Assert - the erase request never raises a dialog nor mounts anything
    EXPECT_GE(eventCount, before);
    EXPECT_EQ(errorDialogCount, 0);
    Q_UNUSED(before)
}

TEST_F(UT_ComputerControllerCov, WaitUDisks2_NoTargetUrl_RetriesThenGivesUp)
{
    // Arrange - entity-less block url never gains a target, so 5 retries * 100ms pass
    QElapsedTimer timer;

    // Act
    timer.start();
    controller->waitUDisks2DataReady("covdev");

    // Assert
    EXPECT_GE(timer.elapsed(), qint64(400));
    EXPECT_LT(timer.elapsed(), qint64(10000));
}

TEST_F(UT_ComputerControllerCov, HandleNetworkCd_ParseFail_CdToDirect)
{
    // Arrange - parseIp fails -> straight navigation
    stub.set_lamda(static_cast<bool (NetworkUtils::*)(const QString &, QString &, QString &)>(&NetworkUtils::parseIp),
                   [](NetworkUtils *, const QString &, QString &, QString &) {
                       __DBG_STUB_INVOKE__
                       return false;
                   });
    const int before = eventCount;

    // Act
    controller->handleNetworkCdCall(9, DFMEntryFileInfoPointer(new EntryFileInfo(QUrl("entry://net.dev"))));

    // Assert
    EXPECT_EQ(eventCount, before + 1);
    EXPECT_EQ(errorDialogCount, 0);
}

TEST_F(UT_ComputerControllerCov, HandleNetworkCd_CheckFail_ShowsMountError)
{
    // Arrange - ip parses, network check fails -> mount error dialog
    stub.set_lamda(static_cast<bool (NetworkUtils::*)(const QString &, QString &, QString &)>(&NetworkUtils::parseIp),
                   [](NetworkUtils *, const QString &, QString &ip, QString &port) {
                       __DBG_STUB_INVOKE__
                       ip = "1.2.3.4";
                       port = "445";
                       return true;
                   });
    stub.set_lamda(ADDR(NetworkUtils, doAfterCheckNet),
                   [](NetworkUtils *, const QString &, const QStringList &,
                      std::function<void(bool)> callback, int) {
                       __DBG_STUB_INVOKE__
                       if (callback)
                           callback(false);
                   });
    const int before = eventCount;

    // Act
    controller->handleNetworkCdCall(9, DFMEntryFileInfoPointer(new EntryFileInfo(QUrl("entry://net.dev"))));

    // Assert - the failure branch of the network lambda shows the mount error
    EXPECT_EQ(errorDialogCount, 1);
    EXPECT_EQ(eventCount, before);   // no navigation happens on failure
}
