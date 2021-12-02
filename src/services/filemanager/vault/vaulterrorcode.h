/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
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

#ifndef VAULTERRORCODE_H
#define VAULTERRORCODE_H

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
    MARS_128_CFB
};

#endif   // VAULTERRORCODE_H
