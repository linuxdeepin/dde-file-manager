// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-gui/sharedqmlengine.h>
#include "sharedqmlengine_p.h"

#include <QQuickItem>
#include <QQmlComponent>
#include <QQmlContext>
#include <QCoreApplication>

Q_LOGGING_CATEGORY(logDFMGui, "org.deepin.dde.filemanager.lib.gui")

DFMGUI_BEGIN_NAMESPACE

QWeakPointer<QQmlEngine> SharedQmlEnginePrivate::s_globalEngine;

SharedQmlEnginePrivate::SharedQmlEnginePrivate(SharedQmlEngine *q)
    : q_ptr(q), sharedEngine(engine())
{
}

/*!
 * \return QQmlComponent 加载完成后继续创建对应的 QML 组件对象，并向外抛出加载和创建完成信号
 *  createFinished()，创建完成后，还需调用 completeCreation() 完成组件创建
 */
bool SharedQmlEnginePrivate::continueLoading()
{
    Q_ASSERT_X(nullptr != component, "Create qml component", "Undefined component");

    if (component->isReady()) {
        rootObject = component->beginCreate(rootContext);
        Q_EMIT q_ptr->createFinished(true);
        return true;
    }

    qCWarning(logDFMGui) << "Loading url failed:" << component->url() << "Error:" << component->errorString();
    Q_EMIT q_ptr->createFinished(false);
    return false;
}

/*!
 * \brief 返回全局的 QQmlEngine 指针，静态变量中保存的是 QWeakPointer , 因此外部所有的
 *  SharedQmlEngine 析构后，静态变量指向引擎同样析构，此设计是为程序退出时释放 QQmlEngine 设计。
 */
QSharedPointer<QQmlEngine> SharedQmlEnginePrivate::engine()
{
    if (auto sharedPtr = s_globalEngine.toStrongRef()) {
        return sharedPtr;
    }

    auto create = QSharedPointer<QQmlEngine>::create();
    s_globalEngine = create.toWeakRef();
    // 初始化设置
    return create;
}

/*!
 * \class SharedQmlEngine
 * \brief 共享的 QQmlEngine ，传入 url 或通过 Applet 的 compnentUrl() 创建 QML 组件。
 * \details 默认同步处理，通过 createFinished() 信号报告创建组件指针，通过调用 completeCreation()
 *  完成创建。
 * \warning qApp 退出前需释放 QQmlEngine，在初始化时使用 WindowManager::engine() 以确定维护的一致
 */
SharedQmlEngine::SharedQmlEngine(QObject *parent)
    : QObject(parent), dptr(new SharedQmlEnginePrivate(this))
{
}

SharedQmlEngine::~SharedQmlEngine()
{
    delete d_func()->component;
}

QObject *SharedQmlEngine::rootObject() const
{
    return d_func()->rootObject;
}

QQmlContext *SharedQmlEngine::rootContext() const
{
    return d_func()->rootContext;
}

QQmlComponent *SharedQmlEngine::mainComponent() const
{
    return d_func()->component;
}

/*!
 * \brief 创建 \a applet 对应的 QQuickItem ， \a async 为 false 时，
 *  此函数*默认期望*同步加载URL信息，调用后通过 completeCreation() 初始化数据，
 *  \a async 为 true 时，请接收 createFinished() 信号，然后调用 completeCreation()
 *  完成创建。
 *
 *  组件的上下文信息 QQmlContext 创建后不会释放，跟随组件的生命周期，
 *  QQmlComponent 跟随 SharedQmlEngine 销毁。
 * \return 执行创建操作是否成功，\a applet 没有组件 Url 等异常时返回 false
 */
bool SharedQmlEngine::create(Applet *applet, bool async)
{
    Q_D(SharedQmlEngine);
    if (!applet || applet->componentUrl().isEmpty()) {
        return false;
    }

    delete d->component;
    d->component = new QQmlComponent(d->sharedEngine.get(), this);
    QObject::connect(d->component, &QQmlComponent::statusChanged, this, &SharedQmlEngine::statusChanged);
    d->component->loadUrl(applet->componentUrl(), async ? QQmlComponent::Asynchronous : QQmlComponent::PreferSynchronous);

    // 用于 QML 组件获取附加属性
    d->rootContext = new QQmlContext(d->sharedEngine.get(), applet);
    d->rootContext->setContextProperty("_dfm_applet", applet);
    d->curApplet = applet;

    if (d->component->isLoading()) {
        QObject::connect(d->component, &QQmlComponent::statusChanged, this, [this]() { d_func()->continueLoading(); });
    } else {
        d->continueLoading();
    }

    return true;
}

/*!
 * \brief 完成当前创建， \a args 会在创建前写入到 QML 组件中
 * \param args QML 组件参数
 */
bool SharedQmlEngine::completeCreation(const QVariantMap &args)
{
    Q_D(SharedQmlEngine);

    if (!d->component) {
        return false;
    }

    if (d->component->status() != QQmlComponent::Ready || d->component->isError()) {
        qCWarning(logDFMGui) << "Loading url failed:" << d->component->url() << "Error:" << d->component->errorString();
        return false;
    }

    if (!d->rootObject) {
        qCWarning(logDFMGui) << "Loading url failed:" << d->component->url() << "Error:" << d->component->errorString();
        return false;
    }

    for (auto it = args.constBegin(); it != args.constEnd(); ++it) {
        d->rootObject->setProperty(it.key().toUtf8().data(), it.value());
    }

    d->component->completeCreate();
    return true;
}

/*!
 * \brief 从 \a url 文件创建 Quick 对象，同时在完成创建前设置 QML 参数 \a args
 * \return 创建的 Quick 对象，不成功返回 nullptr
 */
QObject *SharedQmlEngine::createObject(const QUrl &url, const QVariantMap &args)
{
    QSharedPointer<QQmlEngine> engine = SharedQmlEnginePrivate::engine();
    std::unique_ptr<QQmlComponent> comp = std::make_unique<QQmlComponent>(engine.get());
    comp->loadUrl(url);
    if (comp->isError()) {
        qCWarning(logDFMGui) << QString("Load url failed: %1").arg(comp->errorString());
        return nullptr;
    }

    std::unique_ptr<QQmlContext> context = std::make_unique<QQmlContext>(engine.get(), engine->rootContext());
    std::unique_ptr<QObject> object { comp->createWithInitialProperties(args, context.get()) };
    comp->completeCreate();

    if (!comp->isError() && object) {
        context.release();
        return object.release();
    }

    qCWarning(logDFMGui) << QString("Load url failed: %1").arg(comp->errorString());
    return nullptr;
}

/*!
 * \return 返回当前URL文件加载状态
 */
QQmlComponent::Status SharedQmlEngine::status()
{
    Q_D(SharedQmlEngine);
    if (!d->component) {
        return QQmlComponent::Null;
    }

    return d->component->status();
}

DFMGUI_END_NAMESPACE
