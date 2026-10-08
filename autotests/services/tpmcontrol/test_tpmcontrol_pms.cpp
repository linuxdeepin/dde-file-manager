// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// PMS sev-2 回归测试：tpmcontrol 服务（bug 372451 / 343001）。
// 服务以可执行文件构建，无库可链，此处 unity-include 被测服务实现源码。

#include <gtest/gtest.h>
#include <stubext.h>

#include <QDBusConnection>
#include <QDBusUnixFileDescriptor>
#include <QDataStream>
#include <QFile>
#include <QLibrary>
#include <QTemporaryDir>
#include <QVariantMap>

#include <cstring>

#include "services/tpmcontrol/tpmcontroldbus.h"
#include "services/tpmcontrol/core/tpmwork.h"
#include "services/tpmcontrol/service_tpmcontrol_global.h"
#include "services/common/polkit/policykithelper.h"

// unity-include 被测实现（服务无可链接目标）
#include "services/tpmcontrol/tpmcontroldbus.cpp"
#include "services/tpmcontrol/core/tpmwork.cpp"
#include "services/common/polkit/policykithelper.cpp"

SERVICETPMCONTROL_USE_NAMESPACE

/******************************************************************************/
/***************************** bug 372451 相关桩状态 **************************/
/******************************************************************************/
namespace {
QStringList g_authActions;
bool g_authAllowed { true };
int g_tpmLockout { -2 };
int g_tpmAvailable { 1 };
int g_randomRet { 0 };
QString g_randomOut { "random-data" };
int g_encryptRet { 0 };
QVariantMap g_encryptArgs;
int g_decryptRet { 0 };
QString g_decryptPwd { "decrypted-pwd" };

void resetDBusStubs()
{
    g_authActions.clear();
    g_authAllowed = true;
    g_tpmLockout = -2;
    g_tpmAvailable = 1;
    g_randomRet = 0;
    g_randomOut = "random-data";
    g_encryptRet = 0;
    g_encryptArgs.clear();
    g_decryptRet = 0;
    g_decryptPwd = "decrypted-pwd";
}
}   // namespace

class TPMControlDBusTest : public testing::Test
{
public:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        resetDBusStubs();

        // 构造函数连接 SystemBus，桩掉避免外部依赖
        stub.set_lamda(QOverload<QDBusConnection::BusType, const QString &>::of(&QDBusConnection::connectToBus),
                       [](QDBusConnection::BusType, const QString &) -> QDBusConnection {
            return QDBusConnection(QString());
        });

        // 非 DBus 分发上下文下 message() 会解引用空指针，
        // 桩为携带合法发送者的方法调用消息
        stub.set_lamda(&QDBusContext::message, [](const QDBusContext *) -> QDBusMessage {
            QDBusMessage msg = QDBusMessage::createMethodCall(
                    "org.freedesktop.DBus", "/", "ut.iface", "utMethod");
            return msg;
        });
        // 默认消息的 service() 会解引用空内部指针，同样桩掉
        stub.set_lamda(&QDBusMessage::service, [](const QDBusMessage *) -> QString {
            return QString("ut.sender.bus");
        });

        // PolicyKit 鉴权桩：记录 actionId，返回可配置结果
        stub.set_lamda(&ServiceCommon::PolicyKitHelper::instance,
                       []() -> ServiceCommon::PolicyKitHelper * {
            static ServiceCommon::PolicyKitHelper fakeHelper;
            return &fakeHelper;
        });
        stub.set_lamda(&ServiceCommon::PolicyKitHelper::checkAuthorization,
                       [](ServiceCommon::PolicyKitHelper *, const QString &actionId, const QString &) -> bool {
            g_authActions.append(actionId);
            return g_authAllowed;
        });

        // TPMWork 全部 Pro 方法打桩
        stub.set_lamda(&TPMWork::isTPMAvailable, [](TPMWork *) -> int {
            return g_tpmAvailable;
        });
        stub.set_lamda(&TPMWork::checkTPMLockout, [](TPMWork *) -> int {
            return g_tpmLockout;
        });
        stub.set_lamda(&TPMWork::isSupportAlgo, [](TPMWork *, const QString &algo, bool *support) -> int {
            if (support)
                *support = !algo.isEmpty();
            return 0;
        });
        stub.set_lamda(&TPMWork::ownerAuthStatus, [](TPMWork *) -> int { return 1; });
        stub.set_lamda(&TPMWork::getRandom, [](TPMWork *, int, QString *output) -> int {
            if (output)
                *output = g_randomOut;
            return g_randomRet;
        });
        stub.set_lamda(&TPMWork::encrypt, [](TPMWork *, const QVariantMap &args) -> int {
            g_encryptArgs = args;
            return g_encryptRet;
        });
        stub.set_lamda(&TPMWork::decrypt, [](TPMWork *, const QVariantMap &, QString *pwd) -> int {
            if (pwd)
                *pwd = g_decryptPwd;
            return g_decryptRet;
        });
    }

    void TearDown() override
    {
        stub.clear();
    }

    static QDBusUnixFileDescriptor makeParamsFd(const QVariantMap &args)
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

// PMS:372451 五个公开查询方法（IsTPMAvailable/CheckTPMLockout/IsSupportAlgo/
// OwnerAuthStatus/GetRandom）鉴权时使用不存在的 kQuery action，
// polkit 始终拒绝导致用户无法使用 TPM 功能；修复为统一使用 kAccess action。
TEST_F(TPMControlDBusTest, BUG372451_QueryMethodsUseAccessPolicy)
{
    TPMControlDBus dbus("ut-tpm-bug372451", nullptr);

    bool support = false;
    EXPECT_EQ(dbus.IsTPMAvailable(), g_tpmAvailable);
    EXPECT_EQ(dbus.CheckTPMLockout(), g_tpmLockout);
    EXPECT_EQ(dbus.IsSupportAlgo("sha256", support), 0);
    EXPECT_EQ(dbus.OwnerAuthStatus(), 1);

    QDBusUnixFileDescriptor randomFd;
    EXPECT_EQ(dbus.GetRandom(16, randomFd), kNoError);

    const char *kAccess = PolicyKitActionId::kAccess;
    ASSERT_EQ(g_authActions.size(), 5);
    for (const QString &action : g_authActions)
        EXPECT_EQ(action, QString(kAccess));
}

// PMS:372451 Encrypt/Decrypt 敏感接口同样必须使用 kAccess action，
// 且鉴权通过后参数/结果应正确透传给 TPMWork。
TEST_F(TPMControlDBusTest, BUG372451_EncryptDecryptUseAccessPolicyAndPassArgs)
{
    TPMControlDBus dbus("ut-tpm-bug372451-enc", nullptr);

    QVariantMap args {
        { PropertyKey::kEncryptType, kTpmAndPin },
        { PropertyKey::kDirPath, "/tmp/ut-tpm-343001" },
        { PropertyKey::kPinCode, "1234" }
    };
    QDBusUnixFileDescriptor paramsFd = makeParamsFd(args);
    ASSERT_TRUE(paramsFd.isValid());

    EXPECT_EQ(dbus.Encrypt(paramsFd), g_encryptRet);
    EXPECT_EQ(g_encryptArgs.value(PropertyKey::kEncryptType).toInt(), kTpmAndPin);
    EXPECT_EQ(g_encryptArgs.value(PropertyKey::kPinCode).toString(), QString("1234"));

    // fd 一次性：Decrypt 需要独立的新管道（首个 fd 已被 Encrypt 消费到 EOF）
    QDBusUnixFileDescriptor decryptFd = makeParamsFd(args);
    ASSERT_TRUE(decryptFd.isValid());
    QDBusUnixFileDescriptor passwordFd;
    EXPECT_EQ(dbus.Decrypt(decryptFd, passwordFd), kNoError);

    const char *kAccess = PolicyKitActionId::kAccess;
    ASSERT_EQ(g_authActions.size(), 2);
    EXPECT_EQ(g_authActions.at(0), QString(kAccess));
    EXPECT_EQ(g_authActions.at(1), QString(kAccess));
}

// PMS:372451 鉴权失败时五个查询方法必须返回 kAuthFailed 且不触达 TPM。
TEST_F(TPMControlDBusTest, BUG372451_AuthFailedReturnsKAuthFailed)
{
    g_authAllowed = false;
    TPMControlDBus dbus("ut-tpm-bug372451-deny", nullptr);

    bool support = false;
    EXPECT_EQ(dbus.IsTPMAvailable(), kAuthFailed);
    EXPECT_EQ(dbus.CheckTPMLockout(), kAuthFailed);
    EXPECT_EQ(dbus.IsSupportAlgo("sha256", support), kAuthFailed);
    EXPECT_EQ(dbus.OwnerAuthStatus(), kAuthFailed);

    QDBusUnixFileDescriptor randomFd;
    EXPECT_EQ(dbus.GetRandom(16, randomFd), kAuthFailed);
    // 鉴权失败时也必须给出有效的 fallback fd，避免调用方解引用空 fd
    EXPECT_TRUE(randomFd.isValid());

    EXPECT_EQ(g_authActions.size(), 5);
}

// PMS:372451 CheckTPMLockout 查询结果必须原样透传（锁定状态码不可丢失）。
TEST_F(TPMControlDBusTest, BUG372451_CheckTPMLockoutPassesThroughResult)
{
    g_tpmLockout = 5;
    TPMControlDBus dbus("ut-tpm-bug372451-lock", nullptr);
    EXPECT_EQ(dbus.CheckTPMLockout(), 5);

    g_tpmLockout = 0;
    TPMControlDBus dbus2("ut-tpm-bug372451-unlock", nullptr);
    EXPECT_EQ(dbus2.CheckTPMLockout(), 0);
}

/******************************************************************************/
/***************************** bug 343001 相关桩状态 **************************/
/******************************************************************************/
namespace {
int g_fakeEncCalls { 0 };
TpmProType g_fakeEncType { kCTpmAndPcr };
char *g_fakeEncDirPath { nullptr };
int g_fakeEncRet { 0 };

int fakeUtpm2Encrypt(const Utpm2EncryptParamsByTools *par)
{
    g_fakeEncCalls++;
    g_fakeEncType = par->type;
    g_fakeEncDirPath = par->dirPath;
    return g_fakeEncRet;
}

int fakeUtpm2Decrypt(const Utpm2DecryptParamsByTools *, char *pwd, int *len)
{
    const char *pwdStr = "fake-decrypted";
    strncpy(pwd, pwdStr, strlen(pwdStr));
    *len = static_cast<int>(strlen(pwdStr));
    return 0;
}

int g_fakeDecCalls { 0 };
}   // namespace

class TPMWorkTest : public testing::Test
{
public:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        g_fakeEncCalls = 0;
        g_fakeEncDirPath = nullptr;
        g_fakeEncRet = 0;
        g_fakeDecCalls = 0;

        // TPMWork 构造即加载 libutpm2.so，桩掉避免环境依赖
        stub.set_lamda(&QLibrary::load, [](QLibrary *) -> bool { return true; });
        stub.set_lamda(&QLibrary::isLoaded, [](QLibrary *) -> bool { return true; });
        // resolve 有多个重载，取实例版本（tpmwork 仅使用实例调用）
        stub.set_lamda(QOverload<const char *>::of(&QLibrary::resolve),
                       [](QLibrary *, const char *symbol) -> QFunctionPointer {
            if (strcmp(symbol, "utpm2_encrypt_by_tools") == 0)
                return reinterpret_cast<QFunctionPointer>(&fakeUtpm2Encrypt);
            if (strcmp(symbol, "utpm2_decrypt_by_tools") == 0)
                return reinterpret_cast<QFunctionPointer>(&fakeUtpm2Decrypt);
            return nullptr;
        });
    }

    void TearDown() override
    {
        stub.clear();
    }
};

// PMS:343001 TPMWork::encrypt 生成的 token 文件默认 0600，
// 普通用户（登录进程）无读取权限，导致解密时 Token 信息读为空、密码解密失败；
// 修复为加密完成后将目录内全部文件补上 Group/Other 读权限。
TEST_F(TPMWorkTest, BUG343001_EncryptSetsReadPermissionOnGeneratedFiles)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    // 模拟 utpm2 库生成的 0600 文件
    const QStringList files { "token.bin", "primary.blob" };
    for (const QString &name : files) {
        QFile f(dir.path() + "/" + name);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("utpm2-secret");
        f.close();
        // 模拟 utpm2 库生成的 0600 权限（覆盖 umask 影响）
        ASSERT_TRUE(f.setPermissions(QFile::ReadOwner | QFile::WriteOwner));
        ASSERT_EQ(f.permissions() & (QFile::ReadGroup | QFile::ReadOther),
                  QFile::Permissions())
                << "precondition: file must start without group/other read";
    }

    TPMWork work;
    ASSERT_TRUE(work.isLibraryLoaded());

    const QVariantMap params {
        { PropertyKey::kEncryptType, kTpmAndPin },
        { PropertyKey::kSessionHashAlgo, "sha256" },
        { PropertyKey::kSessionKeyAlgo, "aes256" },
        { PropertyKey::kPrimaryHashAlgo, "sha256" },
        { PropertyKey::kPrimaryKeyAlgo, "rsa2048" },
        { PropertyKey::kMinorHashAlgo, "sha256" },
        { PropertyKey::kMinorKeyAlgo, "ecc256" },
        { PropertyKey::kDirPath, dir.path() },
        { PropertyKey::kPlain, "plain-text" },
        { PropertyKey::kPinCode, "1234" }
    };

    const int ret = work.encrypt(params);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(g_fakeEncCalls, 1);
    EXPECT_EQ(g_fakeEncType, kCTpmAndPin);

    // 回归核心：加密后全部生成文件必须对 Group/Other 可读
    for (const QString &name : files) {
        const QFile::Permissions perms = QFileInfo(dir.path() + "/" + name).permissions();
        EXPECT_TRUE(perms & QFile::ReadGroup) << name.toStdString() << " missing ReadGroup";
        EXPECT_TRUE(perms & QFile::ReadOther) << name.toStdString() << " missing ReadOther";
    }
}

// PMS:343001 非法的加密类型必须返回 kInvalidParam，不得调用底层库。
TEST_F(TPMWorkTest, BUG343001_EncryptInvalidTypeRejected)
{
    TPMWork work;
    ASSERT_TRUE(work.isLibraryLoaded());

    const QVariantMap params {
        { PropertyKey::kEncryptType, 99 },
        { PropertyKey::kDirPath, "/tmp/ut-tpm-invalid" }
    };

    EXPECT_EQ(work.encrypt(params), kInvalidParam);
    EXPECT_EQ(g_fakeEncCalls, 0);
}

// PMS:343001 非法的解密类型同样返回 kInvalidParam。
TEST_F(TPMWorkTest, BUG343001_DecryptInvalidTypeRejected)
{
    TPMWork work;
    ASSERT_TRUE(work.isLibraryLoaded());

    const QVariantMap params {
        { PropertyKey::kEncryptType, kUnknow },
        { PropertyKey::kDirPath, "/tmp/ut-tpm-invalid" }
    };

    QString pwd;
    EXPECT_EQ(work.decrypt(params, &pwd), kInvalidParam);
    EXPECT_EQ(g_fakeDecCalls, 0);
}

// PMS:343001 解密成功路径应把底层库返回的密码带回调用方。
TEST_F(TPMWorkTest, BUG343001_DecryptReturnsPassword)
{
    TPMWork work;
    ASSERT_TRUE(work.isLibraryLoaded());

    const QVariantMap params {
        { PropertyKey::kEncryptType, kTpmAndPin },
        { PropertyKey::kDirPath, "/tmp/ut-tpm-decrypt" },
        { PropertyKey::kPinCode, "1234" }
    };

    QString pwd;
    EXPECT_EQ(work.decrypt(params, &pwd), 0);
    EXPECT_EQ(pwd, QString("fake-decrypted"));
}
