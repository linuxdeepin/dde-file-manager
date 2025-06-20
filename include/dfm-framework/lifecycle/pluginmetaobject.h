// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINMETAOBJECT_H
#define PLUGINMETAOBJECT_H

#include <dfm-framework/lifecycle/pluginquickmetadata.h>
#include <dfm-framework/dfm_framework_global.h>
#include <dfm-framework/lifecycle/plugindepend.h>

#include <QSharedData>
#include <QPluginLoader>

DPF_BEGIN_NAMESPACE

/*!
 * \brief PluginMetaT1 模板类
 * \details 此模板类为扩展特性，可实现不同插件元数据
 *  目前只是预留接口
 * \tparam 传入插件对象接口例如 Plugin
 */
template<class T>
class PluginMetaT1 : public QSharedData
{
    Q_DISABLE_COPY(PluginMetaT1)
public:
    PluginMetaT1() { }
};

/*! 插件json元文件示例
 * \code
 * {
 *   "Name" : "more",
 *   "Version" : "4.8.2",
 *   "CompatVersion" : "4.8.0",
 *   "Category" : "more",
 *   "Description" : "The core plugin for the Qt IDE.",
 *   "Vendor" : "The Qt Company Ltd",
 *   "Copyright" : "(C) 2019 The Qt Company Ltd",
 *   "License" : [
 *       "https://www.gnu.org/licenses/gpl-3.0.html"
 *   ],
 *   "UrlLink" : "http://www.qt.io",
 *   "Depends" : [
 *       {"Name" : "core", "Version" : "4.8.2"},
 *       {"Name" : "other", "Version" : "4.8.2"}
 *   ],
 *   "Quick" : [                                        // 可选字段，需展示界面的插件提供
 *       {
 *           "Url" : "MainWindow.qml",                  // 必须，加载的QML组件文件名，文件位于"插件目录/插件Name/Url"
 *           "Id" : "mainWindow",                       // 必须，用于区分的组件ID，运行时会标记为 "插件名.ID"
 *           "Type" : "[Containment|Panel]",            // 可选，无字段或空内容视为 Applet
 *           "Parent" : "[Depends Plugin Name].[Id]",   // 可选，默认为父组件 "插件名.ID" 的一部分, 插件名字段在 “Depends” 中必须存在
 *           "Applet" : "core.compoment.applet"         // 可选，拓展的 Applet ，需在 AppletFactory 注册
 *       }, {
 *           "Url" : "property.qml",
 *           "Id" : "propertyDialog"
 *       }
 *   ]
 * }
 * \endcode
 */

class Plugin;

/*!
 * \brief The PluginMetaObject class
 *  插件元数据对象
 * \details 该类与SharedPointer配套使用时是线程安全的
 */
class PluginMetaObjectPrivate;
class PluginMetaObject final : public PluginMetaT1<Plugin>
{
    friend class PluginManager;
    friend class PluginManagerPrivate;
    friend Q_CORE_EXPORT QDebug operator<<(QDebug, const PluginMetaObject &);
    Q_DISABLE_COPY(PluginMetaObject)

public:
    enum State {
        kInvalid,   /// 插件未操作获得任何状态
        kReading,   /// 插件正在读取Json
        kReaded,   /// 插件读取Json完毕
        kLoading,   /// 插件正在加载
        kLoaded,   /// 插件已加载
        kInitialized,   /// 插件已经操作Initialized函数
        kStarted,   /// 插件已经操作Start函数
        kStoped,   /// 插件已停操作Stop函数
        kShutdown,   /// 插件卸载并已经释放
    };

    PluginMetaObject();

    bool isVirtual() const;
    QString fileName() const;
    QString iid() const;
    QString name() const;
    QString version() const;
    QString category() const;
    QString description() const;
    QString urlLink() const;
    QList<PluginDepend> depends() const;
    QVariantMap customData() const;
    State pluginState() const;
    QSharedPointer<Plugin> plugin() const;
    QList<PluginQuickMetaPtr> quickMetaData() const;
    QString errorString() const;

private:
    QSharedPointer<PluginMetaObjectPrivate> d;
};

using PluginMetaObjectPointer = QSharedPointer<DPF_NAMESPACE::PluginMetaObject>;
using PluginDependGroup = QList<QPair<PluginMetaObjectPointer, PluginMetaObjectPointer>>;

QT_BEGIN_NAMESPACE
#ifndef QT_NO_DEBUG_STREAM
Q_CORE_EXPORT QDebug operator<<(QDebug, const DPF_NAMESPACE::PluginMetaObject &);
Q_CORE_EXPORT QDebug operator<<(QDebug, const DPF_NAMESPACE::PluginMetaObjectPointer &);
#endif   // QT_NO_DEBUG_STREAM
QT_END_NAMESPACE

DPF_END_NAMESPACE

#endif   // PLUGINMETAOBJECT_H
