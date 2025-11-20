// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PASSWORDMANAGER_H
#define PASSWORDMANAGER_H

#include "dfmplugin_vault_global.h"

#include <cstddef>

namespace dfmplugin_vault {
class PasswordManager
{
public:
    // 创建LUKS容器
    static int createLuksContainer(const char *path,
                                   const char *masterKey,
                                   size_t masterKeySize,
                                   const char *password,
                                   int &slotID);

    // 导出主密钥
    static int exportMasterKey(const char *path,
                               const char *password,
                               char *masterKey,
                               size_t *masterKeySize);

    // 导出主密钥（根据指定的槽号）
    static int exportMasterKeyByKeyslot(const char *path,
                                        const char *password,
                                        int keyslot,
                                        char *masterKey,
                                        size_t *masterKeySize);

    // 添加新密码
    static int addNewPassword(const char *path,
                              const char *existingPassword,
                              const char *newPassword,
                              int &newKeySlotId);

    // 添加新密码（根据已知的 existingPassword 槽号）
    static int addNewPasswordByKeyslot(const char *path,
                                       const char *existingPassword,
                                       int existingPasswordKeyslot,
                                       const char *newPassword,
                                       int &newKeySlotId);

    // 修改密码
    static int changePassword(const char *path,
                              const char *oldPassword,
                              const char *newPassword,
                              int &newKeySlotId);

    // 删除 keyslot
    static int deleteKeyslot(const char *path,
                             int keyslot);

    // 查找密码对应的 keyslot ID
    static int findKeyslotByPassword(const char *path,
                                      const char *password,
                                      int &keyslotId);

    // 验证密码是否正确
    static int verifyPassword(const char *path,
                              const char *password,
                              bool &isRight);

    // 随机字节生成
    static int generateRandomBytes(char *output, size_t size);

    // 生成安全的随机恢复密钥
    static int generateSecureRecoveryKey(char *output, size_t outputSize);

    // 创建密码容器文件
    static int createPasswordContainerFile(const char *path);
};
}

#endif   // PASSWORDMANAGER_H

