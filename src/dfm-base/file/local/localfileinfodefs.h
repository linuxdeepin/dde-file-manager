// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOCALFILEINFODEFS_H
#define LOCALFILEINFODEFS_H

#include <dfm-base/dfm_base_global.h>

DFMBASE_BEGIN_NAMESPACE

// SyncFileInfo 与 AsyncFileInfo 共用的 DFileInfo 属性查询集合。
// 仅查询实际需要的属性以替代默认的 "*"，减少不必要的 I/O 开销。
// 后续如需调整查询属性，只需在此处统一修改。
//
// 注意：
// - standard::is-file、standard::is-dir 不是 GIO 标准属性，而是 dfm-io 自定义属性
//   (AttributeID kStandardIsFile=610, kStandardIsDir=611)，dfm-io 解析属性字符串来
//   决定是否计算这两个值。若移除，isFile()/isDir() 将失效，请勿删除。
// - standard::icon、standard::content-type 被有意排除：查询它们会触发 GIO 的
//   mimetype 检测（额外 I/O 开销），而文件管理器通过 QMimeDatabase 独立计算
//   mimetype，不依赖 GIO 的 content-type。
inline constexpr char kFileAttributes[] { "standard::name,standard::type,standard::is-file,standard::is-dir,"
    "standard::display-name,standard::size,standard::is-symlink,standard::symlink-target,standard::is-hidden,"
    "access::*,time::*,"
    "owner::*,unix::uid,unix::inode,unix::gid,unix::mode,id::filesystem" };

DFMBASE_END_NAMESPACE

#endif   // LOCALFILEINFODEFS_H
