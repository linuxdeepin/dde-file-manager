/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef VAULTGLOBALDEFINE_H
#define VAULTGLOBALDEFINE_H

#include <QString>

#define CONFIG_DIR_NAME                 "vault"
#define VAULT_DECRYPT_DIR_NAME          "vault_unlocked"
#define VAULT_ENCRYPY_DIR_NAME          "vault_encrypted"

#define CRYFS_CONFIG_FILE_NAME          "cryfs.config"

#define PASSWORD_FILE_NAME              "pbkdf2clipher"
#define RSA_PUB_KEY_FILE_NAME           "rsapubkey"
#define RSA_CIPHERTEXT_FILE_NAME        "rsaclipher"
#define PASSWORD_HINT_FILE_NAME         "passwordHint"
#define VAULT_CONFIG_FILE_NAME          "vaultConfig.ini"


#define RANDOM_SALT_LENGTH              10              // 随机盐的字节数
#define ITERATION                       1024            // pbkdf2迭代次数
#define ITERATION_TWO                   10000           // pbkdf2迭代次数
#define PASSWORD_CIPHER_LENGTH          50              // 密码密文长度
#define USER_KEY_LENGTH                 32              // 用户密钥长度
#define USER_KEY_INTERCEPT_INDEX        50              // 用户密钥从公钥中截取的起始点索引

#define ROOT_PROXY                      "pkexec deepin-vault-authenticateProxy"

#define MAXLINE                         1024            // shell命令输出每行最大的字符个数

#define VAULT_BASE_PATH (QDir::homePath() + QString("/.config/Vault"))  //! 获取保险箱创建的目录地址

#define VAULT_BASE_PATH_OLD (QDir::homePath() + QString("/.local/share/applications"))  //! 获取保险箱创建的旧目录地址

#define GET_COMPRESSOR_PID_SHELL(x)     QString("ps -xo pid,cmd | grep /usr/bin/deepin-compressor | grep ")+x+QString(" | grep -v grep | awk '{print $1}'")

//! 保险箱时间配置文件
#define VAULT_TIME_CONFIG_FILE          QString("/../dde-file-manager/vaultTimeConfig.json")

enum EN_VaultState {
    NotExisted = 0,
    Encrypted,
    Unlocked,
    UnderProcess,
    Broken,
    NotAvailable
};

//! 保险箱当前页面标记
enum VaultPageMark {
    UNKNOWN,
    CREATEVAULTPAGE,
    CREATEVAULTPAGE1,
    UNLOCKVAULTPAGE,
    RETRIEVEPASSWORDPAGE,
    DELETEFILEPAGE,
    DELETEVAULTPAGE,
    COPYFILEPAGE,
    CLIPBOARDPAGE,
    VAULTPAGE
};

#endif // VAULTGLOBALDEFINE_H
