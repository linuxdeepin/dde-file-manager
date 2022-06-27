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

namespace dfm_service_common {
#define CPY_NAMESPACE Property

#define CPY_BEGIN_NAMESPACE namespace CPY_NAMESPACE {
#define CPY_END_NAMESPACE }
#define CPY_USE_NAMESPACE using namespace CPY_NAMESPACE;
}

namespace dfm_service_common {

CPY_BEGIN_NAMESPACE
namespace EventType {
extern const int kEvokePropertyDialog;
}

enum FilePropertyControlFilter {
    kNotFilter = 0x00000000,
    kIconTitle = 0x00000001,
    kBasisInfo = 0x00000002,
    kPermission = 0x00000004,
    kFileSizeFiled = 0x00000008,
    kFileCountFiled = 0x00000010,
    kFileTypeFiled = 0x00000020,
    kFilePositionFiled = 0x00000040,
    kFileCreateTimeFiled = 0x0000080,
    kFileAccessedTimeFiled = 0x00000100,
    kFileModifiedTimeFiled = 0x00000200
};

enum BasicFieldExpandEnum {
    kNotAll,
    kFileSize,
    kFileCount,
    kFileType,
    kFilePosition,
    kFileCreateTime,
    kFileAccessedTime,
    kFileModifiedTime,
};

enum BasicExpandType {
    kFieldInsert,
    kFieldReplace
};

typedef QMultiMap<BasicFieldExpandEnum, QPair<QString, QString>> BasicExpand;

//! 定义创建控件函数类型
typedef QWidget *(*createControlViewFunc)(const QUrl &url);
typedef QMap<BasicExpandType, BasicExpand> (*basicViewFieldFunc)(const QUrl &url);

struct DeviceInfo
{
    QIcon icon;
    QUrl deviceUrl;
    QUrl mountPoint;
    QString deviceName;
    QString deviceType;
    QString fileSystem;
    qint64 totalCapacity;
    qint64 availableSpace;
};

CPY_END_NAMESPACE
}
Q_DECLARE_METATYPE(DSC_NAMESPACE::CPY_NAMESPACE::DeviceInfo)
#endif   //PROPERTY_DEFINE_H
