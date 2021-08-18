#include "pluginserviceglobal.h"

DPF_USE_NAMESPACE

namespace dpf::GlobalPrivate{

QHash<ServiceName, PluginService*> PluginServiceGlobal::services{};/// 服务注册列表，1:1关联
QHash<ServiceName, BundleName> PluginServiceGlobal::importers{}; ///服务导入者列表，1:1关联

} //namespace GlobalPrivate


