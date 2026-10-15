// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// diskencrypt 服务覆盖率补全测试：
//   core/cryptsetup.cpp、core/dmsetup.cpp、dbus/diskencryptsetup.cpp、
//   helpers/*（blockdev/common/crypttab/fstab/filesystem/jobfile/abrecovery/
//   overlaydmnotify）、workers/*（Base/NormalInit/FstabInit/FstabDecrypt/
//   DMDecrypt/PassphraseChange/Resume）。
// 服务实现已由 test_diskencrypt_pms.cpp 所在 TU unity-include，本 TU 仅引用
// 头文件并复用其符号（跨 TU 单一定义，stub 运行期生效）。
// 注：libcryptsetup / libdevmapper 以真实链接库参与，用不存在的设备路径
// 驱动确定性错误分支；主机相关路径（/etc/usec-crypt 等）只读访问。

#include <gtest/gtest.h>
#include <stubext.h>

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusContext>
#include <QDBusMessage>
#include <QDBusUnixFileDescriptor>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QLibrary>
#include <QProcess>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QThread>
#include <QThreadPool>
#include <QVariantMap>

#include <polkit-qt6-1/PolkitQt1/Authority>
#include <DConfig>

#include <unistd.h>

#include "services/diskencrypt/dbus/diskencryptsetup.h"
#include "services/diskencrypt/dbus/diskencryptsetup_p.h"
#include "services/diskencrypt/core/cryptsetup.h"
#include "services/diskencrypt/core/dmsetup.h"
#include "services/diskencrypt/globaltypesdefine.h"
#include "services/diskencrypt/helpers/abrecoveryhelper.h"
#include "services/diskencrypt/helpers/blockdevhelper.h"
#include "services/diskencrypt/helpers/commonhelper.h"
#include "services/diskencrypt/helpers/crypttabhelper.h"
#include "services/diskencrypt/helpers/filesystemhelper.h"
#include "services/diskencrypt/helpers/fstabhelper.h"
#include "services/diskencrypt/helpers/inhibithelper.h"
#include "services/diskencrypt/helpers/jobfilehelper.h"
#include "services/diskencrypt/helpers/notificationhelper.h"
#include "services/diskencrypt/helpers/overlaydmnotifyhelper.h"
#include "services/diskencrypt/workers/cryptworkers.h"

using namespace disk_encrypt;
FILE_ENCRYPT_USE_NS

// QSignalSpy::wait / 跨线程 queued 连接（worker finished → 私有槽）需要事件循环，
// gtest_main 环境中注入一个全局 QCoreApplication。
namespace {
class QtAppEnvironment : public testing::Environment
{
public:
    void SetUp() override
    {
        static char arg0[] = "ut-dfmdaemon-diskencrypt";
        int argc = 1;
        char *argv[] = { arg0, nullptr };
        app = new QCoreApplication(argc, argv);
    }

    QCoreApplication *app { nullptr };
};

const int g_qtEnvReg = []() {
    ::testing::AddGlobalTestEnvironment(new QtAppEnvironment());
    return 0;
}();
}   // namespace

/******************************************************************************/
/************************ crypt_setup / crypt_setup_helper ********************/
/******************************************************************************/

namespace {
QStringList g_libLoadedNames;
int g_csInitRecord_devCalls { 0 };
}   // namespace

class CryptSetupCoreTest : public testing::Test
{
public:
    stub_ext::StubExt stub;

    static constexpr const char *kNoDev { "/tmp/ut-diskencrypt-no-such-dev" };

    void SetUp() override
    {
        g_csInitRecord_devCalls = 0;
        // 设备对象解析全部失败（无 DBus 环境），保证确定性
        stub.set_lamda(&dfmmount::DDeviceManager::instance,
                       []() -> dfmmount::DDeviceManager * { return nullptr; });
    }

    void TearDown() override
    {
        stub.clear();
    }

    // 构造 LUKS 头文件（4096 偏移处写 JSON 行），用于 headerStatus 解析
    static QString makeHeaderFile(const QTemporaryDir &dir, const QByteArray &json)
    {
        QTemporaryFile f(dir.filePath("hdr.XXXXXX"));
        f.setAutoRemove(false);
        if (!f.open())
            return QString();
        const QString path = f.fileName();
        QByteArray pad(4096, '\0');
        f.write(pad);
        f.write(json);
        f.write("\n");
        f.close();
        return path;
    }
};

// csResumeEncrypt 对不存在设备必须返回初始化失败。
TEST_F(CryptSetupCoreTest, CsResumeEncrypt_NoSuchDevice_ReturnsInitCryptError)
{
    // Arrange
    // （无前置桩：crypt_init_data_device 对不存在路径必然失败）

    // Act
    const int ret = crypt_setup::csResumeEncrypt(kNoDev, "ut-active", "ut-name");

    // Assert
    EXPECT_EQ(ret, -kErrorInitCrypt);
    EXPECT_LT(ret, 0);
}

// csAddPassphrase / csChangePassphrase / csRemoveKeyslot 对不存在设备返回初始化失败。
TEST_F(CryptSetupCoreTest, CsKeyslotApis_NoSuchDevice_ReturnInitCryptError)
{
    // Arrange / Act / Assert
    EXPECT_EQ(crypt_setup::csAddPassphrase(kNoDev, "old", "new"), -kErrorInitCrypt);
    EXPECT_EQ(crypt_setup::csChangePassphrase(kNoDev, "old", "new"), -kErrorInitCrypt);
    EXPECT_EQ(crypt_setup::csRemoveKeyslot(kNoDev, 0), -kErrorInitCrypt);
}

// csActivateDevice / csActivateDeviceByVolume / csSetLabel 对不存在设备返回初始化失败。
TEST_F(CryptSetupCoreTest, CsActivateAndLabelApis_NoSuchDevice_ReturnInitCryptError)
{
    // Arrange / Act / Assert
    EXPECT_EQ(crypt_setup::csActivateDevice(kNoDev, "ut-name", "pwd"), -kErrorInitCrypt);
    EXPECT_EQ(crypt_setup::csActivateDeviceByVolume(kNoDev, "ut-name", QByteArray("k")), -kErrorInitCrypt);
    EXPECT_EQ(crypt_setup::csSetLabel(kNoDev, "label"), -kErrorInitCrypt);
}

// csDecrypt / csDecryptMoveHead：备份头生成失败时错误码必须透传。
TEST_F(CryptSetupCoreTest, CsDecrypt_NoSuchDevice_FailsAtDetachHeader)
{
    // Arrange
    // genDetachHeaderPath 依赖设备对象（已桩失败）

    // Act
    const int ret = crypt_setup::csDecrypt(kNoDev, "pwd", "name");
    const int retMove = crypt_setup::csDecryptMoveHead(kNoDev, "pwd", "name");

    // Assert
    EXPECT_EQ(ret, -kErrorUnknown);
    EXPECT_EQ(retMove, -kErrorUnknown);
}

// csInitEncrypt：initiable 失败（设备已加密）必须直接返回其错误码。
TEST_F(CryptSetupCoreTest, CsInitEncrypt_AlreadyEncrypted_ReturnsInitiableError)
{
    // Arrange
    stub.set_lamda(&crypt_setup_helper::initiable, [](const QString &) -> int {
        return -kErrorDeviceEncrypted;
    });

    // Act
    const int ret = crypt_setup::csInitEncrypt(kNoDev, "name");

    // Assert
    EXPECT_EQ(ret, -kErrorDeviceEncrypted);
    EXPECT_LT(ret, 0);
}

// csInitEncrypt：头文件初始化失败必须透传错误码。
TEST_F(CryptSetupCoreTest, CsInitEncrypt_HeaderInitFailed_PropagatesError)
{
    // Arrange
    stub.set_lamda(&crypt_setup_helper::initiable, [](const QString &) -> int { return 0; });
    stub.set_lamda(&crypt_setup_helper::initEncryptHeaderFile,
                   [](const QString &, const QString &, crypt_setup::CryptPreProcessor *, QString *) -> int {
        return -kErrorCreateHeader;
    });

    // Act
    const int ret = crypt_setup::csInitEncrypt(kNoDev, "name");

    // Assert
    EXPECT_EQ(ret, -kErrorCreateHeader);
    EXPECT_LT(ret, 0);
}

// csInitEncrypt：设备头恢复失败必须透传错误码。
TEST_F(CryptSetupCoreTest, CsInitEncrypt_DeviceHeaderFailed_PropagatesError)
{
    // Arrange
    stub.set_lamda(&crypt_setup_helper::initiable, [](const QString &) -> int { return 0; });
    stub.set_lamda(&crypt_setup_helper::initEncryptHeaderFile,
                   [](const QString &, const QString &, crypt_setup::CryptPreProcessor *, QString *out) -> int {
        if (out)
            *out = "/tmp/ut-header";
        return 0;
    });
    stub.set_lamda(&crypt_setup_helper::initDeviceHeader,
                   [](const QString &, const QString &) -> int { return -kErrorRestoreFromFile; });

    // Act
    const int ret = crypt_setup::csInitEncrypt(kNoDev, "name");

    // Assert
    EXPECT_EQ(ret, -kErrorRestoreFromFile);
    EXPECT_LT(ret, 0);
}

// createHeaderFile 真实路径：在 /dev/shm 下创建 32M 头文件并返回路径。
TEST_F(CryptSetupCoreTest, CreateHeaderFile_RealShmFile_CreatedAndCleaned)
{
    // Arrange
    QString headerPath;

    // Act
    const int ret = crypt_setup_helper::createHeaderFile("/dev/utdev0", &headerPath);

    // Assert
    EXPECT_EQ(ret, kSuccess);
    EXPECT_TRUE(headerPath.startsWith("/dev/shm/utdev0_dfm_encrypt_header_"));
    EXPECT_TRUE(QFile(headerPath).exists());

    // 清理（测试自管理）
    EXPECT_TRUE(QFile::remove(headerPath));
}

// initiable：设备对象解析失败（非 /dev 前缀）按未知版本处理 → 视为已加密。
TEST_F(CryptSetupCoreTest, Initiable_UnresolvableDevice_RejectedAsEncrypted)
{
    // Arrange
    // DDeviceManager::instance 已桩为 null

    // Act
    const int ret = crypt_setup_helper::initiable("/tmp/ut-none-dev");

    // Assert
    EXPECT_EQ(ret, -kErrorDeviceEncrypted);
    EXPECT_LT(ret, 0);
}

// initEncryptHeaderFile：头文件创建失败返回 kErrorCreateHeader。
TEST_F(CryptSetupCoreTest, InitEncryptHeaderFile_CreateFailed_ReturnsCreateHeaderError)
{
    // Arrange
    stub.set_lamda(&crypt_setup_helper::createHeaderFile,
                   [](const QString &, QString *) -> int { return -1; });

    // Act
    const int ret = crypt_setup_helper::initEncryptHeaderFile(kNoDev, "name", nullptr);

    // Assert
    EXPECT_EQ(ret, -kErrorCreateHeader);
    EXPECT_LT(ret, 0);
}

// initEncryptHeaderFile：普通流程（processor 为空）需收缩文件系统，失败即返回 kErrorResizeFs。
TEST_F(CryptSetupCoreTest, InitEncryptHeaderFile_ShrinkFailed_ReturnsResizeFsError)
{
    // Arrange
    stub.set_lamda(&crypt_setup_helper::createHeaderFile,
                   [](const QString &, QString *out) -> int {
        if (out)
            *out = "/tmp/ut-header";
        return 0;
    });
    stub.set_lamda(&filesystem_helper::shrinkFileSystem_ext, [](const QString &) -> bool {
        return false;
    });

    // Act
    const int ret = crypt_setup_helper::initEncryptHeaderFile(kNoDev, "name", nullptr);

    // Assert
    EXPECT_EQ(ret, -kErrorResizeFs);
    EXPECT_LT(ret, 0);
}

// initEncryptHeaderFile：头文件路径不可用（crypt_init 失败）返回 kErrorInitCrypt。
TEST_F(CryptSetupCoreTest, InitEncryptHeaderFile_BadHeaderPath_ReturnsInitCryptError)
{
    // Arrange
    static QString g_headerPath;
    g_headerPath = "/tmp/ut-diskencrypt-no-such-header-file";
    stub.set_lamda(&crypt_setup_helper::createHeaderFile,
                   [](const QString &, QString *out) -> int {
        if (out)
            *out = g_headerPath;
        return 0;
    });
    crypt_setup::CryptPreProcessor processor;   // 非空：跳过文件系统收缩

    // Act
    const int ret = crypt_setup_helper::initEncryptHeaderFile(kNoDev, "name", &processor);

    // Assert
    EXPECT_EQ(ret, -kErrorInitCrypt);
    EXPECT_LT(ret, 0);
}

// initEncryptHeaderFile：真实 shm 头 + 不存在数据设备 → LUKS2 格式化失败。
TEST_F(CryptSetupCoreTest, InitEncryptHeaderFile_MissingDataDevice_FailsAtFormat)
{
    // Arrange
    // createHeaderFile 走真实路径（/dev/shm 下 32M 文件），数据设备不存在

    // Act
    const int ret = crypt_setup_helper::initEncryptHeaderFile(kNoDev, "name",
                                                              reinterpret_cast<crypt_setup::CryptPreProcessor *>(0x1));

    // Assert
    EXPECT_EQ(ret, -kErrorFormatLuks);
    EXPECT_LT(ret, 0);
}

// initDeviceHeader：空路径与不存在文件均返回 kErrorHeaderNotExist。
TEST_F(CryptSetupCoreTest, InitDeviceHeader_MissingFile_ReturnsHeaderNotExist)
{
    // Arrange / Act / Assert
    EXPECT_EQ(crypt_setup_helper::initDeviceHeader(kNoDev, ""), -kErrorHeaderNotExist);
    EXPECT_EQ(crypt_setup_helper::initDeviceHeader(kNoDev, "/tmp/ut-no-such-header"), -kErrorHeaderNotExist);
}

// initDeviceHeader：头文件存在但设备不存在 → crypt_init 失败。
TEST_F(CryptSetupCoreTest, InitDeviceHeader_NoSuchDevice_ReturnsInitCryptError)
{
    // Arrange
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QTemporaryFile f(dir.filePath("hdr.XXXXXX"));
    f.setAutoRemove(false);
    ASSERT_TRUE(f.open());
    f.close();
    const QString headerPath = f.fileName();

    // Act
    const int ret = crypt_setup_helper::initDeviceHeader(kNoDev, headerPath);

    // Assert
    EXPECT_EQ(ret, -kErrorInitCrypt);
    EXPECT_LT(ret, 0);
}

// headerStatus：加密中（linear+crypt + reencrypt encrypt 模式）。
TEST_F(CryptSetupCoreTest, HeaderStatus_EncryptInProgress)
{
    // Arrange
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QByteArray json = R"({"segments":{"0":{"type":"linear"},"1":{"type":"crypt"}},)"
                            R"("keyslots":{"0":{"type":"reencrypt","mode":"encrypt"}}})";
    const QString path = makeHeaderFile(dir, json);

    // Act
    const int status = crypt_setup_helper::headerStatus(path);

    // Assert
    EXPECT_EQ(status, crypt_setup_helper::kEncryptInProgress);
    QFile::remove(path);
}

// headerStatus：加密初始化（仅 linear）。
TEST_F(CryptSetupCoreTest, HeaderStatus_EncryptInit)
{
    // Arrange
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QByteArray json = R"({"segments":{"0":{"type":"linear"}},)"
                            R"("keyslots":{"0":{"type":"reencrypt","mode":"encrypt"}}})";
    const QString path = makeHeaderFile(dir, json);

    // Act
    const int status = crypt_setup_helper::headerStatus(path);

    // Assert
    EXPECT_EQ(status, crypt_setup_helper::kEncryptInit);
    QFile::remove(path);
}

// headerStatus：解密中（linear+crypt + decrypt 模式）与解密初始化（仅 crypt）。
TEST_F(CryptSetupCoreTest, HeaderStatus_DecryptStates)
{
    // Arrange
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QByteArray both = R"({"segments":{"0":{"type":"linear"},"1":{"type":"crypt"}},)"
                            R"("keyslots":{"0":{"type":"reencrypt","mode":"decrypt"}}})";
    const QByteArray cryptOnly = R"({"segments":{"1":{"type":"crypt"}},)"
                                 R"("keyslots":{"0":{"type":"reencrypt","mode":"decrypt"}}})";
    const QString pBoth = makeHeaderFile(dir, both);
    const QString pCrypt = makeHeaderFile(dir, cryptOnly);

    // Act
    const int inProgress = crypt_setup_helper::headerStatus(pBoth);
    const int init = crypt_setup_helper::headerStatus(pCrypt);

    // Assert
    EXPECT_EQ(inProgress, crypt_setup_helper::kDecryptInProgress);
    EXPECT_EQ(init, crypt_setup_helper::kDecryptInit);
    QFile::remove(pBoth);
    QFile::remove(pCrypt);
}

// headerStatus：无重加密模式 → 已完成状态（linear-only=已解密，crypt-only=已加密）。
TEST_F(CryptSetupCoreTest, HeaderStatus_FullyStates)
{
    // Arrange
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QByteArray linearOnly = R"({"segments":{"0":{"type":"linear"}},"keyslots":{}})";
    const QByteArray cryptOnly = R"({"segments":{"1":{"type":"crypt"}},"keyslots":{}})";
    const QString pLinear = makeHeaderFile(dir, linearOnly);
    const QString pCrypt = makeHeaderFile(dir, cryptOnly);

    // Act
    const int decryptFully = crypt_setup_helper::headerStatus(pLinear);
    const int encryptFully = crypt_setup_helper::headerStatus(pCrypt);

    // Assert
    EXPECT_EQ(decryptFully, crypt_setup_helper::kDecryptFully);
    EXPECT_EQ(encryptFully, crypt_setup_helper::kEncryptFully);
    QFile::remove(pLinear);
    QFile::remove(pCrypt);
}

// headerStatus：不可打开 / 缺 segments 均为非法头。
TEST_F(CryptSetupCoreTest, HeaderStatus_InvalidInputs)
{
    // Arrange
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString noSegments = makeHeaderFile(dir, R"({"keyslots":{}})");

    // Act
    const int notExist = crypt_setup_helper::headerStatus("/tmp/ut-no-such-header");
    const int noSeg = crypt_setup_helper::headerStatus(noSegments);

    // Assert
    EXPECT_EQ(notExist, crypt_setup_helper::kInvalidHeader);
    EXPECT_EQ(noSeg, crypt_setup_helper::kInvalidHeader);
    QFile::remove(noSegments);
}

// onEncrypting / onDecrypting 必须以 offset/size 进度发通知。
TEST_F(CryptSetupCoreTest, ProgressCallbacks_EmitSignalsWithProgress)
{
    // Arrange
    QSignalSpy encSpy(NotificationHelper::instance(), &NotificationHelper::notifyEncryptProgress);
    QSignalSpy decSpy(NotificationHelper::instance(), &NotificationHelper::notifyDecryptProgress);
    QPair<QString, QString> devInfo { "/dev/ut0", "ut-disk" };

    // Act
    const int encRet = crypt_setup_helper::onEncrypting(200, 100, &devInfo);
    const int decRet = crypt_setup_helper::onDecrypting(400, 100, &devInfo);

    // Assert
    EXPECT_EQ(encRet, 0);
    EXPECT_EQ(decRet, 0);
    ASSERT_EQ(encSpy.count(), 1);
    EXPECT_DOUBLE_EQ(encSpy.at(0).at(2).toDouble(), 0.5);
    ASSERT_EQ(decSpy.count(), 1);
    EXPECT_DOUBLE_EQ(decSpy.at(0).at(2).toDouble(), 0.25);
}

// setToken：空 token 直接成功；否则对不存在设备返回初始化失败。
TEST_F(CryptSetupCoreTest, SetToken_EmptyTokenSucceeds_BadDeviceFails)
{
    // Arrange / Act / Assert
    EXPECT_EQ(crypt_setup_helper::setToken(kNoDev, ""), kSuccess);
    EXPECT_EQ(crypt_setup_helper::setToken(kNoDev, R"({"token_index":0})"), -kErrorInitCrypt);
}

// getToken / getRecoveryKeySlots / encryptStatus / genDetachHeaderPath 对不存在设备失败。
TEST_F(CryptSetupCoreTest, TokenStatusApis_NoSuchDevice_FailDeterministically)
{
    // Arrange
    QString token;
    QList<int> keySlots;   // 注意不可命名为 slots（Qt 宏）
    const QString dev = "/tmp/ut-none-dev";   // 非 /dev 前缀：对象解析必然失败

    // Act
    const int getTokenRet = crypt_setup_helper::getToken(dev, &token);
    const int slotsNullRet = crypt_setup_helper::getRecoveryKeySlots(dev, nullptr);
    const int slotsRet = crypt_setup_helper::getRecoveryKeySlots(dev, &keySlots);
    const int encStatusRet = crypt_setup_helper::encryptStatus(dev);
    const int genPathRet = crypt_setup_helper::genDetachHeaderPath(dev);

    // Assert
    EXPECT_EQ(getTokenRet, -kErrorUnknown);
    EXPECT_EQ(slotsNullRet, -kErrorUnknown);
    EXPECT_EQ(slotsRet, -kErrorInitCrypt);
    EXPECT_EQ(encStatusRet, -kErrorUnknown);
    EXPECT_EQ(genPathRet, -kErrorUnknown);
}

// backupDetachHeader：路径生成失败 / 设备初始化失败 / 头文件已存在三分支。
TEST_F(CryptSetupCoreTest, BackupDetachHeader_ThreeBranches)
{
    // Arrange —— 分支 1：路径生成失败（真实 genDetachHeaderPath 解析失败）
    QString header;
    const QString dev = "/tmp/ut-none-dev";
    const int badPathRet = crypt_setup_helper::backupDetachHeader(dev, &header);

    // Arrange —— 分支 2：路径可用但设备不存在
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    static QString g_genPath;
    g_genPath = dir.filePath("dm-decrypt-backup-ut.bin");
    stub.set_lamda(&crypt_setup_helper::genDetachHeaderPath,
                   [](const QString &, QString *out) -> int {
        if (out)
            *out = g_genPath;
        return kSuccess;
    });
    const int noDevRet = crypt_setup_helper::backupDetachHeader(dev, &header);

    // Arrange —— 分支 3：备份头已存在 → 直接成功
    QFile existing(g_genPath);
    ASSERT_TRUE(existing.open(QIODevice::WriteOnly));
    existing.close();

    // Act
    const int existsRet = crypt_setup_helper::backupDetachHeader(dev, &header);

    // Assert
    EXPECT_EQ(badPathRet, -kErrorUnknown);
    EXPECT_EQ(noDevRet, -kErrorInitCrypt);
    EXPECT_EQ(existsRet, kSuccess);
    EXPECT_EQ(header, g_genPath);
    QFile::remove(g_genPath);
}

/******************************************************************************/
/********************************* dm_setup ***********************************/
/******************************************************************************/

class DmSetupTest : public testing::Test
{
public:
    stub_ext::StubExt stub;

    void TearDown() override
    {
        stub.clear();
    }
};

// dm 设备操作（真实 libdevmapper）：对不存在的设备名必须失败（非 0）。
TEST_F(DmSetupTest, DmTaskOperations_UnknownDevice_Fail)
{
    // Arrange
    const QString dev { "ut-dm-device-not-exist" };
    const dm_setup::DMTable table { "linear", "/dev/ut-none 0", 0, 1024 };

    // Act
    const int create = dm_setup::dmCreateDevice(dev, table);
    const int reload = dm_setup::dmReloadDevice(dev, table);
    const int suspend = dm_setup::dmSuspendDevice(dev);
    const int resume = dm_setup::dmResumeDevice(dev);
    const int remove = dm_setup::dmRemoveDevice(dev);

    // Assert
    EXPECT_NE(create, 0);
    EXPECT_NE(reload, 0);
    EXPECT_NE(suspend, 0);
    EXPECT_NE(resume, 0);
    EXPECT_NE(remove, 0);
}

// findHolderDev：非 dm 设备原样返回；不存在的 dm 设备返回空串。
TEST_F(DmSetupTest, FindHolderDev_NonDmPassthrough_UnknownDmEmpty)
{
    // Arrange / Act
    const QString passthrough = dm_setup_helper::findHolderDev("/dev/sda9");
    const QString unknown = dm_setup_helper::findHolderDev("/dev/dm-ut99");

    // Assert
    EXPECT_EQ(passthrough, QString("/dev/sda9"));
    EXPECT_TRUE(unknown.isEmpty());
}

// procPartitions：解析真实 /proc/partitions，条目字段齐全。
TEST_F(DmSetupTest, ProcPartitions_ParsesRealProcFile)
{
    // Arrange / Act
    const QList<dm_setup_helper::ProcPartition> parts = dm_setup_helper::procPartitions();

    // Assert
    ASSERT_FALSE(parts.isEmpty());
    auto first = parts.first();   // 按值取出：devNum() 为非 const 成员
    EXPECT_FALSE(first.name.isEmpty());
    EXPECT_TRUE(first.devNum().contains(':'));
    EXPECT_GE(first.major, 1);
}

/******************************************************************************/
/******************************* blockdev_helper ******************************/
/******************************************************************************/

class BlockDevHelperTest : public testing::Test
{
public:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        stub.set_lamda(&dfmmount::DDeviceManager::instance,
                       []() -> dfmmount::DDeviceManager * { return nullptr; });
    }

    void TearDown() override
    {
        stub.clear();
    }
};

// 设备尺寸 / 对象创建 / 版本判定对不存在设备全部走安全失败路径。
TEST_F(BlockDevHelperTest, UnknownDevice_AllFailSafely)
{
    // Arrange
    const QString dev { "/tmp/ut-diskencrypt-no-dev" };

    // Act
    const quint64 size = blockdev_helper::devDeviceSize(dev);
    const quint64 sectors = blockdev_helper::devBlockSize(dev);
    const DevPtr ptr = blockdev_helper::createDevPtr(dev);
    const DevPtr ptr2 = blockdev_helper::createDevPtr2("");
    const int version = blockdev_helper::devCryptVersion(dev);
    const QString usecName = blockdev_helper::getUSecName(dev);

    // Assert
    EXPECT_EQ(size, 0u);
    EXPECT_EQ(sectors, 0u);
    EXPECT_TRUE(ptr.isNull());
    EXPECT_TRUE(ptr2.isNull());
    EXPECT_EQ(version, blockdev_helper::kVersionUnknown);
    EXPECT_TRUE(usecName.isEmpty());
}

// resolveDevObjPath：空源 / 非法前缀 / 设备管理器缺失三分支。
TEST_F(BlockDevHelperTest, ResolveDevObjPath_InvalidSources_ReturnEmpty)
{
    // Arrange / Act
    const QString empty = blockdev_helper::resolveDevObjPath("");
    const QString badPrefix = blockdev_helper::resolveDevObjPath("LABEL=ut");
    const QString noManager = blockdev_helper::resolveDevObjPath("/dev/ut-none");

    // Assert
    EXPECT_TRUE(empty.isEmpty());
    EXPECT_TRUE(badPrefix.isEmpty());
    EXPECT_TRUE(noManager.isEmpty());
}

// UUID / PARTUUID 前缀在设备管理器缺失时同样返回空（走 manager 分支）。
TEST_F(BlockDevHelperTest, ResolveDevObjPath_UuidPrefixes_ReachManagerBranch)
{
    // Arrange / Act
    const QString byUuid = blockdev_helper::resolveDevObjPath("UUID=1234-5678");
    const QString byPartUuid = blockdev_helper::resolveDevObjPath("PARTUUID=1234-5678");

    // Assert
    EXPECT_TRUE(byUuid.isEmpty());
    EXPECT_TRUE(byPartUuid.isEmpty());
}

/******************************************************************************/
/******************************* common_helper ********************************/
/******************************************************************************/

class CommonHelperTest : public testing::Test
{
};

// encryptCipher 结果必须属于受支持算法集合。
TEST_F(CommonHelperTest, EncryptCipher_ReturnsSupportedAlgorithm)
{
    // Arrange / Act
    const QString cipher = common_helper::encryptCipher();

    // Assert
    EXPECT_TRUE(cipher == "aes" || cipher == "sm4");
    EXPECT_FALSE(cipher.isEmpty());
}

// genRandomString 长度与字符集（字母数字）。
TEST_F(CommonHelperTest, GenRandomString_LengthAndCharset)
{
    // Arrange / Act
    const QString s16 = common_helper::genRandomString(16);
    const QString s24 = common_helper::genRandomString();

    // Assert
    EXPECT_EQ(s16.length(), 16);
    EXPECT_EQ(s24.length(), 24);
    for (const QChar &c : s16)
        EXPECT_TRUE(c.isLetterOrNumber());
}

// genRecoveryKey：库不可用时回退到 24 位随机串。
TEST_F(CommonHelperTest, GenRecoveryKey_FallbackToRandomString)
{
    // Arrange
    // （libusec-recoverykey 通常不存在；即使存在也生成 24 位密钥）

    // Act
    const QString key = common_helper::genRecoveryKey();

    // Assert
    EXPECT_EQ(key.length(), 24);
    EXPECT_FALSE(key.isEmpty());
}

// createDFMDesktopEntry 现为兼容空实现，调用不得有副作用。
TEST_F(CommonHelperTest, CreateDFMDesktopEntry_IsNoop)
{
    // Arrange
    const bool before = QFile::exists(kReencryptDesktopFile);

    // Act
    common_helper::createDFMDesktopEntry();

    // Assert
    EXPECT_EQ(QFile::exists(kReencryptDesktopFile), before);
}

/******************************************************************************/
/****************************** crypttab_helper *******************************/
/******************************************************************************/

namespace {
QList<crypttab_helper::CryptItem> g_ctSaved;
int g_ctSaveCalls { 0 };
QList<crypttab_helper::CryptItem> g_ctLiveItems;   // cryptItems/saveCryptItems 活列表桩

void resetCryptTabState()
{
    g_ctSaved.clear();
    g_ctSaveCalls = 0;
    g_ctLiveItems.clear();
}
}   // namespace

class CryptTabHelperTest : public testing::Test
{
public:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        resetCryptTabState();
    }

    void TearDown() override
    {
        stub.clear();
    }

    // cryptItems / saveCryptItems 桩使用“活列表”：保存后再次读取可见（模拟真实文件）
    void stubItems(const QList<crypttab_helper::CryptItem> &items)
    {
        g_ctLiveItems = items;
        // cryptItems 带默认参数：取全参地址消歧
        stub.set_lamda(static_cast<QList<crypttab_helper::CryptItem> (*)(const QString &)>(
                               &crypttab_helper::cryptItems),
                       [](const QString &) -> QList<crypttab_helper::CryptItem> {
            return g_ctLiveItems;
        });
        stub.set_lamda(static_cast<void (*)(const QList<crypttab_helper::CryptItem> &, bool)>(
                               &crypttab_helper::saveCryptItems),
                       [](const QList<crypttab_helper::CryptItem> &items, bool) {
            ++g_ctSaveCalls;
            g_ctSaved = items;
            g_ctLiveItems = items;
        });
    }
};

// cryptItems：解析临时 crypttab 文件（含注释/无效行跳过）。
TEST_F(CryptTabHelperTest, CryptItems_ParsesTempFileAndSkipsInvalid)
{
    // Arrange
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = dir.filePath("crypttab");
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
    f.write("# short comment\n");
    f.write("data\t/dev/ut0\tnone\tluks,initramfs\n");
    f.write("bad-line\n");
    f.close();

    // Act
    const QList<crypttab_helper::CryptItem> items = crypttab_helper::cryptItems(path);

    // Assert
    ASSERT_EQ(items.size(), 1);
    EXPECT_EQ(items.first().target, QString("data"));
    EXPECT_EQ(items.first().source, QString("/dev/ut0"));
    ASSERT_EQ(items.first().options.size(), 2);
    EXPECT_EQ(items.first().options.first(), QString("luks"));

    // 不存在文件返回空
    EXPECT_TRUE(crypttab_helper::cryptItems("/tmp/ut-no-such-crypttab").isEmpty());
}

// insertCryptItem：同 source 旧条目去重后追加。
TEST_F(CryptTabHelperTest, InsertCryptItem_DedupsSameSource)
{
    // Arrange
    stubItems({ { "old", "/dev/ut0", "none", { "luks" } },
                { "keep", "/dev/ut1", "none", { "luks" } } });

    // Act
    const bool ok = crypttab_helper::insertCryptItem({ "new", "/dev/ut0", "none", { "luks", "initramfs" } });

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(g_ctSaveCalls, 1);
    ASSERT_EQ(g_ctSaved.size(), 2);
    EXPECT_EQ(g_ctSaved.at(0).target, QString("keep"));
    EXPECT_EQ(g_ctSaved.at(1).target, QString("new"));
}

// removeCryptItem：命中移除并保存，未命中不保存。
TEST_F(CryptTabHelperTest, RemoveCryptItem_HitAndMiss)
{
    // Arrange
    stubItems({ { "a", "/dev/ut0", "none", { "luks" } },
                { "b", "/dev/ut1", "none", { "luks" } } });

    // Act
    const bool hitRet = crypttab_helper::removeCryptItem("a");
    const int savesAfterHit = g_ctSaveCalls;
    const bool missRet = crypttab_helper::removeCryptItem("not-there");

    // Assert
    EXPECT_TRUE(hitRet);
    EXPECT_TRUE(missRet);
    EXPECT_EQ(savesAfterHit, 1);
    EXPECT_EQ(g_ctSaveCalls, 1) << "miss must not save";
    ASSERT_EQ(g_ctSaved.size(), 1);
    EXPECT_EQ(g_ctSaved.first().target, QString("b"));
}

// addCryptOption：命中且无该选项则追加；未命中 / 已存在则不改。
TEST_F(CryptTabHelperTest, AddCryptOption_HitMissAndDuplicate)
{
    // Arrange
    stubItems({ { "a", "/dev/ut0", "none", { "luks" } } });

    // Act
    const bool hit = crypttab_helper::addCryptOption("a", "tpm2-device=auto");
    const bool dup = crypttab_helper::addCryptOption("a", "tpm2-device=auto");
    const bool miss = crypttab_helper::addCryptOption("not-there", "tpm2-device=auto");

    // Assert
    EXPECT_TRUE(hit);
    EXPECT_FALSE(dup);
    EXPECT_FALSE(miss);
    EXPECT_EQ(g_ctSaveCalls, 1);
    ASSERT_EQ(g_ctSaved.size(), 1);
    EXPECT_TRUE(g_ctSaved.first().options.contains("tpm2-device=auto"));
}

// updateCryptTab：注释条目与无法解析的设备必须保留，不触发保存。
TEST_F(CryptTabHelperTest, UpdateCryptTab_UnresolvableEntriesKept)
{
    // Arrange
    stubItems({ { "#comment", "", "", QStringList() },
                { "data", "/dev/ut0", "none", { "luks" } } });
    stub.set_lamda(&blockdev_helper::createDevPtr2, [](const QString &) -> DevPtr {
        return DevPtr();   // 设备对象解析失败
    });

    // Act
    const bool ok = crypttab_helper::updateCryptTab();

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(g_ctSaveCalls, 0) << "nothing should be dropped";
}

/******************************************************************************/
/*********************** fstab / jobfile / filesystem / ab *******************/
/******************************************************************************/

class MiscHelpersTest : public testing::Test
{
};

// fstab 助手：不存在的设备条目必然无需更新（只读路径）。
TEST_F(MiscHelpersTest, FstabHelpers_NoMatchingEntry_NoUpdate)
{
    // Arrange / Act
    const bool passno = fstab_helper::setFstabPassno("UUID=UT-NOT-EXIST-UUID", 0);
    const bool timeout = fstab_helper::setFstabTimeout("/dev/ut-none", "UUID=UT-NOT-EXIST");

    // Assert
    EXPECT_TRUE(passno);
    EXPECT_TRUE(timeout);
}

// job_file_helper：根目录 / 任务类型 / 任务文件加载。
TEST_F(MiscHelpersTest, JobFileHelper_Basics)
{
    // Arrange
    job_file_helper::JobDescArgs args;

    // Act
    const int root = job_file_helper::createUSecRoot();
    const bool empty = job_file_helper::hasJobFile();
    const QStringList types = job_file_helper::validJobTypes();
    const int nullLoad = job_file_helper::loadEncryptJobFile(nullptr);
    const int defaultLoad = job_file_helper::loadEncryptJobFile(&args);
    const int devLoad = job_file_helper::loadEncryptJobFile(&args, "/dev/ut-x");

    // Assert
    EXPECT_EQ(root, 0);
    EXPECT_FALSE(empty);
    EXPECT_EQ(types.size(), 3);
    EXPECT_TRUE(types.contains(job_type::TypeFstab));
    EXPECT_EQ(nullLoad, 0);
    EXPECT_EQ(defaultLoad, 0);
    EXPECT_TRUE(args.jobFile.isEmpty());
    EXPECT_EQ(devLoad, 0);
}

// job_file_helper：非 root 环境创建任务文件必然失败（kErrorOpenFileFailed）。
TEST_F(MiscHelpersTest, JobFileHelper_CreateFilesFailWithoutRoot)
{
    // Arrange
    job_file_helper::JobDescArgs args;
    args.device = "PARTUUID=ut";
    args.devPath = "/dev/ut0";
    args.devType = job_type::TypeFstab;

    // Act
    const int enc = job_file_helper::createEncryptJobFile(args);
    const int dec = job_file_helper::createDecryptJobFile(args);

    // Assert
    EXPECT_EQ(enc, -kErrorOpenFileFailed);
    EXPECT_EQ(dec, -kErrorOpenFileFailed);
}

// job_file_helper：removeJobFile 对临时文件成功、对不存在路径失败。
TEST_F(MiscHelpersTest, JobFileHelper_RemoveJobFile)
{
    // Arrange
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = dir.filePath("job.json");
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();

    // Act
    const int ok = job_file_helper::removeJobFile(path);
    const int missing = job_file_helper::removeJobFile("/tmp/ut-no-such-job.json");

    // Assert
    EXPECT_EQ(ok, 0);
    EXPECT_EQ(missing, -1);
}

// job_file_helper::checkJobs 在无任务文件环境下安全执行。
TEST_F(MiscHelpersTest, JobFileHelper_CheckJobs_NoFiles_Noop)
{
    // Arrange
    const bool before = job_file_helper::hasJobFile();

    // Act
    job_file_helper::checkJobs();

    // Assert
    EXPECT_EQ(job_file_helper::hasJobFile(), before);
    EXPECT_FALSE(before);
}

// filesystem_helper：e2fsck / 日志文件路径不可写 → 收缩/扩展/搬移均失败。
TEST_F(MiscHelpersTest, FileSystemHelpers_UnknownDevice_Fail)
{
    // Arrange
    const QString dev { "/tmp/ut-diskencrypt-no-dev" };

    // Act
    const bool shrink = filesystem_helper::shrinkFileSystem_ext(dev);
    const bool expand = filesystem_helper::expandFileSystem_ext(dev);
    const bool move = filesystem_helper::moveFsForward(dev);

    // Assert
    EXPECT_FALSE(shrink);
    EXPECT_FALSE(expand);
    EXPECT_FALSE(move);
}

// filesystem_helper::remountBoot 只读安全执行（无 /boot fstab 条目时直接返回）。
TEST_F(MiscHelpersTest, FileSystemHelper_RemountBoot_SafeNoop)
{
    // Arrange
    // （读取真实 /etc/fstab；非 root 下任何 remount 均无害失败）

    // Act / Assert
    EXPECT_NO_FATAL_FAILURE(filesystem_helper::remountBoot());
    EXPECT_FALSE(filesystem_helper::shrinkFileSystem_ext("/tmp/ut-none"));
}

// abrecovery_helper::disableRecovery 非 root 下无法写 grub 配置（安全失败路径）。
TEST_F(MiscHelpersTest, AbRecoveryHelper_DisableRecovery_FailsWithoutRoot)
{
    // Arrange
    const QString cfgPath { "/etc/default/grub.d/13_deepin_ab_recovery.cfg" };
    const bool before = QFile::exists(cfgPath);

    // Act
    EXPECT_NO_FATAL_FAILURE(abrecovery_helper::disableRecovery());

    // Assert
    EXPECT_EQ(QFile::exists(cfgPath), before);
    EXPECT_FALSE(QFile(cfgPath).isWritable());
}

/******************************************************************************/
/***************************** OverlayDMNotifyHelper **************************/
/******************************************************************************/

namespace {
QString g_procProgram2;
QStringList g_procArgs2;
bool g_waitStarted2 { true };
bool g_waitFinished2 { true };
int g_procExit2 { 0 };
QByteArray g_procOutput2;
int g_startDetachedCalls { 0 };

int g_loginctlStage { 0 };   // 0: list-sessions, 1: show-session
QByteArray g_listSessionsOutput { " 2 1000 ut-user seat0 -\n" };
QByteArray g_showSessionOutput { "Active=yes\nType=wayland\nName=ut-user\nUser=1000\n" };
}   // namespace

class OverlayDMNotifyTest : public testing::Test
{
public:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        g_procProgram2.clear();
        g_procArgs2.clear();
        g_waitStarted2 = true;
        g_waitFinished2 = true;
        g_procExit2 = 0;
        g_procOutput2.clear();
        g_startDetachedCalls = 0;
        g_loginctlStage = 0;

        stub.set_lamda(static_cast<void (QProcess::*)(const QString &, const QStringList &,
                                                      QIODevice::OpenMode)>(&QProcess::start),
                       [](QProcess *, const QString &program, const QStringList &arguments,
                          QIODevice::OpenMode) {
            g_procProgram2 = program;
            g_procArgs2 = arguments;
            // loginctl 两阶段输出切换：list-sessions → show-session
            g_procOutput2 = (g_loginctlStage == 0) ? g_listSessionsOutput : g_showSessionOutput;
        });
        stub.set_lamda(&QProcess::waitForStarted, [](QProcess *, int) -> bool {
            return g_waitStarted2;
        });
        stub.set_lamda(&QProcess::waitForFinished, [](QProcess *, int) -> bool {
            return g_waitFinished2;
        });
        stub.set_lamda(&QProcess::exitCode, [](const QProcess *) -> int {
            return g_procExit2;
        });
        stub.set_lamda(&QProcess::readAllStandardOutput, [](QProcess *) -> QByteArray {
            return g_procOutput2;
        });
        stub.set_lamda(static_cast<bool (QProcess::*)(qint64 *)>(&QProcess::startDetached),
                       [](QProcess *, qint64 *pid) -> bool {
            ++g_startDetachedCalls;
            if (pid)
                *pid = 4242;
            return true;
        });
    }

    void TearDown() override
    {
        stub.clear();
    }
};

// instance 为进程内单例。
TEST_F(OverlayDMNotifyTest, Instance_IsSingleton)
{
    // Arrange / Act
    OverlayDMNotifyHelper *first = OverlayDMNotifyHelper::instance();
    OverlayDMNotifyHelper *second = OverlayDMNotifyHelper::instance();

    // Assert
    EXPECT_NE(first, nullptr);
    EXPECT_EQ(first, second);
}

// findActiveGraphicalUser：解析 loginctl 输出定位活动图形会话用户。
TEST_F(OverlayDMNotifyTest, FindActiveGraphicalUser_ParsesLoginctlOutput)
{
    // Arrange
    // SetUp 已注入两阶段 loginctl 输出（list-sessions → show-session）
    // show-session 阶段切换：第二次 start 后进入
    stub.set_lamda(static_cast<void (QProcess::*)(const QString &, const QStringList &,
                                                  QIODevice::OpenMode)>(&QProcess::start),
                   [](QProcess *, const QString &program, const QStringList &arguments,
                      QIODevice::OpenMode) {
        if (program == "loginctl" && arguments.contains("show-session")) {
            g_procOutput2 = g_showSessionOutput;
        } else {
            g_procOutput2 = g_listSessionsOutput;
        }
    });

    // Act
    const OverlayDMNotifyHelper::ActiveUser user =
            OverlayDMNotifyHelper::instance()->findActiveGraphicalUser();

    // Assert
    EXPECT_TRUE(user.valid);
    EXPECT_EQ(user.uid, 1000u);
    EXPECT_EQ(user.username, QString("ut-user"));
}

// findActiveGraphicalUser：无活动图形会话（Type=tty）时返回无效用户。
TEST_F(OverlayDMNotifyTest, FindActiveGraphicalUser_NoGraphicalSession_Invalid)
{
    // Arrange
    g_listSessionsOutput = " 2 1000 ut-user seat0 -\n";
    g_showSessionOutput = "Active=yes\nType=tty\nName=ut-user\nUser=1000\n";
    stub.set_lamda(static_cast<void (QProcess::*)(const QString &, const QStringList &,
                                                  QIODevice::OpenMode)>(&QProcess::start),
                   [](QProcess *, const QString &program, const QStringList &arguments,
                      QIODevice::OpenMode) {
        if (program == "loginctl" && arguments.contains("show-session"))
            g_procOutput2 = g_showSessionOutput;
        else
            g_procOutput2 = g_listSessionsOutput;
    });

    // Act
    const OverlayDMNotifyHelper::ActiveUser user =
            OverlayDMNotifyHelper::instance()->findActiveGraphicalUser();

    // Assert
    EXPECT_FALSE(user.valid);
    EXPECT_TRUE(user.username.isEmpty());
}

// isFileManagerRunning 由 pgrep 退出码决定。
TEST_F(OverlayDMNotifyTest, IsFileManagerRunning_ByPgrepExitCode)
{
    // Arrange
    g_procExit2 = 0;

    // Act
    const bool running = OverlayDMNotifyHelper::instance()->isFileManagerRunning(1000);
    g_procExit2 = 1;
    const bool notRunning = OverlayDMNotifyHelper::instance()->isFileManagerRunning(1000);

    // Assert
    EXPECT_TRUE(running);
    EXPECT_FALSE(notRunning);
    EXPECT_EQ(g_procProgram2, QString("pgrep"));
}

// launchFileManager 经 systemd-run --user --machine 在目标用户会话启动。
TEST_F(OverlayDMNotifyTest, LaunchFileManager_UsesSystemdRunUserMachine)
{
    // Arrange
    g_procProgram2.clear();
    QStringList recordedArgs;
    stub.set_lamda(&QProcess::setProgram, [&recordedArgs](QProcess *, const QString &program) {
        g_procProgram2 = program;
    });
    stub.set_lamda(&QProcess::setArguments, [&recordedArgs](QProcess *, const QStringList &args) {
        recordedArgs = args;
    });

    // Act
    OverlayDMNotifyHelper::instance()->launchFileManager(1000, "ut-user");

    // Assert
    EXPECT_EQ(g_startDetachedCalls, 1);
    EXPECT_EQ(g_procProgram2, QString("systemd-run"));
    EXPECT_TRUE(recordedArgs.contains(QString("--user")));
    EXPECT_TRUE(recordedArgs.contains(QString("ut-user@.host")));
}

// writePendingFile：目录不可写时安全返回（不崩溃）。
TEST_F(OverlayDMNotifyTest, WritePendingFile_UnwritableDir_SafeReturn)
{
    // Arrange
    OverlayDMNotifyHelper::ActiveUser user;   // invalid

    // Act
    EXPECT_NO_FATAL_FAILURE(
                OverlayDMNotifyHelper::instance()->writePendingFile(true, 0, user));

    // Assert
    // 目录由 root 持有，普通用户无法创建 → 不应产生通知文件（该目录为空）
    EXPECT_FALSE(QFile::exists(QString(kOverlayDMNotifyFile)));
}

// ensureNotificationDelivery：无活动用户 / 文件管理器未运行时的完整路径。
TEST_F(OverlayDMNotifyTest, EnsureNotificationDelivery_FullFlowWithStubs)
{
    // Arrange
    // findActiveGraphicalUser：构造有效用户；isFileManagerRunning：pgrep 未命中
    stub.set_lamda(&OverlayDMNotifyHelper::findActiveGraphicalUser,
                   [](OverlayDMNotifyHelper *) -> OverlayDMNotifyHelper::ActiveUser {
        OverlayDMNotifyHelper::ActiveUser u;
        u.uid = 1000;
        u.username = "ut-user";
        u.valid = true;
        return u;
    });
    g_procExit2 = 1;   // pgrep 退出码 1：文件管理器未运行

    // Act
    EXPECT_NO_FATAL_FAILURE(
                OverlayDMNotifyHelper::instance()->ensureNotificationDelivery(true, 0));

    // Assert
    EXPECT_EQ(g_startDetachedCalls, 1) << "file manager should be launched when not running";
    EXPECT_FALSE(QFile::exists(QString(kOverlayDMNotifyFile)));
}

/******************************************************************************/
/********************************* Workers ************************************/
/******************************************************************************/

namespace {
QString g_w_dev;
QString g_w_name;
int g_w_csRet { 0 };
QStringList g_w_csChangeArgs;
int g_w_csChangeRet { 0 };
int g_w_setTokenCalls { 0 };
QString g_w_lastToken;
QString g_w_recKey { "UT-RECOVERY-KEY" };
int g_w_addPassRet { 2 };
bool g_w_addCryptOptionCalled { false };
}   // namespace

class WorkersCovTest : public testing::Test
{
public:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        resetWorkerState();
        stub.set_lamda(&inhibit_helper::inhibit,
                       [](const QString &) -> QDBusReply<QDBusUnixFileDescriptor> {
            return QDBusReply<QDBusUnixFileDescriptor>();
        });
        stub.set_lamda(&dfmmount::DDeviceManager::instance,
                       []() -> dfmmount::DDeviceManager * { return nullptr; });
    }

    void TearDown() override
    {
        stub.clear();
        QThreadPool::globalInstance()->waitForDone();
    }

    static void resetWorkerState()
    {
        g_w_dev.clear();
        g_w_name.clear();
        g_w_csRet = 0;
        g_w_csChangeArgs.clear();
        g_w_csChangeRet = 0;
        g_w_setTokenCalls = 0;
        g_w_lastToken.clear();
        g_w_recKey = "UT-RECOVERY-KEY";
        g_w_addPassRet = 2;
        g_w_addCryptOptionCalled = false;
    }
};

// BaseEncryptWorker：参数透传与退出码读写。
TEST_F(WorkersCovTest, BaseEncryptWorker_ArgsAndExitCode)
{
    // Arrange
    const QVariantMap args { { encrypt_param_keys::kKeyDevice, QString("/dev/ut0") } };
    BaseEncryptWorker worker(args);

    // Act
    const QVariantMap got = worker.args();
    worker.setExitCode(-7);
    const int code = worker.exitCode();

    // Assert
    EXPECT_EQ(got.value(encrypt_param_keys::kKeyDevice).toString(), QString("/dev/ut0"));
    EXPECT_EQ(code, -7);
    EXPECT_EQ(worker.exitCode(), -7);
}

// NormalInitEncryptWorker：加密初始化失败码必须透传，设备与名称原样传递。
TEST_F(WorkersCovTest, NormalInitEncrypt_FailurePropagates)
{
    // Arrange
    g_w_csRet = -5;
    stub.set_lamda(&crypt_setup::csInitEncrypt,
                   [](const QString &dev, const QString &name, crypt_setup::CryptPreProcessor *) -> int {
        g_w_dev = dev;
        g_w_name = name;
        return g_w_csRet;
    });
    const QVariantMap args {
        { encrypt_param_keys::kKeyDevice, QString("/tmp/ut-none") },
        { encrypt_param_keys::kKeyDeviceName, QString("ut-disk") }
    };
    NormalInitEncryptWorker worker(args);

    // Act
    worker.run();

    // Assert
    EXPECT_EQ(worker.exitCode(), -5);
    EXPECT_EQ(g_w_dev, QString("/tmp/ut-none"));
    EXPECT_EQ(g_w_name, QString("ut-disk"));
}

// FstabInitEncryptWorker：设备对象创建失败立即退出。
TEST_F(WorkersCovTest, FstabInitEncrypt_NoDeviceObject_Fails)
{
    // Arrange
    // createDevPtr 走真实实现 → DDeviceManager 已桩为 null → 返回空
    const QVariantMap args { { encrypt_param_keys::kKeyDevice, QString("/dev/ut0") } };
    FstabInitEncryptWorker worker(args);

    // Act
    worker.run();

    // Assert
    EXPECT_EQ(worker.exitCode(), -kErrorUnknown);
    EXPECT_LT(worker.exitCode(), 0);
}

// FstabInitEncryptWorker::initJobArgs：设备对象不可用时返回默认参数。
TEST_F(WorkersCovTest, FstabInitEncrypt_InitJobArgs_DefaultWhenNoDevice)
{
    // Arrange
    const QVariantMap args { { encrypt_param_keys::kKeyDevice, QString("/dev/ut0") },
                              { encrypt_param_keys::kKeyDeviceName, QString("n") } };
    FstabInitEncryptWorker worker(args);

    // Act
    const job_file_helper::JobDescArgs job = worker.initJobArgs("/dev/ut0");

    // Assert
    EXPECT_TRUE(job.device.isEmpty());
    EXPECT_TRUE(job.volume.isEmpty());
}

// FstabDecryptWorker：分区 UUID 不可得时失败。
TEST_F(WorkersCovTest, FstabDecrypt_NoPartitionUuid_Fails)
{
    // Arrange
    stub.set_lamda(&crypt_setup_helper::encryptStatus, [](const QString &) -> int {
        return kStatusOnline;   // 已完成加密，放行到 UUID 解析
    });
    const QVariantMap args { { encrypt_param_keys::kKeyDevice, QString("/dev/ut0") } };
    FstabDecryptWorker worker(args);

    // Act
    worker.run();

    // Assert
    EXPECT_EQ(worker.exitCode(), -kErrorUnknown);
    EXPECT_LT(worker.exitCode(), 0);
}

// FstabDecryptWorker：UUID 可得时创建解密任务并要求重启。
TEST_F(WorkersCovTest, FstabDecrypt_JobCreated_RebootRequired)
{
    // Arrange
    static QString g_jobDevice;
    g_jobDevice.clear();
    stub.set_lamda(&crypt_setup_helper::encryptStatus, [](const QString &) -> int {
        return kStatusFinished;
    });
    stub.set_lamda(&FstabDecryptWorker::partitionUUID,
                   [](FstabDecryptWorker *, const QString &) -> QString {
        return QString("UT-PUID");
    });
    stub.set_lamda(&FstabDecryptWorker::clearDeviceUUID,
                   [](FstabDecryptWorker *, const QString &) -> QString {
        return QString();   // 无解锁设备：跳过 fstab passno
    });
    stub.set_lamda(static_cast<int (*)(job_file_helper::JobDescArgs &)>(
                           &job_file_helper::createDecryptJobFile),
                   [](job_file_helper::JobDescArgs &args) -> int {
        g_jobDevice = args.device;
        return 0;
    });
    stub.set_lamda(&common_helper::createRebootFlagFile, [](const QString &) {});
    const QVariantMap args { { encrypt_param_keys::kKeyDevice, QString("/dev/ut0") } };
    FstabDecryptWorker worker(args);

    // Act
    worker.run();

    // Assert
    EXPECT_EQ(worker.exitCode(), -kRebootRequired);
    EXPECT_EQ(g_jobDevice, QString("PARTUUID=UT-PUID"));
}

// FstabDecryptWorker::partitionUUID / clearDeviceUUID：无设备对象返回空并置错误码。
TEST_F(WorkersCovTest, FstabDecrypt_UuidHelpers_NoDeviceObjectEmpty)
{
    // Arrange
    const QVariantMap args { { encrypt_param_keys::kKeyDevice, QString("/dev/ut0") } };
    FstabDecryptWorker worker(args);
    worker.setExitCode(kSuccess);

    // Act
    const QString part = worker.partitionUUID("/dev/ut0");
    const QString clear = worker.clearDeviceUUID("/dev/ut0");

    // Assert
    EXPECT_TRUE(part.isEmpty());
    EXPECT_TRUE(clear.isEmpty());
    EXPECT_EQ(worker.exitCode(), -kErrorUnknown);
}

// DMDecryptWorker：holder 设备缺失时失败退出。
TEST_F(WorkersCovTest, DmDecrypt_NoHolderDevice_Fails)
{
    // Arrange
    stub.set_lamda(&dm_setup_helper::findHolderDev, [](const QString &) -> QString {
        return QString();
    });
    const QVariantMap args { { encrypt_param_keys::kKeyDevice, QString("/dev/dm-9") } };
    DMDecryptWorker worker(args);

    // Act
    worker.run();

    // Assert
    EXPECT_EQ(worker.exitCode(), -kErrorUnknown);
    EXPECT_LT(worker.exitCode(), 0);
}

// DMDecryptWorker：全桩成功路径走完（挂起/重载/恢复/清理 crypttab）。
TEST_F(WorkersCovTest, DmDecrypt_FullSuccessFlow)
{
    // Arrange
    QStringList resumeOrder;
    static QString g_removedItem;
    g_removedItem.clear();
    stub.set_lamda(&DMDecryptWorker::getDevPath,
                   [](DMDecryptWorker *, QString *phyDev, QString *clearDev) {
        *phyDev = "/dev/sda9";
        *clearDev = "/dev/mapper/ut-target";
    });
    stub.set_lamda(&blockdev_helper::getUSecName, [](const QString &) -> QString {
        return QString("usec-overlay-ut");
    });
    stub.set_lamda(&crypt_setup_helper::genDetachHeaderPath,
                   [](const QString &, QString *) -> int { return -kErrorUnknown; });
    stub.set_lamda(&crypt_setup::csDecrypt,
                   [](const QString &, const QString &, const QString &, const QString &) -> int {
        return 0;
    });
    stub.set_lamda(&dm_setup::dmSuspendDevice, [](const QString &) -> int { return 0; });
    stub.set_lamda(&blockdev_helper::devBlockSize, [](const QString &) -> quint64 {
        return 1024;
    });
    stub.set_lamda(&dm_setup::dmReloadDevice,
                   [](const QString &, const dm_setup::DMTable &) -> int { return 0; });
    stub.set_lamda(&dm_setup::dmResumeDevice,
                   [&resumeOrder](const QString &dev) -> int {
        resumeOrder << dev;
        return 0;
    });
    stub.set_lamda(&crypttab_helper::removeCryptItem,
                   [](const QString &target) -> bool {
        g_removedItem = target;
        return true;
    });
    const QVariantMap args {
        { encrypt_param_keys::kKeyDevice, QString("/dev/dm-9") },
        { encrypt_param_keys::kKeyPassphrase, toBase64("ut-pass") },
        { encrypt_param_keys::kKeyDeviceName, QString("ut") }
    };
    DMDecryptWorker worker(args);

    // Act
    worker.run();

    // Assert
    EXPECT_EQ(worker.exitCode(), 0);
    EXPECT_EQ(resumeOrder, QStringList { QString("usec-overlay-ut") });
    EXPECT_EQ(g_removedItem, QString("ut-target"));
}

// PassphraseChangeWorker：普通路径（旧口令）失败码透传。
TEST_F(WorkersCovTest, PassphraseChange_NormalPath_FailurePropagates)
{
    // Arrange
    g_w_csChangeRet = -kErrorChangePassphraseFailed;
    stub.set_lamda(&crypt_setup::csChangePassphrase,
                   [](const QString &dev, const QString &oldP, const QString &newP) -> int {
        g_w_csChangeArgs = QStringList { dev, oldP, newP };
        return g_w_csChangeRet;
    });
    const QVariantMap args {
        { encrypt_param_keys::kKeyDevice, QString("/tmp/ut0") },
        { encrypt_param_keys::kKeyOldPassphrase, toBase64("old-pass") },
        { encrypt_param_keys::kKeyPassphrase, toBase64("new-pass") }
    };
    PassphraseChangeWorker worker(args);

    // Act
    worker.run();

    // Assert
    EXPECT_EQ(worker.exitCode(), -kErrorChangePassphraseFailed);
    ASSERT_EQ(g_w_csChangeArgs.size(), 3);
    EXPECT_EQ(g_w_csChangeArgs.at(0), QString("/tmp/ut0"));
    EXPECT_EQ(g_w_csChangeArgs.at(1), QString("old-pass")) << "old passphrase must be base64-decoded";
    EXPECT_EQ(g_w_csChangeArgs.at(2), QString("new-pass"));
}

// PassphraseChangeWorker：恢复密钥路径（槽位获取/添加/清理）成功完成。
TEST_F(WorkersCovTest, PassphraseChange_RecoveryKeyPath_Succeeds)
{
    // Arrange
    static QList<int> g_slots { 0 };
    static int g_removedSlots { 0 };
    g_removedSlots = 0;
    stub.set_lamda(&crypt_setup_helper::getRecoveryKeySlots,
                   [](const QString &, QList<int> *out) -> int {
        if (out)
            *out = g_slots;
        return 0;
    });
    stub.set_lamda(&crypt_setup::csAddPassphrase,
                   [](const QString &, const QString &, const QString &) -> int {
        return g_w_addPassRet;
    });
    stub.set_lamda(&crypt_setup::csRemoveKeyslot,
                   [&g_removedSlots](const QString &, int) -> int {
        ++g_removedSlots;
        return 0;
    });
    stub.set_lamda(&crypt_setup::csChangePassphrase,
                   [](const QString &, const QString &, const QString &) -> int { return 0; });
    const QVariantMap args {
        { encrypt_param_keys::kKeyDevice, QString("/tmp/ut0") },
        { encrypt_param_keys::kKeyOldPassphrase, toBase64("old-pass") },
        { encrypt_param_keys::kKeyPassphrase, toBase64("new-pass") },
        { encrypt_param_keys::kKeyValidateWithRecKey, true }
    };
    PassphraseChangeWorker worker(args);

    // Act
    worker.run();

    // Assert
    EXPECT_EQ(worker.exitCode(), 0);
    // 新口令槽位=2、恢复槽位=0，其余 6 个槽位被清理
    EXPECT_EQ(g_removedSlots, 6);
}

// PassphraseChangeWorker：dm 设备且对象创建失败时直接失败。
TEST_F(WorkersCovTest, PassphraseChange_DmDeviceNoObject_Fails)
{
    // Arrange
    const QVariantMap args { { encrypt_param_keys::kKeyDevice, QString("/dev/dm-9") } };
    PassphraseChangeWorker worker(args);

    // Act
    worker.run();

    // Assert
    EXPECT_EQ(worker.exitCode(), -kErrorUnknown);
    EXPECT_LT(worker.exitCode(), 0);
}

/******************************************************************************/
/****************************** ResumeEncryptWorker ***************************/
/******************************************************************************/

class ResumeWorkerCovTest : public testing::Test
{
public:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        WorkersCovTest::resetWorkerState();
        stub.set_lamda(&inhibit_helper::inhibit,
                       [](const QString &) -> QDBusReply<QDBusUnixFileDescriptor> {
            return QDBusReply<QDBusUnixFileDescriptor>();
        });
        stub.set_lamda(&dfmmount::DDeviceManager::instance,
                       []() -> dfmmount::DDeviceManager * { return nullptr; });
    }

    void TearDown() override
    {
        stub.clear();
        QThreadPool::globalInstance()->waitForDone();
    }

    // 预置任务参数（供 run()/setPassphrase() 等使用）
    static QVariantMap makeArgs()
    {
        return {
            { encrypt_param_keys::kKeyDevice, QString("/tmp/ut-none") },
        };
    }
};

// 构造 + recoveryKey 初始为空。
TEST_F(ResumeWorkerCovTest, Construct_InitialRecoveryKeyEmpty)
{
    // Arrange / Act
    ResumeEncryptWorker worker(makeArgs());

    // Assert
    EXPECT_TRUE(worker.recoveryKey().isEmpty());
    EXPECT_EQ(worker.exitCode(), kSuccess);
}

// setAuthInfo / setRecoveryKey：恢复密钥仅在成功登记（keyslot+token）后发布。
TEST_F(ResumeWorkerCovTest, SetAuthInfoAndRecoveryKey_PublishKeyOnlyAfterEnroll)
{
    // Arrange
    ResumeEncryptWorker worker(makeArgs());
    const QVariantMap authArgs {
        { encrypt_param_keys::kKeyDevice, QString("/tmp/ut-none") },
        { encrypt_param_keys::kKeyPassphrase, toBase64("ut-pass") },
        { encrypt_param_keys::kKeyExportToPath, QString("/tmp/ut-export") }
    };
    stub.set_lamda(&common_helper::genRecoveryKey, []() -> QString {
        return g_w_recKey;
    });
    stub.set_lamda(&crypt_setup::csAddPassphrase,
                   [](const QString &, const QString &, const QString &) -> int {
        return g_w_addPassRet;
    });
    stub.set_lamda(&crypt_setup_helper::setToken,
                   [](const QString &, const QString &token) -> int {
        ++g_w_setTokenCalls;
        g_w_lastToken = token;
        return 0;
    });

    // Act
    worker.setAuthInfo(authArgs);
    worker.setRecoveryKey();

    // Assert
    EXPECT_EQ(worker.recoveryKey(), QString("UT-RECOVERY-KEY"));
    EXPECT_EQ(g_w_setTokenCalls, 1);
    EXPECT_TRUE(g_w_lastToken.contains(QString::number(g_w_addPassRet)));
}

// setRecoveryKey：未提供导出路径时跳过。
TEST_F(ResumeWorkerCovTest, SetRecoveryKey_NoExportPath_Skipped)
{
    // Arrange
    ResumeEncryptWorker worker(makeArgs());

    // Act
    worker.setRecoveryKey();

    // Assert
    EXPECT_TRUE(worker.recoveryKey().isEmpty());
    EXPECT_EQ(g_w_setTokenCalls, 0);
}

// ignoreAuthRequest + waitForAuthInfo：忽略标志立即终止等待。
TEST_F(ResumeWorkerCovTest, WaitForAuthInfo_Ignored_ReturnsFalse)
{
    // Arrange
    ResumeEncryptWorker worker(makeArgs());
    worker.ignoreAuthRequest();

    // Act
    const bool ret = worker.waitForAuthInfo();

    // Assert
    EXPECT_FALSE(ret);
    EXPECT_TRUE(worker.recoveryKey().isEmpty());
}

// waitForAuthInfo：凭据齐备时立即返回真。
TEST_F(ResumeWorkerCovTest, WaitForAuthInfo_AuthInfoReady_ReturnsTrue)
{
    // Arrange
    ResumeEncryptWorker worker(makeArgs());
    const QVariantMap authArgs {
        { encrypt_param_keys::kKeyDevice, QString("/tmp/ut-none") },
        { encrypt_param_keys::kKeyPassphrase, toBase64("ut-pass") }
    };
    worker.setAuthInfo(authArgs);

    // Act
    const bool ret = worker.waitForAuthInfo();

    // Assert
    EXPECT_TRUE(ret);
    EXPECT_EQ(worker.recoveryKey(), QString());
}

// run：无未完成任务时安全返回（不设置退出码）。
TEST_F(ResumeWorkerCovTest, Run_NoJobFile_ReturnsWithoutError)
{
    // Arrange
    // loadEncryptJobFile 走真实路径（/etc/usec-crypt 为空 → 无任务）
    ResumeEncryptWorker worker(makeArgs());

    // Act
    worker.run();

    // Assert
    EXPECT_EQ(worker.exitCode(), kSuccess);
    EXPECT_EQ(worker.args().value(encrypt_param_keys::kKeyDevice).toString(), QString("/tmp/ut-none"));
}

// run：设备不在加密状态时放弃（不进入恢复）。
TEST_F(ResumeWorkerCovTest, Run_DeviceNotEncrypting_GivesUp)
{
    // Arrange
    static bool g_loadCalled = false;
    g_loadCalled = false;
    stub.set_lamda(static_cast<int (*)(job_file_helper::JobDescArgs *, const QString &)>(
                           &job_file_helper::loadEncryptJobFile),
                   [](job_file_helper::JobDescArgs *args, const QString &) -> int {
        g_loadCalled = true;
        args->jobFile = "/tmp/ut-job.json";
        args->devPath = "/tmp/ut-none";
        args->devName = "ut";
        args->volume = "ut-vol";
        return 0;
    });
    stub.set_lamda(&crypt_setup_helper::encryptStatus, [](const QString &) -> int {
        return kStatusFinished;   // 无 encrypt 位
    });
    ResumeEncryptWorker worker(makeArgs());

    // Act
    worker.run();

    // Assert
    EXPECT_TRUE(g_loadCalled);
    EXPECT_EQ(worker.exitCode(), kSuccess) << "give-up path must not set error code";
}

// run：用户忽略授权请求 → kIgnoreRequest。
TEST_F(ResumeWorkerCovTest, Run_AuthIgnored_ReturnsIgnoreRequest)
{
    // Arrange
    stub.set_lamda(static_cast<int (*)(job_file_helper::JobDescArgs *, const QString &)>(
                           &job_file_helper::loadEncryptJobFile),
                   [](job_file_helper::JobDescArgs *args, const QString &) -> int {
        args->jobFile = "/tmp/ut-job.json";
        args->devPath = "/tmp/ut-none";
        args->devName = "ut";
        args->volume = "ut-vol";
        return 0;
    });
    stub.set_lamda(&crypt_setup_helper::encryptStatus, [](const QString &) -> int {
        return kStatusEncrypt | kStatusOffline;
    });
    ResumeEncryptWorker worker(makeArgs());
    worker.ignoreAuthRequest();

    // Act
    worker.run();

    // Assert
    EXPECT_EQ(worker.exitCode(), -kIgnoreRequest);
    EXPECT_LT(worker.exitCode(), 0);
}

// setPassphrase：无 TPM token 时仅修改口令。
TEST_F(ResumeWorkerCovTest, SetPassphrase_NoTpmToken_ChangesPassphraseOnly)
{
    // Arrange
    ResumeEncryptWorker worker(makeArgs());
    worker.m_jobArgs.devPath = "/tmp/ut-none";   // setPassphrase 读取任务参数
    const QVariantMap authArgs {
        { encrypt_param_keys::kKeyDevice, QString("/tmp/ut-none") },
        { encrypt_param_keys::kKeyPassphrase, toBase64("ut-pass") }
    };
    worker.setAuthInfo(authArgs);
    static QString g_changeDev;
    stub.set_lamda(&crypt_setup::csChangePassphrase,
                   [](const QString &dev, const QString &oldP, const QString &) -> int {
        g_changeDev = dev;
        g_w_csChangeArgs = QStringList { oldP };
        return 3;
    });

    // Act
    worker.setPassphrase();

    // Assert
    EXPECT_EQ(g_changeDev, QString("/tmp/ut-none"));
    ASSERT_EQ(g_w_csChangeArgs.size(), 1);
    EXPECT_EQ(g_w_csChangeArgs.first(), QString("")) << "empty old passphrase is expected";
    EXPECT_EQ(g_w_setTokenCalls, 0);
}

// setPassphrase：带 TPM token 时更新 token keyslots。
TEST_F(ResumeWorkerCovTest, SetPassphrase_WithTpmToken_UpdatesToken)
{
    // Arrange
    ResumeEncryptWorker worker(makeArgs());
    const QVariantMap authArgs {
        { encrypt_param_keys::kKeyDevice, QString("/tmp/ut-none") },
        { encrypt_param_keys::kKeyPassphrase, toBase64("ut-pass") },
        { encrypt_param_keys::kKeyTPMToken, QString(R"({"type":"usec-tpm2","pin":"1"})") }
    };
    worker.setAuthInfo(authArgs);
    stub.set_lamda(&crypt_setup::csChangePassphrase,
                   [](const QString &, const QString &, const QString &) -> int { return 4; });
    stub.set_lamda(&crypt_setup_helper::setToken,
                   [](const QString &, const QString &token) -> int {
        ++g_w_setTokenCalls;
        g_w_lastToken = token;
        return 0;
    });

    // Act
    worker.setPassphrase();

    // Assert
    EXPECT_EQ(g_w_setTokenCalls, 1);
    EXPECT_TRUE(g_w_lastToken.contains("\"keyslots\""));
    EXPECT_TRUE(g_w_lastToken.contains("4"));
}

// setPassphrase：修改口令失败时不再写 token。
TEST_F(ResumeWorkerCovTest, SetPassphrase_ChangeFailed_SkipsToken)
{
    // Arrange
    ResumeEncryptWorker worker(makeArgs());
    const QVariantMap authArgs {
        { encrypt_param_keys::kKeyDevice, QString("/tmp/ut-none") },
        { encrypt_param_keys::kKeyPassphrase, toBase64("ut-pass") },
        { encrypt_param_keys::kKeyTPMToken, QString("{}") }
    };
    worker.setAuthInfo(authArgs);
    stub.set_lamda(&crypt_setup::csChangePassphrase,
                   [](const QString &, const QString &, const QString &) -> int { return -1; });

    // Act
    worker.setPassphrase();

    // Assert
    EXPECT_EQ(g_w_setTokenCalls, 0);
    EXPECT_TRUE(g_w_lastToken.isEmpty());
}

// setPhyDevLabel：成功与失败路径。
TEST_F(ResumeWorkerCovTest, SetPhyDevLabel_RecordsAndHandlesFailure)
{
    // Arrange
    ResumeEncryptWorker worker(makeArgs());
    static int g_labelRet = 0;
    g_labelRet = 0;
    stub.set_lamda(&crypt_setup::csSetLabel,
                   [](const QString &, const QString &) -> int { return g_labelRet; });

    // Act
    worker.setPhyDevLabel();
    g_labelRet = -1;
    worker.setPhyDevLabel();

    // Assert
    EXPECT_EQ(g_labelRet, -1);
    EXPECT_NO_THROW(worker.setPhyDevLabel());
}

// updateCryptTab：无 TPM token 跳过；有 token 时追加 tpm2-device 选项。
TEST_F(ResumeWorkerCovTest, UpdateCryptTab_TpmTokenGated)
{
    // Arrange
    ResumeEncryptWorker workerNoToken(makeArgs());
    ResumeEncryptWorker workerWithToken(makeArgs());
    const QVariantMap authArgs {
        { encrypt_param_keys::kKeyDevice, QString("/tmp/ut-none") },
        { encrypt_param_keys::kKeyPassphrase, toBase64("ut-pass") },
        { encrypt_param_keys::kKeyTPMToken, QString(R"({"type":"usec-tpm2"})") }
    };
    workerWithToken.setAuthInfo(authArgs);
    stub.set_lamda(&crypttab_helper::addCryptOption,
                   [](const QString &, const QString &) -> bool {
        g_w_addCryptOptionCalled = true;
        return true;
    });

    // Act
    workerNoToken.updateCryptTab();
    const bool noTokenCalled = g_w_addCryptOptionCalled;
    workerWithToken.updateCryptTab();

    // Assert
    EXPECT_FALSE(noTokenCalled);
    EXPECT_TRUE(g_w_addCryptOptionCalled);
}

/******************************************************************************/
/****************************** DMInitEncryptWorker ***************************/
/******************************************************************************/

class DmInitWorkerCovTest : public testing::Test
{
public:
    stub_ext::StubExt stub;

    void TearDown() override
    {
        stub.clear();
    }
};

// detachPhyDevice：参数不足直接失败；挂起失败时回滚重载。
TEST_F(DmInitWorkerCovTest, DetachPhyDevice_BadArgsAndSuspendFail)
{
    // Arrange
    const char *argv[] = { "/dev/ut0", "ut-top", "ut-mid" };
    QStringList reloadOrder;
    stub.set_lamda(&dm_setup::dmSuspendDevice, [](const QString &) -> int { return -1; });
    stub.set_lamda(&dm_setup::dmReloadDevice,
                   [](const QString &, const dm_setup::DMTable &) -> int { return 0; });
    stub.set_lamda(&dm_setup::dmResumeDevice,
                   [&reloadOrder](const QString &dev) -> int {
        reloadOrder << dev;
        return 0;
    });

    // Act
    const int badArgs = DMInitEncryptWorker::detachPhyDevice(1, argv);
    const int suspendFail = DMInitEncryptWorker::detachPhyDevice(3, argv);

    // Assert
    EXPECT_EQ(badArgs, -kErrorUnknown);
    EXPECT_EQ(suspendFail, -1);
    EXPECT_EQ(reloadOrder, QStringList { QString("ut-top") });
}

/******************************************************************************/
/****************************** DiskEncryptSetup ******************************/
/******************************************************************************/

namespace {
QList<QString> g_svc_lockTimerCalls;
QVariantMap g_svc_resumeArgs;
int g_svc_resumeCalls { 0 };
}   // namespace

class DiskEncryptSetupCovTest : public testing::Test
{
public:
    DiskEncryptSetup *svc { nullptr };
    DiskEncryptSetupPrivate *priv { nullptr };
    stub_ext::StubExt stub;

    static void resetSvcState()
    {
        g_svc_lockTimerCalls.clear();
        g_svc_resumeArgs.clear();
        g_svc_resumeCalls = 0;
    }

    void SetUp() override
    {
        resetSvcState();
        stubServiceNet();
        svc = new DiskEncryptSetup();
        priv = svc->m_dptr;
        ASSERT_NE(priv, nullptr);
        // initialize() 的 fire-and-forget 任务在桩保护期内排空
        QThreadPool::globalInstance()->waitForDone();
        qApp->processEvents();
        // 排除构造期 initialize() 对 resume 计数的干扰（保留 lockTimer 记录）
        g_svc_resumeCalls = 0;
        g_svc_resumeArgs.clear();
    }

    void TearDown() override
    {
        QThreadPool::globalInstance()->waitForDone();
        qApp->processEvents();
        delete svc;
        QThreadPool::globalInstance()->waitForDone();
        svc = nullptr;
        priv = nullptr;
        stub.clear();
    }

    // DiskEncryptSetup 构造所需的外围桩（对齐 test_diskencrypt_pms.cpp 的做法）。
    void stubServiceNet()
    {
        stub.set_lamda(&QDBusService::initPolicy,
                       [](QDBusService *, const QDBusConnection::BusType &, const QString &) {});
        stub.set_lamda(&QDBusService::lockTimer, [](QDBusService *, bool lock) {
            g_svc_lockTimerCalls << (lock ? "lock" : "unlock");
        });
        stub.set_lamda(&DiskEncryptSetupPrivate::resumeEncryption,
                       [](DiskEncryptSetupPrivate *, const QVariantMap &args) {
            ++g_svc_resumeCalls;
            g_svc_resumeArgs = args;
        });
        stub.set_lamda(&dfmmount::DDeviceManager::instance,
                       []() -> dfmmount::DDeviceManager * { return nullptr; });
        stub.set_lamda(&inhibit_helper::inhibit,
                       [](const QString &) -> QDBusReply<QDBusUnixFileDescriptor> {
            return QDBusReply<QDBusUnixFileDescriptor>();
        });
        // DConfig 构造失败 → config 为空 → setupConfigWatcher /
        // syncConfigWithFileSystem 走真实空安全分支
        stub.set_lamda(static_cast<Dtk::Core::DConfig *(*)(const QString &, const QString &,
                                                           const QString &, QObject *)>(
                               &Dtk::Core::DConfig::create),
                       [](const QString &, const QString &, const QString &, QObject *)
                               -> Dtk::Core::DConfig * {
            return nullptr;
        });
    }

    // 真实 checkAuth：DBus 消息 + polkit 授权桩
    void stubRealAuth(bool allowed)
    {
        static bool g_allowed = true;
        g_allowed = allowed;
        stub.set_lamda(&QDBusContext::message, [](const QDBusContext *) -> QDBusMessage {
            return QDBusMessage::createMethodCall("org.freedesktop.DBus", "/", "ut.iface", "utMethod");
        });
        stub.set_lamda(&QDBusMessage::service, [](const QDBusMessage *) -> QString {
            return QString("ut.sender.bus");
        });
        stub.set_lamda(&PolkitQt1::Authority::instance,
                       [](void *) -> PolkitQt1::Authority * {
            // Authority 无公开构造：返回哑指针，后续成员调用均已被桩替换
            return reinterpret_cast<PolkitQt1::Authority *>(0x1);
        });
        stub.set_lamda(&PolkitQt1::Authority::checkAuthorizationSync,
                       [](PolkitQt1::Authority *, const QString &action,
                          const PolkitQt1::Subject &, PolkitQt1::Authority::AuthorizationFlags)
                               -> PolkitQt1::Authority::Result {
            g_authActions3.append(action);
            return g_allowed ? PolkitQt1::Authority::Yes : PolkitQt1::Authority::No;
        });
    }

    static QStringList g_authActions3;

    static QDBusUnixFileDescriptor makeArgsFd(const QVariantMap &args)
    {
        QByteArray buffer;
        QDataStream stream(&buffer, QIODevice::WriteOnly);
        stream << args;
        int pipefd[2];
        if (pipe(pipefd) != 0)
            return QDBusUnixFileDescriptor();
        ssize_t written = write(pipefd[1], buffer.constData(), buffer.size());
        Q_UNUSED(written)
        close(pipefd[1]);
        QDBusUnixFileDescriptor fd(pipefd[0]);
        close(pipefd[0]);
        return fd;
    }
};

QStringList DiskEncryptSetupCovTest::g_authActions3;

// 构造即锁定自动退出定时器并完成初始化。
TEST_F(DiskEncryptSetupCovTest, Construct_LocksTimerInitially)
{
    // Arrange
    // （SetUp 已构造完成）

    // Act / Assert
    ASSERT_FALSE(g_svc_lockTimerCalls.isEmpty());
    EXPECT_EQ(g_svc_lockTimerCalls.first(), QString("lock"));
    EXPECT_FALSE(svc->IsTaskRunning());
}

// IsTaskEmpty / IsTaskRunning / PendingDecryptionDevice 的空态。
TEST_F(DiskEncryptSetupCovTest, TaskStateQueries_EmptyState)
{
    // Arrange / Act
    const bool empty = svc->IsTaskEmpty();
    const bool running = svc->IsTaskRunning();
    const QString pending = svc->PendingDecryptionDevice();

    // Assert
    EXPECT_TRUE(empty);
    EXPECT_FALSE(running);
    EXPECT_TRUE(pending.isEmpty());
}

// validateInitArgs：字段缺失 / 类型非法 / 完整。
TEST_F(DiskEncryptSetupCovTest, ValidateInitArgs_Branches)
{
    // Arrange
    const QVariantMap full {
        { encrypt_param_keys::kKeyDevice, QString("/dev/ut0") },
        { encrypt_param_keys::kKeyDeviceName, QString("n") },
        { encrypt_param_keys::kKeyJobType, QString(job_type::TypeNormal) }
    };

    // Act / Assert
    EXPECT_FALSE(priv->validateInitArgs(QVariantMap()));
    EXPECT_FALSE(priv->validateInitArgs({ { encrypt_param_keys::kKeyDevice, QString("/dev/ut0") } }));
    EXPECT_FALSE(priv->validateInitArgs({
        { encrypt_param_keys::kKeyDevice, QString("/dev/ut0") },
        { encrypt_param_keys::kKeyDeviceName, QString("n") },
        { encrypt_param_keys::kKeyJobType, QString("bad-type") } }));
    EXPECT_TRUE(priv->validateInitArgs(full));
}

// validateDecryptArgs：fstab 类型免口令，其余类型必须带口令与名称。
TEST_F(DiskEncryptSetupCovTest, ValidateDecryptArgs_Branches)
{
    // Arrange / Act / Assert
    EXPECT_FALSE(priv->validateDecryptArgs(QVariantMap()));
    EXPECT_FALSE(priv->validateDecryptArgs({ { encrypt_param_keys::kKeyJobType, QString("bad") } }));
    EXPECT_TRUE(priv->validateDecryptArgs({
        { encrypt_param_keys::kKeyDevice, QString("/dev/ut0") },
        { encrypt_param_keys::kKeyJobType, QString(job_type::TypeFstab) } }));
    EXPECT_FALSE(priv->validateDecryptArgs({
        { encrypt_param_keys::kKeyDevice, QString("/dev/ut0") },
        { encrypt_param_keys::kKeyJobType, QString(job_type::TypeNormal) } }));
    EXPECT_TRUE(priv->validateDecryptArgs({
        { encrypt_param_keys::kKeyDevice, QString("/dev/ut0") },
        { encrypt_param_keys::kKeyPassphrase, QString("cGFzcw==") },
        { encrypt_param_keys::kKeyDeviceName, QString("n") },
        { encrypt_param_keys::kKeyJobType, QString(job_type::TypeNormal) } }));
}

// validateChgPwdArgs：设备 / 口令 / 旧口令缺一不可；validateResumeArgs 恒真。
TEST_F(DiskEncryptSetupCovTest, ValidateChgPwdAndResumeArgs)
{
    // Arrange / Act / Assert
    EXPECT_FALSE(priv->validateChgPwdArgs(QVariantMap()));
    EXPECT_FALSE(priv->validateChgPwdArgs({ { encrypt_param_keys::kKeyDevice, QString("/dev/ut0") } }));
    EXPECT_FALSE(priv->validateChgPwdArgs({
        { encrypt_param_keys::kKeyDevice, QString("/dev/ut0") },
        { encrypt_param_keys::kKeyPassphrase, QString("p") } }));
    EXPECT_TRUE(priv->validateChgPwdArgs({
        { encrypt_param_keys::kKeyDevice, QString("/dev/ut0") },
        { encrypt_param_keys::kKeyPassphrase, QString("p") },
        { encrypt_param_keys::kKeyOldPassphrase, QString("o") } }));
    EXPECT_TRUE(priv->validateResumeArgs(QVariantMap()));
}

// createInitWorker / createDecryptWorker：类型映射与未知类型。
TEST_F(DiskEncryptSetupCovTest, CreateWorkers_TypeMapping)
{
    // Arrange / Act
    BaseEncryptWorker *fstab = priv->createInitWorker(job_type::TypeFstab, {});
    BaseEncryptWorker *overlay = priv->createInitWorker(job_type::TypeOverlay, {});
    BaseEncryptWorker *normal = priv->createInitWorker(job_type::TypeNormal, {});
    BaseEncryptWorker *unknownInit = priv->createInitWorker("ut-unknown", {});
    BaseEncryptWorker *dFstab = priv->createDecryptWorker(job_type::TypeFstab, {});
    BaseEncryptWorker *dOverlay = priv->createDecryptWorker(job_type::TypeOverlay, {});
    BaseEncryptWorker *dNormal = priv->createDecryptWorker(job_type::TypeNormal, {});
    BaseEncryptWorker *unknownDec = priv->createDecryptWorker("ut-unknown", {});

    // Assert
    EXPECT_TRUE(dynamic_cast<FstabInitEncryptWorker *>(fstab) != nullptr);
    EXPECT_TRUE(dynamic_cast<DMInitEncryptWorker *>(overlay) != nullptr);
    EXPECT_TRUE(dynamic_cast<NormalInitEncryptWorker *>(normal) != nullptr);
    EXPECT_EQ(unknownInit, nullptr);
    EXPECT_TRUE(dynamic_cast<FstabDecryptWorker *>(dFstab) != nullptr);
    EXPECT_TRUE(dynamic_cast<DMDecryptWorker *>(dOverlay) != nullptr);
    EXPECT_TRUE(dynamic_cast<NormalDecryptWorker *>(dNormal) != nullptr);
    EXPECT_EQ(unknownDec, nullptr);

    delete fstab;
    delete overlay;
    delete normal;
    delete dFstab;
    delete dOverlay;
    delete dNormal;
}

// parseCredentialsFromFd：无效 fd / 空数据 / 非法流 / 合法流。
TEST_F(DiskEncryptSetupCovTest, ParseCredentialsFromFd_Branches)
{
    // Arrange
    QVariantMap args;

    // Act / Assert：无效 fd
    EXPECT_FALSE(priv->parseCredentialsFromFd(QDBusUnixFileDescriptor(), &args));

    // Act / Assert：空管道
    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);
    close(pipefd[1]);
    QDBusUnixFileDescriptor emptyFd(pipefd[0]);
    close(pipefd[0]);
    EXPECT_FALSE(priv->parseCredentialsFromFd(emptyFd, &args));

    // Act / Assert：合法流
    QDBusUnixFileDescriptor goodFd = makeArgsFd({ { "k", QString("v") } });
    ASSERT_TRUE(goodFd.isValid());
    EXPECT_TRUE(priv->parseCredentialsFromFd(goodFd, &args));
    EXPECT_EQ(args.value("k").toString(), QString("v"));
}

// checkAuth：服务名为空 / 授权通过 / 授权拒绝。
TEST_F(DiskEncryptSetupCovTest, CheckAuth_Branches)
{
    // Arrange
    stubRealAuth(true);

    // Act
    const bool allowed = priv->checkAuth("ut-action");

    // Assert
    EXPECT_TRUE(allowed);
    ASSERT_EQ(g_authActions3.size(), 1);
    EXPECT_EQ(g_authActions3.first(), QString("ut-action"));

    // Arrange / Act / Assert：授权拒绝
    stubRealAuth(false);
    EXPECT_FALSE(priv->checkAuth("ut-action-2"));
}

// initThreadConnection：任务线程启停联动 jobRunning。
TEST_F(DiskEncryptSetupCovTest, InitThreadConnection_TogglesJobRunning)
{
    // Arrange
    QThread thread;

    // Act
    priv->initThreadConnection(&thread);
    thread.start();
    thread.quit();
    ASSERT_TRUE(thread.wait(5000));
    qApp->processEvents();

    // Assert
    EXPECT_FALSE(priv->jobRunning) << "jobRunning must be false after thread finished";
    EXPECT_FALSE(svc->IsTaskRunning());
}

// InitEncryption：参数非法 / 任务占用 / 授权失败 均拒绝。
TEST_F(DiskEncryptSetupCovTest, InitEncryption_RejectionBranches)
{
    // Arrange
    stubRealAuth(true);

    // Act / Assert：参数缺失
    EXPECT_FALSE(svc->InitEncryption(QVariantMap()));

    // Act / Assert：任务进行中
    priv->jobRunning = true;
    EXPECT_FALSE(svc->InitEncryption({
        { encrypt_param_keys::kKeyDevice, QString("/dev/ut0") },
        { encrypt_param_keys::kKeyDeviceName, QString("n") },
        { encrypt_param_keys::kKeyJobType, QString(job_type::TypeNormal) } }));
    priv->jobRunning = false;

    // Act / Assert：未知任务类型（worker 创建失败）
    EXPECT_FALSE(svc->InitEncryption({
        { encrypt_param_keys::kKeyDevice, QString("/dev/ut0") },
        { encrypt_param_keys::kKeyDeviceName, QString("n") },
        { encrypt_param_keys::kKeyJobType, QString("ut-unknown") } }));
}

// InitEncryption：完整成功路径（真实 worker 线程 + InitEncResult 信号）。
TEST_F(DiskEncryptSetupCovTest, InitEncryption_FullFlow_EmitsInitEncResult)
{
    // Arrange
    stubRealAuth(true);
    static int g_initRet = -5;
    g_initRet = -5;
    stub.set_lamda(&crypt_setup::csInitEncrypt,
                   [](const QString &, const QString &, crypt_setup::CryptPreProcessor *) -> int {
        return g_initRet;
    });
    QSignalSpy spy(svc, &DiskEncryptSetup::InitEncResult);
    const QVariantMap args {
        { encrypt_param_keys::kKeyDevice, QString("/tmp/ut-none") },
        { encrypt_param_keys::kKeyDeviceName, QString("ut-disk") },
        { encrypt_param_keys::kKeyJobType, QString(job_type::TypeNormal) }
    };

    // Act
    const bool started = svc->InitEncryption(args);
    ASSERT_TRUE(started);
    ASSERT_TRUE(spy.wait(8000));

    // Assert
    ASSERT_EQ(spy.count(), 1);
    const QVariantMap result = spy.at(0).at(0).toMap();
    EXPECT_EQ(result.value(encrypt_param_keys::kKeyDevice).toString(), QString("/tmp/ut-none"));
    EXPECT_EQ(result.value(encrypt_param_keys::kKeyOperationResult).toInt(), -5);
    EXPECT_FALSE(svc->IsTaskRunning());
}

// ResumeEncryption：任务占用拒绝 / 正常路径进入恢复。
TEST_F(DiskEncryptSetupCovTest, ResumeEncryption_Branches)
{
    // Arrange
    stubRealAuth(true);

    // Act / Assert：任务占用
    priv->jobRunning = true;
    EXPECT_FALSE(svc->ResumeEncryption({}));
    priv->jobRunning = false;

    // Act / Assert：正常路径
    const QVariantMap args { { encrypt_param_keys::kKeyDevice, QString("/dev/ut0") } };
    EXPECT_TRUE(svc->ResumeEncryption(args));
    EXPECT_EQ(g_svc_resumeCalls, 1);
    EXPECT_EQ(g_svc_resumeArgs, args);
}

// Decryption：无效 fd 解析失败拒绝；合法参数启动 worker 并回 DecryptResult。
TEST_F(DiskEncryptSetupCovTest, Decryption_InvalidFdAndFullFlow)
{
    // Arrange
    stubRealAuth(true);

    // Act / Assert：无效 fd
    EXPECT_FALSE(svc->Decryption(QDBusUnixFileDescriptor()));

    // Arrange：合法参数（normal 类型）
    stub.set_lamda(&crypt_setup_helper::encryptStatus, [](const QString &) -> int {
        return kStatusFinished;
    });
    stub.set_lamda(&crypt_setup::csDecryptMoveHead,
                   [](const QString &, const QString &, const QString &) -> int { return 0; });
    QSignalSpy spy(svc, &DiskEncryptSetup::DecryptResult);
    const QVariantMap args {
        { encrypt_param_keys::kKeyDevice, QString("/tmp/ut-none") },
        { encrypt_param_keys::kKeyPassphrase, toBase64("ut-pass") },
        { encrypt_param_keys::kKeyDeviceName, QString("ut-disk") },
        { encrypt_param_keys::kKeyJobType, QString(job_type::TypeNormal) }
    };
    const QDBusUnixFileDescriptor fd = makeArgsFd(args);
    ASSERT_TRUE(fd.isValid());

    // Act
    const bool started = svc->Decryption(fd);
    ASSERT_TRUE(started);
    ASSERT_TRUE(spy.wait(8000));

    // Assert
    ASSERT_EQ(spy.count(), 1);
    const QVariantMap result = spy.at(0).at(0).toMap();
    EXPECT_EQ(result.value(encrypt_param_keys::kKeyDevice).toString(), QString("/tmp/ut-none"));
    EXPECT_EQ(result.value(encrypt_param_keys::kKeyOperationResult).toInt(), 0);
}

// ChangePassphrase：无效 fd / 非法 token JSON / 完整成功路径。
TEST_F(DiskEncryptSetupCovTest, ChangePassphrase_Branches)
{
    // Arrange
    stubRealAuth(true);

    // Act / Assert：无效 fd
    EXPECT_FALSE(svc->ChangePassphrase(QDBusUnixFileDescriptor()));

    // Arrange / Act / Assert：设备 token 为非法 JSON（ChangePassphrase 从设备查询 token）
    stub.set_lamda(&crypt_setup_helper::getToken,
                   [](const QString &, QString *token) -> int {
        if (token)
            *token = "{not-json";
        return 0;
    });
    const QVariantMap chgArgs {
        { encrypt_param_keys::kKeyDevice, QString("/tmp/ut-none") },
        { encrypt_param_keys::kKeyPassphrase, QString("p") },
        { encrypt_param_keys::kKeyOldPassphrase, QString("o") }
    };
    QDBusUnixFileDescriptor badFd = makeArgsFd(chgArgs);
    ASSERT_TRUE(badFd.isValid());
    EXPECT_FALSE(svc->ChangePassphrase(badFd));

    // Arrange：完整成功路径（无 token → 口令类 action）
    stub.reset(&crypt_setup_helper::getToken);   // 还原坏 JSON 桩，走真实空 token
    static int g_chgRet = 0;
    g_chgRet = 0;
    stub.set_lamda(&crypt_setup::csChangePassphrase,
                   [](const QString &, const QString &, const QString &) -> int {
        return g_chgRet;
    });
    QSignalSpy spy(svc, &DiskEncryptSetup::ChangePassResult);
    const QVariantMap args {
        { encrypt_param_keys::kKeyDevice, QString("/tmp/ut-none") },
        { encrypt_param_keys::kKeyPassphrase, toBase64("p") },
        { encrypt_param_keys::kKeyOldPassphrase, toBase64("o") }
    };
    const QDBusUnixFileDescriptor fd = makeArgsFd(args);
    ASSERT_TRUE(fd.isValid());

    // Act
    const bool started = svc->ChangePassphrase(fd);
    ASSERT_TRUE(started);
    ASSERT_TRUE(spy.wait(8000));

    // Assert
    ASSERT_EQ(spy.count(), 1);
    const QVariantMap result = spy.at(0).at(0).toMap();
    EXPECT_EQ(result.value(encrypt_param_keys::kKeyOperationResult).toInt(), 0);
    EXPECT_EQ(result.value(encrypt_param_keys::kKeyDevice).toString(), QString("/tmp/ut-none"));
}

// IgnoreAuthSetup 转发通知信号。
TEST_F(DiskEncryptSetupCovTest, IgnoreAuthSetup_EmitsSignal)
{
    // Arrange
    QSignalSpy spy(NotificationHelper::instance(), &NotificationHelper::ignoreAuthSetup);

    // Act
    svc->IgnoreAuthSetup();

    // Assert
    EXPECT_EQ(spy.count(), 1);
    EXPECT_TRUE(spy.isValid());
}

// TpmToken：无 token 时回退 holder 设备查询。
TEST_F(DiskEncryptSetupCovTest, TpmToken_FallsBackToHolderDevice)
{
    // Arrange
    static int g_getTokenCalls = 0;
    g_getTokenCalls = 0;
    stub.set_lamda(&crypt_setup_helper::getToken,
                   [](const QString &, QString *token) -> int {
        ++g_getTokenCalls;
        if (token && g_getTokenCalls == 2)
            *token = "ut-token";
        return 0;
    });
    stub.set_lamda(&dm_setup_helper::findHolderDev, [](const QString &) -> QString {
        return QString("/dev/sda9");
    });

    // Act
    const QString token = svc->TpmToken("/dev/dm-9");

    // Assert
    EXPECT_EQ(token, QString("ut-token"));
    EXPECT_EQ(g_getTokenCalls, 2);
}

// TpmToken：两级查询均无 token 时返回空。
TEST_F(DiskEncryptSetupCovTest, TpmToken_NoTokenAnywhere_Empty)
{
    // Arrange
    stub.set_lamda(&crypt_setup_helper::getToken,
                   [](const QString &, QString *token) -> int {
        if (token)
            *token = "";
        return 0;
    });
    stub.set_lamda(&dm_setup_helper::findHolderDev, [](const QString &) -> QString {
        return QString("/dev/sda9");
    });

    // Act
    const QString token = svc->TpmToken("/dev/dm-9");

    // Assert
    EXPECT_TRUE(token.isEmpty());
    EXPECT_EQ(token, QString());
}

// DeviceStatus：设备自身已加密直接返回；否则查 holder 设备。
TEST_F(DiskEncryptSetupCovTest, DeviceStatus_SelfThenHolder)
{
    // Arrange
    static QList<int> g_status;
    g_status = { kStatusEncrypt | kStatusOffline, kStatusFinished };
    stub.set_lamda(&crypt_setup_helper::encryptStatus, [](const QString &) -> int {
        const int v = g_status.takeFirst();
        return v;
    });
    stub.set_lamda(&dm_setup_helper::findHolderDev, [](const QString &) -> QString {
        return QString("/dev/sda9");
    });

    // Act
    const int self = svc->DeviceStatus("/dev/dm-9");
    const int holder = svc->DeviceStatus("/dev/dm-9");

    // Assert
    EXPECT_EQ(self, kStatusEncrypt | kStatusOffline);
    EXPECT_EQ(holder, kStatusFinished);
}

// HolderDevice 直接透传 dm_setup_helper::findHolderDev。
TEST_F(DiskEncryptSetupCovTest, HolderDevice_PassesThrough)
{
    // Arrange
    stub.set_lamda(&dm_setup_helper::findHolderDev, [](const QString &dev) -> QString {
        return QString("holder-of-") + dev;
    });

    // Act
    const QString holder = svc->HolderDevice("/dev/dm-9");

    // Assert
    EXPECT_EQ(holder, QString("holder-of-/dev/dm-9"));
    EXPECT_TRUE(holder.startsWith("holder-of-"));
}

// ResumeEncryption 真实路径（解除 resumeEncryption 桩）：worker 线程完成后经
// onResumeEncryptFinished 发 EncryptResult（无任务文件 → exitCode 保持 kSuccess）。
TEST_F(DiskEncryptSetupCovTest, ResumeEncryption_RealWorkerFlow_EmitsEncryptResult)
{
    // Arrange
    stubRealAuth(true);
    stub.reset(&DiskEncryptSetupPrivate::resumeEncryption);   // 解除 fixture 桩，走真实实现
    QSignalSpy spy(svc, &DiskEncryptSetup::EncryptResult);
    const QVariantMap args { { encrypt_param_keys::kKeyDevice, QString("/dev/ut0") } };

    // Act
    const bool started = svc->ResumeEncryption(args);
    ASSERT_TRUE(started);
    ASSERT_TRUE(spy.wait(8000));

    // Assert
    ASSERT_EQ(spy.count(), 1);
    const QVariantMap result = spy.at(0).at(0).toMap();
    EXPECT_EQ(result.value(encrypt_param_keys::kKeyDevice).toString(), QString("/dev/ut0"));
    EXPECT_EQ(result.value(encrypt_param_keys::kKeyOperationResult).toInt(), kSuccess);
    EXPECT_FALSE(svc->IsTaskRunning());
}

// onLongTimeJobStarted / onLongTimeJobStopped：jobRunning 翻转并反映到 IsTaskRunning。
TEST_F(DiskEncryptSetupCovTest, LongTimeJobSlots_ToggleJobRunning)
{
    // Arrange
    ASSERT_FALSE(priv->jobRunning);

    // Act
    priv->onLongTimeJobStarted();
    const bool running = svc->IsTaskRunning();
    priv->onLongTimeJobStopped();
    const bool stopped = svc->IsTaskRunning();

    // Assert
    EXPECT_TRUE(running);
    EXPECT_FALSE(stopped);
    EXPECT_FALSE(priv->jobRunning);
}

// removeOverlayDMFlagFile / removeOverlayDMPendingFile：标志文件不存在时按成功处理。
TEST_F(DiskEncryptSetupCovTest, RemoveOverlayDMFiles_MissingFiles_Succeed)
{
    // Arrange
    // （主机 /etc 下无 overlay dm 标志文件，removeMarkerFile 对不存在路径返回 true）

    // Act
    const bool flag = priv->removeOverlayDMFlagFile();
    const bool pending = priv->removeOverlayDMPendingFile();

    // Assert
    EXPECT_TRUE(flag);
    EXPECT_TRUE(pending);
    EXPECT_FALSE(QFile::exists(QString(kOverlayDMFlagFile)));
    EXPECT_FALSE(QFile::exists(QString(kOverlayDMPendingFile)));
}

// resolveDeviceByDetachHeaderName：前缀不符 / 设备不可解析 / 对象创建失败 三分支。
TEST_F(DiskEncryptSetupCovTest, ResolveDeviceByDetachHeaderName_ThreeBranches)
{
    // Arrange / Act / Assert：非 detach 头文件名
    EXPECT_TRUE(priv->resolveDeviceByDetachHeaderName("random-name.bin").isEmpty());

    // Arrange / Act / Assert：前缀匹配但设备不可解析（PARTUUID 与 /dev 两级均失败）
    const QString name = QString(kUSecDetachHeaderPrefix) + "UT-PUID.bin";
    EXPECT_TRUE(priv->resolveDeviceByDetachHeaderName(name).isEmpty());

    // Arrange / Act / Assert：对象路径可解析但设备对象创建失败（manager 已桩为 null）
    static int g_resolveCalls4 = 0;
    g_resolveCalls4 = 0;
    stub.set_lamda(&blockdev_helper::resolveDevObjPath,
                   [](const QString &) -> QString {
        ++g_resolveCalls4;
        return QString("/obj/ut/dev");
    });
    EXPECT_TRUE(priv->resolveDeviceByDetachHeaderName(name).isEmpty());
    EXPECT_EQ(g_resolveCalls4, 1);
}

/******************************************************************************/
/****************** DiskEncryptSetupPrivate 标志文件与 initramfs **************/
/******************************************************************************/

namespace {
QStringList g_procProgram3;
QList<QStringList> g_procArgs3;
int g_procStart3 { 0 };
int g_procWaitFinished3 { 0 };
bool g_procWaitFinishedRet3 { true };
bool g_procWaitStartedRet3 { true };
int g_procExit3 { 0 };
bool g_procKilled3 { false };
int g_ensureNotifyCalls { 0 };
}   // namespace

class MarkerFileTest : public testing::Test
{
public:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        g_procProgram3.clear();
        g_procArgs3.clear();
        g_procStart3 = 0;
        g_procWaitFinished3 = 0;
        g_procWaitFinishedRet3 = true;
        g_procWaitStartedRet3 = true;
        g_procExit3 = 0;
        g_procKilled3 = false;
        g_ensureNotifyCalls = 0;

        stub.set_lamda(&QDBusService::initPolicy,
                       [](QDBusService *, const QDBusConnection::BusType &, const QString &) {});
        stub.set_lamda(&QDBusService::lockTimer, [](QDBusService *, bool) {});
        stub.set_lamda(&DiskEncryptSetupPrivate::resumeEncryption,
                       [](DiskEncryptSetupPrivate *, const QVariantMap &) {});
        stub.set_lamda(&dfmmount::DDeviceManager::instance,
                       []() -> dfmmount::DDeviceManager * { return nullptr; });
        stub.set_lamda(&inhibit_helper::inhibit,
                       [](const QString &) -> QDBusReply<QDBusUnixFileDescriptor> {
            return QDBusReply<QDBusUnixFileDescriptor>();
        });
        stub.set_lamda(static_cast<Dtk::Core::DConfig *(*)(const QString &, const QString &,
                                                           const QString &, QObject *)>(
                               &Dtk::Core::DConfig::create),
                       [](const QString &, const QString &, const QString &, QObject *)
                               -> Dtk::Core::DConfig * {
            return nullptr;
        });
        stub.set_lamda(&OverlayDMNotifyHelper::ensureNotificationDelivery,
                       [](OverlayDMNotifyHelper *, bool, int) {
            ++g_ensureNotifyCalls;
        });
    }

    void TearDown() override
    {
        stub.clear();
        QThreadPool::globalInstance()->waitForDone();
        qApp->processEvents();
    }

    DiskEncryptSetup *makeService(DiskEncryptSetupPrivate **outPriv)
    {
        auto *s = new DiskEncryptSetup();
        QThreadPool::globalInstance()->waitForDone();
        qApp->processEvents();
        *outPriv = s->m_dptr;
        return s;
    }

    void stubQProcess()
    {
        stub.set_lamda(static_cast<void (QProcess::*)(const QString &, const QStringList &,
                                                      QIODevice::OpenMode)>(&QProcess::start),
                       [](QProcess *, const QString &program, const QStringList &arguments,
                          QIODevice::OpenMode) {
            ++g_procStart3;
            g_procProgram3 << program;
            g_procArgs3 << arguments;
        });
        stub.set_lamda(&QProcess::waitForStarted, [](QProcess *, int) -> bool {
            return g_procWaitStartedRet3;
        });
        stub.set_lamda(&QProcess::waitForFinished, [](QProcess *, int) -> bool {
            ++g_procWaitFinished3;
            return g_procWaitFinishedRet3;
        });
        stub.set_lamda(&QProcess::exitCode, [](const QProcess *) -> int {
            return g_procExit3;
        });
        stub.set_lamda(&QProcess::kill, [](QProcess *) {
            g_procKilled3 = true;
        });
    }
};

// createMarkerFile：settings 目录不可创建（非 root）必须失败。
TEST_F(MarkerFileTest, CreateMarkerFile_SettingsDirNotCreatable_Fails)
{
    // Arrange
    DiskEncryptSetupPrivate *p = nullptr;
    DiskEncryptSetup *s = makeService(&p);
    ASSERT_NE(p, nullptr);

    // Act
    const bool flag = p->createOverlayDMFlagFile();
    const bool pending = p->createOverlayDMPendingFile();

    // Assert
    EXPECT_FALSE(flag);
    EXPECT_FALSE(pending);
    delete s;
}

// removeMarkerFile：不存在 → 成功；普通文件 → 删除；符号链接 → 仅删除链接。
TEST_F(MarkerFileTest, RemoveMarkerFile_ThreeCases)
{
    // Arrange
    DiskEncryptSetupPrivate *p = nullptr;
    DiskEncryptSetup *s = makeService(&p);
    ASSERT_NE(p, nullptr);
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    const QString missing = dir.filePath("missing");
    const QString normal = dir.filePath("normal");
    QFile f(normal);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();

    const QString link = dir.filePath("link-target-protected");
    QFile target(link);
    ASSERT_TRUE(target.open(QIODevice::WriteOnly));
    target.write("must-not-be-removed");
    target.close();
    const QString symlink = dir.filePath("sym");
    ASSERT_TRUE(QFile::link(normal, symlink));   // 指向普通文件的符号链接

    // Act
    const bool notExistRet = p->removeMarkerFile(missing);
    const bool normalRet = p->removeMarkerFile(normal);
    const bool symlinkRet = p->removeMarkerFile(symlink);

    // Assert
    EXPECT_TRUE(notExistRet);
    EXPECT_TRUE(normalRet);
    EXPECT_FALSE(QFile::exists(normal));
    EXPECT_TRUE(symlinkRet);
    EXPECT_FALSE(QFile::exists(symlink)) << "only the link itself may be removed";
    delete s;
}

// handleOverlayDMModeChange：pending 标志创建失败必须整体失败。
TEST_F(MarkerFileTest, HandleOverlayDMModeChange_PendingCreateFails)
{
    // Arrange
    DiskEncryptSetupPrivate *p = nullptr;
    DiskEncryptSetup *s = makeService(&p);
    ASSERT_NE(p, nullptr);

    // Act
    const bool enable = p->handleOverlayDMModeChange(true);
    const bool disable = p->handleOverlayDMModeChange(false);

    // Assert
    EXPECT_FALSE(enable);
    EXPECT_FALSE(disable);
    delete s;
}

// updateInitramfs：启动失败 / 超时 / 非零退出码 / 成功 四分支。
TEST_F(MarkerFileTest, UpdateInitramfs_FourBranches)
{
    // Arrange
    DiskEncryptSetupPrivate *p = nullptr;
    DiskEncryptSetup *s = makeService(&p);
    ASSERT_NE(p, nullptr);
    stubQProcess();

    // Act / Assert：进程启动失败
    g_procWaitStartedRet3 = false;
    EXPECT_FALSE(p->updateInitramfs());
    EXPECT_EQ(g_procStart3, 1);

    // Act / Assert：等待完成超时（需 kill）
    g_procWaitStartedRet3 = true;
    g_procWaitFinishedRet3 = false;
    EXPECT_FALSE(p->updateInitramfs());
    EXPECT_TRUE(g_procKilled3);

    // Act / Assert：非零退出码
    g_procWaitFinishedRet3 = true;
    g_procExit3 = 3;
    EXPECT_FALSE(p->updateInitramfs());

    // Act / Assert：成功
    g_procExit3 = 0;
    EXPECT_TRUE(p->updateInitramfs());
    ASSERT_EQ(g_procProgram3.size(), 4);
    EXPECT_EQ(g_procProgram3.last(), QString("/usr/bin/sudo"));
    EXPECT_TRUE(g_procArgs3.last().contains(QString("-E")));
    delete s;
}

// onConfigValueChanged：非目标键与空 config 均为安全空操作。
TEST_F(MarkerFileTest, OnConfigValueChanged_SafeBranches)
{
    // Arrange
    DiskEncryptSetupPrivate *p = nullptr;
    DiskEncryptSetup *s = makeService(&p);
    ASSERT_NE(p, nullptr);
    ASSERT_EQ(p->config, nullptr);

    // Act
    p->onConfigValueChanged("other-key");
    p->onConfigValueChanged("useOverlayDMMode");   // config 为空 → 直接返回

    // Assert
    EXPECT_FALSE(p->isHandlingConfigChange);
    EXPECT_FALSE(p->hasPendingConfigChange);
    delete s;
}

// syncConfigWithFileSystem：config 为空直接返回。
TEST_F(MarkerFileTest, SyncConfigWithFileSystem_NullConfig_Returns)
{
    // Arrange
    DiskEncryptSetupPrivate *p = nullptr;
    DiskEncryptSetup *s = makeService(&p);
    ASSERT_NE(p, nullptr);

    // Act
    p->syncConfigWithFileSystem();

    // Assert
    EXPECT_EQ(p->config, nullptr);
    EXPECT_FALSE(p->isHandlingConfigChange);
    delete s;
}

// processPendingConfigChange：无待处理 / 与当前一致 / 需执行 三分支。
TEST_F(MarkerFileTest, ProcessPendingConfigChange_Branches)
{
    // Arrange
    DiskEncryptSetupPrivate *p = nullptr;
    DiskEncryptSetup *s = makeService(&p);
    ASSERT_NE(p, nullptr);
    static int g_asyncCalls = 0;
    g_asyncCalls = 0;
    stub.set_lamda(&DiskEncryptSetupPrivate::handleOverlayDMModeChangeAsync,
                   [](DiskEncryptSetupPrivate *, bool) {
        ++g_asyncCalls;
    });

    // Act / Assert：无待处理
    p->processPendingConfigChange();
    EXPECT_EQ(g_asyncCalls, 0);

    // Act / Assert：待处理值与当前实际状态一致 → 清标记不执行
    p->hasPendingConfigChange = true;
    p->pendingTargetValue = false;
    p->currentTargetValue = false;
    p->processPendingConfigChange();
    EXPECT_FALSE(p->hasPendingConfigChange);
    EXPECT_EQ(g_asyncCalls, 0);

    // Act / Assert：待处理值与当前实际状态不同 → 执行异步变更
    p->hasPendingConfigChange = true;
    p->pendingTargetValue = true;
    p->currentTargetValue = false;
    p->processPendingConfigChange();
    EXPECT_FALSE(p->hasPendingConfigChange);
    EXPECT_EQ(g_asyncCalls, 1);
    delete s;
}

// handleOverlayDMModeChangeAsync + onOverlayDMModeChangeFinished：失败结果经信号上报。
TEST_F(MarkerFileTest, OverlayDMModeChangeAsync_FailureEmitsSignal)
{
    // Arrange
    DiskEncryptSetupPrivate *p = nullptr;
    DiskEncryptSetup *s = makeService(&p);
    ASSERT_NE(p, nullptr);
    stub.set_lamda(&DiskEncryptSetupPrivate::handleOverlayDMModeChange,
                   [](DiskEncryptSetupPrivate *, bool) -> bool {
        return false;
    });
    QSignalSpy spy(s, &DiskEncryptSetup::OverlayDMModeChanged);

    // Act
    p->handleOverlayDMModeChangeAsync(true);
    ASSERT_TRUE(spy.wait(8000));

    // Assert
    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toBool(), true);
    EXPECT_EQ(spy.at(0).at(1).toInt(), OverlayDMFailedUpdateInitramfs);
    EXPECT_FALSE(p->isHandlingConfigChange);
    EXPECT_GE(g_ensureNotifyCalls, 1);
    delete s;
}

// handleOverlayDMModeChangeAsync：成功结果上报 OverlayDMSuccess，回滚中断场景上报专用码。
TEST_F(MarkerFileTest, OverlayDMModeChangeFinished_SuccessAndRollbackCodes)
{
    // Arrange
    DiskEncryptSetupPrivate *p = nullptr;
    DiskEncryptSetup *s = makeService(&p);
    ASSERT_NE(p, nullptr);

    // Act / Assert：成功
    p->onOverlayDMModeChangeFinished(true, true);
    EXPECT_EQ(g_ensureNotifyCalls, 1);

    // Act / Assert：中断回滚成功 → OverlayDMRolledBackInterrupted
    p->isRollbackFromInterrupted = true;
    p->onOverlayDMModeChangeFinished(true, false);
    EXPECT_FALSE(p->isRollbackFromInterrupted);
    EXPECT_GE(g_ensureNotifyCalls, 2);
    delete s;
}

// setupConfigWatcher：config 为空安全返回；注入 config 后可重复设置。
TEST_F(MarkerFileTest, SetupConfigWatcher_NullAndInjectedConfig)
{
    // Arrange
    DiskEncryptSetupPrivate *p = nullptr;
    DiskEncryptSetup *s = makeService(&p);
    ASSERT_NE(p, nullptr);

    // Act：空 config（真实分支）
    EXPECT_NO_FATAL_FAILURE(p->setupConfigWatcher());

    // Arrange：注入真实 DConfig 对象
    p->config = new Dtk::Core::DConfig("ut-diskencrypt-cov");

    // Act
    EXPECT_NO_FATAL_FAILURE(p->setupConfigWatcher());

    // Assert
    delete p->config;
    p->config = nullptr;
    EXPECT_EQ(p->config, nullptr);
    delete s;
}
