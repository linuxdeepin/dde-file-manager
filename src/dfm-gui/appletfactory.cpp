// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-gui/appletfactory.h>
#include <dfm-gui/applet.h>

#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/finallyutil.h>
#include <dfm-base/utils/threadcontainer.h>

#include <dfm-framework/lifecycle/lifecycle.h>

#include <QFileInfo>
#include <QDir>

DFMGUI_BEGIN_NAMESPACE

class AppletFactoryData
{
public:
    bool regCreator(const QString &scheme, AppletFactory::CreateFunc creator, QString *errorString, QUrl qmlUrl = {});

    struct CreateInfo
    {
        QUrl qmlComponent;
        AppletFactory::CreateFunc func;
    };

    bool checkRoute { false };
    dfmbase::DThreadMap<QString, CreateInfo> constructList {};
};

bool AppletFactoryData::regCreator(const QString &scheme, AppletFactory::CreateFunc creator, QString *errorString, QUrl qmlUrl)
{
    QString error;
    dfmbase::FinallyUtil finally([&]() {
        if (errorString)
            *errorString = error;
    });

    if (constructList.contains(scheme)) {
        error = "The current url has registered "
                "the associated construction class";
        return false;
    }

    constructList.insert(scheme, { qmlUrl, creator });
    finally.dismiss();
    return true;
}

/*!
 * \class AppletFactory
 * \brief 类似 SchemeFactory 提供拓展 Applet 类创建函数的注册管理
 */

AppletFactory::AppletFactory()
    : d(new AppletFactoryData)
{
}

AppletFactory *AppletFactory::instance()
{
    static AppletFactory ins;
    return &ins;
}

/*!
 * \brief 注册Applet创建器 \a creator 与 \a scheme 的关联
 * \return 注册结果，如果当前已存在 \a scheme 的关联，则返回 false ，错误信息会写入 \a errorString
 */
bool AppletFactory::regCreator(const QString &scheme, CreateFunc creator, QString *errorString)
{
    return d->regCreator(scheme, creator, errorString);
}

/*!
 * \brief 根据需要构造的 \a url 进行顶层类构造，调用该函数存在前置条件，否则将创建空指针
 * （ViewAppletFactory）需要注册scheme到DFMUrlRoute类
 * 需要注册scheme到 AppletFactory 类
 * 构造函数可识别传入的 \a parent 进行绑定或属性设置等处理
 *
 * \return 构造的 Applet 指针
 * （ViewAppletFactory）如果没有注册 scheme 到 DFMUrlRoute，返回空指针
 * 如果没有注册 scheme 与 class 构造函数规则，返回空指针
 * 如果出现错误，错误信息会写入 \a errorString
 */
Applet *AppletFactory::create(const QUrl &url, Containment *parent, QString *errorString)
{
    QString error;
    dfmbase::FinallyUtil finally([&]() {
        if (errorString)
            *errorString = error;
    });

    const QString scheme = url.scheme();
    if (d->checkRoute && !dfmbase::UrlRoute::hasScheme(scheme)) {
        error = "No scheme found for "
                "URL registration";
        return nullptr;
    }

    auto creator = d->constructList.value(scheme);
    if (!creator.func) {
        error = "Scheme should be call registered 'regClass()' function "
                "before create function";
        return nullptr;
    }

    Applet *applet = creator.func(url, parent, &error);
    if (!creator.qmlComponent.isEmpty()) {
        applet->setComponentUrl(creator.qmlComponent);
    }
    return applet;
}

/*!
 * \class ViewAppletFactory
 * \brief 类似 ViewFactory 提供较灵活的 QML 组件注册管理方式
 */
ViewAppletFactory::ViewAppletFactory()
{
    d->checkRoute = true;
}

ViewAppletFactory *ViewAppletFactory::instance()
{
    static ViewAppletFactory ins;
    return &ins;
}

/*!
 * \brief 用于视图模块动态注册QML组件，不同于通过插件元信息的注册，这类 Applet 是匿名的，不会注册
 *  到 Applet 管理中，也不存在默认的依赖关系。
 *  传入插件名 \a plugin 和QML文件 \a qml 用于查找 Applet 关联的QML组件，组件文件路径如下组合：
 *      `[插件路径]/[插件名]/qml路径`
 *  其它参考 AppletFactory::regCreator()
 *
 * \return 是否注册成功，对于 ViewAppletFactory ，如果为查找到 \a plugin 对应插件，或
 *  \a qmlFile 文件不在制定路径，返回 false
 */
bool ViewAppletFactory::regCreator(const QString &plugin, const QString &qmlFile, const QString &scheme,
                                   AppletFactory::CreateFunc creator, QString *errorString)
{
    QString error;
    dfmbase::FinallyUtil finally([&]() {
        if (errorString)
            *errorString = error;
    });

    // 根据插件路径 + 文件名查找
    dpf::PluginMetaObjectPointer info = dpf::LifeCycle::pluginMetaObj(plugin);
    if (!info) {
        error = QStringLiteral("Plugin not found");
        return false;
    }

    QString pluginPath = QFileInfo(info->fileName()).absolutePath();
    QString fullPath = pluginPath + QDir::separator() + info->name() + QDir::separator() + qmlFile;
    if (!QFile::exists(fullPath)) {
        error = QStringLiteral("Qml file not exists");
        return false;
    }

    QUrl qmlUrl = QUrl::fromLocalFile(fullPath);
    return d->regCreator(scheme, creator, &error, qmlUrl);
}

DFMGUI_END_NAMESPACE
