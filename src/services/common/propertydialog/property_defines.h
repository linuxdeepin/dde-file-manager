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
#ifndef PROPERTY_DEFINE_H
#define PROPERTY_DEFINE_H

#include "dfm_common_service_global.h"
#include "dfm-base/utils/finallyutil.h"

#include <QUrl>
#include <QIcon>
#include <QMap>
#include <QDebug>

DSC_BEGIN_NAMESPACE
#define CPY_NAMESPACE Property

#define CPY_BEGIN_NAMESPACE namespace CPY_NAMESPACE {
#define CPY_END_NAMESPACE }
#define CPY_USE_NAMESPACE using namespace CPY_NAMESPACE;
DSC_END_NAMESPACE

DSC_BEGIN_NAMESPACE

CPY_BEGIN_NAMESPACE
namespace EventType {
extern const int kEvokePropertyDialog;
}

namespace EventFilePropertyControlFilter {
extern const int kIconTitle;
extern const int kBasisInfo;
extern const int kPermission;
}

enum BasicFieldExpandEnum : int {
    kNotAll,
    kFileSize,
    kFileCount,
    kFileType,
    kFilePosition,
    kFileCreateTime,
    kFileAccessedTime,
    kFileModifiedTime,
};

enum BasicExpandType : int {
    kFieldFilter,
    kFieldInsert,
    kFieldReplace
};

struct BasicExpand
{
    QMultiMap<BasicFieldExpandEnum, QPair<QString, QString>> expandFieldMap;
};

struct DeviceInfo
{
    QIcon icon;
    QUrl deviceUrl;
    QString deviceName;
    QString deviceType;
    QString fileSystem;
    qint64 totalCapacity;
    qint64 availableSpace;
};

CPY_END_NAMESPACE
DSC_END_NAMESPACE
Q_DECLARE_METATYPE(DSC_NAMESPACE::CPY_NAMESPACE::DeviceInfo)
#endif   //PROPERTY_DEFINE_H
