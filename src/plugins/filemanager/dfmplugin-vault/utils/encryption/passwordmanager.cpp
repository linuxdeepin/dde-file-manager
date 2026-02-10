// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "passwordmanager.h"

#include "dfmplugin_vault_global.h"

#include <libcryptsetup.h>

#include <QFile>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

using namespace dfmplugin_vault;

#define CHECK_ERROR(cond, msg) do { \
    if (cond) { \
        fmCritical() << "Vault PasswordManager:" << msg; \
        if (cd) { \
            crypt_free(cd); \
            cd = Q_NULLPTR; \
        } \
        return -1; \
    } \
} while(0)

int PasswordManager::createLuksContainer(const char *path,
                                        const char *masterKey,
                                        size_t masterKeySize,
                                        const char *password,
                                        int &slotID)
{
    int ret = -1;
    crypt_device *cd = Q_NULLPTR;

    // 初始化 crypt_device
    ret = crypt_init(&cd, path);
    CHECK_ERROR(ret < 0, "Failed to initialize crypt_device");

    // 格式化为 LUKS2
    ret = crypt_format(cd, CRYPT_LUKS2, "aes", "xts-plain64", Q_NULLPTR, masterKey, masterKeySize, Q_NULLPTR);
    CHECK_ERROR(ret < 0, "Failed to format LUKS container");

    // 添加初始密码（使用主密钥添加密码）
    ret = crypt_keyslot_add_by_volume_key(cd, CRYPT_ANY_SLOT, masterKey, masterKeySize, password, strlen(password));
    CHECK_ERROR(ret < 0, "Failed to add initial password");
    slotID = ret;

    if (cd) {
        crypt_free(cd);
        cd = Q_NULLPTR;
    }

    return 0;
}

int PasswordManager::exportMasterKey(const char *path,
                                     const char *password,
                                     char *masterKey,
                                     size_t *masterKeySize)
{
    int ret = -1;
    crypt_device *cd = Q_NULLPTR;

    ret = crypt_init(&cd, path);
    CHECK_ERROR(ret < 0, "Failed to initialize crypt_device");

    ret = crypt_load(cd, CRYPT_LUKS2, Q_NULLPTR);
    CHECK_ERROR(ret < 0, "Failed to load LUKS container");

    int possibleSlots[] = { 0, 1, 2 };
    ret = -1;
    for (int i = 0; i < 3; i++) {
        ret = crypt_volume_key_get(cd, possibleSlots[i], masterKey, masterKeySize, password, strlen(password));
        if (ret >= 0) {
            break;  // 找到正确的槽，退出循环
        }
    }
    CHECK_ERROR(ret < 0, QString("Failed to export master key, error code: %1").arg(ret));

    if (cd) {
        crypt_free(cd);
        cd = Q_NULLPTR;
    }

    return 0;
}

int PasswordManager::exportMasterKeyByKeyslot(const char *path,
                                              const char *password,
                                              int keyslot,
                                              char *masterKey,
                                              size_t *masterKeySize)
{
    int ret = -1;
    crypt_device *cd = Q_NULLPTR;

    ret = crypt_init(&cd, path);
    CHECK_ERROR(ret < 0, "Failed to initialize crypt_device");

    ret = crypt_load(cd, CRYPT_LUKS2, Q_NULLPTR);
    CHECK_ERROR(ret < 0, "Failed to load LUKS container");

    ret = crypt_volume_key_get(cd, keyslot, masterKey, masterKeySize, password, strlen(password));
    CHECK_ERROR(ret < 0, QString("Failed to export master key from keyslot %1, error code: %2").arg(keyslot).arg(ret));

    if (cd) {
        crypt_free(cd);
        cd = Q_NULLPTR;
    }

    return 0;
}

int PasswordManager::addNewPassword(const char *path,
                                     const char *existingPassword,
                                     const char *newPassword,
                                     int &newKeySlotId)
{
    int ret = -1;
    crypt_device *cd = Q_NULLPTR;

    ret = crypt_init(&cd, path);
    CHECK_ERROR(ret < 0, "Failed to initialize crypt_device");

    ret = crypt_load(cd, CRYPT_LUKS2, Q_NULLPTR);
    CHECK_ERROR(ret < 0, "Failed to load LUKS container");

    // crypt_keyslot_add_by_passphrase 的 keyslot 参数是指新密码添加到哪个槽（可以指定具体槽或 CRYPT_ANY_SLOT）
    ret = crypt_keyslot_add_by_passphrase(cd,
                                          CRYPT_ANY_SLOT,
                                          existingPassword,
                                          strlen(existingPassword),
                                          newPassword,
                                          strlen(newPassword));
    CHECK_ERROR(ret < 0, "Failed to add new password");
    newKeySlotId = ret;

    if (cd) {
        crypt_free(cd);
        cd = Q_NULLPTR;
    }

    return 0;
}

int PasswordManager::addNewPasswordByKeyslot(const char *path,
                                              const char *existingPassword,
                                              int existingPasswordKeyslot,
                                              const char *newPassword,
                                              int &newKeySlotId)
{
    int ret = -1;
    crypt_device *cd = Q_NULLPTR;

    ret = crypt_init(&cd, path);
    CHECK_ERROR(ret < 0, "Failed to initialize crypt_device");

    ret = crypt_load(cd, CRYPT_LUKS2, Q_NULLPTR);
    CHECK_ERROR(ret < 0, "Failed to load LUKS container");

    char masterKey[64];
    size_t masterKeySize = 64;
    ret = crypt_volume_key_get(cd, existingPasswordKeyslot, masterKey, &masterKeySize,
                               existingPassword, strlen(existingPassword));
    CHECK_ERROR(ret < 0, "Failed to get master key from existing password keyslot");

    // 使用主密钥添加新密码
    ret = crypt_keyslot_add_by_volume_key(cd, CRYPT_ANY_SLOT, masterKey, masterKeySize,
                                          newPassword, strlen(newPassword));
    CHECK_ERROR(ret < 0, "Failed to add new password");
    newKeySlotId = ret;

    if (cd) {
        crypt_free(cd);
        cd = Q_NULLPTR;
    }

    return 0;
}

int PasswordManager::changePassword(const char *path,
                                    const char *oldPassword,
                                    const char *newPassword,
                                    int &newKeySlotId)
{
    int ret = -1;
    crypt_device *cd = Q_NULLPTR;

    ret = crypt_init(&cd, path);
    CHECK_ERROR(ret < 0, "Failed to initialize crypt_device");

    ret = crypt_load(cd, CRYPT_LUKS2, Q_NULLPTR);
    CHECK_ERROR(ret < 0, "Failed to load LUKS container");

    int possibleSlots[] = { 0, 2 };
    ret = -1;
    for (int i = 0; i < 2; i++) {
        crypt_keyslot_info keyslotStatus = crypt_keyslot_status(cd, possibleSlots[i]);
        if (keyslotStatus != CRYPT_SLOT_ACTIVE) {
            continue;
        }
        ret = crypt_keyslot_change_by_passphrase(cd, possibleSlots[i], possibleSlots[i],
                                                 oldPassword, strlen(oldPassword),
                                                 newPassword, strlen(newPassword));
        if (ret >= 0) {
            newKeySlotId = ret;
            break;
        }
    }
    CHECK_ERROR(ret < 0, "Failed to change password");

    if (cd) {
        crypt_free(cd);
        cd = Q_NULLPTR;
    }

    return 0;
}

int PasswordManager::deleteKeyslot(const char *path,
                                    int keyslot)
{
    int ret = -1;
    crypt_device *cd = Q_NULLPTR;

    ret = crypt_init(&cd, path);
    CHECK_ERROR(ret < 0, "Failed to initialize crypt_device");

    ret = crypt_load(cd, CRYPT_LUKS2, Q_NULLPTR);
    CHECK_ERROR(ret < 0, "Failed to load LUKS container");

    // 检查 keyslot 是否存在且激活
    crypt_keyslot_info keyslotStatus = crypt_keyslot_status(cd, keyslot);
    if (keyslotStatus != CRYPT_SLOT_ACTIVE) {
        // KeySlot 未激活（不存在），视为已删除，直接返回成功
        if (cd) {
            crypt_free(cd);
            cd = Q_NULLPTR;
        }
        return 0;
    }

    // 删除 keyslot
    ret = crypt_keyslot_destroy(cd, keyslot);
    CHECK_ERROR(ret < 0, "Failed to delete keyslot");

    if (cd) {
        crypt_free(cd);
        cd = Q_NULLPTR;
    }

    return 0;
}

int PasswordManager::findKeyslotByPassword(const char *path,
                                            const char *password,
                                            int &keyslotId)
{
    int ret = -1;
    crypt_device *cd = Q_NULLPTR;
    keyslotId = -1;

    ret = crypt_init(&cd, path);
    CHECK_ERROR(ret < 0, "Failed to initialize crypt_device");

    ret = crypt_load(cd, CRYPT_LUKS2, Q_NULLPTR);
    CHECK_ERROR(ret < 0, "Failed to load LUKS container");

    int possibleSlots[] = { 0, 1, 2 };
    ret = -1;
    for (int i = 0; i < 3; i++) {
        ret = crypt_activate_by_passphrase(cd, Q_NULLPTR, possibleSlots[i],
                                           password, strlen(password),
                                           0);
        if (ret >= 0) {
            keyslotId = possibleSlots[i];
            ret = 0;
            break;  // 找到正确的槽，退出循环
        }
    }
    if (ret < 0) {
        keyslotId = -1;
        ret = -1;
    }

    if (cd) {
        crypt_free(cd);
        cd = Q_NULLPTR;
    }

    return ret;
}

int PasswordManager::verifyPassword(const char *path,
                                    const char *password,
                                    bool &isRight)
{
    int ret = -1;
    crypt_device *cd = Q_NULLPTR;
    isRight = false;

    ret = crypt_init(&cd, path);
    CHECK_ERROR(ret < 0, "Failed to initialize crypt_device");

    ret = crypt_load(cd, CRYPT_LUKS2, Q_NULLPTR);
    CHECK_ERROR(ret < 0, "Failed to load LUKS container");

    int possibleSlots[] = { 0, 1, 2 };
    ret = -1;
    for (int i = 0; i < 3; i++) {
        ret = crypt_activate_by_passphrase(cd, Q_NULLPTR, possibleSlots[i],
                                           password, strlen(password),
                                           0);
        if (ret >= 0) {
            isRight = true;
            ret = 0;
            break;  // 找到正确的槽，退出循环
        }
    }
    if (ret < 0) {
        isRight = false;
        ret = 0;  // 验证失败但不返回错误，只设置 isRight = false
    }

    if (cd) {
        crypt_free(cd);
        cd = Q_NULLPTR;
    }

    return ret;
}

namespace {
// 从 /dev/urandom 读取原始随机字节的内部辅助函数
static int readRandomBytesFromDevice(char *output, size_t size)
{
    if (!output) {
        fmCritical() << "Vault PasswordManager: Output buffer cannot be null";
        return -1;
    }

    if (size == 0) {
        fmCritical() << "Vault PasswordManager: Size cannot be zero";
        return -1;
    }

    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        fmCritical() << "Vault PasswordManager: Failed to open /dev/urandom";
        return -1;
    }

    ssize_t bytesRead = read(fd, output, size);
    close(fd);

    if (bytesRead != (ssize_t)size) {
        fmCritical() << QString("Vault PasswordManager: Failed to read random bytes, expected %1 bytes, read %2 bytes").arg(size).arg(bytesRead);
        return -1;
    }

    return 0;
}

// 将随机字节映射到可显示字符集的内部辅助函数
static int mapBytesToDisplayableCharset(const char *randomBytes, size_t randomBytesSize,
                                        char *output, size_t outputSize)
{
    if (!randomBytes || !output) {
        fmCritical() << "Vault PasswordManager: Input or output buffer cannot be null";
        return -1;
    }

    if (randomBytesSize == 0) {
        fmCritical() << "Vault PasswordManager: Random bytes size cannot be zero";
        return -1;
    }

    // 使用更大的字符集增加熵值
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    const size_t charsetSize = strlen(charset);

    // 确保输出缓冲区足够大（随机字节数 + null终止符）
    if (outputSize < randomBytesSize + 1) {
        fmCritical() << QString("Vault PasswordManager: Output buffer size is insufficient, need %1 bytes").arg(randomBytesSize + 1);
        return -1;
    }

    // 将随机字节映射到字符集
    for (size_t i = 0; i < randomBytesSize; i++) {
        // 使用模运算确保索引在有效范围内
        output[i] = charset[static_cast<unsigned char>(randomBytes[i]) % charsetSize];
    }

    output[randomBytesSize] = '\0';   // 添加字符串终止符

    return 0;
}
}  // namespace

int PasswordManager::generateRandomBytes(char *output, size_t size)
{
    if (!output) {
        fmCritical() << "Vault PasswordManager: Output buffer cannot be null";
        return -1;
    }

    if (size == 0) {
        fmCritical() << "Vault PasswordManager: Size cannot be zero";
        return -1;
    }

    // 从设备读取原始随机字节
    char randomBytes[size];
    int ret = readRandomBytesFromDevice(randomBytes, size);
    if (ret != 0) {
        fmCritical() << "Vault PasswordManager: Failed to read random bytes from device";
        return -1;
    }

    // 映射到可显示字符集（输出为可显示的字符串，包含 null 终止符）
    // 注意：调用者需要确保 output 缓冲区至少有 size + 1 字节的空间
    ret = mapBytesToDisplayableCharset(randomBytes, size, output, size + 1);
    if (ret != 0) {
        fmCritical() << "Vault PasswordManager: Failed to map bytes to displayable charset";
        return -1;
    }

    return 0;
}

int PasswordManager::generateSecureRecoveryKey(char *output, size_t outputSize)
{
    if (!output) {
        fmCritical() << "Vault PasswordManager: Output buffer cannot be null";
        return -1;
    }

    // 确保输出缓冲区足够大（32字符 + null终止符）
    if (outputSize < 33) {
        fmCritical() << "Vault PasswordManager: Output buffer size is insufficient, at least 33 bytes required";
        return -1;
    }

    const int keyLength = 32;   // 生成32位恢复密钥

    // 从设备读取原始随机字节
    char randomBytes[keyLength];
    int ret = readRandomBytesFromDevice(randomBytes, keyLength);
    if (ret != 0) {
        fmCritical() << "Vault PasswordManager: Failed to read random bytes from device";
        return -1;
    }

    // 映射到可显示字符集
    ret = mapBytesToDisplayableCharset(randomBytes, keyLength, output, outputSize);
    if (ret != 0) {
        fmCritical() << "Vault PasswordManager: Failed to map bytes to displayable charset";
        return -1;
    }

    return 0;
}

int PasswordManager::createPasswordContainerFile(const char *path)
{
    const qint64 fileSize = 16 * 1024 * 1024;   // 16MB

    QFile file(QString::fromUtf8(path));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        fmCritical() << "Vault PasswordManager: Failed to open file:" << path << file.errorString();
        return -1;
    }

    int fd = file.handle();
    if (fd < 0 || posix_fallocate(fd, 0, fileSize) != 0) {
        fmCritical() << "Vault PasswordManager: Failed to allocate file space";
        file.close();
        return -1;
    }

    file.close();
    return 0;
}

