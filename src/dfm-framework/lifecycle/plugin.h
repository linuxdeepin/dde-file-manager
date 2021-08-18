#ifndef PLUGIN_H
#define PLUGIN_H

#include "dfm-framework/lifecycle/plugincontext.h"
#include "dfm-framework/definitions/globaldefinitions.h"

#include <QObject>
#include <QSharedData>


DPF_BEGIN_NAMESPACE

/// @brief PLUGIN_INTERFACE 默认插件iid
#define PLUGIN_INTERFACE "Deepin.Bundle.org"

class PluginContext;

/**
 * @brief The Plugin class
 * 插件接口类，用于实现插件,使用方式如下
 * @code
 *  class Core : public Plugin
 *  {
 *      Q_OBJECT
 *      Q_PLUGIN_METADATA(IID PLUGIN_INTERFACE FILE "core.json")
 *  public:
 *      virtual void initialized() override;
 *      virtual bool start() override;
 *      virtual PluginMetaObject::ShutDownFlag stop() override;
 *  };
 * @endcode
 * Q_PLUGIN_METADATA 可参阅Qt宏定义
 * PLUGIN_INTERFACE
 */
class Plugin: public QObject
{
    Q_OBJECT
public:
    enum ShutdownFlag {
        Synch, /// 同步释放标识
        Asynch, /// 异步释放标识
    };


    explicit Plugin();

    virtual ~Plugin() override;

    /**
     * @brief Plugin::initialized 插件初始化接口
     * @details 此函数是多线程执行，内部可用于一些线程安全的函数、类操作
     * @return void
     */
    virtual void initialize();

    /**
     * @brief start
     * @return bool 如果返回true则表示当前插件start函数执行正常
     * false则代表当前内部执行存在问题
     */
    virtual bool start(QSharedPointer<PluginContext> context) = 0;

    /**
     * @brief Plugin::stop
     * @return PluginMetaObject::ShutDownFlag 释放的方式
     * 目前支持Synch(同步)与Asynch(异步)
     * 如果使用Asynch，那么插件的构建者应当发送信号
     * @code
     * emit asyncStopFinished
     * @endcode
     * 否则将导致内存泄露，或者无法卸载插件。
     */
    virtual ShutdownFlag stop();

signals:

    /**
     * @brief asyncStopFinished 异步释放完成的信号
     */
    void asyncStopFinished();
};

DPF_END_NAMESPACE

#endif // PLUGIN_H
