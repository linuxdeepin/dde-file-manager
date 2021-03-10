/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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


#ifndef CRYFSERRORCODE_H
#define CRYFSERRORCODE_H

enum class ErrorCode : int
{
  //! 成功
  Success = 0,

  //! 发生的错误没有与之相关的错误代码
  UnspecifiedError = 1,

  //! 命令行参数无效.
  InvalidArguments = 10,

  //! 无法加载配置文件.密码可能不正确
  WrongPassword = 11,

  //! 密码不能为空
  EmptyPassword = 12,

  //! 对于此CryFS版本,文件系统格式太新了.请更新您的CryFS版本.
  TooNewFilesystemFormat = 13,

  //! 对于此CryFS版本,文件系统格式过旧.与--allow-filesystem-upgrade一起运行以对其进行升级.
  TooOldFilesystemFormat = 14,

  //! 文件系统使用的密码与使用--cipher参数在命令行上指定的密码不同.
  WrongCipher = 15,

  //! 基本目录不存在或不可访问（即，不可读取或可写或非目录）
  InaccessibleBaseDir = 16,

  //! 挂载目录不存在或不可访问（即无法读取或写入或不是目录）
  InaccessibleMountDir = 17,

  //! 基本目录不能是安装目录的子目录
  BaseDirInsideMountDir = 18,

  //! 文件系统出了点问题.
  InvalidFilesystem = 19,

  //! 配置文件中的文件系统ID与我们上次从此basedir加载文件系统的时间不同.
  //! 这可能意味着攻击者用另一种文件系统替换了文件系统.
  //! 您可以传递--allow-replaced-filesystem选项以允许此操作.
  FilesystemIdChanged = 20,

  //! 文件系统加密密钥与我们上次加载此文件系统的时间不同.
  //! 这可能意味着攻击者用另一种文件系统替换了文件系统.
  //! 您可以传递--allow-replaced-filesystem选项以允许此操作.
  EncryptionKeyChanged = 21,

  //! 命令行选项和文件系统在是否应该将丢失的块视为完整性违规方面存在分歧.
  FilesystemHasDifferentIntegritySetup = 22,

  //! 文件系统处于单客户端模式,只能在创建它的客户端中使用.
  SingleClientFileSystem = 23,

  //! 先前运行的文件系统检测到完整性违规.
  //! 阻止访问以确保用户注意.
  //! 用户删除完整性状态文件后,将可以再次访问文件系统.
  IntegrityViolationOnPreviousRun = 24,

  //! 检测到完整性违规,并卸载文件系统以确保用户注意.
  IntegrityViolation = 25,

  //! 挂载点不为空
  MountpointNotEmpty = 26,

  //! 挂载目录使用中
  ResourceBusy = 27,

  //! cryfs不存在
  CryfsNotExist = 28,

  //! 挂载目录不存在
  MountdirNotExist = 29,

  //! 挂载目录已加密
  MountdirEncrypted = 30,

  //! 没有权限
  PermissionDenied = 31,

  //! fusermount不存在
  FusermountNotExist = 32,

  //! Cryfs创建的加密文件夹已存在
  EncryptedExist = 33
};

#endif //! CRYFSERRORCODE_H
