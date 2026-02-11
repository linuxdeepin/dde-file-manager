// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMPLUGIN_VAULT_GLOBAL_H
#define DFMPLUGIN_VAULT_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>

#define DPVAULT_NAMESPACE dfmplugin_vault

#define DPVAULT_BEGIN_NAMESPACE namespace dfmplugin_vault {
#define DPVAULT_END_NAMESPACE }
#define DPVAULT_USE_NAMESPACE using namespace dfmplugin_vault;

namespace dfmplugin_vault {
DFM_LOG_USE_CATEGORY(DPVAULT_NAMESPACE)

#define AddATTag(widget, name) \
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName", widget, name)
enum VaultState {
    kUnknow = 0,
    kNotExisted,
    kEncrypted,
    kUnlocked,
    kUnderProcess,
    kBroken,
    kNotAvailable
};

enum VaultPolicyState {
    kUnkonw = 0,
    kNotEnable,
    kEnable
};

enum class ErrorCode : int {
    //! 成功
    kSuccess = 0,

    //! 发生的错误没有与之相关的错误代码
    kUnspecifiedError = 1,

    //! 命令行参数无效.
    kInvalidArguments = 10,

    //! 无法加载配置文件.密码可能不正确
    kWrongPassword = 11,

    //! 密码不能为空
    kEmptyPassword = 12,

    //! 对于此CryFS版本,文件系统格式太新了.请更新您的CryFS版本.
    kTooNewFilesystemFormat = 13,

    //! 对于此CryFS版本,文件系统格式过旧.与--allow-filesystem-upgrade一起运行以对其进行升级.
    kTooOldFilesystemFormat = 14,

    //! 文件系统使用的密码与使用--cipher参数在命令行上指定的密码不同.
    kWrongCipher = 15,

    //! 基本目录不存在或不可访问（即，不可读取或可写或非目录）
    kInaccessibleBaseDir = 16,

    //! 挂载目录不存在或不可访问（即无法读取或写入或不是目录）
    kInaccessibleMountDir = 17,

    //! 基本目录不能是安装目录的子目录
    kBaseDirInsideMountDir = 18,

    //! 文件系统出了点问题.
    kInvalidFilesystem = 19,

    //! 配置文件中的文件系统ID与我们上次从此basedir加载文件系统的时间不同.
    //! 这可能意味着攻击者用另一种文件系统替换了文件系统.
    //! 您可以传递--allow-replaced-filesystem选项以允许此操作.
    kFilesystemIdChanged = 20,

    //! 文件系统加密密钥与我们上次加载此文件系统的时间不同.
    //! 这可能意味着攻击者用另一种文件系统替换了文件系统.
    //! 您可以传递--allow-replaced-filesystem选项以允许此操作.
    kEncryptionKeyChanged = 21,

    //! 命令行选项和文件系统在是否应该将丢失的块视为完整性违规方面存在分歧.
    kFilesystemHasDifferentIntegritySetup = 22,

    //! 文件系统处于单客户端模式,只能在创建它的客户端中使用.
    kSingleClientFileSystem = 23,

    //! 先前运行的文件系统检测到完整性违规.
    //! 阻止访问以确保用户注意.
    //! 用户删除完整性状态文件后,将可以再次访问文件系统.
    kIntegrityViolationOnPreviousRun = 24,

    //! 检测到完整性违规,并卸载文件系统以确保用户注意.
    kIntegrityViolation = 25,

    //! 挂载点不为空
    kMountpointNotEmpty = 26,

    //! 挂载目录使用中
    kResourceBusy = 27,

    //! cryfs不存在
    kCryfsNotExist = 28,

    //! 挂载目录不存在
    kMountdirNotExist = 29,

    //! 挂载目录已加密
    kMountdirEncrypted = 30,

    //! 没有权限
    kPermissionDenied = 31,

    //! fusermount不存在
    kFusermountNotExist = 32,

    //! Cryfs创建的加密文件夹已存在
    kEncryptedExist = 33
};

//! 与cryfs同步，加密算法名称特殊处理，暂不遵循编码规范
enum class EncryptType : int {
    AES_256_GCM = 0,
    AES_256_CFB,
    AES_128_GCM,
    AES_128_CFB,
    TWOFISH_256_GCM,
    TWOFISH_256_CFB,
    TWOFISH_128_GCM,
    TWOFISH_128_CFB,
    SERPENT_256_GCM,
    SERPENT_256_CFB,
    SERPENT_128_GCM,
    SERPENT_128_CFB,
    CAST_256_GCM,
    CAST_256_CFB,
    MARS_256_GCM,
    MARS_256_CFB,
    MARS_128_GCM,
    MARS_128_CFB,
    SM4_128_ECB,
    SM4_128_CBC,
    SM4_128_CFB,
    SM4_128_OFB,
    SM4_128_CTR
};

namespace AcName {
inline constexpr char kAcSidebarVaultMenu[] { "sidebar_vaultitem_menu" };

inline constexpr char kAcLabelVaultStartTitle[] { "label_vault_start_title" };
inline constexpr char kAcLabelVaultStartContent[] { "lable_vault_start_content" };
inline constexpr char kAcLabelVaultStartImage[] { "label_vault_start_image" };
inline constexpr char kAcBtnVaultStartOk[] { "btn_vault_start_ok" };

inline constexpr char kAcLabelVaultSetUnlockTitle[] { "label_vault_setUnlcok_title" };
inline constexpr char kAcLabelVaultSetUnlcokMethod[] { "label_vault_setUnlock_method" };
inline constexpr char kAcComboVaultSetUnlockMethod[] { "combo_vault_setUnlock_method" };
inline constexpr char kAcLabelVaultSetUnlockPassword[] { "label_vault_setUnlock_password" };
inline constexpr char kAcEditVaultSetUnlockPassword[] { "edit_vault_setUnlock_password" };
inline constexpr char kAcLabelVaultSetUnlockRepeatPasswrod[] { "label_vault_setUnlcok_repeatPassword" };
inline constexpr char kAcEditVaultSetUnlockRepeatPassword[] { "edit_vault_setUnlock_repeatPassword" };
inline constexpr char kAcLabelVaultSetUnlockHint[] { "label_vault_setUnlock_hint" };
inline constexpr char kAcEditVaultSetUnlockHint[] { "edit_vault_setUnlock_hint" };
inline constexpr char kAcLabelVaultSetUnlockText[] { "label_vault_setUnlock_text" };
inline constexpr char kAcBtnVaultSetUnlockNext[] { "btn_vault_setUnlock_next" };

inline constexpr char kAcLabelVaultSaveKeyTitle[] { "label_vault_saveKey_Title" };
inline constexpr char kAcLabelVaultSaveKeyContent[] { "label_vault_saveKey_Content" };
inline constexpr char kAcRadioVaultSaveKeyDefault[] { "radio_vault_saveKey_default" };
inline constexpr char kAcLabelVaultSaveKeyDefaultMsg[] { "label_vault_savekey_defaultMsg" };
inline constexpr char kAcRadioVaultSaveKeyOther[] { "radio_vault_saveKey_Other" };
inline constexpr char kAcEditVaultSaveKeyPath[] { "edit_vault_saveKey_path" };
inline constexpr char kAcBtnVaultSaveKeyNext[] { "btn_vault_saveKey_next" };

inline constexpr char kAcLabelVaultFinishTitle[] { "label_vault_finish_title" };
inline constexpr char kAcLabelVaultFinishContent[] { "label_vault_finish_content" };
inline constexpr char kAcLabelVaultFinishVaultImage[] { "label_vault_finish_vaultImage" };
inline constexpr char kAcProgressVaultFinishProgress[] { "progress_vault_finish_progress" };
inline constexpr char kAcLabelVaultFinishProgressHint[] { "label_vault_finish_progressHint" };
inline constexpr char kAcLabelVaultFinishConfirmImage[] { "label_vault_finish_confirmImage" };
inline constexpr char kAcLabelVaultFinishConfirmHint[] { "label_vault_finish_confirmHint" };
inline constexpr char kAcBtnVaultFinishNext[] { "btn_vault_finish_next" };

inline constexpr char kAcEditVaultUnlockPassword[] { "edit_vault_unlock_password" };
inline constexpr char kAcBtnVaultUnlockHint[] { "btn_vault_unlock_hint" };
inline constexpr char kAcLabelVaultUnlockForget[] { "label_vault_unlock_forget" };

inline constexpr char kAcComboVaultRetrieveMethod[] { "combo_vault_retrieve_method" };
inline constexpr char kAcEditVaultRetrieveDefaultPath[] { "edit_vault_retrieve_defaultPath" };
inline constexpr char kAcEditVaultRetrieveOtherPath[] { "edit_vault_retrieve_otherPath" };

inline constexpr char kAcLabelVaultRemoveTitle[] { "label_vault_remove_title" };
inline constexpr char kAcLabelVaultRemoveContent[] { "label_vault_remove_content" };
inline constexpr char kAcEditVaultRemovePassword[] { "edit_vault_remove_password" };
inline constexpr char kAcBtnVaultRemovePasswordHint[] { "btn_vault_remove_passwordHint" };
}

inline constexpr char kDeamonServiceName[] { "org.deepin.Filemanager.AccessControlManager" };
#ifdef COMPILE_ON_V2X
inline constexpr char kAppSessionService[] { "org.deepin.dde.SessionManager1" };
inline constexpr char kAppSessionPath[] { "/org/deepin/dde/SessionManager1" };
#else
inline constexpr char kAppSessionService[] { "com.deepin.SessionManager" };
inline constexpr char kAppSessionPath[] { "/com/deepin/SessionManager" };
#endif
inline constexpr char kNetWorkDBusServiceName[] { "org.deepin.service.SystemNetwork" };
inline constexpr char kNetWorkDBusPath[] { "/org/deepin/service/SystemNetwork" };
inline constexpr char kNetWorkDBusInterfaces[] { "org.deepin.service.SystemNetwork" };

inline constexpr char kFileManagerDBusDaemonName[] { "org.deepin.Filemanager.Daemon" };
inline constexpr char kFileManagerVaultDBusPath[] { "/org/deepin/Filemanager/Daemon/VaultManager" };
inline constexpr char kFileManagerVaultDBusInterfaces[] { "org.deepin.Filemanager.Daemon.VaultManager" };

enum RemoveWidgetType : int {
    kPasswordWidget = 0,
    kRecoveryKeyWidget = 1,
    kRemoveProgressWidget = 2,
    kTpmPinWidget = 3,
    kNoneWidget = 4,

    kUserRemoveWidgetType = 100
};

enum class Connectivity {
    Unknownconnectivity = 0,
    Noconnectivity,
    Portal,
    Limited,
    Full   // 主机已连接到网络，并且似乎能够访问完整的Internet
};

enum EncryptMode {
    kKeyMode = 0,   // 密钥加密
    kTransparentMode   // 透明加密
};

// 加密信息
struct EncryptInfo
{
    EncryptMode mode;
    QString password;
    QString hint;
    QString keyPath;
};

struct Result
{
    bool result;
    QString message;
};

inline constexpr char kVaultDConfigName[] { "org.deepin.dde.file-manager.vault" };
}

#endif   // DFMPLUGIN_VAULT_GLOBAL_H
