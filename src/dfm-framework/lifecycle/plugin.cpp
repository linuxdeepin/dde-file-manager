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

void Plugin::initialize()
{

}

Plugin::ShutdownFlag Plugin::stop()
{
    return ShutdownFlag::Synch;
}
