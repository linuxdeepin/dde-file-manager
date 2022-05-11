/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#ifndef DAEMONPLUGIN_MOUNTCONTROL_GLOBAL_H
#define DAEMONPLUGIN_MOUNTCONTROL_GLOBAL_H

#define DAEMONPMOUNTCONTROL_NAMESPACE daemonplugin_mountcontrol
#define DAEMONPMOUNTCONTROL_BEGIN_NAMESPACE namespace DAEMONPMOUNTCONTROL_NAMESPACE {
#define DAEMONPMOUNTCONTROL_END_NAMESPACE }
#define DAEMONPMOUNTCONTROL_USE_NAMESPACE using namespace DAEMONPMOUNTCONTROL_NAMESPACE;

DAEMONPMOUNTCONTROL_BEGIN_NAMESPACE

namespace MountOpts {
static constexpr char kUser[] { "user" };
static constexpr char kPasswd[] { "passwd" };
static constexpr char kDomain[] { "domain" };
//static constexpr char kFsType[] { "fsType" };
static constexpr char kMountName[] { "mntName" };
}

DAEMONPMOUNTCONTROL_END_NAMESPACE

#endif   // DAEMONPLUGIN_MOUNTCONTROL_GLOBAL_H
