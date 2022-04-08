/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef DFMPLUGIN_EMBLEM_GLOBAL_H
#define DFMPLUGIN_EMBLEM_GLOBAL_H

#define DPEMBLEM_NAMESPACE dfmplugin_emblem

#define DPEMBLEM_BEGIN_NAMESPACE namespace DPEMBLEM_NAMESPACE {
#define DPEMBLEM_END_NAMESPACE }
#define DPEMBLEM_USE_NAMESPACE using namespace DPEMBLEM_NAMESPACE;

#include <QObject>

DPEMBLEM_BEGIN_NAMESPACE

enum class SystemEmblemType : uint8_t {
    kLink,
    kLock,
    kUnreadable,
    kShare
};

// view defines
const double kMinEmblemSize = 12.0;
const double kMaxEmblemSize = 128.0;

DPEMBLEM_END_NAMESPACE

#endif   // DFMPLUGIN_EMBLEM_GLOBAL_H
