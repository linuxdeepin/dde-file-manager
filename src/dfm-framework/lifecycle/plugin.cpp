#include "plugin.h"

DPF_USE_NAMESPACE

/**
 * @brief Plugin::Plugin 插件默认构造接口
 */
Plugin::Plugin()
{

}

/**
 * @brief Plugin::~Plugin 插件析构函数
 */
Plugin::~Plugin()
{

}

/**
 * @brief Plugin::initialized 插件初始化接口
 * @details 此函数是多线程执行，内部可用于一些线程安全的函数、类操作
 * @return void
 */
void Plugin::initialize()
{

}

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
Plugin::ShutdownFlag Plugin::stop()
{
    return ShutdownFlag::Synch;
}
