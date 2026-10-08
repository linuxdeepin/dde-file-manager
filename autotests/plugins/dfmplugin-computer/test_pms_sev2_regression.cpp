// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// PMS sev-2 regression tests for dfmplugin-computer:
//   BUG 324105 convertToDevUrl memory leak (EntryFileInfo must be held by smart pointer)
//   BUG 356073 app-entry double-open (2-arg startDetached after QProcess::splitCommand)
//   BUG 366407 CommonEntryFileEntity::exists/showProgress invokeMethod result misused (deterministic false)
//   BUG 368009 device property change handling (non-loop device with filesystem must not be re-added/removed)
//   BUG 307313 dirAccessPrehandler for /run/media paths (ask user before chmod)

#include <gtest/gtest.h>

#include "stubext.h"
#include "controller/computercontroller.h"
#include "utils/computerutils.h"
#include "utils/computerdatastruct.h"
#include "fileentity/entryfileentities.h"
#include "fileentity/appentryfileentity.h"
#include "events/computereventreceiver.h"
#include "watcher/computeritemwatcher.h"

#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/file/entry/entryfileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/dfm_global_defines.h>

#include <QUrl>
#include <QIcon>
#include <QProcess>
#include <QSignalSpy>
#include <QThread>
#include <QDBusVariant>
#include <QtDBus/QtDBus>
#include <unistd.h>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_computer;

// Q_OBJECT reflection targets for BUG366407 (moc'ed via test_pms_sev2_regression.moc)
class VoidExistsTarget366407 : public QObject
{
    Q_OBJECT
public slots:
    void exists() { }
    void showProgress() { }
};

class BoolExistsTarget366407 : public QObject
{
    Q_OBJECT
public slots:
    bool exists() { return true; }
};

namespace {
void registerEntrySchemes()
{
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
}   // namespace

// ---------------- BUG 324105: convertToDevUrl memory leak ----------------
class UT_PmsRegressionConvertToDevUrl : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        registerEntrySchemes();
    }
    void TearDown() override { stub.clear(); }
    stub_ext::StubExt stub;
};

// The block-device matching loop must build EntryFileInfo via DFMEntryFileInfoPointer
// (the 324105 fix); the case drives the loop over a real device id and asserts the
// match result. Pre-fix this leaked one EntryFileInfo per loop iteration.
TEST_F(UT_PmsRegressionConvertToDevUrl, BUG324105_MatchingBlockEntry_ReturnsDevUrl)
{
    const QString devId = "/org/freedesktop/UDisks2/block_devices/ut-sda1-324105";
    const QUrl devUrl = ComputerUtils::makeBlockDevUrl(devId);
    const QUrl mountUrl = QUrl::fromLocalFile("/media/ut-sda1-324105");

    stub.set_lamda(&UniversalUtils::urlsTransformToLocal,
                   [](const QList<QUrl> &urls, QList<QUrl> *out) -> bool {
                       *out = urls;
                       return true;
                   });
    stub.set_lamda(&DeviceProxyManager::getAllBlockIds,
                   [&devId](DeviceProxyManager *, GlobalServerDefines::DeviceQueryOptions) -> QStringList {
                       return { devId };
                   });
    stub.set_lamda(ADDR(EntryFileInfo, targetUrl),
                   [](EntryFileInfo *) -> QUrl { return QUrl::fromLocalFile("/media/ut-sda1-324105"); });

    const QUrl result = ComputerUtils::convertToDevUrl(mountUrl);
    EXPECT_EQ(devUrl, result);
}

TEST_F(UT_PmsRegressionConvertToDevUrl, BUG324105_NoMatch_ReturnsInvalidUrl)
{
    stub.set_lamda(&UniversalUtils::urlsTransformToLocal,
                   [](const QList<QUrl> &, QList<QUrl> *out) -> bool {
                       out->clear();
                       return false;
                   });
    const QUrl result = ComputerUtils::convertToDevUrl(QUrl::fromLocalFile("/home/ut-no-dev-324105"));
    EXPECT_FALSE(result.isValid());
}

// ---------------- BUG 356073: app entry executed twice ----------------
class UT_PmsRegressionController : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        registerEntrySchemes();
        controller = ComputerController::instance();
        ASSERT_NE(controller, nullptr);
    }
    void TearDown() override { stub.clear(); }
    stub_ext::StubExt stub;
    ComputerController *controller = nullptr;
};

// Fixed code splits the command once and calls the 2-arg QProcess::startDetached
// overload exactly once with program + argument list.
// Note: EntryFileInfo::nameOf is a virtual override (vcall pmf cannot be stubbed
// by cpp-stub); the URL suffix ".appentry" makes the real nameOf(kSuffix)
// return SuffixInfo::kAppEntry through EntryFileInfoPrivate::suffix().
TEST_F(UT_PmsRegressionController, BUG356073_OpenAppEntry_StartsDetachedOnce)
{
    stub.set_lamda(ADDR(EntryFileInfo, isAccessable), [](EntryFileInfo *) -> bool { return true; });
    stub.set_lamda(ADDR(EntryFileInfo, targetUrl), [](EntryFileInfo *) -> QUrl { return QUrl(); });
    stub.set_lamda(ADDR(EntryFileInfo, extraProperty),
                   [](EntryFileInfo *, const QString &key) -> QVariant {
                       if (key == GlobalServerDefines::DeviceProperty::kOptical)
                           return false;
                       if (key == ExtraPropertyName::kExecuteCommand)
                           return QString("gvfs-open --new-window /media/ut-disc-356073");
                       return QVariant();
                   });

    QString program;
    QStringList args;
    int callCount = 0;
    using StartDetachedFn = bool (*)(const QString &, const QStringList &, const QString &, qint64 *);
    stub.set_lamda(static_cast<StartDetachedFn>(&QProcess::startDetached),
                   [&](const QString &prog, const QStringList &argList, const QString &, qint64 *) -> bool {
                       ++callCount;
                       program = prog;
                       args = argList;
                       return true;
                   });

    controller->onOpenItem(0, QUrl("entry:///ut-356073.appentry"));

    EXPECT_EQ(1, callCount);
    EXPECT_EQ(QString("gvfs-open"), program);
    EXPECT_EQ((QStringList { "--new-window", "/media/ut-disc-356073" }), args);
}

// ---------------- BUG 366407: exists()/showProgress() reflection ----------------
class UT_PmsRegressionEntryEntity : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        stub.set_lamda(&ComputerItemWatcher::instance, []() {
            static ComputerItemWatcher mockWatcher;
            return &mockWatcher;
        });
        stub.set_lamda(&ComputerItemWatcher::getComputerInfos, []() -> QHash<QUrl, QVariantMap> {
            return {};
        });
        entity = new CommonEntryFileEntity(QUrl("entry:///ut-366407.common"));
    }
    void TearDown() override
    {
        stub.clear();
        delete entity;
        entity = nullptr;
    }
    stub_ext::StubExt stub;
    CommonEntryFileEntity *entity = nullptr;
};

// Pre-fix: the bool returned by QMetaObject::invokeMethod was fed back as the entity
// state (UB). Post-fix a failed invokeMethod deterministically yields false.
// Note: reflectionObj is owned by the entity (deleted in ~CommonEntryFileEntity),
// so the moc'ed targets must be heap-allocated and handed over.
TEST_F(UT_PmsRegressionEntryEntity, BUG366407_InvokeMethodFailure_ReturnsDeterministicFalse)
{
    entity->reflectionObj = new VoidExistsTarget366407;   // void exists(): bool-returning invokeMethod must fail
    EXPECT_FALSE(entity->exists());
    EXPECT_FALSE(entity->showProgress());
}

TEST_F(UT_PmsRegressionEntryEntity, BUG366407_InvokeMethodSuccess_ReturnsReflectedValue)
{
    entity->reflectionObj = new BoolExistsTarget366407;   // bool exists() returning true
    EXPECT_TRUE(entity->exists());
}

// ---------------- BUG 368009: property change handling ----------------
class UT_PmsRegressionItemWatcher : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        watcher = ComputerItemWatcher::instance();
        ASSERT_NE(watcher, nullptr);
    }
    void TearDown() override { stub.clear(); }
    stub_ext::StubExt stub;
    ComputerItemWatcher *watcher = nullptr;
};

// A non-loop device that already has a filesystem must not be removed/re-added:
// only onUpdateBlockItem fires.
TEST_F(UT_PmsRegressionItemWatcher, BUG368009_HasFileSystem_NonLoop_UpdatesWithoutReAdd)
{
    const QString blockId = "/org/freedesktop/UDisks2/block_devices/ut-368009-noloop";
    stub.set_lamda(&DeviceProxyManager::queryBlockInfo,
                   [](DeviceProxyManager *, const QString &, bool) -> QVariantMap {
                       return { { GlobalServerDefines::DeviceProperty::kIsLoopDevice, false } };
                   });
    int addCount = 0, removeCount = 0, updateCount = 0;
    stub.set_lamda(ADDR(ComputerItemWatcher, addDevice),
                   [&](ComputerItemWatcher *, const QString &, const QUrl &, int, bool) { ++addCount; });
    stub.set_lamda(ADDR(ComputerItemWatcher, removeDevice),
                   [&](ComputerItemWatcher *, const QUrl &) { ++removeCount; });
    stub.set_lamda(ADDR(ComputerItemWatcher, onUpdateBlockItem),
                   [&](ComputerItemWatcher *, const QString &) { ++updateCount; });

    QDBusVariant var(QVariant(true));
    watcher->onDevicePropertyChangedQDBusVar(blockId,
                                             GlobalServerDefines::DeviceProperty::kHasFileSystem,
                                             var);
    EXPECT_EQ(0, addCount);
    EXPECT_EQ(0, removeCount);
    EXPECT_EQ(1, updateCount);
}

// Loop devices keep the add/remove handling (regression guard for the fix scope).
TEST_F(UT_PmsRegressionItemWatcher, BUG368009_HasFileSystem_Loop_StillAdds)
{
    const QString blockId = "/org/freedesktop/UDisks2/block_devices/ut-368009-loop";
    stub.set_lamda(&DeviceProxyManager::queryBlockInfo,
                   [](DeviceProxyManager *, const QString &, bool) -> QVariantMap {
                       return { { GlobalServerDefines::DeviceProperty::kIsLoopDevice, true } };
                   });
    int addCount = 0;
    stub.set_lamda(ADDR(ComputerItemWatcher, addDevice),
                   [&](ComputerItemWatcher *, const QString &, const QUrl &, int, bool) { ++addCount; });

    QDBusVariant var(QVariant(true));
    watcher->onDevicePropertyChangedQDBusVar(blockId,
                                             GlobalServerDefines::DeviceProperty::kHasFileSystem,
                                             var);
    EXPECT_EQ(1, addCount);
}

// ---------------- BUG 307313: dirAccessPrehandler for /run/media ----------------
class UT_PmsRegressionPrehandler : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        receiver = ComputerEventReceiver::instance();
        ASSERT_NE(receiver, nullptr);
    }
    void TearDown() override { stub.clear(); }

    static QString flagFile(const QString &devDesc)
    {
        return QString("/tmp/dfm_%1_%2_ignore_request_permission_in_current_session")
                .arg(getuid())
                .arg(devDesc);
    }
    stub_ext::StubExt stub;
    ComputerEventReceiver *receiver = nullptr;
};

// Regression: a /run/media mount (not only /media/) must reach the chmod-ask flow.
TEST_F(UT_PmsRegressionPrehandler, BUG307313_RunMediaPath_AskAndChmod)
{
    const QString devDesc = "ut-307313-ask";
    const QString flag = flagFile(devDesc);
    QFile::remove(flag);

    // Note: SyncFileInfo::isAttributes is a virtual override (vcall pmf cannot be
    // stubbed); the url below points to a nonexistent path so the real
    // isAttributes(writable/executable/readable) all yield false and the flow
    // reaches the device/chmod section.
    stub.set_lamda(&DeviceProxyManager::isMptOfDevice,
                   [&devDesc](DeviceProxyManager *, const QString &, QString &id) -> bool {
                       id = "/org/freedesktop/UDisks2/block_devices/" + devDesc;
                       return true;
                   });
    stub.set_lamda(&DeviceProxyManager::queryBlockInfo,
                   [](DeviceProxyManager *, const QString &, bool) -> QVariantMap {
                       return {
                           { GlobalServerDefines::DeviceProperty::kHintSystem, true },
                           { GlobalServerDefines::DeviceProperty::kFileSystem, "ext4" },
                           { GlobalServerDefines::DeviceProperty::kIsLoopDevice, false }
                       };
                   });
    stub.set_lamda(static_cast<QString (*)(const QVariantMap &)>(&DeviceUtils::convertSuitableDisplayName),
                   [](const QVariantMap &) -> QString { return QString("UT Disk 307313"); });
    stub.set_lamda(ADDR(ComputerEventReceiver, askForConfirmChmod),
                   [](const QString &) -> bool { return true; });

    QString dbusMethod;
    QList<QVariant> dbusArgs;
    stub.set_lamda(ADDR(QDBusInterface, callWithArgumentList),
                   [&](QDBusAbstractInterface *, QDBus::CallMode, const QString &method, const QList<QVariant> &args) -> QDBusMessage {
                       dbusMethod = method;
                       dbusArgs = args;
                       return QDBusMessage();
                   });

    bool afterCalled = false;
    ComputerEventReceiver::dirAccessPrehandler(0, QUrl("file:///run/media/_udev_/ut-disk-307313"),
                                               [&afterCalled] { afterCalled = true; });

    EXPECT_TRUE(afterCalled);
    EXPECT_EQ(QString("Chmod"), dbusMethod);
    ASSERT_EQ(2, dbusArgs.size());   // { path, ACCESSPERMS }
    EXPECT_EQ(QString("/run/media/_udev_/ut-disk-307313"), dbusArgs.first().toString());

    QFile::remove(flag);
}

// User dismissal persists an ignore-flag file and must not invoke the chmod dbus call.
TEST_F(UT_PmsRegressionPrehandler, BUG307313_UserDismissed_CreatesIgnoreFlag)
{
    const QString devDesc = "ut-307313-dismiss";
    const QString flag = flagFile(devDesc);
    QFile::remove(flag);

    // Same as above: nonexistent path keeps the real isAttributes on the false branch.
    stub.set_lamda(&DeviceProxyManager::isMptOfDevice,
                   [&devDesc](DeviceProxyManager *, const QString &, QString &id) -> bool {
                       id = "/org/freedesktop/UDisks2/block_devices/" + devDesc;
                       return true;
                   });
    stub.set_lamda(&DeviceProxyManager::queryBlockInfo,
                   [](DeviceProxyManager *, const QString &, bool) -> QVariantMap {
                       return {
                           { GlobalServerDefines::DeviceProperty::kHintSystem, true },
                           { GlobalServerDefines::DeviceProperty::kFileSystem, "ext4" },
                           { GlobalServerDefines::DeviceProperty::kIsLoopDevice, false }
                       };
                   });
    stub.set_lamda(static_cast<QString (*)(const QVariantMap &)>(&DeviceUtils::convertSuitableDisplayName),
                   [](const QVariantMap &) -> QString { return QString("UT Disk 307313b"); });
    stub.set_lamda(ADDR(ComputerEventReceiver, askForConfirmChmod),
                   [](const QString &) -> bool { return false; });

    int dbusCalls = 0;
    stub.set_lamda(ADDR(QDBusInterface, callWithArgumentList),
                   [&](QDBusAbstractInterface *, QDBus::CallMode, const QString &, const QList<QVariant> &) -> QDBusMessage {
                       ++dbusCalls;
                       return QDBusMessage();
                   });

    bool afterCalled = false;
    ComputerEventReceiver::dirAccessPrehandler(0, QUrl("file:///media/ut-disk-307313"),
                                               [&afterCalled] { afterCalled = true; });

    EXPECT_TRUE(afterCalled);
    EXPECT_EQ(0, dbusCalls);
    EXPECT_TRUE(QFile::exists(flag));   // dismissal remembered for the session

    QFile::remove(flag);
}

#include "test_pms_sev2_regression.moc"
