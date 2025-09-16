// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pathgroupstrategy.h"

#include <dfm-base/dfm_log_defines.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/utils/fileutils.h>

#include <QDebug>

DPWORKSPACE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace GlobalServerDefines::DeviceProperty;

QStringList PathGroupStrategy::getPathOrder()
{
    return {
        "system",   // 系统盘
        "data",   // 数据盘
        // other分区是动态的，不在此处硬编码
    };
}

QHash<QString, QString> PathGroupStrategy::getDisplayNames()
{
    return {
        { "system", QObject::tr("System Disk") },
        { "data", QObject::tr("Data Disk") }
        // other分区显示名称是动态的，不在此处硬编码
    };
}

PathGroupStrategy::PathGroupStrategy(QObject *parent)
    : AbstractGroupStrategy(parent)
{
    fmDebug() << "PathGroupStrategy: Initialized";
}

PathGroupStrategy::~PathGroupStrategy()
{
    fmDebug() << "PathGroupStrategy: Destroyed";
}

QString PathGroupStrategy::getGroupKey(const FileInfoPointer &info) const
{
    if (!info) {
        fmWarning() << "PathGroupStrategy: Invalid fileInfo";
        return "other";
    }

    QString path = info->urlOf(UrlInfoType::kCustomerStartUrl).path();
    if (path.isEmpty()) {
        fmWarning() << "PathGroupStrategy: Empty file path for" << info->urlOf(UrlInfoType::kUrl).toString();
        return "other";
    }

    path = FileUtils::bindPathTransform(path, true);
    QString groupKey = classifyByPath(path);

    fmDebug() << "PathGroupStrategy: File" << info->urlOf(UrlInfoType::kUrl).toString()
              << "path:" << path << "-> group:" << groupKey;

    return groupKey;
}

QString PathGroupStrategy::getGroupDisplayName(const QString &groupKey) const
{
    if (groupKey.startsWith("other_")) {
        QVariantMap deviceInfo = otherGroupInfos.value(groupKey);
        QString label = deviceInfo.value(kIdLabel).toString();
        quint64 size = deviceInfo.value(kSizeTotal).toULongLong();
        return DeviceUtils::nameOfDefault(label, size);
    }

    return getDisplayNames().value(groupKey, groupKey);
}

QStringList PathGroupStrategy::getGroupOrder() const
{
    // 获取基础的固定顺序
    QStringList pathOrder = getPathOrder();

    // 添加动态的other组到末尾
    for (auto it = otherGroupInfos.constBegin(); it != otherGroupInfos.constEnd(); ++it) {
        if (!pathOrder.contains(it.key())) {
            pathOrder.append(it.key());
        }
    }

    return pathOrder;
}

int PathGroupStrategy::getGroupDisplayOrder(const QString &groupKey) const
{
    // 获取完整的组顺序列表
    QStringList pathOrder = getGroupOrder();
    int index = pathOrder.indexOf(groupKey);
    if (index == -1) {
        index = pathOrder.size();   // Unknown groups go to the end
    }

    return index;
}

bool PathGroupStrategy::isGroupVisible(const QString &groupKey, const QList<FileInfoPointer> &infos) const
{
    Q_UNUSED(groupKey)
    // A group is visible if it has at least one file info
    return !infos.isEmpty();
}

QString PathGroupStrategy::getStrategyName() const
{
    return GroupStrategty::kCustomPath;
}

QString PathGroupStrategy::classifyByPath(const QString &path) const
{
    const auto datas = DevProxyMng->queryDeviceInfoByPath(path);
    QVariantMap clearInfo = datas.value(BlockAdditionalProperty::kClearBlockProperty).toMap();
    QString label = clearInfo.value(kIdLabel, datas.value(kIdLabel)).toString();

    if (DeviceUtils::isSystemDisk(clearInfo.isEmpty() ? datas : clearInfo))
        return "system";

    if (DeviceUtils::isDataDisk(clearInfo.isEmpty() ? datas : clearInfo))
        return "data";

    // 对于其他分区，生成唯一的组键并保存设备信息
    QString devicePath = datas.value(kMountPoint).toString();
    if (devicePath.isEmpty()) {
        devicePath = path;
    }

    QString otherKey = "other_" + devicePath;
    otherGroupInfos[otherKey] = clearInfo.isEmpty() ? datas : clearInfo;
    return otherKey;
}