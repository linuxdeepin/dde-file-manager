// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-gui/appletmanager.h>
#include <dfm-gui/applet.h>
#include <dfm-gui/containment.h>
#include <dfm-gui/panel.h>
#include <dfm-gui/appletfactory.h>
#include "appletmanager_p.h"
#include "applet_p.h"

#include <dfm-framework/lifecycle/lifecycle.h>
#include <dfm-base/utils/finallyutil.h>

#include <QFile>

#include <mutex>

#include <QElapsedTimer>

DFMGUI_BEGIN_NAMESPACE

// Applet 类型字段
inline constexpr char kApplet[] { "Applet" };
// Applet 类型字段 - 容器 Containment
inline constexpr char kContainment[] { "Containment" };
// Applet 类型字段 - 面板 Panel
inline constexpr char kPanel[] { "Panel" };

AppletManagerPrivate::AppletManagerPrivate(AppletManager *q)
    : q_ptr(q), root(AppletTemplateNode::Ptr::create())
{
}

/*!
 * \brief 解析静态的插件中的 QML 组件依赖关系，这个关系由插件 json 数据决定，
 *      静态的组件间包含关系，用于构建默认界面。
 *      注意同运行时依赖关系区分，程序运行时可将其它小组件嵌入到非静态依赖的父组件中，
 *      这些依赖关系变更在 rootApplet (顶层节点)层级之下进行，目前未设计动态增加顶层节点。
 *      e.g.: 从小组件列表选取一个小组件拖拽到桌面。
 * \sa initRootApplets()
 */
void AppletManagerPrivate::parseDefaultRootTemplates()
{
    Q_Q(AppletManager);
    // 借用已调整加载顺序(loadQueue)的插件列表，仅获取 Quick plugin
    QList<dpf::PluginMetaObjectPointer> quickPlugins = dpf::LifeCycle::pluginSortedMetaObjs(
            [](const dpf::PluginMetaObjectPointer &ptr) -> bool { return !ptr->quickMetaData().isEmpty(); });
    QString errorString;

    for (const dpf::PluginMetaObjectPointer &metaPtr : quickPlugins) {
        QList<dpf::PluginQuickMetaPtr> quickInfoList = metaPtr->quickMetaData();

        for (const dpf::PluginQuickMetaPtr &infoPtr : quickInfoList) {
            bool ret { q->registeApplet(infoPtr) };
            Q_ASSERT_X(ret, "dfm-gui", errorString.toLocal8Bit().data());
            if (!ret)
                qCWarning(logDFMGui) << errorString;
        }
    }
}

/*!
 * \brief 从模板节点 \a node 创建对应的 Applet ，并将父节点设置为 \a parent，
 *  若 \a recursive 为 true，将按照模板树递归创建子 Applet 。
 * \return 构造的 Applet
 */
Applet *AppletManagerPrivate::createAppletFromNode(const AppletTemplateNode::Ptr &node, Containment *parent, bool recursive)
{
    Q_Q(AppletManager);
    if (!node || !node->quickInfoPtr) {
        return nullptr;
    }

    Applet *applet = q->createAppletFromInfo(node->quickInfoPtr, parent, &errorString);
    if (!applet) {
        return nullptr;
    }

    if (recursive
        && applet->flags().testFlag(Applet::kContainment)
        && !node->childNode.isEmpty()) {

        Containment *containment = qobject_cast<Containment *>(applet);
        if (!containment) {
            return nullptr;
        }

        for (auto childNode : std::as_const(node->childNode)) {
            Applet *childApplet = createAppletFromNode(childNode, containment);
            containment->appendApplet(childApplet);
        }
    }

    return applet;
}

/*!
 * \brief 释放顶层模板和缓存
 */
void AppletManagerPrivate::clear()
{
    cacheIDToNode.clear();
    root->childNode.clear();
}

/*!
 * \brief 将配置文件的 "type" 字段字符串 \a typeString 转换为 Applet 标识
 * \param typeString "type" 字段字符串
 * \return Applet 标识，标记配置的 Applet 界面控件类型
 */
Applet::Flags AppletManagerPrivate::flagFromString(const QString &typeString)
{
    if (typeString.isEmpty() || kApplet == typeString) {
        return Applet::kApplet;
    } else if (kContainment == typeString) {
        return Applet::kContainment;
    } else if (kPanel == typeString) {
        return Applet::kPanel;
    }

    return Applet::kUnknown;
}

/*!
 * \class AppletManager
 * \brief Applet 管理器
 * \details 用于从已加载的插件信息中提取 QQuick 组件信息，并按照组件依赖树创建信息模板
 *      后续的窗体创建时，根据模板创建对应的界面。
 *      运行时动态注册的 Applet 元信息将在下一次创建顶层 Applet 时体现。
 *
 *      此类设计于：
 *      1. 通过 DFM 事件系统动态注册通用组件
 *      2. 通过配置信息恢复界面组件配置（用于桌面）
 */

AppletManager::AppletManager(QObject *parent)
    : QObject(parent), dptr(new AppletManagerPrivate(this))
{
}

AppletManager::~AppletManager()
{
    d_func()->clear();
}

AppletManager *AppletManager::instance()
{
    static AppletManager manager;
    return &manager;
}

/*!
 * \brief 初始化 rootApplet (顶层节点) , \a configFile 为缓存配置文件，
 *      设计在需要恢复子组件信息的应用中使用，例如 dde-desktop
 */
void AppletManager::initRootTemplates(const QString &configFile)
{
    // TODO: 用于桌面的配置信息获取
    Q_UNUSED(configFile)

    static std::once_flag flag;
    std::call_once(flag, [this]() { d_func()->parseDefaultRootTemplates(); });
}

/*!
 * \return 返回顶层 Window 节点的模板 ID 列表，用于后续 createWindowContainment() 创建容器
 */
QList<QString> AppletManager::panelIdList() const
{
    Q_D(const AppletManager);
    QList<QString> templates;
    for (const auto &node : std::as_const(d->root->childNode)) {
        if (node->flag.testFlag(Applet::kPanel)) {
            templates.append(node->cachedId);
        }
    }

    return templates;
}

/*!
 * \brief 根据模板 ID 创建对应的 Panel ，仅创建对应的 Panel ，不会递归创建
 * \return Panel 指针
 */
Panel *AppletManager::createPanel(const QString &templateId)
{
    Q_D(AppletManager);
    if (auto node = d->cacheIDToNode.value(templateId)) {
        if (node->flag.testFlag(Applet::kPanel)) {
            // 仅创建当前 Applet
            std::unique_ptr<Applet> applet(d->createAppletFromNode(node, nullptr, false));
            if (!applet) {
                return nullptr;
            }

            if (Panel *panel = qobject_cast<Panel *>(applet.get())) {
                applet.release();
                return panel;
            }

            d->errorString = QString("%1 is not based on class panel").arg(templateId);
        } else {
            d->errorString = QString("%1 is not panel").arg(templateId);
        }
    } else {
        d->errorString = QString("Not found %1 in applet template.").arg(templateId);
    }

    return nullptr;
}

/*!
 * \brief 根据插件名和组件 ID 查找创建对应的 Panel
 * \return Panel 指针，未指定父对象，AppletManager 也不会维护此指针的生命周期。
 */
Panel *AppletManager::createPanel(const QString &pluginName, const QString &quickId)
{
    return createPanel(d_func()->generateId(pluginName, quickId));
}

/*!
 * \brief 填充容器 \a containment 的子 Applets ，此函数用于调整初始化流程，主窗体创建完成后
 *  再进行子控件的填充
 */
void AppletManager::fillChildren(Containment *containment)
{
    if (!containment) {
        return;
    }

    Q_D(AppletManager);
    QString templateId = d->generateId(containment->plugin(), containment->id());
    if (auto node = d->cacheIDToNode.value(templateId)) {
        for (auto childNode : std::as_const(node->childNode)) {
            Applet *childApplet = d->createAppletFromNode(childNode, containment);
            containment->appendApplet(childApplet);
        }
    }
}

/*!
 * \return 返回当前已注册的所有 Applet 的模板 ID 列表
 */
QList<QString> AppletManager::allAppletTemplateIdList() const
{
    return d_func()->cacheIDToNode.keys();
}

/*!
 * \brief 根据模板 ID 查找创建对应的 Applet ，会遍历 Applet 的子节点查找创建
 * \return Applet 指针，创建失败返回 nullptr ，错误信息通过 lastError() 获取
 */
Applet *AppletManager::createAppletRecursive(const QString &templateId, Containment *parent)
{
    Q_D(AppletManager);
    if (auto node = d->cacheIDToNode.value(templateId)) {
        return d->createAppletFromNode(node, parent);
    }

    return nullptr;
}

/*!
 * \brief 根据插件名和组件 ID 查找创建对应的 Applet
 * \return Applet 指针，创建失败返回 nullptr ，错误信息通过 lastError() 获取
 */
Applet *AppletManager::createAppletRecursive(const QString &pluginName, const QString &quickId, Containment *parent)
{
    return createAppletRecursive(d_func()->generateId(pluginName, quickId), parent);
}

/*!
 * \return 返回最后一次调用失败的错误信息
 */
QString AppletManager::lastError() const
{
    return d_func()->errorString;
}

/*!
 * \brief 根据 Quick 组件元信息注册到 Applet 模板树中并返回结果，下一此创建此 Quick 组件的顶层 Panel 时，
 *      将包含其对应的 Applet .
 * \return 是否注册成功，若注册失败，错误信息通过 lastError() 获取
 */
bool AppletManager::registeApplet(const dpf::PluginQuickMetaPtr &infoPtr)
{
    Q_D(AppletManager);
    if (!infoPtr || infoPtr->plugin().isEmpty() || infoPtr->id().isEmpty() || infoPtr->url().isEmpty()) {
        d->errorString = QStringLiteral("Invalid plugin quick meta info.");
        return false;
    }

    QString generatedId = d->generateId(infoPtr->plugin(), infoPtr->id());
    Applet::Flags flag = d->flagFromString(infoPtr->type());
    if (Applet::kUnknown == flag) {
        d->errorString = QString("Unknown applet type %1.").arg(infoPtr->type());
        return false;
    }

    if (infoPtr->parent().isEmpty()) {
        if (flag.testFlag(Applet::kContainment)) {
            if (!d->cacheIDToNode.contains(generatedId)) {
                auto node = AppletTemplateNode::Ptr::create();
                node->flag = flag;
                node->cachedId = generatedId;
                node->quickInfoPtr = infoPtr;
                node->parent = d->root.toWeakRef();
                d->root->childNode.append(node);
                d->cacheIDToNode.insert(generatedId, node);
                return true;
            }

            d->errorString = QString("duplicate id %1").arg(generatedId);
        } else {
            // 顶层节点必须是容器
            d->errorString = QString("root applet must be containment");
        }

    } else {
        if (auto parNode = d->cacheIDToNode.value(infoPtr->parent())) {
            if (flag.testFlag(Applet::kPanel)) {
                d->errorString = QString("Window must be root containment");

            } else if (!d->cacheIDToNode.contains(generatedId)) {
                auto node = AppletTemplateNode::Ptr::create();
                node->flag = flag;
                node->cachedId = generatedId;
                node->quickInfoPtr = infoPtr;
                node->parent = parNode.toWeakRef();
                parNode->childNode.append(node);
                d->cacheIDToNode.insert(generatedId, node);
                return true;

            } else {
                d->errorString = QString("duplicate id %1").arg(generatedId);
            }

        } else {
            // 未查找到父组件
            d->errorString = QString("not find parent %1").arg(infoPtr->parent());
        }
    }

    return false;
}

/*!
 * \brief 将插件 \a pluginName 中的 \a quickId 组件从模板树中移除
 * \return 返回是否取消注册成功，false表面当前 Applet 模板树中没有找到对应的 Quick 组件元信息
 */
bool AppletManager::unRegisteApplet(const QString &pluginName, const QString &quickId)
{
    Q_D(AppletManager);
    QString generatedId = d->generateId(pluginName, quickId);

    auto findItr = d->cacheIDToNode.find(generatedId);
    if (findItr != d->cacheIDToNode.end()) {
        auto parNode = findItr.value()->parent.toStrongRef();
        if (parNode) {
            parNode->childNode.removeOne(findItr.value());
        }
        d->cacheIDToNode.erase(findItr);
        return true;
    }

    return false;
}

/*!
 * \brief 通过插件元信息 \a metaPtr 创建 Applet ，可用于创建重复的组件嵌入其他容器。
 * \return 创建的 Applet 若插件信息异常，将返回 false
 */
Applet *AppletManager::createAppletFromInfo(const dpf::PluginQuickMetaPtr &metaPtr, Containment *parent, QString *errorString)
{
    QString error;
    dfmbase::FinallyUtil finally([&]() {
        if (errorString)
            *errorString = error;
    });

    if (!metaPtr) {
        error = QStringLiteral("Input plugin quick meta info not valid.");
        return nullptr;
    }

    if (metaPtr->url().isEmpty() || metaPtr->id().isEmpty()) {
        error = QString("Unkown applet id %1 or url %2 is empty.").arg(metaPtr->id()).arg(metaPtr->url().toString());
        return nullptr;
    }

    Applet::Flags flag = AppletManagerPrivate::flagFromString(metaPtr->type());
    if (Applet::kUnknown == flag) {
        error = QString("Unkown applet %1.%2 type %3").arg(metaPtr->plugin()).arg(metaPtr->id()).arg(metaPtr->type());
        return nullptr;
    }

    Applet *appletPtr = nullptr;
    if (!metaPtr->applet().isEmpty()) {
        appletPtr = AppletFactory::instance()->create(metaPtr->applet(), parent, &error);
        if (!appletPtr) {
            return nullptr;
        }

        if (flag.testFlag(Applet::kContainment) && !qobject_cast<Containment *>(appletPtr)) {
            error = QString("Extendsion applet %1.%2 sets type to containment but not based on containment")
                            .arg(metaPtr->plugin())
                            .arg(metaPtr->id());
            appletPtr->deleteLater();
            return nullptr;
        }
    }

    if (!appletPtr) {
        switch (flag) {
        case Applet::kApplet:
            appletPtr = new Applet(parent);
            break;
        case Applet::kContainment:
            appletPtr = new Containment(parent);
            break;
        case Applet::kPanel:
            appletPtr = new Panel(parent);
            break;
        }
    }
    Q_ASSERT_X(appletPtr, "Applet load", "Create new applet failed!");

    appletPtr->dptr->metaPtr = metaPtr;
    appletPtr->setComponentUrl(metaPtr->url());
    return appletPtr;
}

DFMGUI_END_NAMESPACE
