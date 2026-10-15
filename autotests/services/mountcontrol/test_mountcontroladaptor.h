// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// mountcontrol 服务 DBus 适配器桩头：
// 真实的 mountcontroladaptor.h 由服务的 dependencies.cmake 经 qt6_add_dbus_adaptor
// 在服务构建目录生成，测试目标（unity-include 服务实现源码）不可见。
// 本文件放置于测试目录（在测试目标的 -I 搜索路径内），提供满足
// private/mountcontroldbus_p.h 编译依赖的最小适配器实现。
// 该文件不得包含 Q_OBJECT，避免 AUTOMOC 生成多余元对象。

#ifndef MOUNTCONTROLADAPTOR_H
#define MOUNTCONTROLADAPTOR_H

#include <QDBusAbstractAdaptor>
#include <QObject>

class MountControlAdaptor : public QDBusAbstractAdaptor
{
public:
    explicit MountControlAdaptor(QObject *parent)
        : QDBusAbstractAdaptor(parent) { }
};

#endif   // MOUNTCONTROLADAPTOR_H
