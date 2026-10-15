// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// tpmcontrol 服务覆盖率补全测试：
//   TPMWork 剩余 Pro 接口（isTPMAvailable / checkTPMLockout / getRandom /
//   isSupportAlgo / ownerAuthStatus）与库未加载分支；
//   TPMControlDBus 的 fd 解析 / 回包通道 / 失败回退 fd 路径。
// 服务实现已由 test_tpmcontrol_pms.cpp 所在 TU unity-include，
// 本 TU 仅引用头文件并复用其符号（stub 运行期生效，跨 TU 单一定义）。

#include <gtest/gtest.h>
#include <stubext.h>

#include <QDBusConnection>
#include <QDBusContext>
#include <QDBusMessage>
#include <QDBusUnixFileDescriptor>
#include <QDataStream>
#include <QLibrary>
#include <QProcess>
#include <QTemporaryDir>
#include <QVariantMap>

#include <cstring>
#include <unistd.h>

#include "services/tpmcontrol/tpmcontroldbus.h"
#include "services/tpmcontrol/core/tpmwork.h"
#include "services/tpmcontrol/service_tpmcontrol_global.h"
#include "services/common/polkit/policykithelper.h"

SERVICETPMCONTROL_USE_NAMESPACE

/******************************************************************************/
/***************************** TPMWork 底层接口 ********************************/
/******************************************************************************/

namespace {
int g_checkTpmRet { 1 };
int g_checkTpmCalls { 0 };
int g_getRandomRet { 0 };
QByteArray g_getRandomData { "ABCD" };
int g_getRandomCalls { 0 };
int g_checkAlgoRet { 0 };
bool g_checkAlgoSupport { true };
int g_getcapRet { 0 };
QByteArray g_getcapValue { "1" };

int fakeCheckTpm()
{
    ++g_checkTpmCalls;
    return g_checkTpmRet;
}

int fakeGetRandom(int, char *buf)
{
    ++g_getRandomCalls;
    if (g_getRandomRet == 0)
        memcpy(buf, g_getRandomData.constData(), g_getRandomData.size());
    return g_getRandomRet;
}

int fakeCheckAlgo(const char *, bool *support)
{
    if (support)
        *support = g_checkAlgoSupport;
    return g_checkAlgoRet;
}

int fakeGetcapVarProp(const char *, char *buf, size_t size)
{
    if (g_getcapRet != 0)
        return g_getcapRet;
    snprintf(buf, size, "%s", g_getcapValue.constData());
    return 0;
}

void resetWorkState()
{
    g_checkTpmRet = 1;
    g_checkTpmCalls = 0;
    g_getRandomRet = 0;
    g_getRandomData = "ABCD";
    g_getRandomCalls = 0;
    g_checkAlgoRet = 0;
    g_checkAlgoSupport = true;
    g_getcapRet = 0;
    g_getcapValue = "1";
}
}   // namespace

class TPMWorkProTest : public testing::Test
{
public:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        resetWorkState();
        stub.set_lamda(&QLibrary::load, [](QLibrary *) -> bool { return true; });
        stub.set_lamda(&QLibrary::isLoaded, [](QLibrary *) -> bool { return true; });
        stub.set_lamda(QOverload<const char *>::of(&QLibrary::resolve),
                       [](QLibrary *, const char *symbol) -> QFunctionPointer {
            if (strcmp(symbol, "utpm2_check_tpm_by_tools") == 0)
                return reinterpret_cast<QFunctionPointer>(&fakeCheckTpm);
            if (strcmp(symbol, "utpm2_get_random_by_tools") == 0)
                return reinterpret_cast<QFunctionPointer>(&fakeGetRandom);
            if (strcmp(symbol, "utpm2_check_alg_by_tools") == 0)
                return reinterpret_cast<QFunctionPointer>(&fakeCheckAlgo);
            if (strcmp(symbol, "utpm2_getcap_varprop_by_tools") == 0)
                return reinterpret_cast<QFunctionPointer>(&fakeGetcapVarProp);
            return nullptr;
        });
    }

    void TearDown() override
    {
        stub.clear();
    }
};

// isTPMAvailable 透传底层库返回值。
TEST_F(TPMWorkProTest, IsTPMAvailable_PassesThroughLibraryResult)
{
    // Arrange
    TPMWork work;
    ASSERT_TRUE(work.isLibraryLoaded());

    // Act
    const int yes = work.isTPMAvailable();
    g_checkTpmRet = 0;
    const int no = work.isTPMAvailable();

    // Assert
    EXPECT_EQ(yes, 1);
    EXPECT_EQ(no, 0);
    EXPECT_EQ(g_checkTpmCalls, 2);
}

// 库未加载时 isTPMAvailable 返回 kLibLoadFailed 且不触达底层。
TEST_F(TPMWorkProTest, IsTPMAvailable_LibraryNotLoaded_FailsFast)
{
    // Arrange
    TPMWork work;
    stub.set_lamda(&QLibrary::isLoaded, [](QLibrary *) -> bool { return false; });

    // Act
    const int ret = work.isTPMAvailable();

    // Assert
    EXPECT_EQ(ret, kLibLoadFailed);
    EXPECT_EQ(g_checkTpmCalls, 0);
}

// 库符号缺失（resolve 失败）时返回 kLibLoadFailed。
TEST_F(TPMWorkProTest, IsTPMAvailable_SymbolMissing_FailsFast)
{
    // Arrange
    stub.set_lamda(QOverload<const char *>::of(&QLibrary::resolve),
                   [](QLibrary *, const char *) -> QFunctionPointer { return nullptr; });
    TPMWork work;

    // Act
    const int ret = work.isTPMAvailable();

    // Assert
    EXPECT_EQ(ret, kLibLoadFailed);
    EXPECT_EQ(g_checkTpmCalls, 0);
}

// getRandom 成功路径：输出底层库写入的随机串。
TEST_F(TPMWorkProTest, GetRandom_Success_ReturnsRandomData)
{
    // Arrange
    TPMWork work;
    QString output;

    // Act
    const int ret = work.getRandom(4, &output);

    // Assert
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(output, QString("ABCD"));
    EXPECT_EQ(g_getRandomCalls, 1);
}

// getRandom 尺寸非法（非正数 / 超限）必须返回 kInvalidParam。
TEST_F(TPMWorkProTest, GetRandom_InvalidSize_Rejected)
{
    // Arrange
    TPMWork work;
    QString output;

    // Act
    const int zero = work.getRandom(0, &output);
    const int negative = work.getRandom(-1, &output);
    const int huge = work.getRandom(4097, &output);

    // Assert
    EXPECT_EQ(zero, kInvalidParam);
    EXPECT_EQ(negative, kInvalidParam);
    EXPECT_EQ(huge, kInvalidParam);
    EXPECT_EQ(g_getRandomCalls, 0) << "invalid size must not reach the library";
}

// getRandom 底层失败时原样返回错误码且不写输出。
TEST_F(TPMWorkProTest, GetRandom_LibraryFailed_PropagatesCode)
{
    // Arrange
    g_getRandomRet = -6;
    TPMWork work;
    QString output { "stale" };

    // Act
    const int ret = work.getRandom(4, &output);

    // Assert
    EXPECT_EQ(ret, -6);
    EXPECT_EQ(output, QString("stale")) << "output must be untouched on failure";
    EXPECT_EQ(g_getRandomCalls, 1);
}

// isSupportAlgo 把算法名与结果指针透传给底层库。
TEST_F(TPMWorkProTest, IsSupportAlgo_PassesArgsAndResultThrough)
{
    // Arrange
    g_checkAlgoSupport = true;
    g_checkAlgoRet = 0;
    TPMWork work;
    bool support = false;

    // Act
    const int ret = work.isSupportAlgo("sha256", &support);

    // Assert
    EXPECT_EQ(ret, 0);
    EXPECT_TRUE(support);

    // Act（不支持场景）
    g_checkAlgoSupport = false;
    bool support2 = true;
    const int ret2 = work.isSupportAlgo("md5", &support2);

    // Assert
    EXPECT_EQ(ret2, 0);
    EXPECT_FALSE(support2);
}

// isSupportAlgo 库未加载时返回 kLibLoadFailed。
TEST_F(TPMWorkProTest, IsSupportAlgo_LibraryNotLoaded_FailsFast)
{
    // Arrange
    stub.set_lamda(&QLibrary::isLoaded, [](QLibrary *) -> bool { return false; });
    TPMWork work;
    bool support = false;

    // Act
    const int ret = work.isSupportAlgo("sha256", &support);

    // Assert
    EXPECT_EQ(ret, kLibLoadFailed);
    EXPECT_FALSE(support);
}

// ownerAuthStatus 成功路径返回底层查询值。
TEST_F(TPMWorkProTest, OwnerAuthStatus_SuccessReturnsQueriedValue)
{
    // Arrange
    g_getcapValue = "1";
    TPMWork work;

    // Act
    const int set = work.ownerAuthStatus();
    g_getcapValue = "0";
    const int clear = work.ownerAuthStatus();

    // Assert
    EXPECT_EQ(set, 1);
    EXPECT_EQ(clear, 0);
}

// ownerAuthStatus 底层查询失败时返回 -2。
TEST_F(TPMWorkProTest, OwnerAuthStatus_QueryFailed_ReturnsMinusTwo)
{
    // Arrange
    g_getcapRet = -3;
    TPMWork work;

    // Act
    const int ret = work.ownerAuthStatus();

    // Assert
    EXPECT_EQ(ret, -2);
    EXPECT_NE(ret, 0);
}

/******************************************************************************/
/***************************** checkTPMLockout ********************************/
/******************************************************************************/

namespace {
QString g_procProgram;
QStringList g_procArgs;
bool g_waitStarted { true };
bool g_waitFinished { true };
int g_procExitCode { 0 };
QByteArray g_procOutput;
int g_lockoutStartCalls { 0 };
int g_lockoutKillCalls { 0 };
}   // namespace

class CheckTPMLockoutTest : public testing::Test
{
public:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        g_procProgram.clear();
        g_procArgs.clear();
        g_waitStarted = true;
        g_waitFinished = true;
        g_procExitCode = 0;
        g_procOutput.clear();
        g_lockoutStartCalls = 0;
        g_lockoutKillCalls = 0;

        stub.set_lamda(static_cast<void (QProcess::*)(const QString &, const QStringList &,
                                                      QIODevice::OpenMode)>(&QProcess::start),
                       [](QProcess *, const QString &program, const QStringList &arguments,
                          QIODevice::OpenMode) {
            ++g_lockoutStartCalls;
            g_procProgram = program;
            g_procArgs = arguments;
        });
        stub.set_lamda(&QProcess::waitForStarted, [](QProcess *, int) -> bool {
            return g_waitStarted;
        });
        stub.set_lamda(&QProcess::waitForFinished, [](QProcess *, int) -> bool {
            return g_waitFinished;
        });
        stub.set_lamda(&QProcess::exitCode, [](const QProcess *) -> int {
            return g_procExitCode;
        });
        stub.set_lamda(&QProcess::readAllStandardOutput,
                       [](QProcess *) -> QByteArray {
            return g_procOutput;
        });
        stub.set_lamda(&QProcess::kill, [](QProcess *) {
            ++g_lockoutKillCalls;
        });
    }

    void TearDown() override
    {
        stub.clear();
    }
};

// 正常解析：输出含 inLockout: 1/0 时分别返回 1/0。
TEST_F(CheckTPMLockoutTest, ParseInLockoutLine_ReturnsLockoutValue)
{
    // Arrange
    TPMWork work;
    g_procOutput = "TPM2_PT_PERMANENT:\n  ownerAuthSet: 1\n  inLockout: 1\n";

    // Act
    const int locked = work.checkTPMLockout();
    g_procOutput = "  inLockout: 0\n";
    const int unlocked = work.checkTPMLockout();

    // Assert
    EXPECT_EQ(locked, 1);
    EXPECT_EQ(unlocked, 0);
    EXPECT_EQ(g_lockoutStartCalls, 2);
}

// 启动参数必须为 tpm2_getcap properties-variable。
TEST_F(CheckTPMLockoutTest, UsesTpm2GetcapCommand)
{
    // Arrange
    TPMWork work;
    g_procOutput = "  inLockout: 0\n";

    // Act
    work.checkTPMLockout();

    // Assert
    EXPECT_EQ(g_procProgram, QString("tpm2_getcap"));
    ASSERT_EQ(g_procArgs.size(), 1);
    EXPECT_EQ(g_procArgs.first(), QString("properties-variable"));
}

// 输出中无 inLockout 行时返回 -2（未找到标记）。
TEST_F(CheckTPMLockoutTest, NoInLockoutLine_ReturnsMinusTwo)
{
    // Arrange
    TPMWork work;
    g_procOutput = "TPM2_PT_PERMANENT:\n  ownerAuthSet: 1\n";

    // Act
    const int ret = work.checkTPMLockout();

    // Assert
    EXPECT_EQ(ret, -2);
    EXPECT_EQ(g_lockoutKillCalls, 0);
}

// tpm2_getcap 启动失败 / 超时 / 非零退出码均返回 -1，超时场景需杀进程。
TEST_F(CheckTPMLockoutTest, ProcessFailures_ReturnMinusOne)
{
    // Arrange
    TPMWork work;

    // Act / Assert：启动失败
    g_waitStarted = false;
    EXPECT_EQ(work.checkTPMLockout(), -1);

    // Act / Assert：等待完成超时（需 kill 遗留进程）
    g_waitStarted = true;
    g_waitFinished = false;
    EXPECT_EQ(work.checkTPMLockout(), -1);
    EXPECT_EQ(g_lockoutKillCalls, 1);

    // Act / Assert：非零退出码
    g_waitFinished = true;
    g_procExitCode = 2;
    EXPECT_EQ(work.checkTPMLockout(), -1);
}

/******************************************************************************/
/**************************** TPMControlDBus 补充 *****************************/
/******************************************************************************/

namespace {
QStringList g_authActions2;
bool g_authAllowed2 { true };
int g_randomRet2 { 0 };
int g_decryptRet2 { 0 };
QString g_decryptPwd2 { "decrypted" };
int g_tpmAvailRet2 { 1 };
int g_tpmAvailCalls2 { 0 };
int g_lockoutRet2 { 0 };
int g_lockoutCalls2 { 0 };
int g_supportRet2 { 0 };
bool g_supportVal2 { true };
int g_ownerRet2 { 1 };
int g_ownerCalls2 { 0 };
}   // namespace

class TPMControlDBusCovTest : public testing::Test
{
public:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        g_authActions2.clear();
        g_authAllowed2 = true;
        g_randomRet2 = 0;
        g_decryptRet2 = 0;
        g_tpmAvailRet2 = 1;
        g_tpmAvailCalls2 = 0;
        g_lockoutRet2 = 0;
        g_lockoutCalls2 = 0;
        g_supportRet2 = 0;
        g_supportVal2 = true;
        g_ownerRet2 = 1;
        g_ownerCalls2 = 0;

        stub.set_lamda(QOverload<QDBusConnection::BusType, const QString &>::of(&QDBusConnection::connectToBus),
                       [](QDBusConnection::BusType, const QString &) -> QDBusConnection {
            return QDBusConnection(QString());
        });
        stub.set_lamda(&QDBusContext::message, [](const QDBusContext *) -> QDBusMessage {
            return QDBusMessage::createMethodCall("org.freedesktop.DBus", "/", "ut.iface", "utMethod");
        });
        stub.set_lamda(&QDBusMessage::service, [](const QDBusMessage *) -> QString {
            return QString("ut.sender.bus");
        });
        stub.set_lamda(&ServiceCommon::PolicyKitHelper::instance,
                       []() -> ServiceCommon::PolicyKitHelper * {
            static ServiceCommon::PolicyKitHelper fakeHelper;
            return &fakeHelper;
        });
        stub.set_lamda(&ServiceCommon::PolicyKitHelper::checkAuthorization,
                       [](ServiceCommon::PolicyKitHelper *, const QString &actionId, const QString &) -> bool {
            g_authActions2.append(actionId);
            return g_authAllowed2;
        });
        stub.set_lamda(&TPMWork::getRandom, [](TPMWork *, int, QString *output) -> int {
            if (output)
                *output = "rnd";
            return g_randomRet2;
        });
        stub.set_lamda(&TPMWork::decrypt, [](TPMWork *, const QVariantMap &, QString *pwd) -> int {
            if (pwd)
                *pwd = g_decryptPwd2;
            return g_decryptRet2;
        });
        stub.set_lamda(&TPMWork::isTPMAvailable, [](TPMWork *) -> int {
            ++g_tpmAvailCalls2;
            return g_tpmAvailRet2;
        });
        stub.set_lamda(&TPMWork::checkTPMLockout, [](TPMWork *) -> int {
            ++g_lockoutCalls2;
            return g_lockoutRet2;
        });
        stub.set_lamda(&TPMWork::isSupportAlgo,
                       [](TPMWork *, const QString &algo, bool *support) -> int {
            if (support)
                *support = g_supportVal2 && algo == "sha256";
            return g_supportRet2;
        });
        stub.set_lamda(&TPMWork::ownerAuthStatus, [](TPMWork *) -> int {
            ++g_ownerCalls2;
            return g_ownerRet2;
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

// Encrypt 收到无效 fd 时必须返回 kFdReadFailed。
TEST_F(TPMControlDBusCovTest, Encrypt_InvalidFd_ReturnsFdReadFailed)
{
    // Arrange
    TPMControlDBus dbus("ut-tpm-cov-enc", nullptr);
    QDBusUnixFileDescriptor invalid;

    // Act
    const int ret = dbus.Encrypt(invalid);

    // Assert
    EXPECT_EQ(ret, kFdReadFailed);
    EXPECT_EQ(g_authActions2.size(), 1) << "auth must still run before fd parsing";
}

// Decrypt 收到无效 fd 时返回 kFdReadFailed 且必须提供回退 fd。
TEST_F(TPMControlDBusCovTest, Decrypt_InvalidFd_ReturnsFdReadFailedWithFallback)
{
    // Arrange
    TPMControlDBus dbus("ut-tpm-cov-dec", nullptr);
    QDBusUnixFileDescriptor invalid;
    QDBusUnixFileDescriptor passwordFd;

    // Act
    const int ret = dbus.Decrypt(invalid, passwordFd);

    // Assert
    EXPECT_EQ(ret, kFdReadFailed);
    EXPECT_TRUE(passwordFd.isValid()) << "fallback fd must be provided on read failure";
}

// Decrypt 底层失败时透传错误码并提供回退 fd。
TEST_F(TPMControlDBusCovTest, Decrypt_LibraryFailed_PropagatesCodeWithFallback)
{
    // Arrange
    g_decryptRet2 = -7;
    TPMControlDBus dbus("ut-tpm-cov-dec2", nullptr);
    QDBusUnixFileDescriptor paramsFd = makeParamsFd({
        { PropertyKey::kEncryptType, kTpmAndPin },
        { PropertyKey::kDirPath, "/tmp/ut-tpm-cov" }
    });
    ASSERT_TRUE(paramsFd.isValid());
    QDBusUnixFileDescriptor passwordFd;

    // Act
    const int ret = dbus.Decrypt(paramsFd, passwordFd);

    // Assert
    EXPECT_EQ(ret, -7);
    EXPECT_TRUE(passwordFd.isValid()) << "fallback fd must be provided on decrypt failure";
}

// GetRandom 底层失败时透传错误码并提供回退 fd。
TEST_F(TPMControlDBusCovTest, GetRandom_LibraryFailed_PropagatesCodeWithFallback)
{
    // Arrange
    g_randomRet2 = -8;
    TPMControlDBus dbus("ut-tpm-cov-rnd", nullptr);
    QDBusUnixFileDescriptor randomFd;

    // Act
    const int ret = dbus.GetRandom(16, randomFd);

    // Assert
    EXPECT_EQ(ret, -8);
    EXPECT_TRUE(randomFd.isValid()) << "fallback fd must be provided on getRandom failure";
}

// parseCredentialsFromFd：空管道与非法数据流必须解析失败。
TEST_F(TPMControlDBusCovTest, ParseCredentialsFromFd_EmptyPipeAndGarbage_Fail)
{
    // Arrange
    TPMControlDBus dbus("ut-tpm-cov-parse", nullptr);
    QVariantMap args;

    // Act：空管道（EOF 立即到达）
    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);
    close(pipefd[1]);
    QDBusUnixFileDescriptor emptyFd(pipefd[0]);
    close(pipefd[0]);
    const bool emptyRet = dbus.parseCredentialsFromFd(emptyFd, &args);

    // Act：非序列化数据
    int pipefd2[2];
    ASSERT_EQ(pipe(pipefd2), 0);
    ASSERT_EQ(write(pipefd2[1], "not-a-qdatastream-payload", 26), static_cast<ssize_t>(26));
    close(pipefd2[1]);
    QDBusUnixFileDescriptor garbageFd(pipefd2[0]);
    close(pipefd2[0]);
    const bool garbageRet = dbus.parseCredentialsFromFd(garbageFd, &args);

    // Assert
    EXPECT_FALSE(emptyRet);
    EXPECT_FALSE(garbageRet);
}

// parseCredentialsFromFd：合法 QVariantMap 数据流必须解析成功。
TEST_F(TPMControlDBusCovTest, ParseCredentialsFromFd_ValidStream_Succeeds)
{
    // Arrange
    TPMControlDBus dbus("ut-tpm-cov-parse2", nullptr);
    const QVariantMap sent { { PropertyKey::kPinCode, QString("1234") },
                             { PropertyKey::kDirPath, QString("/tmp/ut-x") } };
    QDBusUnixFileDescriptor fd = makeParamsFd(sent);
    ASSERT_TRUE(fd.isValid());
    QVariantMap parsed;

    // Act
    const bool ok = dbus.parseCredentialsFromFd(fd, &parsed);

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(parsed.value(PropertyKey::kPinCode).toString(), QString("1234"));
    EXPECT_EQ(parsed.value(PropertyKey::kDirPath).toString(), QString("/tmp/ut-x"));
}

// sendDataViaFd：写入数据后必须得到携带全部内容的可读 fd。
TEST_F(TPMControlDBusCovTest, SendDataViaFd_ProducesReadableFdWithPayload)
{
    // Arrange
    TPMControlDBus dbus("ut-tpm-cov-send", nullptr);
    const QByteArray payload { "ut-payload-42" };
    QDBusUnixFileDescriptor fd;

    // Act
    const bool ok = dbus.sendDataViaFd(payload, fd);

    // Assert
    EXPECT_TRUE(ok);
    ASSERT_TRUE(fd.isValid());
    char buf[64] { 0 };
    const ssize_t n = ::read(fd.fileDescriptor(), buf, sizeof(buf) - 1);
    EXPECT_EQ(n, static_cast<ssize_t>(payload.size()));
    EXPECT_EQ(QByteArray(buf), payload);
}

// IsTPMAvailable：鉴权通过透传底层结果，鉴权失败返回 kAuthFailed 且不触达底层。
TEST_F(TPMControlDBusCovTest, IsTPMAvailable_AuthPassthroughAndFailure)
{
    // Arrange
    TPMControlDBus dbus("ut-tpm-cov-avail", nullptr);
    g_tpmAvailRet2 = 1;

    // Act
    const int yes = dbus.IsTPMAvailable();
    g_authAllowed2 = false;
    const int denied = dbus.IsTPMAvailable();

    // Assert
    EXPECT_EQ(yes, 1);
    EXPECT_EQ(denied, kAuthFailed);
    EXPECT_EQ(g_tpmAvailCalls2, 1) << "auth failure must not reach the library";
    ASSERT_EQ(g_authActions2.size(), 2);
}

// CheckTPMLockout：鉴权通过透传 lockout 查询结果，鉴权失败返回 kAuthFailed。
TEST_F(TPMControlDBusCovTest, CheckTPMLockout_AuthPassthroughAndFailure)
{
    // Arrange
    TPMControlDBus dbus("ut-tpm-cov-lockout", nullptr);
    g_lockoutRet2 = 1;

    // Act
    const int locked = dbus.CheckTPMLockout();
    g_authAllowed2 = false;
    const int denied = dbus.CheckTPMLockout();

    // Assert
    EXPECT_EQ(locked, 1);
    EXPECT_EQ(denied, kAuthFailed);
    EXPECT_EQ(g_lockoutCalls2, 1);
}

// IsSupportAlgo：鉴权通过透传算法支持结果，鉴权失败返回 kAuthFailed。
TEST_F(TPMControlDBusCovTest, IsSupportAlgo_AuthPassthroughAndFailure)
{
    // Arrange
    TPMControlDBus dbus("ut-tpm-cov-algo", nullptr);
    bool support = false;

    // Act
    const int ret = dbus.IsSupportAlgo("sha256", support);
    g_authAllowed2 = false;
    bool support2 = true;
    const int denied = dbus.IsSupportAlgo("sha256", support2);

    // Assert
    EXPECT_EQ(ret, kNoError);
    EXPECT_TRUE(support);
    EXPECT_EQ(denied, kAuthFailed);
    EXPECT_TRUE(support2) << "output must be untouched on auth failure";
}

// OwnerAuthStatus：鉴权通过透传查询值，鉴权失败返回 kAuthFailed。
TEST_F(TPMControlDBusCovTest, OwnerAuthStatus_AuthPassthroughAndFailure)
{
    // Arrange
    TPMControlDBus dbus("ut-tpm-cov-owner", nullptr);
    g_ownerRet2 = 1;

    // Act
    const int set = dbus.OwnerAuthStatus();
    g_authAllowed2 = false;
    const int denied = dbus.OwnerAuthStatus();

    // Assert
    EXPECT_EQ(set, 1);
    EXPECT_EQ(denied, kAuthFailed);
    EXPECT_EQ(g_ownerCalls2, 1);
}

// GetRandom 成功路径：底层随机串经 fd 回传，返回 kNoError。
TEST_F(TPMControlDBusCovTest, GetRandom_Success_SendsDataViaFd)
{
    // Arrange
    TPMControlDBus dbus("ut-tpm-cov-rnd3", nullptr);
    QDBusUnixFileDescriptor fd;

    // Act
    const int ret = dbus.GetRandom(16, fd);

    // Assert
    EXPECT_EQ(ret, kNoError);
    ASSERT_TRUE(fd.isValid());
    char buf[32] { 0 };
    const ssize_t n = ::read(fd.fileDescriptor(), buf, sizeof(buf) - 1);
    EXPECT_EQ(QByteArray(buf, n), QByteArray("rnd"));
}

/******************************************************************************/
/***************************** plugin.cpp 入口 ********************************/
/******************************************************************************/

// 服务实现源码在 pms TU 中，plugin.cpp 本 TU 单独 unity-include（无日志类别注册冲突）。
#include "services/tpmcontrol/plugin.cpp"

class TPMControlPluginTest : public testing::Test
{
public:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        stub.set_lamda(QOverload<QDBusConnection::BusType, const QString &>::of(&QDBusConnection::connectToBus),
                       [](QDBusConnection::BusType, const QString &) -> QDBusConnection {
            return QDBusConnection(QString());
        });
        stub.set_lamda(&QLibrary::load, [](QLibrary *) -> bool { return false; });
    }

    void TearDown() override
    {
        stub.clear();
    }
};

// DSMRegister/DSMUnRegister 创建并销毁 DBus 服务对象。
TEST_F(TPMControlPluginTest, DsmRegisterAndUnregister_Succeed)
{
    // Arrange
    // （SetUp 中已屏蔽系统总线与 libutpm2 依赖）

    // Act
    const int regRet = DSMRegister("ut-tpmcontrol-plugin", nullptr);
    const int unregRet = DSMUnRegister("ut-tpmcontrol-plugin", nullptr);

    // Assert
    EXPECT_EQ(regRet, 0);
    EXPECT_EQ(unregRet, 0);
}
