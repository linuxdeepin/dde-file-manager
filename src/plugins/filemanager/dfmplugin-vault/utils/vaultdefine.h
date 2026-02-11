// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTGLOBALDEFINE_H
#define VAULTGLOBALDEFINE_H

#include "dfmplugin_vault_global.h"

#include <QString>
#include <QDir>

namespace dfmplugin_vault {

inline constexpr char kVaultDecryptDirName[] { "vault_unlocked" };
inline constexpr char kVaultEncrypyDirName[] { "vault_encrypted" };

inline constexpr char kCryfsConfigFileName[] { "cryfs.config" };

inline constexpr char kPasswordFileName[] { "pbkdf2clipher" };
inline constexpr char kRSAPUBKeyFileName[] { "rsapubkey" };
inline constexpr char kRSACiphertextFileName[] { "rsaclipher" };
inline constexpr char kPasswordHintFileName[] { "passwordHint" };
inline constexpr char kVaultConfigFileName[] { "vaultConfig.ini" };

//propertydailog and detaillview property change
inline constexpr char kFieldReplace[] { "kFieldReplace" };
inline constexpr char kFieldInsert[] { "kFieldInsert" };

//public
inline constexpr char kNotAll[] { "kNotAll" };
inline constexpr char kFileSize[] { "kFileSize" };
inline constexpr char kFileType[] { "kFileType" };

//propertydailog
inline constexpr char kFileCount[] { "kFileCount" };
inline constexpr char kFilePosition[] { "kFilePosition" };
inline constexpr char kFileCreateTime[] { "kFileCreateTime" };
inline constexpr char kFileAccessedTime[] { "kFileAccessedTime" };
inline constexpr char kFileModifiedTime[] { "kFileModifiedTime" };

//detailview
inline constexpr char kFileName[] { "kFileName" };
inline constexpr char kFileViewSize[] { "kFileViewSize" };
inline constexpr char kFileDuration[] { "kFileDuration" };
inline constexpr char kFileInterviewTime[] { "kFileInterviewTime" };
inline constexpr char kFileChangeTIme[] { "kFileChangeTIme" };

//property Filter
inline constexpr char kNotFilter[] { "kNotFilter" };
inline constexpr char kIconTitle[] { "kIconTitle" };
inline constexpr char kBasisInfo[] { "kBasisInfo" };
inline constexpr char kPermission[] { "kPermission" };
inline constexpr char kFileSizeFiled[] { "kFileSizeFiled" };
inline constexpr char kFileCountFiled[] { "kFileCountFiled" };
inline constexpr char kFileTypeFiled[] { "kFileTypeFiled" };
inline constexpr char kFilePositionFiled[] { "kFilePositionFiled" };
inline constexpr char kFileCreateTimeFiled[] { "kFileCreateTimeFiled" };
inline constexpr char kFileAccessedTimeFiled[] { "kFileAccessedTimeFiled" };
inline constexpr char kFileModifiedTimeFiled[] { "kFileModifiedTimeFiled" };

inline constexpr char kBasicView[] { "kBasicView" };
inline constexpr char kIconView[] { "kIconView" };
inline constexpr char kFileNameField[] { "kFileNameField" };
inline constexpr char kFileSizeField[] { "kFileSizeField" };
inline constexpr char kFileViewSizeField[] { "kFileViewSizeField" };
inline constexpr char kFileDurationField[] { "kFileDurationField" };
inline constexpr char kFileTypeField[] { "kFileTypeField" };
inline constexpr char kFileInterviewTimeField[] { "kFileInterviewTimeField" };
inline constexpr char kFileChangeTimeField[] { "kFileChangeTimeField" };

inline constexpr int kRandomSaltLength { 10 };   //! 随机盐的字节数
inline constexpr int kIteration { 1024 };   //! pbkdf2迭代次数
inline constexpr int kIterationTwo { 10000 };   //! pbkdf2迭代次数
inline constexpr int kPasswordCipherLength { 50 };   //! 密码密文长度
inline constexpr int kUserKeyLength { 32 };   //! 用户密钥长度
inline constexpr int kUserKeyInterceptIndex { 50 };   //! 用户密钥从公钥中截取的起始点索引

inline constexpr char kRootProxy[] { "pkexec deepin-vault-authenticateProxy" };

// NOTE: Not DBus! This is polkit policy (from com.deepin.filemanager.vault.policy)
inline constexpr char kPolkitVaultCreate[] { "com.deepin.filemanager.daemon.VaultManager.Create" };
inline constexpr char kPolkitVaultRemove[] { "com.deepin.filemanager.daemon.VaultManager.Remove" };
inline constexpr char kPolkitVaultRetrieve[] { "com.deepin.filemanager.vault.VerifyKey.RetrievePassword" };
inline constexpr int kBuffterMaxLine { 1024 };   //! shell命令输出每行最大的字符个数

inline const QString kVaultBasePath(QDir::homePath() + QString("/.config/Vault"));   //!! 获取保险箱创建的目录地址

inline const QString kVaultBasePathOld(QDir::homePath() + QString("/.local/share/applications"));   //!! 获取保险箱创建的旧目录地址

inline const QString getCompressorPidShell(QString value)
{
    return QString("ps -xo pid,cmd | grep /usr/bin/deepin-compressor | grep ") + value + QString(" | grep -v grep | awk '{print $1}'");
}

//!! 保险箱时间配置文件
inline constexpr char kVaultTimeConfigFileSuffix[] { "/../dde-file-manager/vaultTimeConfig.json" };
inline constexpr char kVaultTimeConfigFile[] { "/../dde-file-manager/vaultTimeConfig" };

inline constexpr char kjsonGroupName[] { "VaultTime" };
inline constexpr char kjsonKeyInterviewItme[] { "InterviewTime" };
inline constexpr char kjsonKeyLockTime[] { "LockTime" };
inline constexpr char kjsonKeyCreateTime[] { "CreateTime" };

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
}
#endif   //! VAULTGLOBALDEFINE_H
