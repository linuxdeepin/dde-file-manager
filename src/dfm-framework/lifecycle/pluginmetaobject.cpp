// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/pluginmetaobject_p.h"

#include <dfm-framework/lifecycle/pluginmetaobject.h>
#include <dfm-framework/lifecycle/pluginquickmetadata.h>

#include <QDebug>

DPF_BEGIN_NAMESPACE

/*!
 * \brief PluginMetaObject::version
 * 获取插件元数据中插件版本
 * \return
 */
QString PluginMetaObject::version() const
{
    return d->version;
}

/*!
 * \brief PluginMetaObject::description
 * 获取插件元数据中插件描述
 * \return
 */
QString PluginMetaObject::description() const
{
    return d->description;
}

/*!
 * \brief PluginMetaObject::category
 * 获取插件元数据中插件类别
 * \return
 */
QString PluginMetaObject::category() const
{
    return d->category;
}

/*!
 * \brief PluginMetaObject::urlLink
 * 获取插件元数据中插件主页链接
 * \return
 */
QString PluginMetaObject::urlLink() const
{
    return d->urlLink;
}

/*!
 * \brief PluginMetaObject::depends
 * 获取插件元数据中插件启动依赖
 * \return
 */
QList<PluginDepend> PluginMetaObject::depends() const
{
    return d->depends;
}

QVariantMap PluginMetaObject::customData() const
{
    return d->customData;
}

/*!
 * \brief PluginMetaObject::pluginState
 * 获取插件当前状态
 * \return
 */
PluginMetaObject::State PluginMetaObject::pluginState() const
{
    return d->state;
}

/*!
 * \brief PluginMetaObject::plugin
 * 获取插件对象接口
 * \return
 */
QSharedPointer<Plugin> PluginMetaObject::plugin() const
{
    return d->plugin;
}

/*!
 * \brief PluginMetaObject::errorString
 * 获取插件加载错误信息
 * \return
 */
QString PluginMetaObject::errorString() const
{
    return d->error;
}

/*!
 * \return 返回 QML Applet 组件元信息列表
 */
QList<PluginQuickMetaPtr> PluginMetaObject::quickMetaData() const
{
    return d->quickMetaList;
}

PluginMetaObject::PluginMetaObject()
    : d(new PluginMetaObjectPrivate(this))
{
}

bool PluginMetaObject::isVirtual() const
{
    return d->isVirtual;
}

/*!
 * \brief PluginMetaObject::fileName
 *  获取插件文件路径名称
 * \return
 */
QString PluginMetaObject::fileName() const
{
    return d->loader->fileName();
}

/*!
 * \brief PluginMetaObject::iid
 *  获取插件元数据中iid
 * \return
 */
QString PluginMetaObject::iid() const
{
    return d->iid;
}

/*!
 * \brief PluginMetaObject::name
 *  获取插件元数据中名称
 * \return
 */
QString PluginMetaObject::name() const
{
    return d->name;
}

/*!
 * \brief PluginDepend::PluginDepend
 *  构造函数
 */
PluginDepend::PluginDepend()
{
}

/*!
 * \brief PluginDepend::PluginDepend
 * 拷贝构造
 * \param depend
 */
PluginDepend::PluginDepend(const PluginDepend &depend)
{
    pluginName = depend.name();
    pluginVersion = depend.version();
}

/*!
 * \brief PluginDepend::operator =
 * 赋值拷贝
 * \param depend
 * \return
 */
PluginDepend &PluginDepend::operator=(const PluginDepend &depend)
{
    pluginName = depend.name();
    pluginVersion = depend.version();
    return *this;
}

QT_BEGIN_NAMESPACE
/*!
 * \brief operator <<
 * 重定向全局Debug打印PluginDepend对象的函数
 * \param out
 * \param depend
 * \return
 */
Q_CORE_EXPORT QDebug operator<<(QDebug out, const DPF_NAMESPACE::PluginDepend &depend)
{
    DPF_USE_NAMESPACE
    out << "PluginDepend(" << QString("0x%0").arg(qint64(&depend), 0, 16) << "){";
    out << kPluginName << " : " << depend.name() << "; ";
    out << kPluginVersion << " : " << depend.version() << "; ";
    out << "}";
    return out;
}

/*!
 * \brief operator <<
 * 重定向全局Debug打印PluginMetaObject对象的函数
 * \param out
 * \param metaObj
 * \return
 */
Q_CORE_EXPORT QDebug operator<<(QDebug out, const DPF_NAMESPACE::PluginMetaObject &metaObj)
{
    DPF_USE_NAMESPACE
    out << "PluginMetaObject(" << QString("0x%0").arg(qint64(&metaObj), 0, 16) << "){";
    out << "IID"
        << ":" << metaObj.iid() << "; ";
    out << kPluginName << ":" << metaObj.name() << "; ";
    out << kPluginVersion << ":" << metaObj.version() << "; ";
    out << kPluginCategory << ":" << metaObj.category() << "; ";
    out << kPluginDescription << ":" << metaObj.description() << "; ";
    out << kPluginUrlLink << ":" << metaObj.urlLink() << "; ";
    out << kPluginDepends << ":" << metaObj.depends() << ";";
    out << kCustomData << ":" << metaObj.customData();
    out << "isVirtual"
        << ":" << metaObj.isVirtual() << "; ";
    out << "}";
    return out;
}

/*!
 * \brief operator <<
 * 重定向全局Debug入口函数
 * \param out
 * \param pointer
 * \return
 */
Q_CORE_EXPORT QDebug operator<<(QDebug out, const DPF_NAMESPACE::PluginMetaObjectPointer &pointer)
{
    out << *pointer;
    return out;
}

/*!
 * \brief 重定向全局Debug打印 PluginQuickMetaData 对象的函数
 */
Q_CORE_EXPORT QDebug operator<<(QDebug out, const DPF_NAMESPACE::PluginQuickMetaData &quickMeta)
{
    DPF_USE_NAMESPACE
    out << "PluginQuickInfo(" << QString("0x%0").arg(qint64(&quickMeta), 0, 16) << "){";
    out << kQuickUrl << " : " << quickMeta.url() << "; ";
    out << kQuickId << " : " << quickMeta.id() << "; ";
    out << kQuickType << " : " << quickMeta.type() << "; ";
    out << kQuickParent << " : " << quickMeta.parent() << "; ";
    out << kQuickApplet << " : " << quickMeta.applet() << "; ";
    out << "}";
    return out;
}

/*!
 * \brief 重定向全局Debug打印 PluginQuickMetaPtr 的函数
 */
Q_CORE_EXPORT QDebug operator<<(QDebug out, const DPF_NAMESPACE::PluginQuickMetaPtr &quickMetaPtr)
{
    out << *quickMetaPtr;
    return out;
}

QT_END_NAMESPACE

DPF_END_NAMESPACE
