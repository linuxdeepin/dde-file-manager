#include "pluginmetaobject.h"
#include <QDebug>

DPF_BEGIN_NAMESPACE

/**
 * @brief PluginMetaObject::version
 * 获取插件元数据中插件版本
 * @return
 */
QString PluginMetaObject::version() const
{
    return m_version;
}

/**
 * @brief PluginMetaObject::compatVersion
 * 获取插件元数据中兼容版本
 * @return
 */
QString PluginMetaObject::compatVersion() const
{
    return m_compatVersion;
}

/**
 * @brief PluginMetaObject::vendor
 * 获取插件元数据中插件所有者
 * @return
 */
QString PluginMetaObject::vendor() const
{
    return m_vendor;
}

/**
 * @brief PluginMetaObject::copyright
 * 获取插件元数据中的插件版权
 * @return
 */
QString PluginMetaObject::copyright() const
{
    return m_copyright;
}

/**
 * @brief PluginMetaObject::license
 * 获取插件元数据中开源许可协议
 * @return
 */
QStringList PluginMetaObject::license() const
{
    return m_license;
}

/**
 * @brief PluginMetaObject::description
 * 获取插件元数据中插件描述
 * @return
 */
QString PluginMetaObject::description() const
{
    return m_description;
}

/**
 * @brief PluginMetaObject::category
 * 获取插件元数据中插件类别
 * @return
 */
QString PluginMetaObject::category() const
{
    return m_category;
}

/**
 * @brief PluginMetaObject::urlLink
 * 获取插件元数据中插件主页链接
 * @return
 */
QString PluginMetaObject::urlLink() const
{
    return m_urlLink;
}

/**
 * @brief PluginMetaObject::depends
 * 获取插件元数据中插件启动依赖
 * @return
 */
QList<PluginDepend> PluginMetaObject::depends() const
{
    return m_depends;
}

/**
 * @brief PluginMetaObject::pluginState
 * 获取插件当前状态
 * @return
 */
PluginMetaObject::State PluginMetaObject::pluginState() const
{
    return m_state;
}

/**
 * @brief PluginMetaObject::plugin
 * 获取插件对象接口
 * @return
 */
QSharedPointer<Plugin> PluginMetaObject::plugin()
{
    return m_plugin;
}

/**
 * @brief PluginMetaObject::loaderErrorString
 * 获取插件加载错误信息
 * @return
 */
QString PluginMetaObject::loaderErrorString()
{
    return m_loader->errorString();
}

/**
 * @brief 默认构造函数
 */
PluginMetaObject::PluginMetaObject()
    : m_loader(new QPluginLoader(nullptr))
{

}

/**
 * @brief 拷贝构造函数
 */
PluginMetaObject::PluginMetaObject(const PluginMetaObject &meta)
{
    m_iid = meta.iid();
    m_name = meta.name();
    m_version = meta.version();
    m_compatVersion = meta.compatVersion();
    m_category = meta.category();
    m_license = meta.license();
    m_description = meta.description();
    m_urlLink = meta.urlLink();
    m_depends = meta.depends();
    m_state = pluginState();
    m_plugin = plugin();
    m_loader = meta.m_loader;
}
/**
  * @brief 赋值拷贝
  */
PluginMetaObject &PluginMetaObject::operator =(const PluginMetaObject &meta)
{
    m_iid = meta.iid();
    m_name = meta.name();
    m_version = meta.version();
    m_compatVersion = meta.compatVersion();
    m_category = meta.category();
    m_license = meta.license();
    m_description = meta.description();
    m_urlLink = meta.urlLink();
    m_depends = meta.depends();
    m_state = pluginState();
    m_plugin = plugin();
    m_loader = meta.m_loader;
    return *this;
}

/**
 * @brief PluginMetaObject::fileName
 *  获取插件文件路径名称
 * @return
 */
QString PluginMetaObject::fileName() const
{
    return m_loader->fileName();
}

/**
 * @brief PluginMetaObject::iid
 *  获取插件元数据中iid
 * @return
 */
QString PluginMetaObject::iid() const
{
    return m_iid;
}

/**
 * @brief PluginMetaObject::name
 *  获取插件元数据中名称
 * @return
 */
QString PluginMetaObject::name() const
{
    return m_name;
}

/**
 * @brief PluginDepend::PluginDepend
 *  构造函数
 */
PluginDepend::PluginDepend()
{

}

/**
 * @brief PluginDepend::PluginDepend
 * 拷贝构造
 * @param depend
 */
PluginDepend::PluginDepend(const PluginDepend &depend)
{
    m_name = depend.name();
    m_version = depend.version();
}

/**
 * @brief PluginDepend::operator =
 * 赋值拷贝
 * @param depend
 * @return
 */
PluginDepend &PluginDepend::operator =(const PluginDepend &depend)
{
    m_name = depend.name();
    m_version = depend.version();
    return *this;
}


QT_BEGIN_NAMESPACE
/**
 * @brief operator <<
 * 重定向全局Debug打印PluginDepend对象的函数
 * @param out
 * @param depend
 * @return
 */
Q_CORE_EXPORT QDebug operator<<(QDebug out, const DPF_NAMESPACE::PluginDepend &depend)
{
    DPF_USE_NAMESPACE
    out << "PluginDepend(" <<  QString("0x%0").arg(qint64(&depend),0,16) << "){";
    out << PLUGIN_NAME << " : " << depend.name() << "; ";
    out << PLUGIN_VERSION << " : " << depend.version() << "; ";
    out << "}";
    return out;
}

/**
 * @brief operator <<
 * 重定向全局Debug打印PluginMetaObject对象的函数
 * @param out
 * @param metaObj
 * @return
 */
Q_CORE_EXPORT QDebug operator<< (QDebug out, const DPF_NAMESPACE::PluginMetaObject &metaObj)
{
    DPF_USE_NAMESPACE
    out << "PluginMetaObject(" << QString("0x%0").arg(qint64(&metaObj),0,16) << "){";
    out << "IID" << ":" << metaObj.iid() << "; ";
    out << PLUGIN_NAME << ":" << metaObj.name() << "; ";
    out << PLUGIN_VERSION << ":" << metaObj.version() << "; ";
    out << PLUGIN_COMPATVERSION << ":" << metaObj.compatVersion() << "; ";
    out << PLUGIN_CATEGORY << ":" << metaObj.category() << "; ";
    out << PLUGIN_VENDOR << ":" << metaObj.vendor() << "; ";
    out << PLUGIN_COPYRIGHT << ":" << metaObj.copyright() << "; ";
    out << PLUGIN_DESCRIPTION << ":" << metaObj.description() << "; ";
    out << PLUGIN_LICENSE << ":" << metaObj.license() << "; ";
    out << PLUGIN_URLLINK << ":" << metaObj.urlLink() << "; ";
    out << PLUGIN_DEPENDS << ":" << metaObj.depends() << ";";
    out << "}";
    return out;
}

/**
 * @brief operator <<
 * 重定向全局Debug入口函数
 * @param out
 * @param pointer
 * @return
 */
Q_CORE_EXPORT QDebug operator<<(QDebug out, const DPF_NAMESPACE::PluginMetaObjectPointer &pointer)
{
    out << *pointer;
    return out;
}

QT_END_NAMESPACE

DPF_END_NAMESPACE
