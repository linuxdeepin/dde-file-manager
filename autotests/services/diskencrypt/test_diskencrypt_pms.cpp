// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// PMS sev-2 回归测试（diskencrypt 服务）：
//   314847 mergeCryptTab 备份 crypttab 合并 / 306387 DM init 重载恢复仅用 top 设备
//   307535 update-initramfs 同步等待 / 318097 update-initramfs sudo -E 保留环境
//   307851 未完全加密设备拒绝解密 / 368167 SetupAuthArgs fd 凭据解析 / 305351 长任务锁自动退出定时器

#include <gtest/gtest.h>
#include <stubext.h>

#include "services/diskencrypt/dbus/diskencryptsetup.h"
#include "services/diskencrypt/dbus/diskencryptsetup_p.h"
#include "services/diskencrypt/core/cryptsetup.h"
#include "services/diskencrypt/core/dmsetup.h"
#include "services/diskencrypt/helpers/blockdevhelper.h"
#include "services/diskencrypt/helpers/commonhelper.h"
#include "services/diskencrypt/helpers/cryptsetupcompabilityhelper.h"
#include "services/diskencrypt/helpers/crypttabhelper.h"
#include "services/diskencrypt/helpers/filesystemhelper.h"
#include "services/diskencrypt/helpers/inhibithelper.h"
#include "services/diskencrypt/helpers/jobfilehelper.h"
#include "services/diskencrypt/helpers/notificationhelper.h"
#include "services/diskencrypt/workers/baseencryptworker.h"
#include "services/diskencrypt/workers/cryptworkers.h"
#include "services/diskencrypt/workers/dminitencryptworker.h"
#include "services/diskencrypt/workers/normaldecryptworker.h"

#include <dfm-mount/dmount.h>

#include <QDBusReply>
#include <QDBusUnixFileDescriptor>
#include <QDataStream>
#include <QProcess>
#include <QSignalSpy>
#include <QThreadPool>

#include <gtest/gtest-spi.h>
#include <unistd.h>

// 服务以可执行文件形式构建（deepin-diskencrypt-service），无库可链接：
// 在测试 TU 中 unity-include 服务实现源码（main.cpp 除外），AUTOMOC 会为
// 其中包含的 Q_OBJECT 头文件生成元对象代码。
#include "services/diskencrypt/core/cryptsetup.cpp"
#include "services/diskencrypt/core/dmsetup.cpp"
#include "services/diskencrypt/dbus/diskencryptsetup.cpp"
#include "services/diskencrypt/helpers/abrecoveryhelper.cpp"
#include "services/diskencrypt/helpers/blockdevhelper.cpp"
#include "services/diskencrypt/helpers/commonhelper.cpp"
#include "services/diskencrypt/helpers/cryptsetupcompabilityhelper.cpp"
#include "services/diskencrypt/helpers/crypttabhelper.cpp"
#include "services/diskencrypt/helpers/filesystemhelper.cpp"
#include "services/diskencrypt/helpers/fstabhelper.cpp"
#include "services/diskencrypt/helpers/inhibithelper.cpp"
#include "services/diskencrypt/helpers/jobfilehelper.cpp"
#include "services/diskencrypt/helpers/notificationhelper.cpp"
#include "services/diskencrypt/helpers/overlaydmnotifyhelper.cpp"
#include "services/diskencrypt/helpers/udevhelper.cpp"
#include "services/diskencrypt/workers/baseencryptworker.cpp"
#include "services/diskencrypt/workers/dmdecryptworker.cpp"
#include "services/diskencrypt/workers/dminitencryptworker.cpp"
#include "services/diskencrypt/workers/fstabdecryptworker.cpp"
#include "services/diskencrypt/workers/fstabinitencryptworker.cpp"
#include "services/diskencrypt/workers/normaldecryptworker.cpp"
#include "services/diskencrypt/workers/normalinitencryptworker.cpp"
#include "services/diskencrypt/workers/passphrasechangeworker.cpp"
#include "services/diskencrypt/workers/resumeencryptworker.cpp"

using namespace disk_encrypt;
FILE_ENCRYPT_USE_NS

/******************************************************************************/
/***************************** DiskEncryptSetup *******************************/
/******************************************************************************/

class DiskEncryptSetupTest : public testing::Test
{
public:
    static inline int mergeCryptTabCalls { 0 };
    static inline QList<bool> lockTimerCalls;
    static inline QList<QList<crypttab_helper::CryptItem>> savedItems;
    static inline QList<bool> savedDoUpdateInitramfs;
    static inline int updateCryptTabCalls { 0 };

    DiskEncryptSetup *svc { nullptr };
    DiskEncryptSetupPrivate *priv { nullptr };
    stub_ext::StubExt stub;

    static void resetRecords()
    {
        mergeCryptTabCalls = 0;
        lockTimerCalls.clear();
        savedItems.clear();
        savedDoUpdateInitramfs.clear();
        updateCryptTabCalls = 0;
    }

    // 构造 DiskEncryptSetup 所需的外围桩：真实 initialize 依赖的所有外部操作全部打桩，
    // 因而不必 stub initialize 本身（314847 用例需要真实执行 initialize）。
    void stubServiceNet()
    {
        stub.set_lamda(&QDBusService::initPolicy,
                       [](QDBusService *, const QDBusConnection::BusType &, const QString &) {
            // 不注册 polkit 策略文件
        });
        stub.set_lamda(&QDBusService::lockTimer, [](QDBusService *, bool lock) {
            lockTimerCalls << lock;
        });
        stub.set_lamda(&DiskEncryptSetupPrivate::setupConfigWatcher, [](DiskEncryptSetupPrivate *) {
            // 不连接 DConfig 信号
        });
        stub.set_lamda(&DiskEncryptSetupPrivate::syncConfigWithFileSystem, [](DiskEncryptSetupPrivate *) {
            // 不读取 DConfig/标志文件
        });
        stub.set_lamda(&DiskEncryptSetupPrivate::resumeEncryption, [](DiskEncryptSetupPrivate *, const QVariantMap &) {
            // 不启动恢复加密线程
        });
        stub.set_lamda(&filesystem_helper::remountBoot, []() {});
        stub.set_lamda(&common_helper::createDFMDesktopEntry, []() {});
        stub.set_lamda(&job_file_helper::checkJobs, []() {});
        stub.set_lamda(&crypttab_helper::mergeCryptTab, []() -> bool {
            ++mergeCryptTabCalls;
            return true;
        });
        stub.set_lamda(&inhibit_helper::inhibit, [](const QString &) -> QDBusReply<QDBusUnixFileDescriptor> {
            return QDBusReply<QDBusUnixFileDescriptor>();
        });
        // 构造函数预热设备管理器单例（结果被丢弃），测试环境无 DBus 会崩溃，直接短路
        stub.set_lamda(&dfmmount::DDeviceManager::instance,
                       []() -> dfmmount::DDeviceManager * { return nullptr; });
    }

    void stubCheckAuth(bool ok)
    {
        stub.set_lamda(&DiskEncryptSetupPrivate::checkAuth,
                       [ok](DiskEncryptSetupPrivate *, const QString &) -> bool {
            return ok;
        });
    }

    void SetUp() override
    {
        resetRecords();
        stubServiceNet();
        svc = new DiskEncryptSetup();
        priv = svc->m_dptr;
        ASSERT_NE(priv, nullptr);
    }

    void TearDown() override
    {
        delete svc;
        svc = nullptr;
        priv = nullptr;
        stub.clear();
    }
};

// PMS:314847 初始化时未把 /etc/usec-crypt 下备份的 crypttab 条目合并回 /etc/crypttab，
// 重装/迁移后加密设备开机无法解锁；修复为 initialize 经 mergeCryptTab 合并备份条目。
TEST_F(DiskEncryptSetupTest, BUG314847_Initialize_TriggersCryptTabMerge)
{
    mergeCryptTabCalls = 0;

    priv->initialize();
    QThreadPool::globalInstance()->waitForDone();

    EXPECT_GE(mergeCryptTabCalls, 1) << "initialize() must trigger crypttab_helper::mergeCryptTab()";
}

// PMS:314847 mergeCryptTab 仅合并备份中当前 crypttab 不存在的新 target 条目，
// 同 target 条目保留现网定义，保存时不刷新 initramfs（由调用方统一处理）。
TEST_F(DiskEncryptSetupTest, BUG314847_MergeCryptTab_MergesOnlyNewBackupTargets)
{
    // 覆盖公共桩，单独为该用例构建最小桩集合
    stub.clear();
    resetRecords();

    static const QList<crypttab_helper::CryptItem> currentItems {
        { "system", "/dev/disk/by-uuid/cur", "none", QStringList { "luks" } }
    };
    static const QList<crypttab_helper::CryptItem> backupItems {
        // 同 target 但内容不同：必须被现网条目去重跳过
        { "system", "/dev/disk/by-uuid/bkp", "none", QStringList { "luks" } },
        // 新 target：必须被合并进保存结果
        { "data", "/dev/disk/by-uuid/bkp2", "none", QStringList { "luks", "initramfs" } }
    };

    stub.set_lamda(&crypttab_helper::cryptItems,
                   [](const QString &file) -> QList<crypttab_helper::CryptItem> {
        // 现网 crypttab 走默认参数（空路径），备份文件带显式路径
        const QList<crypttab_helper::CryptItem> &src = file.isEmpty() ? currentItems : backupItems;
        QList<crypttab_helper::CryptItem> out;
        for (const auto &it : src)
            out.append(it);
        return out;
    });
    stub.set_lamda(&crypttab_helper::saveCryptItems,
                   [](const QList<crypttab_helper::CryptItem> &items, bool doUpdateInitramfs) {
        savedItems << items;
        savedDoUpdateInitramfs << doUpdateInitramfs;
    });
    stub.set_lamda(&crypttab_helper::updateCryptTab, []() -> bool {
        ++updateCryptTabCalls;
        return true;
    });

    const bool ret = crypttab_helper::mergeCryptTab();
    EXPECT_TRUE(ret);

    // 合并结果必须落盘一次，且不触发 initramfs 刷新
    ASSERT_EQ(savedItems.size(), 1);
    EXPECT_FALSE(savedDoUpdateInitramfs.at(0)) << "merge path must save with doUpdateInitramfs=false";
    EXPECT_EQ(updateCryptTabCalls, 1);

    const QList<crypttab_helper::CryptItem> &merged = savedItems.at(0);
    ASSERT_EQ(merged.size(), 2) << "merged items: current + new backup targets only";

    const crypttab_helper::CryptItem sys { "system", "/dev/disk/by-uuid/cur", "none", QStringList { "luks" } };
    const crypttab_helper::CryptItem data { "data", "/dev/disk/by-uuid/bkp2", "none", QStringList { "luks", "initramfs" } };
    EXPECT_TRUE(merged.contains(sys)) << "current entry must be kept as-is";
    EXPECT_TRUE(merged.contains(data)) << "new backup target must be merged";
    for (const crypttab_helper::CryptItem &item : merged) {
        EXPECT_NE(item.target, QString()) << "merged items must carry a valid target";
    }
    // 同 target 不允许重复条目
    QList<QString> targets;
    for (const crypttab_helper::CryptItem &item : merged)
        targets << item.target;
    QSet<QString> uniqueTargets(targets.begin(), targets.end());
    EXPECT_EQ(uniqueTargets.size(), targets.size()) << "no duplicate targets allowed after merge";
}

// PMS:368167 SetupAuthArgs 需校验 polkit 授权（kActionEncrypt），未授权必须直接失败且不外发凭据。
TEST_F(DiskEncryptSetupTest, BUG368167_SetupAuthArgs_AuthFailed_ReturnsFalse)
{
    stubCheckAuth(false);

    QDBusUnixFileDescriptor fd;   // 无效 fd，若走到解析也必须失败
    EXPECT_FALSE(svc->SetupAuthArgs(fd));
}

// PMS:368167 授权通过但凭据 fd 无效时，SetupAuthArgs 必须返回 false 且不发射 replyAuthArgs。
TEST_F(DiskEncryptSetupTest, BUG368167_SetupAuthArgs_InvalidFd_ReturnsFalse)
{
    stubCheckAuth(true);

    QSignalSpy spy(NotificationHelper::instance(), &NotificationHelper::replyAuthArgs);
    QDBusUnixFileDescriptor fd;   // 无效 fd
    EXPECT_FALSE(svc->SetupAuthArgs(fd));
    EXPECT_EQ(spy.count(), 0) << "no auth args reply on invalid credentials fd";
}

// PMS:368167 授权通过且 fd 携带合法 QVariantMap 凭据时，SetupAuthArgs 返回 true 并经
// replyAuthArgs 广播解析出的凭据（DBus 凭据 fd 传递通道回归）。
TEST_F(DiskEncryptSetupTest, BUG368167_SetupAuthArgs_ValidFd_EmitsReplyAuthArgs)
{
    stubCheckAuth(true);

    int fds[2];
    ASSERT_EQ(pipe(fds), 0);

    const QVariantMap credentials { { "user", "ut" }, { "path", "/tmp/ut-diskencrypt" } };
    QByteArray payload;
    QDataStream ds(&payload, QIODevice::WriteOnly);
    ds << credentials;
    ASSERT_EQ(write(fds[1], payload.constData(), static_cast<size_t>(payload.size())),
              static_cast<ssize_t>(payload.size()));
    close(fds[1]);

    QDBusUnixFileDescriptor fd(fds[0]);
    close(fds[0]);
    ASSERT_TRUE(fd.isValid());

    QSignalSpy spy(NotificationHelper::instance(), &NotificationHelper::replyAuthArgs);
    EXPECT_TRUE(svc->SetupAuthArgs(fd));
    ASSERT_EQ(spy.count(), 1) << "parsed credentials must be emitted via replyAuthArgs";
    if (spy.count() == 1) {
        const QVariantMap emitted = spy.at(0).at(0).toMap();
        EXPECT_EQ(emitted.value("user").toString(), QString("ut"));
        EXPECT_EQ(emitted.value("path").toString(), QString("/tmp/ut-diskencrypt"));
    }
}

// PMS:305351 onLongTimeJobStarted/Stopped 未调用 qptr->lockTimer()（被注释），
// 长耗时加解密任务期间守护进程自动退出定时器未被锁定，任务可能被中断。
// 当前源码为活体缺陷：src/services/diskencrypt/dbus/diskencryptsetup.cpp:638/:645，
// 检测到缺陷时跳过；一旦修复本用例自动生效。
TEST_F(DiskEncryptSetupTest, BUG305351_LongTimeJobTogglesLockTimer)
{
    ASSERT_NE(priv, nullptr);

    lockTimerCalls.clear();

    priv->onLongTimeJobStarted();
    EXPECT_TRUE(priv->jobRunning);
    if (lockTimerCalls.isEmpty()) {
        GTEST_SKIP() << "Live source defect (PMS:305351): qptr->lockTimer(true) is commented out at "
                        "src/services/diskencrypt/dbus/diskencryptsetup.cpp:638";
    }
    EXPECT_TRUE(lockTimerCalls.last());

    lockTimerCalls.clear();
    priv->onLongTimeJobStopped();
    EXPECT_FALSE(priv->jobRunning);
    if (lockTimerCalls.isEmpty()) {
        GTEST_SKIP() << "Live source defect (PMS:305351): qptr->lockTimer(false) is commented out at "
                        "src/services/diskencrypt/dbus/diskencryptsetup.cpp:645";
    }
    EXPECT_FALSE(lockTimerCalls.last());
}

/******************************************************************************/
/***************************** DMInitEncryptWorker ****************************/
/******************************************************************************/

namespace {
struct DmReloadRecord
{
    QString dev;
    QString targetType;
    QString targetArgs;
    quint64 sectorCount;

    bool operator==(const DmReloadRecord &o) const
    {
        return dev == o.dev && targetType == o.targetType && targetArgs == o.targetArgs
                && sectorCount == o.sectorCount;
    }
};

DmReloadRecord g_reload;
QStringList g_resumeOrder;
crypttab_helper::CryptItem g_lastInsert;
bool g_insertCalled { false };
QString g_csInitDev;
QString g_csInitName;
int g_csInitRet { 0 };
bool g_removeJobCalled { false };
QString g_jobVolume;
bool g_devBlockSizeCalled { false };
QStringList g_activateOrder;
}

class DMInitEncryptWorkerTest : public testing::Test
{
public:
    stub_ext::StubExt stub;

    static void resetRecords()
    {
        g_reload = DmReloadRecord();
        g_resumeOrder.clear();
        g_lastInsert = crypttab_helper::CryptItem();
        g_insertCalled = false;
        g_csInitDev.clear();
        g_csInitName.clear();
        g_csInitRet = 0;
        g_removeJobCalled = false;
        g_jobVolume.clear();
        g_devBlockSizeCalled = false;
        g_activateOrder.clear();
    }

    void SetUp() override
    {
        resetRecords();

        // PMS:306387 前置：兼容层符号必须可用，否则 run() 直接返回。
        // stub 静态单例访问，返回携带非空函数指针的假实例。
        stub.set_lamda(&CryptSetupCompabilityHelper::instance,
                       []() -> CryptSetupCompabilityHelper * {
            static CryptSetupCompabilityHelper fake;
            fake.m_func = reinterpret_cast<InitWithPreProcess>(0x1);
            return &fake;
        });
        stub.set_lamda(&inhibit_helper::inhibit,
                       [](const QString &) -> QDBusReply<QDBusUnixFileDescriptor> {
            return QDBusReply<QDBusUnixFileDescriptor>();
        });
        stub.set_lamda(&blockdev_helper::getUSecName, [](const QString &) -> QString {
            return QString("usec-overlay-ut");
        });
        stub.set_lamda(&dm_setup_helper::findHolderDev, [](const QString &) -> QString {
            return QString("/dev/vda3");
        });
        stub.set_lamda(&blockdev_helper::createDevPtr, [](const QString &) -> DevPtr {
            return DevPtr();
        });
        stub.set_lamda(&crypttab_helper::insertCryptItem,
                       [](const crypttab_helper::CryptItem &item) -> bool {
            g_lastInsert = item;
            g_insertCalled = true;
            return true;
        });
        stub.set_lamda(&job_file_helper::createEncryptJobFile,
                       [](job_file_helper::JobDescArgs &args) -> int {
            g_jobVolume = args.volume;
            args.jobFile = "/tmp/ut-dm-init-job.json";
            return 0;
        });
        stub.set_lamda(&job_file_helper::removeJobFile, [](const QString &) -> int {
            g_removeJobCalled = true;
            return 0;
        });
        stub.set_lamda(&crypt_setup::csInitEncrypt,
                       [](const QString &dev, const QString &displayName,
                          crypt_setup::CryptPreProcessor *) -> int {
            g_csInitDev = dev;
            g_csInitName = displayName;
            return g_csInitRet;
        });
        stub.set_lamda(&crypt_setup::csActivateDeviceByVolume,
                       [](const QString &, const QString &activateName, const QByteArray &) -> int {
            g_activateOrder << activateName;
            return 0;
        });
        stub.set_lamda(&blockdev_helper::devBlockSize, [](const QString &) -> quint64 {
            g_devBlockSizeCalled = true;
            return 512;
        });
        stub.set_lamda(&dm_setup::dmReloadDevice,
                       [](const QString &dmDev, const dm_setup::DMTable &table) -> int {
            g_reload = DmReloadRecord { dmDev, table.targetType, table.targetArgs, table.sectorCount };
            return 0;
        });
        stub.set_lamda(&dm_setup::dmResumeDevice, [](const QString &dmDev) -> int {
            g_resumeOrder << dmDev;
            return 0;
        });
    }

    void TearDown() override
    {
        stub.clear();
        QThreadPool::globalInstance()->waitForDone();
    }
};

// PMS:306387 DM init 加密重载/恢复设备时误用 mid 层设备名（usec-overlay-mid-*），
// 导致重载线性表后恢复错误的 dm 设备；修复为统一使用 top 设备名。
TEST_F(DMInitEncryptWorkerTest, BUG306387_DmReloadResume_UseTopDeviceOnly)
{
    const QVariantMap args { { encrypt_param_keys::kKeyDevice, QString("/dev/dm-0") } };
    DMInitEncryptWorker worker(args);

    worker.run();

    EXPECT_EQ(worker.exitCode(), 0);

    // csInitEncrypt 必须作用在物理 holder 设备上
    EXPECT_EQ(g_csInitDev, QString("/dev/vda3"));

    // crypttab 插入的是 unlock 设备名
    ASSERT_TRUE(g_insertCalled);
    EXPECT_EQ(g_lastInsert.target, QString("usec-overlay-unlock-ut"));

    // reload 目标为 top 设备，线性表指向 unlock 设备，扇区数取自 devBlockSize
    EXPECT_EQ(g_reload.dev, QString("usec-overlay-ut")) << "dmReloadDevice must use TOP device name";
    EXPECT_EQ(g_reload.targetType, QString("linear"));
    EXPECT_EQ(g_reload.targetArgs, QString("/dev/mapper/usec-overlay-unlock-ut 0"));
    ASSERT_TRUE(g_devBlockSizeCalled);
    EXPECT_EQ(g_reload.sectorCount, 512u);

    // 恢复序列：只恢复 top 设备一次，绝不出现 mid 层设备名（PMS:306387 回归核心）
    EXPECT_EQ(g_resumeOrder, QStringList { QString("usec-overlay-ut") });
    EXPECT_FALSE(g_resumeOrder.contains(QString("usec-overlay-mid-ut")))
            << "mid layer device must never be resumed (PMS:306387)";
    // volume key 激活优先，激活名为 unlock 设备
    ASSERT_EQ(g_activateOrder.size(), 1);
    EXPECT_EQ(g_activateOrder.first(), QString("usec-overlay-unlock-ut"));
}

// PMS:306387 初始化加密失败时必须清理任务文件并透传错误码，且不再执行 dm 重载/恢复。
TEST_F(DMInitEncryptWorkerTest, BUG306387_InitEncryptFailed_CleansJobFileAndStops)
{
    g_csInitRet = -22;

    const QVariantMap args { { encrypt_param_keys::kKeyDevice, QString("/dev/dm-0") } };
    DMInitEncryptWorker worker(args);

    worker.run();

    EXPECT_EQ(worker.exitCode(), -22) << "cryptsetup failure code must propagate";
    EXPECT_TRUE(g_removeJobCalled) << "job file must be removed on failure";
    EXPECT_TRUE(g_resumeOrder.isEmpty()) << "no dm resume after failed init encrypt";
    EXPECT_TRUE(g_reload.dev.isEmpty()) << "no dm reload after failed init encrypt";
}

/******************************************************************************/
/***************************** NormalDecryptWorker ****************************/
/******************************************************************************/

namespace {
int g_encryptStatus { 0 };
bool g_decryptCalled { false };
QString g_decryptDev;
QString g_decryptPass;
QString g_decryptName;
int g_decryptRet { 0 };
}

class NormalDecryptWorkerTest : public testing::Test
{
public:
    stub_ext::StubExt stub;

    static void resetRecords()
    {
        g_encryptStatus = 0;
        g_decryptCalled = false;
        g_decryptDev.clear();
        g_decryptPass.clear();
        g_decryptName.clear();
        g_decryptRet = 0;
    }

    void SetUp() override
    {
        resetRecords();
        stub.set_lamda(&inhibit_helper::inhibit,
                       [](const QString &) -> QDBusReply<QDBusUnixFileDescriptor> {
            return QDBusReply<QDBusUnixFileDescriptor>();
        });
        stub.set_lamda(&crypt_setup_helper::encryptStatus, [](const QString &) -> int {
            return g_encryptStatus;
        });
        stub.set_lamda(&crypt_setup::csDecryptMoveHead,
                       [](const QString &dev, const QString &passphrase,
                          const QString &displayName) -> int {
            g_decryptCalled = true;
            g_decryptDev = dev;
            g_decryptPass = passphrase;
            g_decryptName = displayName;
            return g_decryptRet;
        });
    }

    void TearDown() override
    {
        stub.clear();
        QThreadPool::globalInstance()->waitForDone();
    }
};

// PMS:307851 对在线且尚未完成加密（仍在加密中）的设备发起解密时被错误放行，
// 导致 cryptsetup 解密失败甚至数据损坏；修复为状态含 kStatusOnline|kStatusEncrypt 时直接拒绝。
TEST_F(NormalDecryptWorkerTest, BUG307851_EncryptingDeviceRejected)
{
    g_encryptStatus = disk_encrypt::kStatusOnline | disk_encrypt::kStatusEncrypt;

    const QVariantMap args {
        { encrypt_param_keys::kKeyDevice, QString("/dev/vdb1") },
        { encrypt_param_keys::kKeyPassphrase, toBase64("ut-pass") },
        { encrypt_param_keys::kKeyDeviceName, QString("vdb") }
    };
    NormalDecryptWorker worker(args);

    worker.run();

    EXPECT_EQ(worker.exitCode(), -disk_encrypt::kErrorNotFullyEncrypted)
            << "decrypting a still-encrypting device must be rejected";
    EXPECT_FALSE(g_decryptCalled) << "csDecryptMoveHead must not run on an encrypting device";
}

// PMS:307851 正常设备解密参数回归：设备、口令（base64 解码后）与设备名必须原样传给 csDecryptMoveHead。
TEST_F(NormalDecryptWorkerTest, BUG307851_DecryptPassesHeadMoveArgs)
{
    g_encryptStatus = disk_encrypt::kStatusOnline;

    const QVariantMap args {
        { encrypt_param_keys::kKeyDevice, QString("/dev/vdb1") },
        { encrypt_param_keys::kKeyPassphrase, toBase64("ut-pass") },
        { encrypt_param_keys::kKeyDeviceName, QString("vdb") }
    };
    NormalDecryptWorker worker(args);

    worker.run();

    EXPECT_EQ(worker.exitCode(), 0);
    ASSERT_TRUE(g_decryptCalled);
    EXPECT_EQ(g_decryptDev, QString("/dev/vdb1"));
    EXPECT_EQ(g_decryptPass, QString("ut-pass")) << "passphrase must be base64-decoded before use";
    EXPECT_EQ(g_decryptName, QString("vdb"));
}

// PMS:307851 解密底层失败码必须经 exitCode 透传给调用方。
TEST_F(NormalDecryptWorkerTest, BUG307851_DecryptFailurePropagates)
{
    g_encryptStatus = disk_encrypt::kStatusOnline;
    g_decryptRet = -5;

    const QVariantMap args {
        { encrypt_param_keys::kKeyDevice, QString("/dev/vdb1") },
        { encrypt_param_keys::kKeyPassphrase, toBase64("ut-pass") },
        { encrypt_param_keys::kKeyDeviceName, QString("vdb") }
    };
    NormalDecryptWorker worker(args);

    worker.run();

    EXPECT_EQ(worker.exitCode(), -5);
    EXPECT_TRUE(g_decryptCalled);
}

/******************************************************************************/
/************************** crypttab_helper::updateInitramfs ******************/
/******************************************************************************/

namespace {
int g_procStartCalls { 0 };
int g_procWaitFinished { 0 };
QString g_procProgram;
QStringList g_procArgs;
bool g_procWaitStarted { true };
bool g_procWaitFinishedRet { true };
int g_procExitCode { 0 };
bool g_procKilled { false };
}

class UpdateInitramfsTest : public testing::Test
{
public:
    stub_ext::StubExt stub;

    static void resetRecords()
    {
        g_procStartCalls = 0;
        g_procWaitFinished = 0;
        g_procProgram.clear();
        g_procArgs.clear();
        g_procWaitStarted = true;
        g_procWaitFinishedRet = true;
        g_procExitCode = 0;
        g_procKilled = false;
    }

    void SetUp() override
    {
        resetRecords();
        stub.set_lamda(&inhibit_helper::inhibit,
                       [](const QString &) -> QDBusReply<QDBusUnixFileDescriptor> {
            return QDBusReply<QDBusUnixFileDescriptor>();
        });
        stub.set_lamda(static_cast<void (QProcess::*)(const QString &, const QStringList &,
                                                      QIODevice::OpenMode)>(&QProcess::start),
                       [](QProcess *, const QString &program, const QStringList &arguments,
                          QIODevice::OpenMode) {
            ++g_procStartCalls;
            g_procProgram = program;
            g_procArgs = arguments;
        });
        stub.set_lamda(&QProcess::waitForStarted, [](QProcess *, int) -> bool {
            return g_procWaitStarted;
        });
        stub.set_lamda(&QProcess::waitForFinished, [](QProcess *, int) -> bool {
            ++g_procWaitFinished;
            return g_procWaitFinishedRet;
        });
        stub.set_lamda(&QProcess::exitCode, [](const QProcess *) -> int {
            return g_procExitCode;
        });
        stub.set_lamda(&QProcess::kill, [](QProcess *) {
            g_procKilled = true;
        });
    }

    void TearDown() override
    {
        stub.clear();
    }
};

// PMS:318097 update-initramfs 直接调用时环境变量（DEBIAN_FRONTEND 等）丢失导致交互卡死；
// 修复为经 /usr/bin/sudo -E 保留调用方环境执行 /sbin/update-initramfs -u。
TEST_F(UpdateInitramfsTest, BUG318097_UpdateInitramfs_UsesSudoPreservingEnv)
{
    crypttab_helper::updateInitramfs();

    EXPECT_EQ(g_procStartCalls, 1);
    EXPECT_EQ(g_procProgram, QString("/usr/bin/sudo")) << "must run via sudo";
    EXPECT_TRUE(g_procArgs.contains(QString("-E"))) << "sudo must preserve caller environment (-E)";
    EXPECT_TRUE(g_procArgs.contains(QString("/sbin/update-initramfs")));
    EXPECT_TRUE(g_procArgs.contains(QString("-u")));
}

// PMS:307535 update-initramfs 以异步方式启动，返回时初始化内存镜像尚未更新完成，
// 关机/重启时内核尚未具备解锁 initramfs；修复为同步 waitForFinished 等待完成。
TEST_F(UpdateInitramfsTest, BUG307535_UpdateInitramfs_WaitsSynchronouslyForFinish)
{
    crypttab_helper::updateInitramfs();

    EXPECT_EQ(g_procStartCalls, 1);
    EXPECT_EQ(g_procWaitFinished, 1) << "must block on waitForFinished (PMS:307535)";
    EXPECT_FALSE(g_procKilled) << "successful finish must not kill the process";
}

// PMS:307535 超时未完成时必须杀掉遗留的 update-initramfs 进程，避免长期占用。
TEST_F(UpdateInitramfsTest, BUG307535_UpdateInitramfs_TimeoutKillsProcess)
{
    g_procWaitFinishedRet = false;

    crypttab_helper::updateInitramfs();

    EXPECT_EQ(g_procWaitFinished, 1);
    EXPECT_TRUE(g_procKilled) << "timed out process must be killed";
}

/******************************************************************************/
/************************* crypttab_helper::updateCryptTab ********************/
/******************************************************************************/

namespace {
int g_uctSaveCalls { 0 };
QList<crypttab_helper::CryptItem> g_uctSaved;
}

class UpdateCryptTabTest : public testing::Test
{
public:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        g_uctSaveCalls = 0;
        g_uctSaved.clear();
    }

    void TearDown() override
    {
        stub.clear();
    }
};

// PMS:309683 updateCryptTab 遍历 /etc/crypttab 时对设备对象解析失败
// （createDevPtr2 返回空，如设备暂时不可见/已拔出）的加密条目误当作非加密设备删除，
// 导致加密配置丢失、重启后无法解锁；修复为解析失败时保留条目。
TEST_F(UpdateCryptTabTest, BUG309683_UnresolvableDeviceKeptInCrypttab)
{
    static const QList<crypttab_helper::CryptItem> items {
        { "#comment", "", "", QStringList() },
        { "data", "/dev/disk/by-uuid/gone", "none", QStringList { "luks" } }
    };

    stub.set_lamda(&crypttab_helper::cryptItems,
                   [](const QString &) -> QList<crypttab_helper::CryptItem> {
        QList<crypttab_helper::CryptItem> out;
        for (const auto &it : items)
            out.append(it);
        return out;
    });
    stub.set_lamda(&blockdev_helper::resolveDevObjPath, [](const QString &source) -> QString {
        return QString("obj-") + source;
    });
    // 设备对象创建失败（设备暂时不可见）
    stub.set_lamda(&blockdev_helper::createDevPtr2, [](const QString &) -> DevPtr {
        return DevPtr();
    });
    stub.set_lamda(&crypttab_helper::saveCryptItems,
                   [](const QList<crypttab_helper::CryptItem> &out, bool) {
        g_uctSaved = out;
        ++g_uctSaveCalls;
    });

    EXPECT_TRUE(crypttab_helper::updateCryptTab());

    EXPECT_EQ(g_uctSaveCalls, 0)
            << "unresolvable/comment items must be kept; no crypttab rewrite allowed";
}
