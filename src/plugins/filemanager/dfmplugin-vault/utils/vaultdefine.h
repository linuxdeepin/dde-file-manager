/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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

#include "dfmplugin_vault_global.h"

#include <QString>
#include <QDir>

DPVAULT_BEGIN_NAMESPACE

constexpr char kVaultDecryptDirName[] { "vault_unlocked" };
constexpr char kVaultEncrypyDirName[] { "vault_encrypted" };

constexpr char kCryfsConfigFileName[] { "cryfs.config" };

constexpr char kPasswordFileName[] { "pbkdf2clipher" };
constexpr char kRSAPUBKeyFileName[] { "rsapubkey" };
constexpr char kRSACiphertextFileName[] { "rsaclipher" };
constexpr char kPasswordHintFileName[] { "passwordHint" };
constexpr char kVaultConfigFileName[] { "vaultConfig.ini" };

constexpr int kRandomSaltLength { 10 };   //! 随机盐的字节数
constexpr int kIteration { 1024 };   //! pbkdf2迭代次数
constexpr int kIterationTwo { 10000 };   //! pbkdf2迭代次数
constexpr int kPasswordCipherLength { 50 };   //! 密码密文长度
constexpr int kUserKeyLength { 32 };   //! 用户密钥长度
constexpr int kUserKeyInterceptIndex { 50 };   //! 用户密钥从公钥中截取的起始点索引

constexpr char kRootProxy[] { "pkexec deepin-vault-authenticateProxy" };

constexpr char kPolkitVaultCreate[] { "com.deepin.filemanager.daemon.VaultManager.Create" };
constexpr char kPolkitVaultRemove[] { "com.deepin.filemanager.daemon.VaultManager.Remove" };

constexpr int kBuffterMaxLine { 1024 };   //! shell命令输出每行最大的字符个数

const QString kVaultBasePath(QDir::homePath() + QString("/.config/Vault"));   //!! 获取保险箱创建的目录地址

const QString kVaultBasePathOld(QDir::homePath() + QString("/.local/share/applications"));   //!! 获取保险箱创建的旧目录地址

static const QString getCompressorPidShell(QString value)
{
    return QString("ps -xo pid,cmd | grep /usr/bin/deepin-compressor | grep ") + value + QString(" | grep -v grep | awk '{print $1}'");
}

//!! 保险箱时间配置文件
constexpr char kVaultTimeConfigFileSuffix[] { "/../dde-file-manager/vaultTimeConfig.json" };
constexpr char kVaultTimeConfigFile[] { "/../dde-file-manager/vaultTimeConfig" };

constexpr char kjsonGroupName[] { "VaultTime" };
constexpr char kjsonKeyInterviewItme[] { "InterviewTime" };
constexpr char kjsonKeyLockTime[] { "LockTime" };
constexpr char kjsonKeyCreateTime[] { "CreateTime" };

enum class VaultState : int {
    kNotExisted = 0,
    kEncrypted,
    kUnlocked,
    kUnderProcess,
    kBroken,
    kNotAvailable
};

//!! 保险箱当前页面标记
enum VaultPageMark {
    kUnknown,
    kCreateVaultPage,
    kCreateVaultPage1,
    kUnlockVaultPage,
    kRetrievePasswordPage,
    kDeletePage,
    kDeleteVaultPage,
    kCopyFilePage,
    kClipboardPage,
    kVaultPage
};

enum PageType : int {
    kUnlockPage = 0,
    kRecoverPage = 1,
    kRetrievePage = 2,
    kPasswordRecoverPage = 3
};
DPVAULT_END_NAMESPACE
#endif   //! VAULTGLOBALDEFINE_H
