// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// accesscontrol 服务单元测试：服务无可链接目标，此处 unity-include 被测服务
// 实现源码，并对 DBus / dfm-mount / libcryptsetup / polkit / DConfig 等外部
// 依赖打桩隔离。

#include <gtest/gtest.h>
#include <stubext.h>

#include <dlfcn.h>

#include <DConfig>
#include <QDBusConnection>
#include <QDBusContext>
#include <QDBusUnixFileDescriptor>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QSignalSpy>
#include <QThread>
#include <QThreadPool>

#include <libcryptsetup.h>
#include <unistd.h>

#include "dfm-mount/dblockdevice.h"
#include "dfm-mount/dblockmonitor.h"
#include "dfm-mount/base/ddevicemonitor.h"

#include "services/accesscontrol/accesscontroldbus.h"
#include "services/accesscontrol/utils.h"

// unity-include 被测实现（服务无可链接目标）
#include "services/accesscontrol/accesscontroldbus.cpp"
#include "services/accesscontrol/plugin.cpp"
#include "services/accesscontrol/utils.cpp"
#include "services/common/polkit/policykithelper.cpp"

SERVICEACCESSCONTROL_USE_NAMESPACE
// dfmmount 与 service_accesscontrol 均有 Utils，这里用别名避免歧义
using AccUtils = SERVICEACCESSCONTROL_NAMESPACE::Utils;
namespace utm = DFMMOUNT;

namespace {
// ------------------------- libcryptsetup 桩状态 -------------------------
int g_cryptInitRet { 0 };
int g_cryptLoadRet { 0 };
int g_cryptActivateRet { 0 };
int g_cryptKeyslotRet { 0 };
int g_cryptFreeCalls { 0 };
QByteArray g_cryptDeviceName { "ut-crypt-dev" };

// ------------------------- 服务桩状态 -------------------------
QVector<int> g_checkPwdSeq;   // AccUtils::checkDiskPassword 依次返回
QVector<int> g_changePwdSeq;   // AccUtils::changeDiskPassword 依次返回
int g_checkPwdCalls { 0 };
int g_changePwdCalls { 0 };
int g_polkitOk { 0 };
QStringList g_encryptedDisks;
QString g_decryptEcho;   // FileUtils::decryptString 的回显值
int g_setFileModeCalls { 0 };
uint g_setFileModeLastMode { 0 };

// dfm-mount 设备桩状态
bool g_startMonitorRet { true };
QStringList g_monitorDevices;
utm::DBlockDevice *g_fakeDevice { nullptr };
bool g_canPowerOff { false };
QVariant g_connBusProp { "usb" };
QStringList g_mediaCompat;
bool g_hintSystem { false };
QString g_devFile { "/dev/sdz1" };
QString g_devMountPoint;
QString g_devFsType { "vfat" };
bool g_removable { true };
bool g_optical { false };
bool g_hasFs { true };
bool g_powerOffRet { true };
int g_powerOffCalls { 0 };
bool g_unmountOk { true };
int g_mountCalls { 0 };
int g_umountCalls { 0 };

void resetGlobals()
{
    g_cryptInitRet = 0;
    g_cryptLoadRet = 0;
    g_cryptActivateRet = 0;
    g_cryptKeyslotRet = 0;
    g_cryptFreeCalls = 0;
    g_checkPwdSeq.clear();
    g_changePwdSeq.clear();
    g_checkPwdCalls = 0;
    g_changePwdCalls = 0;
    g_polkitOk = 0;
    g_encryptedDisks.clear();
    g_decryptEcho = "ut-decrypted";
    g_setFileModeCalls = 0;
    g_setFileModeLastMode = 0;
    g_startMonitorRet = true;
    g_monitorDevices.clear();
    g_canPowerOff = false;
    g_connBusProp = "usb";
    g_mediaCompat.clear();
    g_hintSystem = false;
    g_devFile = "/dev/sdz1";
    g_devMountPoint = QString();
    g_devFsType = "vfat";
    g_removable = true;
    g_optical = false;
    g_hasFs = true;
    g_powerOffRet = true;
    g_powerOffCalls = 0;
    g_unmountOk = true;
    g_mountCalls = 0;
    g_umountCalls = 0;
}

void pumpEvents(int ms)
{
    QElapsedTimer t;
    t.start();
    while (t.elapsed() < ms) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        QThread::msleep(20);
    }
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
}
}   // namespace


namespace {
// dfm-mount 的 utm::DDevice/utm::utm::DDeviceMonitor 虚方法为跨库虚函数：成员指针只是
// vtable 偏移，无法直接取地址打桩，通过 dlsym 解析导出符号地址再 patch
// （虚分派与直接调用都会命中符号入口）。
template <typename FnPtr, typename Lamda>
bool stubDlsym(stub_ext::StubExt &stub, const char *mangled, Lamda fake)
{
    void *addr = ::dlsym(RTLD_DEFAULT, mangled);
    return addr ? stub.set_lamda(reinterpret_cast<FnPtr>(addr), fake) : false;
}
}   // namespace

class AccessControlServiceTest : public testing::Test
{
public:
    stub_ext::StubExt stub;
    QTemporaryDir cfgDir;

    void SetUp() override
    {
        resetGlobals();

        // QDBusContext::message() 在无 DBus 上下文时会解引用空指针，桩为空消息
        static QDBusMessage s_emptyDBusMsg;
        stub.set_lamda(static_cast<const QDBusMessage &(QDBusContext::*)() const>(&QDBusContext::message),
                       [](QDBusContext *) -> const QDBusMessage & {
            return s_emptyDBusMsg;
        });

        ASSERT_TRUE(cfgDir.isValid());

        // 隔离真实 DBus
        stub.set_lamda(static_cast<QDBusConnection (*)()>(&QDBusConnection::systemBus),
                       []() -> QDBusConnection {
            return QDBusConnection(QString());
        });
        stub.set_lamda(
                static_cast<QDBusConnection (*)(QDBusConnection::BusType, const QString &)>(&QDBusConnection::connectToBus),
                [](QDBusConnection::BusType, const QString &) -> QDBusConnection {
            return QDBusConnection(QString());
        });

        // 策略文件重定向到临时目录（避免读写 /etc）
        static QString s_devCfg;
        static QString s_vaultCfg;
        s_devCfg = cfgDir.filePath("devAccessConfig.json");
        s_vaultCfg = cfgDir.filePath("vaultAccessConfig.json");
        stub.set_lamda(&AccUtils::devConfigPath, []() -> QString {
            return s_devCfg;
        });
        stub.set_lamda(&AccUtils::valultConfigPath, []() -> QString {
            return s_vaultCfg;
        });

        // dfm-mount 监控桩
        stubDlsym<bool (*)(utm::DDeviceMonitor *)>(stub, "_ZN8dfmmount14DDeviceMonitor12startMonitorEv",
                       [](utm::DDeviceMonitor *) -> bool { return g_startMonitorRet; });
        stubDlsym<QStringList (*)(utm::DDeviceMonitor *)>(stub, "_ZNK8dfmmount14DDeviceMonitor10getDevicesEv",
                       [](utm::DDeviceMonitor *) -> QStringList { return g_monitorDevices; });
        stubDlsym<QSharedPointer<utm::DDevice> (*)(utm::DDeviceMonitor *, const QString &)>(
                stub, "_ZNK8dfmmount14DDeviceMonitor16createDeviceByIdERK7QString",
                [](utm::DDeviceMonitor *, const QString &) -> QSharedPointer<utm::DDevice> {
            // 无操作删除器：QSharedPointer 引用计数归零时不得 delete 全局伪设备，
            // 否则跨用例复用会 use-after-free / double free（进程退出时泄漏一个对象，可接受）
            return QSharedPointer<utm::DDevice>(g_fakeDevice, [](utm::DDevice *) {});
        });
        stubDeviceMethods();

        // libcryptsetup C 接口桩
        stub.set_lamda(&crypt_init, [](crypt_device **cd, const char *) -> int {
            // 首分支用例会传 nullptr 校验 kInitFailed 提前返回，桩内不得解引用
            if (cd)
                *cd = reinterpret_cast<crypt_device *>(quintptr(0x10));
            return g_cryptInitRet;
        });
        stub.set_lamda(&crypt_load, [](crypt_device *, const char *, void *) -> int {
            return g_cryptLoadRet;
        });
        stub.set_lamda(&crypt_activate_by_passphrase,
                       [](crypt_device *, const char *, int, const char *, size_t, uint32_t) -> int {
            return g_cryptActivateRet;
        });
        stub.set_lamda(&crypt_get_device_name, [](crypt_device *) -> const char * {
            return g_cryptDeviceName.constData();
        });
        stub.set_lamda(&crypt_keyslot_change_by_passphrase,
                       [](crypt_device *, int, int, const char *, size_t, const char *, size_t) -> int {
            return g_cryptKeyslotRet;
        });
        stub.set_lamda(&crypt_free, [](crypt_device *) { ++g_cryptFreeCalls; });

        // polkit 授权桩（PolicyKitHelper 已 unity-include 进本 TU）
        stub.set_lamda(&ServiceCommon::PolicyKitHelper::checkAuthorization,
                       [](ServiceCommon::PolicyKitHelper *, const QString &, const QString &) -> bool {
            return g_polkitOk == 1;
        });

        // DConfig 桩：避免依赖真实 dconfig 后端
        stub.set_lamda(
                static_cast<Dtk::Core::DConfig *(*)(const QString &, const QString &, const QString &, QObject *)>(&Dtk::Core::DConfig::create),
                [](const QString &, const QString &, const QString &, QObject *) -> Dtk::Core::DConfig * {
            return nullptr;
        });
    }

    void TearDown() override
    {
        stub.clear();
        resetGlobals();
        QThreadPool::globalInstance()->waitForDone(3000);
    }

    // 为 utm::DBlockDevice 实例的方法打桩（构造私有实例后所有访问走桩）
    void stubDeviceMethods()
    {
        stub.set_lamda(static_cast<bool (utm::DBlockDevice::*)() const>(&utm::DBlockDevice::canPowerOff),
                       [](utm::DBlockDevice *) -> bool { return g_canPowerOff; });
        stubDlsym<QVariant (*)(utm::DDevice *, utm::Property)>(stub, "_ZNK8dfmmount7DDevice11getPropertyENS_8PropertyE",
                       [](utm::DDevice *, utm::Property prop) -> QVariant {
            if (prop == utm::Property::kDriveConnectionBus)
                return g_connBusProp;
            if (prop == utm::Property::kBlockUserspaceMountOptions)
                return QVariant(QStringList { "uhelper=udisks2" });
            return QVariant();
        });
        stub.set_lamda(static_cast<QStringList (utm::DBlockDevice::*)() const>(&utm::DBlockDevice::mediaCompatibility),
                       [](utm::DBlockDevice *) -> QStringList { return g_mediaCompat; });
        stub.set_lamda(static_cast<bool (utm::DBlockDevice::*)() const>(&utm::DBlockDevice::hintSystem),
                       [](utm::DBlockDevice *) -> bool { return g_hintSystem; });
        stub.set_lamda(static_cast<QString (utm::DBlockDevice::*)() const>(&utm::DBlockDevice::device),
                       [](utm::DBlockDevice *) -> QString { return g_devFile; });
        stubDlsym<QString (*)(utm::DDevice *)>(stub, "_ZNK8dfmmount7DDevice10mountPointEv",
                       [](utm::DDevice *) -> QString { return g_devMountPoint; });
        stubDlsym<QString (*)(utm::DDevice *)>(stub, "_ZNK8dfmmount7DDevice10fileSystemEv",
                       [](utm::DDevice *) -> QString { return g_devFsType; });
        stub.set_lamda(static_cast<bool (utm::DBlockDevice::*)() const>(&utm::DBlockDevice::removable),
                       [](utm::DBlockDevice *) -> bool { return g_removable; });
        stub.set_lamda(static_cast<bool (utm::DBlockDevice::*)() const>(&utm::DBlockDevice::optical),
                       [](utm::DBlockDevice *) -> bool { return g_optical; });
        stub.set_lamda(static_cast<bool (utm::DBlockDevice::*)() const>(&utm::DBlockDevice::hasFileSystem),
                       [](utm::DBlockDevice *) -> bool { return g_hasFs; });
        stub.set_lamda(static_cast<bool (utm::DBlockDevice::*)(const QVariantMap &)>(&utm::DBlockDevice::powerOff),
                       [](utm::DBlockDevice *, const QVariantMap &) -> bool {
            ++g_powerOffCalls;
            return g_powerOffRet;
        });
        stub.set_lamda(static_cast<utm::OperationErrorInfo (utm::DDevice::*)() const>(&utm::DDevice::lastError),
                       [](utm::DDevice *) -> utm::OperationErrorInfo { return utm::OperationErrorInfo {}; });
        stubDlsym<void (*)(utm::DDevice *, const QVariantMap &, utm::DeviceOperateCallback)>(
                stub, "_ZN8dfmmount7DDevice12unmountAsyncERK4QMapI7QString8QVariantESt8functionIFvbRKNS_18OperationErrorInfoEEE",
                [](utm::DDevice *, const QVariantMap &, utm::DeviceOperateCallback cb) {
            if (cb)
                cb(g_unmountOk, utm::OperationErrorInfo {});
        });
        // mount/umount 系统调用桩（避免真实 syscall）
        stub.set_lamda(&mount, [](const char *, const char *, const char *, unsigned long, const void *) -> int {
            ++g_mountCalls;
            return 0;
        });
        stub.set_lamda(&umount, [](const char *) -> int {
            ++g_umountCalls;
            return 0;
        });
    }

    // 创建真实的 utm::DBlockDevice 实例（私有构造，借助 -fno-access-control 调用；
    // 其所有方法均被桩替换，不会触碰真实 udisks2）
    utm::DBlockDevice *makeFakeDevice()
    {
        if (!g_fakeDevice)
            g_fakeDevice = new utm::DBlockDevice(
                    nullptr, "/org/freedesktop/UDisks2/block_devices/ut", nullptr);
        return g_fakeDevice;
    }

    // 构造已授权的凭据管道（QDataStream 序列化 old/new 密码）
    static QDBusUnixFileDescriptor makeCredentials(const QString &oldPwd, const QString &newPwd)
    {
        int fds[2];
        if (pipe(fds) != 0)
            return QDBusUnixFileDescriptor();
        QByteArray buf;
        QDataStream w(&buf, QIODevice::WriteOnly);
        w << oldPwd << newPwd;
        (void)!::write(fds[1], buf.constData(), buf.size());
        ::close(fds[1]);
        QDBusUnixFileDescriptor fd(fds[0]);
        ::close(fds[0]);
        return fd;
    }

    // 桩掉 Utils 的磁盘密码校验/修改（用于 DBus 层流程测试）
    void stubUtilsPasswordFlows()
    {
        stub.set_lamda(&AccUtils::checkDiskPassword,
                       [](crypt_device **, const char *, const char *) -> DPCErrorCode {
            const int i = qMin(g_checkPwdCalls, g_checkPwdSeq.size() - 1);
            ++g_checkPwdCalls;
            return static_cast<DPCErrorCode>(g_checkPwdSeq.value(i, kNoError));
        });
        stub.set_lamda(&AccUtils::changeDiskPassword,
                       [](crypt_device *, const char *, const char *) -> DPCErrorCode {
            const int i = qMin(g_changePwdCalls, g_changePwdSeq.size() - 1);
            ++g_changePwdCalls;
            return static_cast<DPCErrorCode>(g_changePwdSeq.value(i, kNoError));
        });
    }
};

/******************************************************************************/
/******************************* Utils 直测 **********************************/
/******************************************************************************/

TEST_F(AccessControlServiceTest, ConfigPaths_RedirectedToIsolatedCfgDir)
{
    // Arrange / Act：SetUp 已将路径重定向到临时目录，验证重定向生效且文件名固定
    const QString dev = AccUtils::devConfigPath();
    const QString vault = AccUtils::valultConfigPath();

    // Assert
    EXPECT_TRUE(dev.endsWith("devAccessConfig.json"));
    EXPECT_TRUE(vault.endsWith("vaultAccessConfig.json"));
    EXPECT_NE(dev, vault);
}

TEST_F(AccessControlServiceTest, AccessMode_EmptyPath_Disable)
{
    // Arrange / Act
    const int mode = AccUtils::accessMode("");

    // Assert
    EXPECT_EQ(mode, kPolicyDisable);
}

TEST_F(AccessControlServiceTest, AccessMode_WritablePath_Rw)
{
    // Arrange
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    // Act
    const int mode = AccUtils::accessMode(dir.path());

    // Assert
    EXPECT_EQ(mode, kPolicyRw);
}

TEST_F(AccessControlServiceTest, AccessMode_ReadOnlyFile_Ronly)
{
    // Arrange
    QTemporaryFile f;
    ASSERT_TRUE(f.open());
    f.close();
    QFile file(f.fileName());
    ASSERT_TRUE(file.setPermissions(QFileDevice::ReadOwner));

    // Act
    const int mode = AccUtils::accessMode(f.fileName());

    // Assert
    EXPECT_EQ(mode, kPolicyRonly);
}

TEST_F(AccessControlServiceTest, AccessMode_InaccessibleFile_Disable)
{
    // Arrange
    QTemporaryFile f;
    ASSERT_TRUE(f.open());
    f.close();
    QFile file(f.fileName());
    ASSERT_TRUE(file.setPermissions(QFileDevice::Permissions {}));

    // Act
    const int mode = AccUtils::accessMode(f.fileName());

    // Assert
    EXPECT_EQ(mode, kPolicyDisable);
}

TEST_F(AccessControlServiceTest, SetFileMode_ChangesModeOnRegularFile)
{
    // Arrange
    QTemporaryFile f;
    ASSERT_TRUE(f.open());
    f.close();
    QFile::Permissions before = QFileInfo(f.fileName()).permissions();
    EXPECT_FALSE(before & QFile::ExeOwner);

    // Act
    const int ret = AccUtils::setFileMode(f.fileName(), 0755);

    // Assert
    EXPECT_EQ(ret, 0);
    EXPECT_TRUE(QFileInfo(f.fileName()).permissions() & QFile::ExeOwner);
}

TEST_F(AccessControlServiceTest, SetFileMode_MissingPath_Fails)
{
    // Arrange / Act
    const int ret = AccUtils::setFileMode(cfgDir.filePath("no-such-file"), 0777);

    // Assert
    EXPECT_EQ(ret, -1);
}

TEST_F(AccessControlServiceTest, SetFileMode_SymlinkRejected_Fails)
{
    // Arrange
    QTemporaryFile target;
    ASSERT_TRUE(target.open());
    const QString link = cfgDir.filePath("ut-link");
    ASSERT_TRUE(QFile::link(target.fileName(), link));

    // Act
    const int ret = AccUtils::setFileMode(link, 0777);

    // Assert
    EXPECT_EQ(ret, -1);   // O_NOFOLLOW → ELOOP
}

TEST_F(AccessControlServiceTest, IsValidDevPolicy_FullMatrix)
{
    // Arrange
    const QVariantMap valid {
        { kKeyInvoker, "ut-invoker" },
        { kKeyType, kTypeBlock },
        { kKeyPolicy, kPolicyRonly }
    };

    // Act / Assert
    EXPECT_TRUE(AccUtils::isValidDevPolicy(valid, "ut-invoker"));
    EXPECT_FALSE(AccUtils::isValidDevPolicy(valid, "other-invoker"));
    QVariantMap noInvoker = valid;
    noInvoker.remove(kKeyInvoker);
    EXPECT_FALSE(AccUtils::isValidDevPolicy(noInvoker, "ut-invoker"));
    QVariantMap emptyInvoker = valid;
    emptyInvoker[kKeyInvoker] = "";
    EXPECT_FALSE(AccUtils::isValidDevPolicy(emptyInvoker, "ut-invoker"));
    QVariantMap badType = valid;
    badType[kKeyType] = kTypeInvalid;
    EXPECT_FALSE(AccUtils::isValidDevPolicy(badType, "ut-invoker"));
    QVariantMap bigType = valid;
    bigType[kKeyType] = kTypeBlock | kTypeOptical | kTypeProtocol | 0x08;
    EXPECT_FALSE(AccUtils::isValidDevPolicy(bigType, "ut-invoker"));
    QVariantMap badPolicy = valid;
    badPolicy[kKeyPolicy] = kPolicyRw + 1;
    EXPECT_FALSE(AccUtils::isValidDevPolicy(badPolicy, "ut-invoker"));
    QVariantMap negPolicy = valid;
    negPolicy[kKeyPolicy] = -1;
    EXPECT_FALSE(AccUtils::isValidDevPolicy(negPolicy, "ut-invoker"));
    QVariantMap allTypes = valid;
    allTypes[kKeyType] = kTypeBlock | kTypeOptical | kTypeProtocol;
    EXPECT_TRUE(AccUtils::isValidDevPolicy(allTypes, "ut-invoker"));
}

TEST_F(AccessControlServiceTest, IsValidVaultPolicy_Validation)
{
    // Arrange
    const QVariantMap valid { { kPolicyType, 1 }, { kVaultHideState, 0 } };
    const QVariantMap badType { { kPolicyType, -1 }, { kVaultHideState, 0 } };
    const QVariantMap badHide { { kPolicyType, 1 }, { kVaultHideState, -2 } };

    // Act / Assert
    EXPECT_TRUE(AccUtils::isValidVaultPolicy(valid));
    EXPECT_FALSE(AccUtils::isValidVaultPolicy(badType));
    EXPECT_FALSE(AccUtils::isValidVaultPolicy(badHide));
}

TEST_F(AccessControlServiceTest, SaveAndLoadDevPolicy_RoundTrip)
{
    // Arrange
    const QVariantMap global { { kKeyInvoker, "ut-invoker" }, { kKeyType, kTypeBlock }, { kKeyPolicy, kPolicyRonly } };
    const QVariantMap perDev { { kKeyInvoker, "ut-invoker" }, { kKeyType, kTypeOptical },
                               { kKeyPolicy, kPolicyRw }, { kKeyDevice, "/dev/sr0" } };

    // Act
    AccUtils::saveDevPolicy(global);
    AccUtils::saveDevPolicy(perDev);
    DevPolicyType loaded;
    AccUtils::loadDevPolicy(&loaded);

    // Assert：loadDevPolicy 仅加载 global==1 的条目，per-device 策略不进结果集
    EXPECT_EQ(loaded.size(), 1);
    EXPECT_EQ(loaded.value(kTypeBlock).first, "ut-invoker");
    EXPECT_EQ(loaded.value(kTypeBlock).second, kPolicyRonly);
    EXPECT_FALSE(loaded.contains(kTypeOptical));

    // 再次保存同 invoker+type 的全局策略 → 更新而非追加
    QVariantMap updated = global;
    updated[kKeyPolicy] = kPolicyDisable;
    AccUtils::saveDevPolicy(updated);
    DevPolicyType reloaded;
    AccUtils::loadDevPolicy(&reloaded);
    EXPECT_EQ(reloaded.size(), 1);
    EXPECT_EQ(reloaded.value(kTypeBlock).second, kPolicyDisable);
}

TEST_F(AccessControlServiceTest, LoadDevPolicy_MalformedJson_LoadsNothing)
{
    // Arrange：预置一条策略，观察畸形 JSON 是否会污染既有内容
    QFile bad(cfgDir.filePath("devAccessConfig.json"));
    ASSERT_TRUE(bad.open(QIODevice::WriteOnly));
    bad.write("not-a-json-array");
    bad.close();

    // Act
    DevPolicyType loaded;
    loaded.insert(kTypeBlock, { "old", kPolicyRw });
    AccUtils::loadDevPolicy(&loaded);

    // Assert：解析失败提前返回，不 clear 也不插入新条目，既有内容原样保留
    EXPECT_EQ(loaded.size(), 1);
    EXPECT_EQ(loaded.value(kTypeBlock).first, QString("old"));
    EXPECT_EQ(loaded.value(kTypeBlock).second, kPolicyRw);
}

TEST_F(AccessControlServiceTest, LoadDevPolicy_NonGlobalEntriesSkipped)
{
    // Arrange
    QJsonArray arr;
    QJsonObject nonGlobal;
    nonGlobal.insert(kKeyGlobal, 0);
    nonGlobal.insert(kKeyType, kTypeBlock);
    nonGlobal.insert(kKeyPolicy, kPolicyRonly);
    nonGlobal.insert(kKeyInvoker, "someone");
    arr.append(nonGlobal);
    QJsonObject globalEntry;
    globalEntry.insert(kKeyGlobal, 1);
    globalEntry.insert(kKeyType, kTypeProtocol);
    globalEntry.insert(kKeyPolicy, kPolicyDisable);
    globalEntry.insert(kKeyInvoker, "ut-invoker");
    arr.append(globalEntry);
    QFile cfg(cfgDir.filePath("devAccessConfig.json"));
    ASSERT_TRUE(cfg.open(QIODevice::WriteOnly));
    cfg.write(QJsonDocument(arr).toJson());
    cfg.close();

    // Act
    DevPolicyType loaded;
    AccUtils::loadDevPolicy(&loaded);

    // Assert
    EXPECT_EQ(loaded.size(), 1);
    EXPECT_TRUE(loaded.contains(kTypeProtocol));
}

TEST_F(AccessControlServiceTest, SaveAndLoadVaultPolicy_RoundTrip)
{
    // Arrange
    const QVariantMap policy { { kPolicyType, 1 }, { kVaultHideState, 0 }, { kPolicyState, 2 } };

    // Act
    AccUtils::saveVaultPolicy(policy);
    VaultPolicyType loaded;
    AccUtils::loadVaultPolicy(&loaded);

    // Assert
    EXPECT_EQ(loaded.value(kPolicyType), 1);
    EXPECT_EQ(loaded.value(kVaultHideState), 0);
    EXPECT_EQ(loaded.value(kPolicyState), 2);
}

TEST_F(AccessControlServiceTest, LoadVaultPolicy_MalformedJson_LoadsNothing)
{
    // Arrange：预置一条策略，观察畸形 JSON 是否会污染既有内容
    QFile bad(cfgDir.filePath("vaultAccessConfig.json"));
    ASSERT_TRUE(bad.open(QIODevice::WriteOnly));
    bad.write("{{{");
    bad.close();

    // Act
    VaultPolicyType loaded;
    loaded.insert(kPolicyType, 9);
    AccUtils::loadVaultPolicy(&loaded);

    // Assert：解析失败提前返回，不 clear 也不插入新条目，既有内容原样保留
    EXPECT_EQ(loaded.size(), 1);
    EXPECT_EQ(loaded.value(kPolicyType), 9);
}

TEST_F(AccessControlServiceTest, CheckDiskPassword_AllBranches)
{
    // Arrange / Act / Assert
    g_cryptInitRet = -1;
    EXPECT_EQ(AccUtils::checkDiskPassword(nullptr, "pwd", "/dev/sdz1"), kInitFailed);

    g_cryptInitRet = 0;
    g_cryptLoadRet = -2;
    crypt_device *cd = nullptr;
    EXPECT_EQ(AccUtils::checkDiskPassword(&cd, "pwd", "/dev/sdz1"), kDeviceLoadFailed);
    EXPECT_EQ(g_cryptFreeCalls, 1);

    g_cryptLoadRet = 0;
    g_cryptActivateRet = -1;
    EXPECT_EQ(AccUtils::checkDiskPassword(&cd, "pwd", "/dev/sdz1"), kPasswordWrong);
    EXPECT_EQ(g_cryptFreeCalls, 2);

    g_cryptActivateRet = 0;
    EXPECT_EQ(AccUtils::checkDiskPassword(&cd, "pwd", "/dev/sdz1"), kNoError);
}

TEST_F(AccessControlServiceTest, ChangeDiskPassword_AllBranches)
{
    // Arrange
    crypt_device *cd = reinterpret_cast<crypt_device *>(quintptr(0x20));

    // Act / Assert
    g_cryptKeyslotRet = -3;
    EXPECT_EQ(AccUtils::changeDiskPassword(cd, "old", "new"), kPasswordChangeFailed);
    EXPECT_EQ(g_cryptFreeCalls, 1);

    g_cryptKeyslotRet = 2;
    EXPECT_EQ(AccUtils::changeDiskPassword(cd, "old", "new"), kNoError);
    EXPECT_EQ(g_cryptFreeCalls, 2);
}

/******************************************************************************/
/****************************** DBus 层测试 **********************************/
/******************************************************************************/

TEST_F(AccessControlServiceTest, DBus_Lifecycle_ConstructsAndDestructs)
{
    // Arrange / Act
    AccessControlDBus dbus("ut.accesscontrol.service");

    // Assert
    EXPECT_FALSE(dbus.monitor.isNull());
    EXPECT_NE(dbus.adaptor, nullptr);
}

TEST_F(AccessControlServiceTest, CheckAuthentication_DefaultAndStubbed)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");

    // Act
    const bool denied = dbus.checkAuthentication("ut-action");
    g_polkitOk = 1;
    const bool granted = dbus.checkAuthentication("ut-action");

    // Assert
    EXPECT_FALSE(denied);
    EXPECT_TRUE(granted);
}

TEST_F(AccessControlServiceTest, ChangeDiskPassword_AuthFailed_EmitsAuthError)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");
    QSignalSpy checkedSpy(&dbus, &AccessControlDBus::DiskPasswordChecked);
    QSignalSpy changedSpy(&dbus, &AccessControlDBus::DiskPasswordChanged);

    // Act
    dbus.ChangeDiskPassword(QDBusUnixFileDescriptor());

    // Assert
    ASSERT_EQ(checkedSpy.count(), 1);
    EXPECT_EQ(checkedSpy.takeFirst().at(0).toInt(), kAuthenticationFailed);
    EXPECT_EQ(changedSpy.count(), 0);
}

TEST_F(AccessControlServiceTest, ChangeDiskPassword_InvalidFd_EmitsInvalidArgs)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");
    g_polkitOk = 1;
    QSignalSpy spy(&dbus, &AccessControlDBus::DiskPasswordChecked);

    // Act
    dbus.ChangeDiskPassword(QDBusUnixFileDescriptor());

    // Assert
    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toInt(), AccessControlDBus::kInvalidArgs);
}

TEST_F(AccessControlServiceTest, ChangeDiskPassword_EmptyPipe_EmitsInvalidArgs)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");
    g_polkitOk = 1;
    int fds[2];
    ASSERT_EQ(pipe(fds), 0);
    ::close(fds[1]);   // 空管道
    QDBusUnixFileDescriptor fd(fds[0]);
    ::close(fds[0]);
    QSignalSpy spy(&dbus, &AccessControlDBus::DiskPasswordChecked);

    // Act
    dbus.ChangeDiskPassword(fd);

    // Assert
    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toInt(), AccessControlDBus::kInvalidArgs);
}

TEST_F(AccessControlServiceTest, ChangeDiskPassword_MalformedStream_EmitsInvalidArgs)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");
    g_polkitOk = 1;
    int fds[2];
    ASSERT_EQ(pipe(fds), 0);
    (void)!::write(fds[1], "zz", 2);
    ::close(fds[1]);
    QDBusUnixFileDescriptor fd(fds[0]);
    ::close(fds[0]);
    QSignalSpy spy(&dbus, &AccessControlDBus::DiskPasswordChecked);

    // Act
    dbus.ChangeDiskPassword(fd);

    // Assert
    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toInt(), AccessControlDBus::kInvalidArgs);
}

TEST_F(AccessControlServiceTest, ChangeDiskPassword_NoEncryptedDisks_EmitsSuccessAndAsyncFailure)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");
    g_polkitOk = 1;
    g_encryptedDisks = {};
    stub.set_lamda(&DeviceUtils::encryptedDisks, []() -> QStringList {
        return g_encryptedDisks;
    });
    QSignalSpy checkedSpy(&dbus, &AccessControlDBus::DiskPasswordChecked);
    QSignalSpy changedSpy(&dbus, &AccessControlDBus::DiskPasswordChanged);

    // Act
    dbus.ChangeDiskPassword(makeCredentials("old", "new"));
    pumpEvents(900);   // 驱动 QTimer::singleShot(500) 的收尾 lambda

    // Assert
    ASSERT_EQ(checkedSpy.count(), 1);
    EXPECT_EQ(checkedSpy.takeFirst().at(0).toInt(), kNoError);
    ASSERT_EQ(changedSpy.count(), 1);
    EXPECT_EQ(changedSpy.takeFirst().at(0).toInt(), kAccessDiskFailed);
}

TEST_F(AccessControlServiceTest, ChangeDiskPassword_WrongPasswordOnFirstDisk_EmitsWrong)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");
    g_polkitOk = 1;
    g_encryptedDisks = { "/dev/sdz1" };
    stub.set_lamda(&DeviceUtils::encryptedDisks, []() -> QStringList {
        return g_encryptedDisks;
    });
    stubUtilsPasswordFlows();
    g_checkPwdSeq = { kPasswordWrong };
    g_checkPwdCalls = 0;
    QSignalSpy checkedSpy(&dbus, &AccessControlDBus::DiskPasswordChecked);
    QSignalSpy changedSpy(&dbus, &AccessControlDBus::DiskPasswordChanged);

    // Act
    dbus.ChangeDiskPassword(makeCredentials("old", "new"));

    // Assert
    ASSERT_EQ(checkedSpy.count(), 1);
    EXPECT_EQ(checkedSpy.takeFirst().at(0).toInt(), kPasswordWrong);
    EXPECT_EQ(changedSpy.count(), 0);
}

TEST_F(AccessControlServiceTest, ChangeDiskPassword_InconsistentPasswords_RollsBack)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");
    g_polkitOk = 1;
    g_encryptedDisks = { "/dev/sdz1", "/dev/sdz2" };
    stub.set_lamda(&DeviceUtils::encryptedDisks, []() -> QStringList {
        return g_encryptedDisks;
    });
    stub.set_lamda(&FileUtils::decryptString, [](const QString &str) -> QString {
        return str;
    });
    stubUtilsPasswordFlows();
    g_checkPwdSeq = { kNoError, kPasswordWrong, kNoError };   // 正序两块盘 + 回滚一块
    g_changePwdSeq = { kNoError, kNoError };
    g_checkPwdCalls = 0;
    g_changePwdCalls = 0;
    QSignalSpy checkedSpy(&dbus, &AccessControlDBus::DiskPasswordChecked);
    QSignalSpy changedSpy(&dbus, &AccessControlDBus::DiskPasswordChanged);

    // Act
    dbus.ChangeDiskPassword(makeCredentials("old", "new"));

    // Assert
    ASSERT_EQ(checkedSpy.count(), 1);
    EXPECT_EQ(checkedSpy.takeFirst().at(0).toInt(), kNoError);
    ASSERT_EQ(changedSpy.count(), 1);
    EXPECT_EQ(changedSpy.takeFirst().at(0).toInt(), kPasswordInconsistent);
}

TEST_F(AccessControlServiceTest, ChangeDiskPassword_AllDisksOk_EmitsSuccess)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");
    g_polkitOk = 1;
    g_encryptedDisks = { "/dev/sdz1" };
    stub.set_lamda(&DeviceUtils::encryptedDisks, []() -> QStringList {
        return g_encryptedDisks;
    });
    stub.set_lamda(&FileUtils::decryptString, [](const QString &str) -> QString {
        return str;
    });
    stubUtilsPasswordFlows();
    g_checkPwdSeq = { kNoError };
    g_changePwdSeq = { kNoError };
    g_checkPwdCalls = 0;
    g_changePwdCalls = 0;
    QSignalSpy checkedSpy(&dbus, &AccessControlDBus::DiskPasswordChecked);
    QSignalSpy changedSpy(&dbus, &AccessControlDBus::DiskPasswordChanged);

    // Act
    dbus.ChangeDiskPassword(makeCredentials("old", "new"));

    // Assert
    ASSERT_EQ(checkedSpy.count(), 1);
    EXPECT_EQ(checkedSpy.takeFirst().at(0).toInt(), kNoError);
    ASSERT_EQ(changedSpy.count(), 1);
    EXPECT_EQ(changedSpy.takeFirst().at(0).toInt(), kNoError);
}

TEST_F(AccessControlServiceTest, ChangeDiskPassword_FirstDiskLoadFailed_EmitsLoadError)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");
    g_polkitOk = 1;
    g_encryptedDisks = { "/dev/sdz1" };
    stub.set_lamda(&DeviceUtils::encryptedDisks, []() -> QStringList {
        return g_encryptedDisks;
    });
    stub.set_lamda(&FileUtils::decryptString, [](const QString &str) -> QString {
        return str;
    });
    stubUtilsPasswordFlows();
    g_checkPwdSeq = { kDeviceLoadFailed };
    g_checkPwdCalls = 0;
    QSignalSpy checkedSpy(&dbus, &AccessControlDBus::DiskPasswordChecked);
    QSignalSpy changedSpy(&dbus, &AccessControlDBus::DiskPasswordChanged);

    // Act
    dbus.ChangeDiskPassword(makeCredentials("old", "new"));

    // Assert
    ASSERT_EQ(changedSpy.count(), 1);
    EXPECT_EQ(changedSpy.takeFirst().at(0).toInt(), kDeviceLoadFailed);
}

TEST_F(AccessControlServiceTest, Chmod_AuthFailed_ReturnsFalse)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");

    // Act
    const bool ok = dbus.Chmod(cfgDir.path(), 0777);

    // Assert
    EXPECT_FALSE(ok);
}

TEST_F(AccessControlServiceTest, Chmod_EmptyPath_ReturnsFalse)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");
    g_polkitOk = 1;

    // Act
    const bool ok = dbus.Chmod("", 0777);

    // Assert
    EXPECT_FALSE(ok);
}

TEST_F(AccessControlServiceTest, Chmod_ValidPath_AppliesMode)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");
    g_polkitOk = 1;
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString target = dir.filePath("ut-chmod-target");
    QFile f(target);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();

    // Act
    const bool ok = dbus.Chmod(target, 0777);

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_TRUE(QFileInfo(target).permissions() & QFile::ExeOther);
}

TEST_F(AccessControlServiceTest, Chmod_MissingPath_ReturnsFalse)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");
    g_polkitOk = 1;

    // Act
    const bool ok = dbus.Chmod(cfgDir.filePath("no-such-path"), 0777);

    // Assert
    EXPECT_FALSE(ok);
}

TEST_F(AccessControlServiceTest, OnBlockDevAdded_NullDevice_EarlyReturn)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");
    g_fakeDevice = nullptr;

    // Act
    dbus.onBlockDevAdded("ut-device-id");

    // Assert
    EXPECT_EQ(g_powerOffCalls, 0);
}

TEST_F(AccessControlServiceTest, OnBlockDevAdded_NotPowerOffable_EarlyReturn)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");
    makeFakeDevice();
    g_canPowerOff = false;

    // Act
    dbus.onBlockDevAdded("ut-device-id");

    // Assert
    EXPECT_EQ(g_powerOffCalls, 0);
}

TEST_F(AccessControlServiceTest, OnBlockDevAdded_NonUsbBus_EarlyReturn)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");
    makeFakeDevice();
    g_canPowerOff = true;
    g_connBusProp = "sata";

    // Act
    dbus.onBlockDevAdded("ut-device-id");

    // Assert
    EXPECT_EQ(g_powerOffCalls, 0);
}

TEST_F(AccessControlServiceTest, OnBlockDevAdded_NonOpticalMedia_EarlyReturn)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");
    makeFakeDevice();
    g_canPowerOff = true;
    g_connBusProp = "usb";
    g_mediaCompat = { "media" };

    // Act
    dbus.onBlockDevAdded("ut-device-id");

    // Assert
    EXPECT_EQ(g_powerOffCalls, 0);
}

TEST_F(AccessControlServiceTest, OnBlockDevAdded_NoOpticalPolicy_EarlyReturn)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");
    makeFakeDevice();
    g_canPowerOff = true;
    g_mediaCompat = { "optical" };

    // Act
    dbus.onBlockDevAdded("ut-device-id");

    // Assert
    EXPECT_EQ(g_powerOffCalls, 0);
}

TEST_F(AccessControlServiceTest, OnBlockDevAdded_OpticalDisablePolicy_PowersOff)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");
    makeFakeDevice();
    g_canPowerOff = true;
    g_mediaCompat = { "optical_cd", "optical_dvd" };
    dbus.globalDevPolicies.insert(kTypeOptical, { "ut-invoker", kPolicyDisable });

    // Act
    dbus.onBlockDevAdded("ut-device-id");
    QThreadPool::globalInstance()->waitForDone(3000);

    // Assert
    EXPECT_EQ(g_powerOffCalls, 1);
}

TEST_F(AccessControlServiceTest, OnBlockDevAdded_OpticalRwPolicy_NoPowerOff)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");
    makeFakeDevice();
    g_canPowerOff = true;
    g_mediaCompat = { "optical" };
    dbus.globalDevPolicies.insert(kTypeOptical, { "ut-invoker", kPolicyRw });

    // Act
    dbus.onBlockDevAdded("ut-device-id");
    QThreadPool::globalInstance()->waitForDone(3000);

    // Assert
    EXPECT_EQ(g_powerOffCalls, 0);
}

TEST_F(AccessControlServiceTest, OnBlockDevMounted_NullOrSystemDevice_EarlyReturn)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");
    g_fakeDevice = nullptr;

    // Act
    dbus.onBlockDevMounted("ut-device-id", "/media/ut/disk");

    // Assert
    EXPECT_EQ(g_mountCalls, 0);
    EXPECT_EQ(g_setFileModeCalls, 0);
}

TEST_F(AccessControlServiceTest, OnBlockDevMounted_SystemDiskSkipped)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");
    makeFakeDevice();
    g_hintSystem = true;

    // Act
    dbus.onBlockDevMounted("ut-device-id", "/media/ut/disk");

    // Assert
    EXPECT_EQ(g_mountCalls, 0);
}

TEST_F(AccessControlServiceTest, OnBlockDevMounted_BlockRonlyPolicy_RemountsReadOnly)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");
    makeFakeDevice();
    g_devMountPoint = "/media/ut/disk";   // accessMode 可写 ≠ 策略只读 → 触发 remount
    dbus.globalDevPolicies.insert(kTypeBlock, { "ut-invoker", kPolicyRonly });

    // Act
    dbus.onBlockDevMounted("ut-device-id", g_devMountPoint);
    QThreadPool::globalInstance()->waitForDone(3000);

    // Assert
    EXPECT_EQ(g_mountCalls, 1);   // MS_REMOUNT | MS_RDONLY
    EXPECT_EQ(g_umountCalls, 0);
}

TEST_F(AccessControlServiceTest, OnBlockDevMounted_BlockDisablePolicy_DoesNotRemountOrChmod)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");
    makeFakeDevice();
    g_devMountPoint = cfgDir.path();   // accessMode=RW(2) != disable(0) → 进入 disable 分支（无实际动作）
    dbus.globalDevPolicies.insert(kTypeBlock, { "ut-invoker", kPolicyDisable });

    // Act
    dbus.onBlockDevMounted("ut-device-id", g_devMountPoint);
    QThreadPool::globalInstance()->waitForDone(3000);

    // Assert
    EXPECT_EQ(g_mountCalls, 0);
    EXPECT_EQ(g_umountCalls, 0);
    EXPECT_EQ(g_setFileModeCalls, 0);   // policy != Rw → 提前返回
}

TEST_F(AccessControlServiceTest, OnBlockDevMounted_RwPolicy_ChmodsUdisksMount)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");
    makeFakeDevice();
    g_devMountPoint = cfgDir.path();
    dbus.globalDevPolicies.insert(kTypeBlock, { "ut-invoker", kPolicyRw });
    stub.set_lamda(&AccUtils::setFileMode, [](const QString &, uint mode) -> int {
        ++g_setFileModeCalls;
        g_setFileModeLastMode = mode;
        return 0;
    });

    // Act
    dbus.onBlockDevMounted("ut-device-id", g_devMountPoint);
    QThreadPool::globalInstance()->waitForDone(3000);

    // Assert
    EXPECT_EQ(g_setFileModeCalls, 1);
    EXPECT_EQ(g_setFileModeLastMode, 0777);
}

TEST_F(AccessControlServiceTest, ChangeMountedOnInit_NoPolicies_NothingHappens)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");
    g_monitorDevices = { "ut-device-id" };

    // Act
    dbus.changeMountedOnInit();

    // Assert
    EXPECT_EQ(g_mountCalls, 0);
    EXPECT_EQ(g_umountCalls, 0);
}

TEST_F(AccessControlServiceTest, ChangeMountedOnInit_AllPolicies_AppliesEach)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");
    makeFakeDevice();
    dbus.globalDevPolicies.insert(kTypeBlock, { "ut-invoker", kPolicyRonly });
    dbus.globalDevPolicies.insert(kTypeOptical, { "ut-invoker", kPolicyRw });
    dbus.globalDevPolicies.insert(kTypeProtocol, { "ut-invoker", kPolicyRw });
    g_monitorDevices = { "ut-device-id" };
    g_devMountPoint = cfgDir.path();   // 存在的可写目录：accessMode=Rw ≠ 策略 Ronly → 触发重挂载

    // Act
    dbus.changeMountedOnInit();
    QThreadPool::globalInstance()->waitForDone(3000);

    // Assert
    EXPECT_EQ(g_mountCalls, 1);   // block 设备被重挂载
}

TEST_F(AccessControlServiceTest, ChangeMountedBlock_MismatchMode_Remounts)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");
    makeFakeDevice();
    g_monitorDevices = { "ut-device-id" };
    g_devMountPoint = "/media/ut/disk";   // accessMode(mpt) 为 RW ≠ Ronly

    // Act
    dbus.changeMountedBlock(kPolicyRonly, "");
    QThreadPool::globalInstance()->waitForDone(3000);

    // Assert
    EXPECT_EQ(g_mountCalls, 1);
}

TEST_F(AccessControlServiceTest, ChangeMountedBlock_MatchingMode_Skips)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");
    makeFakeDevice();
    g_monitorDevices = { "ut-device-id" };
    g_devMountPoint = cfgDir.path();   // RW == kPolicyRw

    // Act
    dbus.changeMountedBlock(kPolicyRw, "");
    QThreadPool::globalInstance()->waitForDone(3000);

    // Assert
    EXPECT_EQ(g_mountCalls, 0);
}

TEST_F(AccessControlServiceTest, ChangeMountedBlock_DisableMode_Umounts)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");
    makeFakeDevice();
    g_monitorDevices = { "ut-device-id" };
    g_devMountPoint = cfgDir.path();   // 存在的可写目录：accessMode=Rw ≠ Disable → 触发卸载

    // Act
    dbus.changeMountedBlock(kPolicyDisable, "");
    QThreadPool::globalInstance()->waitForDone(3000);

    // Assert
    EXPECT_EQ(g_umountCalls, 1);
    EXPECT_EQ(g_mountCalls, 0);
}

TEST_F(AccessControlServiceTest, ChangeMountedBlock_NonRemovableOrOpticalSkipped)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");
    makeFakeDevice();
    g_monitorDevices = { "ut-device-id" };
    g_removable = false;

    // Act
    dbus.changeMountedBlock(kPolicyRonly, "");
    QThreadPool::globalInstance()->waitForDone(3000);

    // Assert
    EXPECT_EQ(g_mountCalls, 0);
}

TEST_F(AccessControlServiceTest, ChangeMountedOptical_NotDisableMode_NoAction)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");
    g_monitorDevices = { "ut-device-id" };

    // Act
    dbus.changeMountedOptical(kPolicyRw, "");
    QThreadPool::globalInstance()->waitForDone(3000);

    // Assert
    EXPECT_EQ(g_powerOffCalls, 0);
    EXPECT_EQ(g_umountCalls, 0);
}

TEST_F(AccessControlServiceTest, ChangeMountedOptical_DisableWithMountedOptical_UnmountsAndPowersOff)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");
    makeFakeDevice();
    g_monitorDevices = { "ut-device-id" };
    g_mediaCompat = { "optical" };
    g_devMountPoint = "/media/ut/cdrom";
    g_unmountOk = true;

    // Act
    dbus.changeMountedOptical(kPolicyDisable, "");
    QThreadPool::globalInstance()->waitForDone(3000);

    // Assert
    EXPECT_EQ(g_powerOffCalls, 1);
}

TEST_F(AccessControlServiceTest, ChangeMountedOptical_DisableUnmountFailed_NoPowerOff)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");
    makeFakeDevice();
    g_monitorDevices = { "ut-device-id" };
    g_mediaCompat = { "optical" };
    g_devMountPoint = "/media/ut/cdrom";
    g_unmountOk = false;

    // Act
    dbus.changeMountedOptical(kPolicyDisable, "");
    QThreadPool::globalInstance()->waitForDone(3000);

    // Assert
    EXPECT_EQ(g_powerOffCalls, 0);
}

TEST_F(AccessControlServiceTest, ChangeMountedOptical_NoMountPoint_SkipsUnmount)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");
    makeFakeDevice();
    g_monitorDevices = { "ut-device-id" };
    g_mediaCompat = { "optical" };
    g_devMountPoint = QString();

    // Act
    dbus.changeMountedOptical(kPolicyDisable, "");
    QThreadPool::globalInstance()->waitForDone(3000);

    // Assert
    EXPECT_EQ(g_powerOffCalls, 0);
}

TEST_F(AccessControlServiceTest, ChangeMountedProtocol_IsNoOp)
{
    // Arrange
    AccessControlDBus dbus("ut.accesscontrol.service");

    // Act
    dbus.changeMountedProtocol(kPolicyDisable, "/dev/sdz1");

    // Assert
    EXPECT_EQ(g_mountCalls, 0);
    EXPECT_EQ(g_umountCalls, 0);
    EXPECT_EQ(g_powerOffCalls, 0);
}

/******************************************************************************/
/****************************** 插件入口测试 **********************************/
/******************************************************************************/

TEST_F(AccessControlServiceTest, DsmRegisterAndUnRegister_Lifecycle)
{
    // Arrange / Act
    const int reg = DSMRegister("ut.accesscontrol.service", nullptr);
    QThreadPool::globalInstance()->waitForDone(3000);   // activateDiskEncryptService 的后台调用

    // Assert
    EXPECT_EQ(reg, 0);

    // Act
    const int unreg = DSMUnRegister("ut.accesscontrol.service", nullptr);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

    // Assert
    EXPECT_EQ(unreg, 0);
}

TEST_F(AccessControlServiceTest, TR_StaticTranslations_FallBackToSource)
{
    // Arrange / Act：未加载翻译 catalog 时 tr 应回显源文
    const QString tr = AccessControlDBus::tr("Mount");

    // Assert
    EXPECT_EQ(tr, QStringLiteral("Mount"));
    EXPECT_FALSE(AccessControlDBus::tr("Unmount").isEmpty());
}

/******************************************************************************/
/****************************** 日志类别覆盖 **********************************/
/******************************************************************************/

TEST_F(AccessControlServiceTest, LogCategory_RegisteredAndFetchable)
{
    // Arrange
    const QLoggingCategory *viaHelper = &service_accesscontrol::__getLogCategoryName();

    // Act
    const QLoggingCategory *registered = &service_accesscontrol::__logservice_accesscontrol();

    // Assert
    EXPECT_EQ(viaHelper, registered);
    EXPECT_STREQ(registered->categoryName(), "org.deepin.dde.filemanager.plugin.service_accesscontrol");
}
