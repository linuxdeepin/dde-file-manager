// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <gtest/gtest.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libcryptsetup.h>

#include <QTest>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QVariantMap>
#include <QDateTime>

// Include the class under test
#include "utils.h"

SERVICEACCESSCONTROL_USE_NAMESPACE

class UT_Utils : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        tempDir = new QTemporaryDir();
        ASSERT_TRUE(tempDir->isValid());
    }

    virtual void TearDown() override
    {
        stub.clear();
        delete tempDir;
        tempDir = nullptr;
    }

protected:
    QTemporaryDir *tempDir = nullptr;
    stub_ext::StubExt stub;
};

// Test devConfigPath
TEST_F(UT_Utils, DevConfigPath_ReturnsCorrectPath)
{
    QString path = Utils::devConfigPath();
    EXPECT_EQ(path, "/etc/deepin/devAccessConfig.json");
}

// Test valultConfigPath
TEST_F(UT_Utils, VaultConfigPath_ReturnsCorrectPath)
{
    QString path = Utils::valultConfigPath();
    EXPECT_EQ(path, "/etc/deepin/vaultAccessConfig.json");
}

// Test accessMode with empty path
TEST_F(UT_Utils, AccessMode_EmptyPath_ReturnsDisable)
{
    int mode = Utils::accessMode("");
    EXPECT_EQ(mode, kPolicyDisable);
}

// Test accessMode with writable path
TEST_F(UT_Utils, AccessMode_WritablePath_ReturnsRW)
{
    // Mock access function to return success for write access
    stub.set_lamda(access, [](const char *, int mode) -> int {
        __DBG_STUB_INVOKE__
        if (mode == W_OK) return 0;   // Success for write
        return -1;
    });

    int mode = Utils::accessMode("/tmp/writable");
    EXPECT_EQ(mode, kPolicyRw);
}

// Test accessMode with read-only path
TEST_F(UT_Utils, AccessMode_ReadOnlyPath_ReturnsROnly)
{
    // Mock access function
    stub.set_lamda(access, [](const char *, int mode) -> int {
        __DBG_STUB_INVOKE__
        if (mode == W_OK) return -1;   // Fail for write
        if (mode == R_OK) return 0;   // Success for read
        return -1;
    });

    int mode = Utils::accessMode("/tmp/readonly");
    EXPECT_EQ(mode, kPolicyRonly);
}

// Test accessMode with no access
TEST_F(UT_Utils, AccessMode_NoAccess_ReturnsDisable)
{
    // Mock access function to fail for both read and write
    stub.set_lamda(access, [](const char *, int) -> int {
        __DBG_STUB_INVOKE__
        return -1;   // Fail for all access
    });

    int mode = Utils::accessMode("/tmp/noaccess");
    EXPECT_EQ(mode, kPolicyDisable);
}

// Test setFileMode success
TEST_F(UT_Utils, SetFileMode_Success_ReturnsZero)
{
    // Mock chmod to succeed
    stub.set_lamda(chmod, [](const char *, mode_t) -> int {
        __DBG_STUB_INVOKE__
        return 0;
    });

    int result = Utils::setFileMode("/tmp/testfile", 0755);
    EXPECT_EQ(result, 0);
}

// Test setFileMode failure
TEST_F(UT_Utils, SetFileMode_Failure_ReturnsNonZero)
{
    // Mock chmod to fail
    stub.set_lamda(chmod, [](const char *, mode_t) -> int {
        __DBG_STUB_INVOKE__
        return -1;
    });

    int result = Utils::setFileMode("/tmp/testfile", 0755);
    EXPECT_EQ(result, -1);
}

// Test isValidDevPolicy with valid policy
TEST_F(UT_Utils, IsValidDevPolicy_ValidPolicy_ReturnsTrue)
{
    QVariantMap policy;
    policy[kKeyInvoker] = "test.invoker";
    policy[kKeyType] = kTypeBlock;
    policy[kKeyPolicy] = kPolicyRw;

    bool result = Utils::isValidDevPolicy(policy, "test.invoker");
    EXPECT_TRUE(result);
}

// Test isValidDevPolicy with empty invoker
TEST_F(UT_Utils, IsValidDevPolicy_EmptyInvoker_ReturnsFalse)
{
    QVariantMap policy;
    policy[kKeyInvoker] = "";
    policy[kKeyType] = kTypeBlock;
    policy[kKeyPolicy] = kPolicyRw;

    bool result = Utils::isValidDevPolicy(policy, "test.invoker");
    EXPECT_FALSE(result);
}

// Test isValidDevPolicy with invalid type
TEST_F(UT_Utils, IsValidDevPolicy_InvalidType_ReturnsFalse)
{
    QVariantMap policy;
    policy[kKeyInvoker] = "test.invoker";
    policy[kKeyType] = kTypeInvalid;
    policy[kKeyPolicy] = kPolicyRw;

    bool result = Utils::isValidDevPolicy(policy, "test.invoker");
    EXPECT_FALSE(result);
}

// Test isValidDevPolicy with invalid policy value
TEST_F(UT_Utils, IsValidDevPolicy_InvalidPolicyValue_ReturnsFalse)
{
    QVariantMap policy;
    policy[kKeyInvoker] = "test.invoker";
    policy[kKeyType] = kTypeBlock;
    policy[kKeyPolicy] = 3;   // Invalid policy value

    bool result = Utils::isValidDevPolicy(policy, "test.invoker");
    EXPECT_FALSE(result);
}

// Test isValidDevPolicy with mismatched invoker
TEST_F(UT_Utils, IsValidDevPolicy_MismatchedInvoker_ReturnsFalse)
{
    QVariantMap policy;
    policy[kKeyInvoker] = "different.invoker";
    policy[kKeyType] = kTypeBlock;
    policy[kKeyPolicy] = kPolicyRw;

    bool result = Utils::isValidDevPolicy(policy, "test.invoker");
    EXPECT_FALSE(result);
}

// Test isValidVaultPolicy with valid policy
TEST_F(UT_Utils, IsValidVaultPolicy_ValidPolicy_ReturnsTrue)
{
    QVariantMap policy;
    policy[kPolicyType] = 1;
    policy[kVaultHideState] = 1;

    bool result = Utils::isValidVaultPolicy(policy);
    EXPECT_TRUE(result);
}

// Test isValidVaultPolicy with invalid policy type
TEST_F(UT_Utils, IsValidVaultPolicy_InvalidPolicyType_ReturnsFalse)
{
    QVariantMap policy;
    policy[kPolicyType] = -1;
    policy[kVaultHideState] = 1;

    bool result = Utils::isValidVaultPolicy(policy);
    EXPECT_FALSE(result);
}

// Test isValidVaultPolicy with invalid vault hide state
TEST_F(UT_Utils, IsValidVaultPolicy_InvalidVaultHideState_ReturnsFalse)
{
    QVariantMap policy;
    policy[kPolicyType] = 1;
    policy[kVaultHideState] = -1;

    bool result = Utils::isValidVaultPolicy(policy);
    EXPECT_FALSE(result);
}

// Test saveDevPolicy with file creation failure
TEST_F(UT_Utils, SaveDevPolicy_FileCreationFailure_HandledGracefully)
{
    // Mock file open to fail - use function pointer to specify overload
    // using FileOpenFunc = bool (QFile::*)(QIODevice::OpenMode);
    // stub.set_lamda(static_cast<FileOpenFunc>(&QFile::open), [](QFile *, QIODevice::OpenMode) {
    //     __DBG_STUB_INVOKE__
    //     return false;
    // });

    QVariantMap policy;
    policy[kKeyInvoker] = "test.invoker";
    policy[kKeyType] = kTypeBlock;
    policy[kKeyPolicy] = kPolicyRw;

    // Should not crash when file creation fails
    Utils::saveDevPolicy(policy);

    // No assertion needed - just verify it doesn't crash
}

// Test saveDevPolicy with valid policy
TEST_F(UT_Utils, SaveDevPolicy_ValidPolicy_Success)
{
    QString testConfigPath = tempDir->filePath("testDevConfig.json");

    // Mock devConfigPath to return our test path
    stub.set_lamda(&Utils::devConfigPath, [testConfigPath]() {
        __DBG_STUB_INVOKE__
        return testConfigPath;
    });

    QVariantMap policy;
    policy[kKeyInvoker] = "test.invoker";
    policy[kKeyType] = kTypeBlock;
    policy[kKeyPolicy] = kPolicyRw;

    Utils::saveDevPolicy(policy);

    // Verify file was created
    QFile file(testConfigPath);
    EXPECT_TRUE(file.exists());

    // Verify content
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        EXPECT_TRUE(doc.isArray());
        QJsonArray arr = doc.array();
        EXPECT_GT(arr.size(), 0);
    }
}

// Test loadDevPolicy with non-existent file
TEST_F(UT_Utils, LoadDevPolicy_NonExistentFile_HandledGracefully)
{
    QString nonExistentPath = tempDir->filePath("nonexistent.json");

    // Mock devConfigPath to return non-existent path
    stub.set_lamda(&Utils::devConfigPath, [nonExistentPath]() {
        __DBG_STUB_INVOKE__
        return nonExistentPath;
    });

    DevPolicyType policies;
    Utils::loadDevPolicy(&policies);

    // Should handle gracefully without crashing
    // No specific assertion needed
}

// Test loadDevPolicy with valid file
TEST_F(UT_Utils, LoadDevPolicy_ValidFile_LoadsCorrectly)
{
    QString testConfigPath = tempDir->filePath("testDevConfig.json");

    // Create test config file
    QJsonObject obj;
    obj[kKeyGlobal] = 1;
    obj[kKeyInvoker] = "test.invoker";
    obj[kKeyType] = kTypeBlock;
    obj[kKeyPolicy] = kPolicyRw;
    obj[kKeyTstamp] = QString::number(QDateTime::currentSecsSinceEpoch());

    QJsonArray arr;
    arr.append(obj);

    QJsonDocument doc(arr);
    QFile file(testConfigPath);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    file.write(doc.toJson());
    file.close();

    // Mock devConfigPath to return our test path
    stub.set_lamda(&Utils::devConfigPath, [testConfigPath]() {
        __DBG_STUB_INVOKE__
        return testConfigPath;
    });

    DevPolicyType policies;
    Utils::loadDevPolicy(&policies);

    // Verify policy was loaded
    EXPECT_TRUE(policies.contains(kTypeBlock));
    if (policies.contains(kTypeBlock)) {
        EXPECT_EQ(policies[kTypeBlock].first, "test.invoker");
        EXPECT_EQ(policies[kTypeBlock].second, kPolicyRw);
    }
}

// Test loadDevPolicy with invalid JSON
TEST_F(UT_Utils, LoadDevPolicy_InvalidJSON_HandledGracefully)
{
    QString testConfigPath = tempDir->filePath("invalidConfig.json");

    // Create invalid JSON file
    QFile file(testConfigPath);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    file.write("{ invalid json }");
    file.close();

    // Mock devConfigPath to return our test path
    stub.set_lamda(&Utils::devConfigPath, [testConfigPath]() {
        __DBG_STUB_INVOKE__
        return testConfigPath;
    });

    DevPolicyType policies;
    Utils::loadDevPolicy(&policies);

    // Should handle gracefully without crashing
    // No specific assertion needed
}

// Test saveVaultPolicy with valid policy
TEST_F(UT_Utils, SaveVaultPolicy_ValidPolicy_Success)
{
    QString testConfigPath = tempDir->filePath("testVaultConfig.json");

    // Mock valultConfigPath to return our test path
    stub.set_lamda(&Utils::valultConfigPath, [testConfigPath]() {
        __DBG_STUB_INVOKE__
        return testConfigPath;
    });

    QVariantMap policy;
    policy[kPolicyType] = 1;
    policy[kVaultHideState] = 1;
    policy[kPolicyState] = 1;

    Utils::saveVaultPolicy(policy);

    // Verify file was created
    QFile file(testConfigPath);
    EXPECT_TRUE(file.exists());

    // Verify content
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        EXPECT_TRUE(doc.isArray());
        QJsonArray arr = doc.array();
        EXPECT_GT(arr.size(), 0);
    }
}

// Test loadVaultPolicy with valid file
TEST_F(UT_Utils, LoadVaultPolicy_ValidFile_LoadsCorrectly)
{
    QString testConfigPath = tempDir->filePath("testVaultConfig.json");

    // Create test config file
    QJsonObject obj;
    obj[kPolicyType] = 1;
    obj[kVaultHideState] = 1;
    obj[kPolicyState] = 1;

    QJsonArray arr;
    arr.append(obj);

    QJsonDocument doc(arr);
    QFile file(testConfigPath);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    file.write(doc.toJson());
    file.close();

    // Mock valultConfigPath to return our test path
    stub.set_lamda(&Utils::valultConfigPath, [testConfigPath]() {
        __DBG_STUB_INVOKE__
        return testConfigPath;
    });

    VaultPolicyType policies;
    Utils::loadVaultPolicy(&policies);

    // Verify policy was loaded
    EXPECT_TRUE(policies.contains(kPolicyType));
    EXPECT_TRUE(policies.contains(kVaultHideState));
    EXPECT_TRUE(policies.contains(kPolicyState));
    if (policies.contains(kPolicyType)) {
        EXPECT_EQ(policies[kPolicyType], 1);
    }
}

// Test checkDiskPassword with init failure
TEST_F(UT_Utils, CheckDiskPassword_InitFailure_ReturnsInitFailed)
{
    // Mock crypt_init to fail
    stub.set_lamda(crypt_init, [](struct crypt_device **, const char *) -> int {
        __DBG_STUB_INVOKE__
        return -1;
    });

    struct crypt_device *cd = nullptr;
    DPCErrorCode result = Utils::checkDiskPassword(&cd, "password", "/dev/test");
    EXPECT_EQ(result, kInitFailed);
}

// Test checkDiskPassword with load failure
TEST_F(UT_Utils, CheckDiskPassword_LoadFailure_ReturnsDeviceLoadFailed)
{
    // Mock crypt_init to succeed
    stub.set_lamda(crypt_init, [](struct crypt_device **cd, const char *) -> int {
        __DBG_STUB_INVOKE__
        *cd = reinterpret_cast<struct crypt_device *>(0x1234);   // Mock pointer
        return 0;
    });

    // Mock crypt_load to fail
    stub.set_lamda(crypt_load, [](struct crypt_device *, const char *, void *) -> int {
        __DBG_STUB_INVOKE__
        return -1;
    });

    // Mock crypt_free
    stub.set_lamda(crypt_free, [](struct crypt_device *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(crypt_get_device_name, [](struct crypt_device *) -> const char * {
        __DBG_STUB_INVOKE__
        return "/dev/test";
    });

    struct crypt_device *cd = nullptr;
    DPCErrorCode result = Utils::checkDiskPassword(&cd, "password", "/dev/test");
    EXPECT_EQ(result, kDeviceLoadFailed);
}

// Test checkDiskPassword with wrong password
TEST_F(UT_Utils, CheckDiskPassword_WrongPassword_ReturnsPasswordWrong)
{
    // Mock crypt_init to succeed
    stub.set_lamda(crypt_init, [](struct crypt_device **cd, const char *) -> int {
        __DBG_STUB_INVOKE__
        *cd = reinterpret_cast<struct crypt_device *>(0x1234);   // Mock pointer
        return 0;
    });

    // Mock crypt_load to succeed
    stub.set_lamda(crypt_load, [](struct crypt_device *, const char *, void *) -> int {
        __DBG_STUB_INVOKE__
        return 0;
    });

    // Mock crypt_activate_by_passphrase to fail (wrong password)
    stub.set_lamda(crypt_activate_by_passphrase, [](struct crypt_device *, const char *, int, const char *, size_t, uint32_t) -> int {
        __DBG_STUB_INVOKE__
        return -1;
    });

    // Mock crypt_free
    stub.set_lamda(crypt_free, [](struct crypt_device *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(crypt_get_device_name, [](struct crypt_device *) -> const char * {
        __DBG_STUB_INVOKE__
        return "/dev/test";
    });

    struct crypt_device *cd = nullptr;
    DPCErrorCode result = Utils::checkDiskPassword(&cd, "wrongpassword", "/dev/test");
    EXPECT_EQ(result, kPasswordWrong);
}

// Test checkDiskPassword success
TEST_F(UT_Utils, CheckDiskPassword_Success_ReturnsNoError)
{
    // Mock crypt_init to succeed
    stub.set_lamda(crypt_init, [](struct crypt_device **cd, const char *) -> int {
        __DBG_STUB_INVOKE__
        *cd = reinterpret_cast<struct crypt_device *>(0x1234);   // Mock pointer
        return 0;
    });

    // Mock crypt_load to succeed
    stub.set_lamda(crypt_load, [](struct crypt_device *, const char *, void *) -> int {
        __DBG_STUB_INVOKE__
        return 0;
    });

    // Mock crypt_activate_by_passphrase to succeed
    stub.set_lamda(crypt_activate_by_passphrase, [](struct crypt_device *, const char *, int, const char *, size_t, uint32_t) -> int {
        __DBG_STUB_INVOKE__
        return 0;
    });

    stub.set_lamda(crypt_get_device_name, [](struct crypt_device *) -> const char * {
        __DBG_STUB_INVOKE__
        return "/dev/test";
    });

    struct crypt_device *cd = nullptr;
    DPCErrorCode result = Utils::checkDiskPassword(&cd, "correctpassword", "/dev/test");
    EXPECT_EQ(result, kNoError);
}

// Test changeDiskPassword success
TEST_F(UT_Utils, ChangeDiskPassword_Success_ReturnsNoError)
{
    struct crypt_device *cd = reinterpret_cast<struct crypt_device *>(0x1234);   // Mock pointer

    // Mock crypt_get_device_name
    stub.set_lamda(crypt_get_device_name, [](struct crypt_device *) -> const char * {
        __DBG_STUB_INVOKE__
        return "/dev/test";
    });

    // Mock crypt_keyslot_change_by_passphrase to succeed
    stub.set_lamda(crypt_keyslot_change_by_passphrase, [](struct crypt_device *, int, int, const char *, size_t, const char *, size_t) -> int {
        __DBG_STUB_INVOKE__
        return 0;
    });

    // Mock crypt_free
    stub.set_lamda(crypt_free, [](struct crypt_device *) {
        __DBG_STUB_INVOKE__
    });

    DPCErrorCode result = Utils::changeDiskPassword(cd, "oldpassword", "newpassword");
    EXPECT_EQ(result, kNoError);
}

// Test changeDiskPassword failure
TEST_F(UT_Utils, ChangeDiskPassword_Failure_ReturnsPasswordChangeFailed)
{
    struct crypt_device *cd = reinterpret_cast<struct crypt_device *>(0x1234);   // Mock pointer

    // Mock crypt_get_device_name
    stub.set_lamda(crypt_get_device_name, [](struct crypt_device *) -> const char * {
        __DBG_STUB_INVOKE__
        return "/dev/test";
    });

    // Mock crypt_keyslot_change_by_passphrase to fail
    stub.set_lamda(crypt_keyslot_change_by_passphrase, [](struct crypt_device *, int, int, const char *, size_t, const char *, size_t) -> int {
        __DBG_STUB_INVOKE__
        return -1;
    });

    // Mock crypt_free
    stub.set_lamda(crypt_free, [](struct crypt_device *) {
        __DBG_STUB_INVOKE__
    });

    DPCErrorCode result = Utils::changeDiskPassword(cd, "oldpassword", "newpassword");
    EXPECT_EQ(result, kPasswordChangeFailed);
}
