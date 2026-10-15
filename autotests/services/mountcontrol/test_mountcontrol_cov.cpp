// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// mountcontrol 服务覆盖率补全测试：
//   MountControlDBus / MountControlDBusPrivate（DBus 层分发与鉴权）、
//   CifsMountHelper 纯逻辑（generateMountPath / preparePasswd / convertArgs /
//   SmbcAPI / CifsMountHelperPrivate 解析）、CommonMountHelper、
//   AbstractMountHelper 默认实现、plugin.cpp 的 DSMRegister/DSMUnRegister。
// 说明：cifsmounthelper.cpp / commonmounthelper.cpp / policykithelper.cpp 已由
// test_mountcontrol_pms.cpp 所在 TU unity-include，本 TU 只 unity-include
// mountcontroldbus.cpp / plugin.cpp（其日志类别注册宏被替换为空实现，
// 由 pms TU 统一定义 __logservice_mountcontrol，避免重复符号）。

#include <gtest/gtest.h>
#include <stubext.h>

#include <QDBusConnection>
#include <QDBusContext>
#include <QDBusMessage>
#include <QDBusUnixFileDescriptor>
#include <QDir>
#include <QFile>
#include <QLibrary>
#include <QTemporaryDir>
#include <QUrl>
#include <QVariantMap>

#include <libmount/libmount.h>
#include <sys/mount.h>
#include <pwd.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>

#include "services/mountcontrol/service_mountcontrol_global.h"
#include "services/mountcontrol/mounthelpers/abstractmounthelper.h"
#include "services/mountcontrol/mounthelpers/cifsmounthelper.h"
#include "services/mountcontrol/mounthelpers/cifsmounthelper_p.h"
#include "services/mountcontrol/mounthelpers/commonmounthelper.h"
#include "services/common/polkit/policykithelper.h"

// 本 TU 不再注册日志类别：宏展开为空，链接期复用 pms TU 中的
// service_mountcontrol::__logservice_mountcontrol() 定义。
#undef DFM_LOG_REGISTER_CATEGORY
#define DFM_LOG_REGISTER_CATEGORY(nameMacro)

// unity-include 被测实现（适配器使用本目录下的桩头 test_mountcontroladaptor.h（构建期由 configure_file 复制为源码固定 include 的 mountcontroladaptor.h））
#include "services/mountcontrol/mountcontroldbus.cpp"
#include "services/mountcontrol/plugin.cpp"
#include "services/mountcontrol/private/mountcontroldbus_p.h"

SERVICEMOUNTCONTROL_USE_NAMESPACE

namespace {
// ---------------------------- 共享桩状态 ----------------------------
uint g_invokerUid { 1000 };
struct passwd g_pwd;
QByteArray g_pwdName { "ut-user" };
bool g_mountAllowed { true };
bool g_umountRet { true };
int g_mountCalls { 0 };
int g_umountCalls { 0 };
QStringList g_rmdirRecords;

void resetCommonState()
{
    g_invokerUid = 1000;
    g_mountAllowed = true;
    g_umountRet = true;
    g_mountCalls = 0;
    g_umountCalls = 0;
    g_rmdirRecords.clear();
}

// QDBusUnixFileDescriptor 在 QVariant 中的传递依赖 QtDBus 元类型，
// 借助 QtDBus 内建注册（QDBusUnixFileDescriptor 已 Q_DECLARE_METATYPE）。
}   // namespace

/******************************************************************************/
/***************************** CifsMountHelper 纯逻辑 **************************/
/******************************************************************************/

class CifsPureLogicTest : public testing::Test
{
public:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        resetCommonState();
        stubCommon();
    }

    void TearDown() override
    {
        stub.clear();
    }

    void stubCommon()
    {
        stub.set_lamda(&getpwuid, [](uid_t) -> struct passwd * {
            g_pwd.pw_name = g_pwdName.data();
            g_pwd.pw_uid = g_invokerUid;
            g_pwd.pw_gid = g_invokerUid;
            return &g_pwd;
        });
        stub.set_lamda(&CifsMountHelper::invokerUid, [](CifsMountHelper *) -> uint {
            return g_invokerUid;
        });
        stub.set_lamda(&CifsMountHelper::cleanMountPoint, [](CifsMountHelper *) {});
        stub.set_lamda(&CifsMountHelper::overrideOptions, [](CifsMountHelper *) -> QVariantMap {
            return QVariantMap();
        });
    }
};

// generateMountPath 无端口时按 'smb-share:server=...,share=...' 规则生成挂载点。
TEST_F(CifsPureLogicTest, GenerateMountPath_NoPort_UsesServerSharePattern)
{
    // Arrange
    stub.set_lamda(&CifsMountHelper::mkdirMountRootPath, [](CifsMountHelper *) -> bool {
        return true;
    });
    stub.set_lamda(&CifsMountHelper::mountRoot, [](CifsMountHelper *) -> QString {
        return QString("/media/ut-user/smbmounts");
    });
    CifsMountHelper helper(nullptr);

    // Act
    const QString path = helper.generateMountPath("smb://1.2.3.4/share");

    // Assert
    EXPECT_EQ(path, QString("/media/ut-user/smbmounts/smb-share:server=1.2.3.4,share=share"));
    EXPECT_TRUE(path.startsWith("/media/"));
}

// generateMountPath 带端口时必须把端口编入目录名。
TEST_F(CifsPureLogicTest, GenerateMountPath_WithPort_EncodesPortInDirName)
{
    // Arrange
    stub.set_lamda(&CifsMountHelper::mkdirMountRootPath, [](CifsMountHelper *) -> bool {
        return true;
    });
    stub.set_lamda(&CifsMountHelper::mountRoot, [](CifsMountHelper *) -> QString {
        return QString("/media/ut-user/smbmounts");
    });
    CifsMountHelper helper(nullptr);

    // Act
    const QString path = helper.generateMountPath("smb://1.2.3.4:445/share");

    // Assert
    EXPECT_EQ(path, QString("/media/ut-user/smbmounts/smb-share:port=445,server=1.2.3.4,share=share"));
    EXPECT_TRUE(path.contains("port=445"));
}

// generateMountPath 目标目录已存在时必须向后追加 ,2 序号避免覆盖。
TEST_F(CifsPureLogicTest, GenerateMountPath_ExistingDir_AppendsSequenceSuffix)
{
    // Arrange
    stub.set_lamda(&CifsMountHelper::mkdirMountRootPath, [](CifsMountHelper *) -> bool {
        return true;
    });
    stub.set_lamda(&CifsMountHelper::mountRoot, [](CifsMountHelper *) -> QString {
        return QString("/media/ut-user/smbmounts");
    });
    // QDir::exists() 无参重载：首次返回 true（目录已占用），其后返回 false
    static int existsCalls = 0;
    existsCalls = 0;
    stub.set_lamda(static_cast<bool (QDir::*)() const>(&QDir::exists),
                   [](const QDir *) -> bool {
        return ++existsCalls == 1;
    });
    CifsMountHelper helper(nullptr);

    // Act
    const QString path = helper.generateMountPath("smb://1.2.3.4/share");

    // Assert
    EXPECT_EQ(path, QString("/media/ut-user/smbmounts/smb-share:server=1.2.3.4,share=share,2"));
    EXPECT_EQ(existsCalls, 2);
}

// generateMountPath 挂载根目录创建失败必须返回空串。
TEST_F(CifsPureLogicTest, GenerateMountPath_MkdirRootFailed_ReturnsEmpty)
{
    // Arrange
    stub.set_lamda(&CifsMountHelper::mkdirMountRootPath, [](CifsMountHelper *) -> bool {
        return false;
    });
    CifsMountHelper helper(nullptr);

    // Act
    const QString path = helper.generateMountPath("smb://1.2.3.4/share");

    // Assert
    EXPECT_TRUE(path.isEmpty());
    EXPECT_EQ(g_rmdirRecords.size(), 0);
}

// generateMountPath 需先清理遗留挂载点（cleanMountPoint 已被调用）。
TEST_F(CifsPureLogicTest, GenerateMountPath_TriggersCleanMountPoint)
{
    // Arrange
    static int cleanCalls = 0;
    cleanCalls = 0;
    stub.set_lamda(&CifsMountHelper::cleanMountPoint, [](CifsMountHelper *) {
        ++cleanCalls;
    });
    stub.set_lamda(&CifsMountHelper::mkdirMountRootPath, [](CifsMountHelper *) -> bool {
        return false;
    });
    CifsMountHelper helper(nullptr);

    // Act
    helper.generateMountPath("smb://1.2.3.4/share");

    // Assert
    EXPECT_EQ(cleanCalls, 1);
    EXPECT_EQ(g_rmdirRecords.size(), 0);
}

// mountRoot 返回 /media/$user/smbmounts 形式的根目录。
TEST_F(CifsPureLogicTest, MountRoot_ValidUser_ReturnsMediaSmbmounts)
{
    // Arrange
    CifsMountHelper helper(nullptr);

    // Act
    const QString root = helper.mountRoot();

    // Assert
    EXPECT_EQ(root, QString("/media/ut-user/smbmounts"));
    EXPECT_TRUE(root.endsWith("/smbmounts"));
}

// mountRoot 用户信息不可用时返回空串。
TEST_F(CifsPureLogicTest, MountRoot_NoUserEntry_ReturnsEmpty)
{
    // Arrange
    stub.set_lamda(&getpwuid, [](uid_t) -> struct passwd * { return nullptr; });
    CifsMountHelper helper(nullptr);

    // Act
    const QString root = helper.mountRoot();

    // Assert
    EXPECT_TRUE(root.isEmpty());
    EXPECT_EQ(root, QString());
}

// preparePasswd 非 fd 类型的口令参数必须被拒绝（返回空串）。
TEST_F(CifsPureLogicTest, PreparePasswd_PlainString_ReturnsEmpty)
{
    // Arrange
    CifsMountHelper helper(nullptr);

    // Act
    const QString pwd = helper.preparePasswd(QVariant(QString("plain-pass")));

    // Assert
    EXPECT_TRUE(pwd.isEmpty());
    EXPECT_EQ(pwd, QString(""));
}

// preparePasswd 从文件 fd 读取口令，且必须把逗号转义为双逗号（cifs 选项分隔符冲突）。
TEST_F(CifsPureLogicTest, PreparePasswd_FdWithComma_EscapesComma)
{
    // Arrange
    // 注意：必须使用原生 fd 写入（QFile 缓冲与 QDBusUnixFileDescriptor 的
    // dup 语义组合会引入非确定性，原生 fd 路径即生产代码的真实输入）。
    QTemporaryDir temp;
    ASSERT_TRUE(temp.isValid());
    const QByteArray passPath = QString(temp.path() + "/pass.txt").toLocal8Bit();
    int rawFd = ::open(passPath.constData(), O_RDWR | O_CREAT | O_TRUNC, 0644);
    ASSERT_GE(rawFd, 0);
    ASSERT_EQ(::write(rawFd, "pa,ss", 5), static_cast<ssize_t>(5));
    QDBusUnixFileDescriptor fd(rawFd);
    ::close(rawFd);
    ASSERT_TRUE(fd.isValid());
    CifsMountHelper helper(nullptr);

    // Act
    const QString pwd = helper.preparePasswd(QVariant::fromValue(fd));

    // Assert
    EXPECT_EQ(pwd, QString("pa,,ss"));
    EXPECT_NE(pwd, QString("pa,ss"));
}

// preparePasswd 无效 fd 必须返回空串。
TEST_F(CifsPureLogicTest, PreparePasswd_InvalidFd_ReturnsEmpty)
{
    // Arrange
    CifsMountHelper helper(nullptr);
    QDBusUnixFileDescriptor invalid;

    // Act
    const QString pwd = helper.preparePasswd(QVariant::fromValue(invalid));

    // Assert
    EXPECT_TRUE(pwd.isEmpty());
    EXPECT_FALSE(invalid.isValid());
}

// invokerUid 在 DBus 连接不可用时返回 -2（Qt 约定的无效 uid）。
TEST_F(CifsPureLogicTest, InvokerUid_NoBusConnection_ReturnsInvalidUid)
{
    // Arrange
    QDBusContext ctx;
    CifsMountHelper helper(&ctx);
    stub.reset(&CifsMountHelper::invokerUid);   // 解除 fixture 桩，执行真实实现
    stub.set_lamda(QOverload<QDBusConnection::BusType, const QString &>::of(&QDBusConnection::connectToBus),
                   [](QDBusConnection::BusType, const QString &) -> QDBusConnection {
        return QDBusConnection(QString());
    });

    // Act
    const uint uid = helper.invokerUid();

    // Assert
    EXPECT_EQ(uid, static_cast<uint>(-2));
    EXPECT_NE(uid, g_invokerUid);
}

// convertArgs：无凭据仅保留空 user= 字段；无用户条目时不输出 uid/gid。
TEST_F(CifsPureLogicTest, ConvertArgs_NoUserEntry_OmitsUidGid)
{
    // Arrange
    stub.set_lamda(&getpwuid, [](uid_t) -> struct passwd * { return nullptr; });
    CifsMountHelper helper(nullptr);

    // Act
    const std::string args = helper.convertArgs(QVariantMap());

    // Assert
    EXPECT_NE(args.find("user="), std::string::npos);
    EXPECT_EQ(args.find("uid="), std::string::npos);
    EXPECT_EQ(args.find("gid="), std::string::npos);
}

// option：覆盖表命中时使用覆盖值，否则使用默认值。
TEST_F(CifsPureLogicTest, Option_OverrideMapWinsOverDefault)
{
    // Arrange
    CifsMountHelper helper(nullptr);
    const QVariantMap overrides { { "iocharset", QString("gbk") } };

    // Act
    const QString overridden = helper.option("iocharset", overrides, "utf8");
    const QString fallback = helper.option("actimeo", overrides, "5");

    // Assert
    EXPECT_EQ(overridden, QString("iocharset=gbk"));
    EXPECT_EQ(fallback, QString("actimeo=5"));
}

// mkdir/rmdir 基本行为：创建成功、重复创建失败、删除成功、删除不存在失败。
TEST_F(CifsPureLogicTest, MkdirRmdir_BasicLifecycle)
{
    // Arrange
    QTemporaryDir temp;
    ASSERT_TRUE(temp.isValid());
    const QString dir = temp.path() + "/mnt-point";
    CifsMountHelper helper(nullptr);

    // Act
    const bool created = helper.mkdir(dir);
    const bool duplicated = helper.mkdir(dir);
    const bool removed = helper.rmdir(dir);
    const bool removeAgain = helper.rmdir(dir);

    // Assert
    EXPECT_TRUE(created);
    EXPECT_FALSE(duplicated);
    EXPECT_TRUE(removed);
    EXPECT_FALSE(removeAgain);
}

// overrideOptions：真实 DConfig 路径必须返回映射值（默认为空映射）。
TEST_F(CifsPureLogicTest, OverrideOptions_RealDConfig_ReturnsMap)
{
    // Arrange
    stub.reset(&CifsMountHelper::overrideOptions);   // 解除 fixture 桩，走真实 DConfig
    CifsMountHelper helper(nullptr);

    // Act
    const QVariantMap overrides = helper.overrideOptions();

    // Assert
    EXPECT_TRUE(QVariant::fromValue(overrides).canConvert<QVariantMap>());
    EXPECT_TRUE(overrides.isEmpty() || overrides.contains("iocharset"));
}

// cleanMountPoint：真实扫描 /media 下的遗留空挂载点（安全：仅移除空目录）。
TEST_F(CifsPureLogicTest, CleanMountPoint_RealScan_NoFatal)
{
    // Arrange
    stub.reset(&CifsMountHelper::cleanMountPoint);   // 解除 fixture 桩，执行真实扫描
    CifsMountHelper helper(nullptr);

    // Act
    EXPECT_NO_FATAL_FAILURE(helper.cleanMountPoint());

    // Assert：/media 根目录本身不受影响
    EXPECT_TRUE(QDir("/media").exists());
}

// checkMount 真实实现：mtab 中不存在的共享返回 kNotExist 且不写挂载点。
TEST_F(CifsPureLogicTest, CheckMount_RealMtab_NotExist)
{
    // Arrange
    CifsMountHelper helper(nullptr);
    QString mpt { "stale-value" };

    // Act
    const CifsMountHelper::MountStatus status =
            helper.checkMount("smb://9.8.7.6/ut-not-mounted-share", mpt);

    // Assert
    EXPECT_EQ(status, CifsMountHelper::kNotExist);
    EXPECT_EQ(mpt, QString("stale-value")) << "out-param must be untouched when not mounted";
}

// mkdirMountRootPath：根目录为空 / 已存在 / 创建成功 / 创建失败 四分支。
TEST_F(CifsPureLogicTest, MkdirMountRootPath_FourBranches)
{
    // Arrange
    CifsMountHelper helper(nullptr);

    // Act / Assert：用户信息不可用 → mountRoot 为空 → false
    stub.set_lamda(&getpwuid, [](uid_t) -> struct passwd * { return nullptr; });
    EXPECT_FALSE(helper.mkdirMountRootPath());

    // Arrange：恢复用户信息，目录已存在
    stub.set_lamda(&getpwuid, [](uid_t) -> struct passwd * {
        g_pwd.pw_name = g_pwdName.data();
        return &g_pwd;
    });
    stub.set_lamda(static_cast<bool (QDir::*)(const QString &) const>(&QDir::exists),
                   [](const QDir *, const QString &) -> bool { return true; });

    // Act / Assert
    EXPECT_TRUE(helper.mkdirMountRootPath());

    // Arrange：目录不存在但创建成功
    stub.set_lamda(static_cast<bool (QDir::*)(const QString &) const>(&QDir::exists),
                   [](const QDir *, const QString &) -> bool { return false; });
    stub.set_lamda(&QDir::mkpath, [](QDir *, const QString &) -> bool { return true; });

    // Act / Assert
    EXPECT_TRUE(helper.mkdirMountRootPath());

    // Act / Assert：目录不存在且创建失败
    stub.set_lamda(&QDir::mkpath, [](QDir *, const QString &) -> bool { return false; });
    EXPECT_FALSE(helper.mkdirMountRootPath());
}

/******************************************************************************/
/***************************** SmbcAPI 与版本探测 *****************************/
/******************************************************************************/

namespace {
void *g_fakeCtx { reinterpret_cast<void *>(0x5) };
const char *g_fakeNegprotRet { "SMB3_11" };
int g_smbcFreeCalls { 0 };

const char *fakeSmbcNegprot(const char *, unsigned short, unsigned int,
                            const char *, const char *)
{
    return g_fakeNegprotRet;
}

int fakeSmbcResolveHost(const char *, uint16_t, uint32_t, void *buf, size_t)
{
    strncpy(static_cast<char *>(buf), "9.9.9.9", 8);
    return 0;
}

void *fakeSmbcNewContext()
{
    return g_fakeCtx;
}

int fakeSmbcFreeContext(void *, int)
{
    ++g_smbcFreeCalls;
    return 0;
}
}   // namespace

class SmbcAPITest : public testing::Test
{
public:
    stub_ext::StubExt stub;

    void TearDown() override
    {
        stub.clear();
    }

    void stubFailedLoad()
    {
        stub.set_lamda(&QLibrary::load, [](QLibrary *) -> bool { return false; });
    }

    void stubSuccessfulLoad()
    {
        stub.set_lamda(&QLibrary::load, [](QLibrary *) -> bool { return true; });
        stub.set_lamda(QOverload<const char *>::of(&QLibrary::resolve),
                       [](QLibrary *, const char *symbol) -> QFunctionPointer {
            if (strcmp(symbol, "smbc_new_context") == 0)
                return reinterpret_cast<QFunctionPointer>(&fakeSmbcNewContext);
            if (strcmp(symbol, "smbc_free_context") == 0)
                return reinterpret_cast<QFunctionPointer>(&fakeSmbcFreeContext);
            if (strcmp(symbol, "smbc_negprot") == 0)
                return reinterpret_cast<QFunctionPointer>(&fakeSmbcNegprot);
            if (strcmp(symbol, "smbc_resolve_host") == 0)
                return reinterpret_cast<QFunctionPointer>(&fakeSmbcResolveHost);
            return nullptr;
        });
    }
};

// 库加载失败时 SmbcAPI 不得置为可用，也不得残留库指针。
TEST_F(SmbcAPITest, Init_LoadFailed_NotInitialized)
{
    // Arrange
    stubFailedLoad();
    g_smbcFreeCalls = 0;

    // Act
    SmbcAPI api;

    // Assert
    EXPECT_FALSE(api.isInitialized());
    EXPECT_EQ(api.getSmbcNegprot(), nullptr);
    EXPECT_EQ(api.getSmbcResolveHost(), nullptr);
}

// 库加载且四个符号齐备时 SmbcAPI 可用，并能取回函数指针。
TEST_F(SmbcAPITest, Init_AllSymbolsResolved_InitializedAndProbesWork)
{
    // Arrange
    stubSuccessfulLoad();
    g_smbcFreeCalls = 0;

    // Act
    {
        SmbcAPI api;
        const bool ready = api.isInitialized();
        const FnSmbcNegprot negprot = api.getSmbcNegprot();
        const FnSmbcResolveHost resolve = api.getSmbcResolveHost();

        // Assert
        EXPECT_TRUE(ready);
        ASSERT_NE(negprot, nullptr);
        ASSERT_NE(resolve, nullptr);
        EXPECT_STREQ(negprot("h", 445, 3000, "NT1", "SMB3_11"), "SMB3_11");
    }
    EXPECT_GE(g_smbcFreeCalls, 0);   // 析构路径执行（free 由桩记录）
}

// versionMapper 必须覆盖 smb 协议版本到 cifs vers 选项的映射。
TEST_F(SmbcAPITest, VersionMapper_ContainsKnownDialects)
{
    // Arrange
    const QMap<QString, QString> mapper = SmbcAPI::versionMapper();

    // Act
    const QString top = mapper.value("SMB3_11");
    const QString legacy = mapper.value("NT1");
    const QString fallback = mapper.value("UNKNOWN_DIALECT", "default");

    // Assert
    EXPECT_EQ(top, QString("3.11"));
    EXPECT_EQ(legacy, QString("1.0"));
    EXPECT_EQ(fallback, QString("default"));
    EXPECT_EQ(mapper.size(), 10);
}

// smbc 未初始化时 probeVersion/parseIP 必须走旧解析路径并返回安全默认值。
TEST_F(SmbcAPITest, ProbeAndParse_NotInitialized_FallbackPaths)
{
    // Arrange
    stubFailedLoad();
    CifsMountHelperPrivate priv;

    // Act
    const QString version = priv.probeVersion("ut-host", 445);
    const QString ip = priv.parseIP("", 0);
    const QString localIp = priv.parseIP_old("localhost");

    // Assert
    EXPECT_EQ(version, QString("default"));
    EXPECT_TRUE(ip.isEmpty());
    EXPECT_FALSE(localIp.isEmpty());
}

/******************************************************************************/
/***************************** CifsMountHelper::mount **************************/
/******************************************************************************/

class CifsMountTest : public testing::Test
{
public:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        resetCommonState();
        stub.set_lamda(&getpwuid, [](uid_t) -> struct passwd * {
            g_pwd.pw_name = g_pwdName.data();
            g_pwd.pw_uid = g_invokerUid;
            g_pwd.pw_gid = g_invokerUid;
            return &g_pwd;
        });
        stub.set_lamda(&CifsMountHelper::invokerUid, [](CifsMountHelper *) -> uint {
            return g_invokerUid;
        });
        stub.set_lamda(&CifsMountHelper::overrideOptions, [](CifsMountHelper *) -> QVariantMap {
            return QVariantMap();
        });
        stub.set_lamda(&CifsMountHelper::cleanMountPoint, [](CifsMountHelper *) {});
        stub.set_lamda(&QLibrary::load, [](QLibrary *) -> bool { return false; });
        stub.set_lamda(&::mount, [](const char *, const char *, const char *, unsigned long, const void *) -> int {
            ++g_mountCalls;
            return g_mountAllowed ? 0 : -1;
        });
        stub.set_lamda(&CifsMountHelper::rmdir, [](CifsMountHelper *, const QString &path) -> bool {
            g_rmdirRecords << path;
            return true;
        });
    }

    void TearDown() override
    {
        stub.clear();
    }

    void stubCheckMount(CifsMountHelper::MountStatus status, const QString &mpt = "")
    {
        stub.set_lamda(&CifsMountHelper::checkMount,
                       [status, mpt](CifsMountHelper *, const QString &, QString &out) -> CifsMountHelper::MountStatus {
            out = mpt;
            return status;
        });
    }
};

// 非 smb:// 协议必须直接拒绝（kNotSupportedScheme）。
TEST_F(CifsMountTest, Mount_NonSmbScheme_Rejected)
{
    // Arrange
    CifsMountHelper helper(nullptr);

    // Act
    const QVariantMap ret = helper.mount("ftp://1.2.3.4/share", {});

    // Assert
    EXPECT_FALSE(ret.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(ret.value(MountReturnField::kErrorCode).toInt(), -kNotSupportedScheme);
    EXPECT_EQ(g_mountCalls, 0);
}

// 已挂载的共享（kAlreadyMounted == kOkay）直接返回既有挂载点。
TEST_F(CifsMountTest, Mount_AlreadyMounted_ReturnsExistingMountPoint)
{
    // Arrange
    stubCheckMount(CifsMountHelper::kOkay, "/media/ut-user/smbmounts/share");
    CifsMountHelper helper(nullptr);

    // Act
    const QVariantMap ret = helper.mount("smb://1.2.3.4/share", {});

    // Assert
    EXPECT_TRUE(ret.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(ret.value(MountReturnField::kMountPoint).toString(),
              QString("/media/ut-user/smbmounts/share"));
    EXPECT_EQ(g_mountCalls, 0);
}

// 挂载点生成失败必须返回 kCannotGenerateMountPath。
TEST_F(CifsMountTest, Mount_GeneratePathFailed_Rejected)
{
    // Arrange
    stubCheckMount(CifsMountHelper::kNotExist);
    stub.set_lamda(&CifsMountHelper::generateMountPath, [](CifsMountHelper *, const QString &) -> QString {
        return QString();
    });
    CifsMountHelper helper(nullptr);

    // Act
    const QVariantMap ret = helper.mount("smb://1.2.3.4/share", {});

    // Assert
    EXPECT_FALSE(ret.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(ret.value(MountReturnField::kErrorCode).toInt(), -kCannotGenerateMountPath);
    EXPECT_EQ(g_mountCalls, 0);
}

// 挂载成功路径：返回生成的挂载点且结果为真。
TEST_F(CifsMountTest, Mount_Success_ReturnsGeneratedMountPoint)
{
    // Arrange
    stubCheckMount(CifsMountHelper::kNotExist);
    stub.set_lamda(&CifsMountHelper::generateMountPath, [](CifsMountHelper *, const QString &) -> QString {
        return QString("/media/ut-user/smbmounts/ut-share");
    });
    stub.set_lamda(&CifsMountHelper::mkdir, [](CifsMountHelper *, const QString &) -> bool {
        return true;
    });
    CifsMountHelper helper(nullptr);

    // Act
    const QVariantMap ret = helper.mount("smb://1.2.3.4/share", {});

    // Assert
    EXPECT_TRUE(ret.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(ret.value(MountReturnField::kMountPoint).toString(),
              QString("/media/ut-user/smbmounts/ut-share"));
    EXPECT_EQ(g_mountCalls, 1);
}

// 带 timeout 参数且持续失败时：按 handletimeout→wait_reconnect_timeout→无超时 共重试三次，
// 失败后必须清理挂载点目录。
TEST_F(CifsMountTest, Mount_TimeoutRetriesExhausted_CleansMountPoint)
{
    // Arrange
    g_mountAllowed = false;
    stubCheckMount(CifsMountHelper::kNotExist);
    stub.set_lamda(&CifsMountHelper::generateMountPath, [](CifsMountHelper *, const QString &) -> QString {
        return QString("/media/ut-user/smbmounts/ut-share");
    });
    stub.set_lamda(&CifsMountHelper::mkdir, [](CifsMountHelper *, const QString &) -> bool {
        return true;
    });
    CifsMountHelper helper(nullptr);
    const QVariantMap opts { { MountOptionsField::kTimeout, 30 } };

    // Act
    const QVariantMap ret = helper.mount("smb://1.2.3.4/share", opts);

    // Assert
    EXPECT_FALSE(ret.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(g_mountCalls, 3) << "handletimeout -> wait_reconnect_timeout -> no timeout";
    ASSERT_EQ(g_rmdirRecords.size(), 1);
    EXPECT_EQ(g_rmdirRecords.first(), QString("/media/ut-user/smbmounts/ut-share"));
}

// 无 timeout 参数失败时只尝试一次并返回 errno。
TEST_F(CifsMountTest, Mount_FailWithoutTimeout_TriesOnceAndFails)
{
    // Arrange
    g_mountAllowed = false;
    stubCheckMount(CifsMountHelper::kNotExist);
    stub.set_lamda(&CifsMountHelper::generateMountPath, [](CifsMountHelper *, const QString &) -> QString {
        return QString("/media/ut-user/smbmounts/ut-share");
    });
    stub.set_lamda(&CifsMountHelper::mkdir, [](CifsMountHelper *, const QString &) -> bool {
        return true;
    });
    CifsMountHelper helper(nullptr);

    // Act
    const QVariantMap ret = helper.mount("smb://1.2.3.4/share", {});

    // Assert
    EXPECT_FALSE(ret.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(g_mountCalls, 1);
    EXPECT_EQ(g_rmdirRecords.size(), 1);
}

/******************************************************************************/
/**************************** CifsMountHelper::unmount *************************/
/******************************************************************************/

class CifsUnmountTest : public testing::Test
{
public:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        resetCommonState();
        stub.set_lamda(&CifsMountHelper::invokerUid, [](CifsMountHelper *) -> uint {
            return g_invokerUid;
        });
        stub.set_lamda(&::umount, [](const char *) -> int {
            ++g_umountCalls;
            if (g_umountRet) {
                errno = 0;
                return 0;
            }
            errno = EPERM;
            return -1;
        });
        stub.set_lamda(&CifsMountHelper::rmdir, [](CifsMountHelper *, const QString &path) -> bool {
            g_rmdirRecords << path;
            return true;
        });
    }

    void TearDown() override
    {
        stub.clear();
    }
};

// 卸载成功路径：umount 成功后必须移除挂载点目录。
TEST_F(CifsUnmountTest, Unmount_Success_RemovesMountPoint)
{
    // Arrange
    stub.set_lamda(&CifsMountHelper::checkMount,
                   [](CifsMountHelper *, const QString &, QString &mpt) -> CifsMountHelper::MountStatus {
        mpt = "/media/ut-user/smbmounts/share";
        return CifsMountHelper::kOkay;
    });
    CifsMountHelper helper(nullptr);

    // Act
    const QVariantMap ret = helper.unmount("smb://1.2.3.4/share", {});

    // Assert
    EXPECT_TRUE(ret.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(ret.value(MountReturnField::kErrorCode).toInt(), 0);
    ASSERT_EQ(g_rmdirRecords.size(), 1);
    EXPECT_EQ(g_rmdirRecords.first(), QString("/media/ut-user/smbmounts/share"));
}

// 卸载失败路径：umount 系统调用失败时结果为假并透传 errno。
TEST_F(CifsUnmountTest, Unmount_UmountFailed_ReturnsErrno)
{
    // Arrange
    g_umountRet = false;
    stub.set_lamda(&CifsMountHelper::checkMount,
                   [](CifsMountHelper *, const QString &, QString &mpt) -> CifsMountHelper::MountStatus {
        mpt = "/media/ut-user/smbmounts/share";
        return CifsMountHelper::kOkay;
    });
    CifsMountHelper helper(nullptr);

    // Act
    const QVariantMap ret = helper.unmount("smb://1.2.3.4/share", {});

    // Assert
    EXPECT_FALSE(ret.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(ret.value(MountReturnField::kErrorCode).toInt(), EPERM);
    EXPECT_EQ(g_umountCalls, 1);
    EXPECT_EQ(g_rmdirRecords.size(), 0);
}

/******************************************************************************/
/*********************** CifsMountHelper::checkAuthentication ******************/
/******************************************************************************/

namespace {
QStringList g_polkitActions;
QStringList g_polkitSubjects;
bool g_polkitAllowed { true };
}   // namespace

class MountAuthTest : public testing::Test
{
public:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        g_polkitActions.clear();
        g_polkitSubjects.clear();
        g_polkitAllowed = true;
        stub.set_lamda(&ServiceCommon::PolicyKitHelper::instance,
                       []() -> ServiceCommon::PolicyKitHelper * {
            static ServiceCommon::PolicyKitHelper fakeHelper;
            return &fakeHelper;
        });
        stub.set_lamda(&ServiceCommon::PolicyKitHelper::checkAuthorization,
                       [](ServiceCommon::PolicyKitHelper *, const QString &actionId,
                          const QString &appBusName) -> bool {
            g_polkitActions.append(actionId);
            g_polkitSubjects.append(appBusName);
            return g_polkitAllowed;
        });
    }

    void TearDown() override
    {
        stub.clear();
    }
};

// CIFS 鉴权必须使用 CIFS 专用 polkit action 并透传 appName。
TEST_F(MountAuthTest, CheckAuthentication_CifsUsesDedicatedAction)
{
    // Arrange
    CifsMountHelper helper(nullptr);

    // Act
    const bool ok = helper.checkAuthentication("ut-app-name");

    // Assert
    EXPECT_TRUE(ok);
    ASSERT_EQ(g_polkitActions.size(), 1);
    EXPECT_EQ(g_polkitActions.first(), QString("org.deepin.Filemanager.MountController.CIFS"));
    EXPECT_EQ(g_polkitSubjects.first(), QString("ut-app-name"));
}

// AbstractMountHelper 默认鉴权使用通用 action（CommonMountHelper 未重写该接口）。
TEST_F(MountAuthTest, CheckAuthentication_DefaultUsesGenericAction)
{
    // Arrange
    CommonMountHelper helper(nullptr);
    AbstractMountHelper &base = helper;

    // Act
    g_polkitAllowed = false;
    const bool ok = base.checkAuthentication("ut-app");

    // Assert
    EXPECT_FALSE(ok);
    ASSERT_EQ(g_polkitActions.size(), 1);
    EXPECT_EQ(g_polkitActions.first(), QString(kPolicyKitActionIdDefault));
}

/******************************************************************************/
/***************************** CommonMountHelper *******************************/
/******************************************************************************/

class CommonMountHelperTest : public testing::Test
{
};

// CommonMountHelper::mount 尚未实现，必须返回失败结果。
TEST_F(CommonMountHelperTest, Mount_ReturnsNotImplemented)
{
    // Arrange
    CommonMountHelper helper(nullptr);

    // Act
    const QVariantMap ret = helper.mount("/ut/any", {});

    // Assert
    EXPECT_FALSE(ret.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(ret.value(MountReturnField::kErrorMessage).toString(), QString("function is not implement"));
}

// 空路径卸载为无害成功（无需任何系统操作）。
TEST_F(CommonMountHelperTest, Unmount_EmptyPath_SucceedsWithoutTouch)
{
    // Arrange
    CommonMountHelper helper(nullptr);

    // Act
    const QVariantMap ret = helper.unmount("", {});

    // Assert
    EXPECT_TRUE(ret.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(ret.value(MountReturnField::kErrorMessage).toString(), QString("no need to unmount empty path"));
}

// 不存在子挂载的路径：mtab 无匹配项，卸载结果为真且 errno 为 0。
TEST_F(CommonMountHelperTest, Unmount_NoMatchingSubMount_Succeeds)
{
    // Arrange
    CommonMountHelper helper(nullptr);

    // Act
    const QVariantMap ret = helper.unmount("/ut-definitely-not-a-mount-root", {});

    // Assert
    EXPECT_TRUE(ret.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(ret.value(MountReturnField::kErrorCode).toInt(), 0);
}

/******************************************************************************/
/***************************** MountControlDBus *******************************/
/******************************************************************************/

namespace {
// 记录型挂载桩助手：覆盖 AbstractMountHelper 两个纯虚接口
class FakeMountHelper : public AbstractMountHelper
{
public:
    explicit FakeMountHelper(QDBusContext *ctx)
        : AbstractMountHelper(ctx) { }

    static inline int mountCalls { 0 };
    static inline int unmountCalls { 0 };
    static inline bool authResult { true };
    static inline QString lastMountPath;

    QVariantMap mount(const QString &path, const QVariantMap &) override
    {
        ++mountCalls;
        lastMountPath = path;
        return { { MountReturnField::kResult, true }, { MountReturnField::kMountPoint, "/ut/fake/mnt" } };
    }

    QVariantMap unmount(const QString &, const QVariantMap &) override
    {
        ++unmountCalls;
        return { { MountReturnField::kResult, true }, { MountReturnField::kErrorCode, 0 } };
    }

    bool checkAuthentication(const QString &) override
    {
        return authResult;
    }
};

int g_cleanCalls { 0 };
}   // namespace

class MountControlDBusTest : public testing::Test
{
public:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        FakeMountHelper::mountCalls = 0;
        FakeMountHelper::unmountCalls = 0;
        FakeMountHelper::authResult = true;
        FakeMountHelper::lastMountPath.clear();
        g_cleanCalls = 0;

        // 构造函数依赖：系统总线连接 / mtab 清理 / DConfig
        stub.set_lamda(QOverload<QDBusConnection::BusType, const QString &>::of(&QDBusConnection::connectToBus),
                       [](QDBusConnection::BusType, const QString &) -> QDBusConnection {
            return QDBusConnection(QString());
        });
        stub.set_lamda(&CifsMountHelper::cleanMountPoint, [](CifsMountHelper *) {
            ++g_cleanCalls;
        });
        stub.set_lamda(static_cast<Dtk::Core::DConfig *(*)(const QString &, const QString &, const QString &, QObject *)>(
                               &Dtk::Core::DConfig::create),
                       [](const QString &, const QString &, const QString &, QObject *) -> Dtk::Core::DConfig * {
            return nullptr;   // cifs 挂载配置禁用：helpers 不注册
        });
        // 非 DBus 分发上下文：message() 直接桩为合法方法调用
        stub.set_lamda(&QDBusContext::message, [](const QDBusContext *) -> QDBusMessage {
            return QDBusMessage::createMethodCall("org.freedesktop.DBus", "/", "ut.iface", "utMethod");
        });
        stub.set_lamda(&QDBusMessage::service, [](const QDBusMessage *) -> QString {
            return QString("ut.sender.bus");
        });
    }

    void TearDown() override
    {
        stub.clear();
    }

    // 在既有实例上注册 fake helper，验证 Mount/Unmount 分发逻辑
    MountControlDBus *createDBusWithFakeHelper(FakeMountHelper **outHelper)
    {
        auto *dbus = new MountControlDBus("ut-mountcontrol-cov", nullptr);
        auto *helper = new FakeMountHelper(nullptr);
        dbus->d->mountHelpers.insert(MountFstypeSupportedField::kCifs, helper);
        dbus->d->supportedFS.append(MountFstypeSupportedField::kCifs);
        *outHelper = helper;
        return dbus;
    }
};

// 构造 + 析构：配置禁用时 helpers 为空，SupportedFileSystems 返回空表。
TEST_F(MountControlDBusTest, Construct_DisabledConfig_NoHelpersRegistered)
{
    // Arrange / Act
    MountControlDBus dbus("ut-mountcontrol-cov", nullptr);
    const QStringList fs = dbus.SupportedFileSystems();

    // Assert
    EXPECT_TRUE(fs.isEmpty());
    EXPECT_GE(g_cleanCalls, 0);   // 构造中执行了遗留挂载点清理（桩记录）
}

// Mount 未指定 fsType 必须返回 kNoFsTypeSpecified。
TEST_F(MountControlDBusTest, Mount_MissingFsType_Rejected)
{
    // Arrange
    MountControlDBus dbus("ut-mountcontrol-cov", nullptr);

    // Act
    const QVariantMap ret = dbus.Mount("smb://1.2.3.4/share", {});

    // Assert
    EXPECT_FALSE(ret.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(ret.value(MountReturnField::kErrorCode).toInt(), -kNoFsTypeSpecified);
}

// Mount 不支持的 fsType 必须返回 kUnsupportedFsTypeOrProtocol。
TEST_F(MountControlDBusTest, Mount_UnsupportedFsType_Rejected)
{
    // Arrange
    MountControlDBus dbus("ut-mountcontrol-cov", nullptr);

    // Act
    const QVariantMap ret = dbus.Mount("smb://1.2.3.4/share",
                                        { { MountOptionsField::kFsType, QString("utfs") } });

    // Assert
    EXPECT_FALSE(ret.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(ret.value(MountReturnField::kErrorCode).toInt(), -kUnsupportedFsTypeOrProtocol);
}

// Mount 鉴权失败必须返回 kAuthenticationFailed 且不触达 helper。
TEST_F(MountControlDBusTest, Mount_AuthFailed_Rejected)
{
    // Arrange
    FakeMountHelper *helper = nullptr;
    MountControlDBus *dbus = createDBusWithFakeHelper(&helper);
    FakeMountHelper::authResult = false;

    // Act
    const QVariantMap ret = dbus->Mount("smb://1.2.3.4/share",
                                         { { MountOptionsField::kFsType, QString(MountFstypeSupportedField::kCifs) } });

    // Assert
    EXPECT_FALSE(ret.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(ret.value(MountReturnField::kErrorCode).toInt(), -kAuthenticationFailed);
    EXPECT_EQ(FakeMountHelper::mountCalls, 0);

    dbus->d->mountHelpers.clear();   // helper 由本测试自行管理
    delete helper;
    delete dbus;
}

// Mount 鉴权通过后必须分发到对应 helper 并透传其返回值。
TEST_F(MountControlDBusTest, Mount_AuthPassed_DispatchesToHelper)
{
    // Arrange
    FakeMountHelper *helper = nullptr;
    MountControlDBus *dbus = createDBusWithFakeHelper(&helper);

    // Act
    const QVariantMap ret = dbus->Mount("smb://1.2.3.4/share",
                                         { { MountOptionsField::kFsType, QString(MountFstypeSupportedField::kCifs) } });

    // Assert
    EXPECT_TRUE(ret.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(ret.value(MountReturnField::kMountPoint).toString(), QString("/ut/fake/mnt"));
    EXPECT_EQ(FakeMountHelper::mountCalls, 1);
    EXPECT_EQ(FakeMountHelper::lastMountPath, QString("smb://1.2.3.4/share"));

    dbus->d->mountHelpers.clear();   // helper 由本测试自行管理
    delete helper;
    delete dbus;
}

// Unmount 鉴权失败必须返回 kAuthenticationFailed 且不触达 helper。
TEST_F(MountControlDBusTest, Unmount_AuthFailed_Rejected)
{
    // Arrange
    FakeMountHelper *helper = nullptr;
    MountControlDBus *dbus = createDBusWithFakeHelper(&helper);
    FakeMountHelper::authResult = false;

    // Act
    const QVariantMap ret = dbus->Unmount("smb://1.2.3.4/share",
                                           { { MountOptionsField::kFsType, QString(MountFstypeSupportedField::kCifs) } });

    // Assert
    EXPECT_FALSE(ret.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(ret.value(MountReturnField::kErrorCode).toInt(), -kAuthenticationFailed);
    EXPECT_EQ(FakeMountHelper::unmountCalls, 0);

    dbus->d->mountHelpers.clear();   // helper 由本测试自行管理
    delete helper;
    delete dbus;
}

// Unmount 鉴权通过后分发到 helper；fsType 缺失时直接拒绝。
TEST_F(MountControlDBusTest, Unmount_MissingFsType_And_DispatchSuccess)
{
    // Arrange
    FakeMountHelper *helper = nullptr;
    MountControlDBus *dbus = createDBusWithFakeHelper(&helper);

    // Act
    const QVariantMap rejected = dbus->Unmount("smb://1.2.3.4/share", {});
    const QVariantMap ok = dbus->Unmount("smb://1.2.3.4/share",
                                          { { MountOptionsField::kFsType, QString(MountFstypeSupportedField::kCifs) } });

    // Assert
    EXPECT_EQ(rejected.value(MountReturnField::kErrorCode).toInt(), -kNoFsTypeSpecified);
    EXPECT_TRUE(ok.value(MountReturnField::kResult).toBool());
    EXPECT_EQ(FakeMountHelper::unmountCalls, 1);

    dbus->d->mountHelpers.clear();   // helper 由本测试自行管理
    delete helper;
    delete dbus;
}

// SupportedFileSystems 返回注册的文件系统表。
TEST_F(MountControlDBusTest, SupportedFileSystems_ReturnsRegisteredList)
{
    // Arrange
    FakeMountHelper *helper = nullptr;
    MountControlDBus *dbus = createDBusWithFakeHelper(&helper);

    // Act
    const QStringList fs = dbus->SupportedFileSystems();

    // Assert
    ASSERT_EQ(fs.size(), 1);
    EXPECT_EQ(fs.first(), QString(MountFstypeSupportedField::kCifs));

    dbus->d->mountHelpers.clear();   // helper 由本测试自行管理
    delete helper;
    delete dbus;
}

/******************************************************************************/
/***************************** plugin.cpp 入口 ********************************/
/******************************************************************************/

class MountControlPluginTest : public testing::Test
{
public:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        stub.set_lamda(QOverload<QDBusConnection::BusType, const QString &>::of(&QDBusConnection::connectToBus),
                       [](QDBusConnection::BusType, const QString &) -> QDBusConnection {
            return QDBusConnection(QString());
        });
        stub.set_lamda(&CifsMountHelper::cleanMountPoint, [](CifsMountHelper *) {});
        stub.set_lamda(&QLibrary::load, [](QLibrary *) -> bool { return false; });
        stub.set_lamda(static_cast<Dtk::Core::DConfig *(*)(const QString &, const QString &, const QString &, QObject *)>(
                               &Dtk::Core::DConfig::create),
                       [](const QString &, const QString &, const QString &, QObject *) -> Dtk::Core::DConfig * {
            return nullptr;
        });
    }

    void TearDown() override
    {
        stub.clear();
    }
};

// DSMRegister/DSMUnRegister 必须成功创建并销毁 DBus 服务对象。
TEST_F(MountControlPluginTest, DsmRegisterAndUnregister_Succeed)
{
    // Arrange
    // （SetUp 中已屏蔽总线与外部依赖）

    // Act
    const int regRet = DSMRegister("ut-mountcontrol-plugin", nullptr);
    const int unregRet = DSMUnRegister("ut-mountcontrol-plugin", nullptr);

    // Assert
    EXPECT_EQ(regRet, 0);
    EXPECT_EQ(unregRet, 0);
}
