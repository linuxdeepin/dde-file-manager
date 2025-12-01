// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "passwordmanager.h"

#include "dfmplugin_vault_global.h"

#include <libcryptsetup.h>

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

    // 使用更大的字符集增加熵值
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    const size_t charsetSize = strlen(charset);
    const int keyLength = 32;   // 生成32位恢复密钥

    // 使用密码学安全的随机数生成器
    char randomBytes[keyLength];
    int ret = generateRandomBytes(randomBytes, keyLength);
    if (ret != 0) {
        fmCritical() << "Vault PasswordManager: Failed to generate random bytes";
        return -1;
    }

    // 将随机字节映射到字符集
    for (int i = 0; i < keyLength; i++) {
        // 使用模运算确保索引在有效范围内
        output[i] = charset[randomBytes[i] % charsetSize];
    }

    output[keyLength] = '\0';   // 添加字符串终止符

    return 0;
}

int PasswordManager::createPasswordContainerFile(const char *path)
{
    QString command = QString("dd if=/dev/zero of=%1 bs=1M count=2 2>/dev/null").arg(path);

    int result = system(command.toStdString().c_str());
    if (result != 0) {
        fmCritical() << "Vault PasswordManager: Failed to create password container file";
        return -1;
    }

    return 0;
}

